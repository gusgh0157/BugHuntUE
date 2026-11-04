#include "Core/BugHuntGradeCommandlet.h"
#include "Core/BugHuntLabRunner.h"
#include "Core/BugHuntHang.h"
#include "Core/BugHuntReport.h"
#include "Core/BugHuntText.h"
#include "Algo/Find.h"
#include "BugHuntUE.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

// 채점기 출력 문구. grade-report.json 의 키와 result 값(PASS/FAIL/...)은 언어와 상관없이 그대로다.
// 採点機の出力文言。grade-report.json のキーと result の値（PASS/FAIL/...）は言語に関係なくそのまま。
namespace GradeText
{
// 자식 로그의 결과 줄 앞부분 (BugHuntLab.cpp 의 LabText::Result 와 같은 모양) / 子ログの結果行の先頭（BugHuntLab.cpp の LabText::Result と同じ形）
BH_TEXT(ResultPrefix, "==== 랩 %d 결과: PASS ", "==== ラボ %d 結果: PASS ");
BH_TEXT(NeedsGPU,    "GPU 렌더링 결과가 필요한 랩 (-nullrhi 로는 판정 불가). grade.bat -gpu 로 채점하세요.", "GPU のレンダリング結果が必要なラボ（-nullrhi では判定不可）。grade.bat -gpu で採点してください。");
BH_TEXT(TimedOut,    "%.0f초 안에 끝나지 않아 프로세스를 종료함 (진짜로 굳었나요?)", "%.0f 秒以内に終わらなかったためプロセスを終了（本当に固まっていませんか？）");
BH_TEXT(NoResult,    "결과 줄 없이 종료됨 (종료 코드 %d). 로그: %s", "結果行なしで終了（終了コード %d）。ログ: %s");
BH_TEXT(More,        " 외 %d개", " ほか %d 件");
BH_TEXT(Passed,      "검사 %d개 통과", "チェック %d 件合格");
BH_TEXT(Banner,      "BugHunt UE 채점: 랩 %d개, %s, 랩마다 새 프로세스 (%s)", "BugHunt UE 採点: ラボ %d 個、%s、ラボごとに新しいプロセス（%s）");
BH_TEXT(PublicInput, "공개 입력", "公開入力");
BH_TEXT(HiddenSeed,  "숨은 입력 시드 %d", "隠し入力シード %d");
BH_TEXT(UseGPU,      "GPU 사용", "GPU 使用");
BH_TEXT(LogDir,      "랩별 로그: %s", "ラボごとのログ: %s");
BH_TEXT(Row,         " %2d  %d부  %-7s %6.1fs  %s  - %s", " %2d  %d部  %-7s %6.1fs  %s  - %s");
BH_TEXT(Header,      "  #  부   결과       시간  랩  - 내용", "  #  部   結果       時間  ラボ  - 内容");
BH_TEXT(Score,       "점수: %d / %d  (SKIP %d개 제외%s)", "スコア: %d / %d（SKIP %d 件を除く%s）");
BH_TEXT(Perfect,     ", 만점!", "、満点！");
BH_TEXT(OutFile,     "결과 파일: %s", "結果ファイル: %s");
}

namespace
{
const TCHAR* GEdition = TEXT("unreal");
const int32 GVersion = 1;

struct FGradeOptions
{
	TSet<int32> Labs;          // 비었으면 전부
	int32 Seed = 0;
	double TimeoutSeconds = 180.0;
	bool bUseGPU = false;
	FString OutPath;
	FString LogDir;
};

struct FLabResult
{
	FString Result;
	FString Detail;
	int32 Ms = 0;
};

// 자식 프로세스 로그에서 읽어 낸 것
struct FLogSummary
{
	bool bHasResult = false;
	int32 Passed = 0;
	int32 Failed = 0;
	TArray<FString> FailLines;
	TArray<FString> SkipLines;
	FString CrashLine;
	FString HangLine;
};

void Out(const FString& Line)
{
	// 카테고리 없이 Log 수준으로 찍어야 "LogBugHunt: Display:" 머리말 없이 표가 나온다.
	// (grade.bat 은 -AllowStdOutLogVerbosity 로 Log 수준도 콘솔에 보이게 하고, 엔진 로그는 -LogCmds 로 줄인다.)
	GLog->Log(Line);
}

void ParseLabList(const FString& Spec, TSet<int32>& OutLabs)
{
	TArray<FString> Parts;
	Spec.ParseIntoArray(Parts, TEXT(","));
	for (const FString& Part : Parts)
	{
		FString Left, Right;
		if (Part.Split(TEXT("-"), &Left, &Right))
		{
			for (int32 N = FCString::Atoi(*Left); N <= FCString::Atoi(*Right); ++N)
			{
				OutLabs.Add(N);
			}
		}
		else
		{
			OutLabs.Add(FCString::Atoi(*Part));
		}
	}
}

FLogSummary ReadLog(const FString& Path, int32 LabNumber)
{
	FLogSummary Summary;
	FString Text;
	if (!FFileHelper::LoadFileToString(Text, *Path))
	{
		return Summary;
	}

	// 자식 프로세스의 언어와 상관없이 두 언어 모두 알아본다. / 子プロセスの言語に関係なく、両言語とも認識します。
	const FString ResultPrefixes[] = {
		FString::Printf(GradeText::ResultPrefix::KoText, LabNumber),
		FString::Printf(GradeText::ResultPrefix::JaText, LabNumber) };
	const FString Tag = TEXT("LogBugHunt: ");
	// 크래시 표시. 앞의 것일수록 내용 칸에 쓸 만하다 ("Unhandled Exception: EXCEPTION_ACCESS_VIOLATION reading address ...").
	const TCHAR* CrashMarkers[] = { TEXT("Unhandled Exception"), TEXT("Assertion failed"), TEXT("Fatal error"), TEXT("=== Critical error") };
	int32 CrashRank = UE_ARRAY_COUNT(CrashMarkers);

	TArray<FString> Lines;
	Text.ParseIntoArrayLines(Lines);
	for (const FString& Line : Lines)
	{
		for (int32 Rank = 0; Rank < CrashRank; ++Rank)
		{
			const int32 At = Line.Find(CrashMarkers[Rank], ESearchCase::CaseSensitive);
			if (At != INDEX_NONE)
			{
				Summary.CrashLine = Line.Mid(At).TrimStartAndEnd();
				CrashRank = Rank;
				break;
			}
		}

		const int32 TagAt = Line.Find(Tag, ESearchCase::CaseSensitive);
		if (TagAt == INDEX_NONE)
		{
			continue;
		}
		FString Message = Line.Mid(TagAt + Tag.Len());
		Message.RemoveFromStart(TEXT("Display: "));

		const FString* ResultPrefix = Algo::FindByPredicate(ResultPrefixes, [&Message](const FString& P) { return Message.StartsWith(P); });
		if (ResultPrefix)
		{
			// "==== 랩 N 결과: PASS 3 / FAIL 1 ===="
			FString PassPart, FailPart;
			Message.Mid(ResultPrefix->Len()).Split(TEXT(" / FAIL "), &PassPart, &FailPart);
			Summary.bHasResult = true;
			Summary.Passed = FCString::Atoi(*PassPart);
			Summary.Failed = FCString::Atoi(*FailPart);
		}
		else if (Message.StartsWith(TEXT("[FAIL] ")))
		{
			Summary.FailLines.Add(Message.Mid(7));
			if (Summary.HangLine.IsEmpty() && BugHunt::MatchesAny<HangText::HangWord>(Message))
			{
				Summary.HangLine = Message.Mid(7);
			}
		}
		else if (Message.StartsWith(TEXT("[SKIP] ")))
		{
			Summary.SkipLines.Add(Message.Mid(7));
		}
	}
	return Summary;
}

FString Quote(const FString& S)
{
	return FString::Printf(TEXT("\"%s\""), *S);
}

FProcHandle Launch(const FString& Args)
{
	UE_LOG(LogBugHunt, Verbose, TEXT("실행 / 実行: %s"), *Args);
	return FPlatformProcess::CreateProc(FPlatformProcess::ExecutablePath(), *Args,
		/*bLaunchDetached*/ false, /*bLaunchHidden*/ true, /*bLaunchReallyHidden*/ true,
		nullptr, 0, nullptr, nullptr);
}

// 프로세스가 끝나기를 Deadline(FPlatformTime::Seconds 기준)까지 기다린다. 넘으면 죽이고 false.
bool WaitOrKill(FProcHandle& Proc, double Deadline, int32& OutExitCode)
{
	bool bFinished = true;
	while (FPlatformProcess::IsProcRunning(Proc))
	{
		if (FPlatformTime::Seconds() > Deadline)
		{
			FPlatformProcess::TerminateProc(Proc, /*KillTree*/ true);
			bFinished = false;
			break;
		}
		FPlatformProcess::Sleep(0.1f);
	}
	OutExitCode = 0;
	if (bFinished)
	{
		FPlatformProcess::GetProcReturnCode(Proc, &OutExitCode);
	}
	FPlatformProcess::CloseProc(Proc);
	return bFinished;
}

FLabResult GradeLab(const BugHunt::FLabInfo& Lab, const FGradeOptions& Options)
{
	FLabResult Result;
	if (Lab.bNeedsGPU && !Options.bUseGPU)
	{
		Result.Result = TEXT("SKIP");
		Result.Detail = BugHunt::Tr<GradeText::NeedsGPU>();
		return Result;
	}

	const FString Project = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
	FString Common = TEXT(" -game -unattended -nosplash -NoVerifyGC -BugHuntExit");
	Common += Options.bUseGPU ? TEXT(" -windowed -ResX=640 -ResY=360") : TEXT(" -nullrhi");
	Common += FString::Printf(TEXT(" -BugHuntSeed=%d"), Options.Seed);
	// 자식 프로세스도 채점기와 같은 언어로 / 子プロセスも採点機と同じ言語で
	Common += FString::Printf(TEXT(" -BugHuntLang=%s"), BugHunt::LangCode());

	const FString ServerLog = FPaths::Combine(Options.LogDir, FString::Printf(TEXT("Lab%02d.log"), Lab.Number));
	const FString ClientLog = FPaths::Combine(Options.LogDir, FString::Printf(TEXT("Lab%02d_client.log"), Lab.Number));
	IFileManager::Get().Delete(*ServerLog);
	IFileManager::Get().Delete(*ClientLog);

	const double Start = FPlatformTime::Seconds();
	const double Deadline = Start + Options.TimeoutSeconds;
	bool bFinished = false;
	int32 ExitCode = 0;
	TOptional<FLogSummary> Client;

	if (!Lab.bNetworked)
	{
		FProcHandle Proc = Launch(Quote(Project) + Common
			+ FString::Printf(TEXT(" -BugHuntLab=%d -abslog=%s"), Lab.Number, *Quote(ServerLog)));
		bFinished = WaitOrKill(Proc, Deadline, ExitCode);
	}
	else
	{
		// 멀티플레이 랩(랩 28, 39, 46, 47): 리슨 서버 프로세스가 랩을 돌리고, 클라이언트 프로세스가 접속한다.
		FString Map = TEXT("/Engine/Maps/Templates/Template_Default");
		GConfig->GetString(TEXT("/Script/EngineSettings.GameMapsSettings"), TEXT("GameDefaultMap"), Map, GEngineIni);
		const int32 Port = 17700 + Lab.Number;

		FProcHandle Server = Launch(Quote(Project) + TEXT(" ") + Quote(Map + TEXT("?listen")) + Common
			+ FString::Printf(TEXT(" -port=%d -BugHuntLab=%d -BugHuntWaitForClient -abslog=%s"), Port, Lab.Number, *Quote(ServerLog)));
		FPlatformProcess::Sleep(5.f);
		FProcHandle ClientProc = Launch(Quote(Project) + FString::Printf(TEXT(" 127.0.0.1:%d"), Port) + Common
			+ FString::Printf(TEXT(" -abslog=%s"), *Quote(ClientLog)));

		bFinished = WaitOrKill(Server, Deadline, ExitCode);
		int32 ClientExit = 0;
		WaitOrKill(ClientProc, FMath::Min(Deadline, FPlatformTime::Seconds() + 30.0), ClientExit);
		Client = ReadLog(ClientLog, Lab.Number);
	}
	Result.Ms = FMath::RoundToInt((FPlatformTime::Seconds() - Start) * 1000.0);

	const FLogSummary Main = ReadLog(ServerLog, Lab.Number);
	TArray<FString> FailLines = Main.FailLines;
	FString CrashLine = Main.CrashLine;
	FString HangLine = Main.HangLine;
	if (Client)
	{
		for (const FString& Line : Client->FailLines)
		{
			const bool bTagged = Line.StartsWith(ReportText::Client::KoText) || Line.StartsWith(ReportText::Client::JaText);
			FailLines.Add(bTagged ? Line : FString(BugHunt::Tr<ReportText::Client>()) + TEXT(" ") + Line);
		}
		CrashLine = CrashLine.IsEmpty() && !Client->CrashLine.IsEmpty() ? FString(BugHunt::Tr<ReportText::Client>()) + TEXT(" ") + Client->CrashLine : CrashLine;
		HangLine = HangLine.IsEmpty() ? Client->HangLine : HangLine;
	}

	if (!bFinished && !Main.bHasResult)
	{
		Result.Result = TEXT("TIMEOUT");
		Result.Detail = BugHunt::Fmt<GradeText::TimedOut>(Options.TimeoutSeconds);
	}
	else if (!CrashLine.IsEmpty())
	{
		Result.Result = TEXT("CRASH");
		Result.Detail = CrashLine;
	}
	else if (!Main.bHasResult)
	{
		Result.Result = TEXT("CRASH");
		Result.Detail = BugHunt::Fmt<GradeText::NoResult>(ExitCode, *ServerLog);
	}
	else if (!HangLine.IsEmpty())
	{
		Result.Result = TEXT("TIMEOUT");
		Result.Detail = HangLine;
	}
	else if (Main.Failed > 0 || FailLines.Num() > 0)
	{
		Result.Result = TEXT("FAIL");
		Result.Detail = FailLines.Num() > 0 ? FailLines[0] : TEXT("FAIL");
		if (FailLines.Num() > 1)
		{
			Result.Detail += BugHunt::Fmt<GradeText::More>(FailLines.Num() - 1);
		}
	}
	else if (Main.Passed == 0 && Main.SkipLines.Num() > 0)
	{
		Result.Result = TEXT("SKIP");
		Result.Detail = Main.SkipLines[0];
	}
	else
	{
		Result.Result = TEXT("PASS");
		Result.Detail = BugHunt::Fmt<GradeText::Passed>(Main.Passed);
	}
	return Result;
}
}

UBugHuntGradeCommandlet::UBugHuntGradeCommandlet()
{
	LogToConsole = true;
	ShowErrorCount = false;
}

int32 UBugHuntGradeCommandlet::Main(const FString& Params)
{
	FGradeOptions Options;
	FString LabSpec;
	if (FParse::Value(*Params, TEXT("labs="), LabSpec, /*bShouldStopOnSeparator*/ false))
	{
		ParseLabList(LabSpec, Options.Labs);
	}
	if (FParse::Param(*Params, TEXT("public")))
	{
		Options.Seed = 0;
	}
	else if (!FParse::Value(*Params, TEXT("seed="), Options.Seed) || Options.Seed == 0)
	{
		Options.Seed = 100000 + static_cast<int32>(FDateTime::Now().GetTicks() % 900000);
	}
	FParse::Value(*Params, TEXT("timeout="), Options.TimeoutSeconds);
	Options.bUseGPU = FParse::Param(*Params, TEXT("gpu"));
	if (!FParse::Value(*Params, TEXT("out="), Options.OutPath))
	{
		Options.OutPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("grade-report.json"));
	}
	Options.OutPath = FPaths::ConvertRelativePathToFull(Options.OutPath);
	Options.LogDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Grade")));
	IFileManager::Get().MakeDirectory(*Options.LogDir, true);

	TArray<BugHunt::FLabInfo> Labs = BugHunt::GetLabs();
	Labs.RemoveAll([&Options](const BugHunt::FLabInfo& Lab) { return Options.Labs.Num() > 0 && !Options.Labs.Contains(Lab.Number); });

	Out(TEXT(""));
	Out(BugHunt::Fmt<GradeText::Banner>(Labs.Num(),
		Options.Seed == 0 ? BugHunt::Tr<GradeText::PublicInput>() : *BugHunt::Fmt<GradeText::HiddenSeed>(Options.Seed),
		Options.bUseGPU ? BugHunt::Tr<GradeText::UseGPU>() : TEXT("-nullrhi")));
	Out(BugHunt::Fmt<GradeText::LogDir>(*Options.LogDir));
	Out(TEXT(""));

	TArray<TSharedPtr<FJsonValue>> JsonLabs;
	int32 Passed = 0;
	int32 Skipped = 0;
	TArray<FString> Table;
	for (const BugHunt::FLabInfo& Lab : Labs)
	{
		const FLabResult Result = GradeLab(Lab, Options);
		Passed += Result.Result == TEXT("PASS") ? 1 : 0;
		Skipped += Result.Result == TEXT("SKIP") ? 1 : 0;

		const FString Row = BugHunt::Fmt<GradeText::Row>(
			Lab.Number, Lab.Part, *Result.Result, Result.Ms / 1000.0, *Lab.Title, *Result.Detail);
		Out(Row);
		Table.Add(Row);

		TSharedRef<FJsonObject> Entry = MakeShared<FJsonObject>();
		Entry->SetNumberField(TEXT("id"), Lab.Number);
		Entry->SetStringField(TEXT("title"), Lab.Title);
		Entry->SetNumberField(TEXT("part"), Lab.Part);
		Entry->SetStringField(TEXT("result"), Result.Result);
		Entry->SetNumberField(TEXT("ms"), Result.Ms);
		Entry->SetStringField(TEXT("detail"), Result.Detail);
		JsonLabs.Add(MakeShared<FJsonValueObject>(Entry));
	}

	TSharedRef<FJsonObject> Report = MakeShared<FJsonObject>();
	Report->SetStringField(TEXT("edition"), GEdition);
	Report->SetNumberField(TEXT("version"), GVersion);
	Report->SetNumberField(TEXT("total"), Labs.Num());
	Report->SetNumberField(TEXT("passed"), Passed);
	Report->SetArrayField(TEXT("labs"), JsonLabs);
	FString Json;
	FJsonSerializer::Serialize(Report, TJsonWriterFactory<>::Create(&Json));
	FFileHelper::SaveStringToFile(Json, *Options.OutPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

	// 표를 한 번 더, 맨 끝에 모아서 (위쪽은 엔진 로그와 섞여 있을 수 있다)
	const int32 Graded = Labs.Num() - Skipped;
	TArray<FString> Summary;
	Summary.Add(TEXT(""));
	Summary.Add(BugHunt::Tr<GradeText::Header>());
	Summary.Append(Table);
	Summary.Add(TEXT(""));
	Summary.Add(BugHunt::Fmt<GradeText::Score>(Passed, Graded, Skipped,
		Passed == Graded ? BugHunt::Tr<GradeText::Perfect>() : TEXT("")));
	Summary.Add(BugHunt::Fmt<GradeText::OutFile>(*Options.OutPath));
	for (const FString& Line : Summary)
	{
		Out(Line);
	}
	FFileHelper::SaveStringToFile(FString::Join(Summary, TEXT("\r\n")) + TEXT("\r\n"),
		*FPaths::Combine(Options.LogDir, TEXT("grade-summary.txt")), FFileHelper::EEncodingOptions::ForceUTF8);
	return Passed == Graded ? 0 : 1;
}
