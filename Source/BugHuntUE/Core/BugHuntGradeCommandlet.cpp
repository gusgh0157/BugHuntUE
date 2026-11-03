#include "Core/BugHuntGradeCommandlet.h"
#include "Core/BugHuntLabRunner.h"
#include "BugHuntUE.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
const TCHAR* GEdition = TEXT("unreal");
const TCHAR* GVersion = TEXT("1.0");

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

	const FString ResultPrefix = FString::Printf(TEXT("==== 랩 %d 결과: PASS "), LabNumber);
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

		if (Message.StartsWith(ResultPrefix))
		{
			// "==== 랩 N 결과: PASS 3 / FAIL 1 ===="
			FString PassPart, FailPart;
			Message.Mid(ResultPrefix.Len()).Split(TEXT(" / FAIL "), &PassPart, &FailPart);
			Summary.bHasResult = true;
			Summary.Passed = FCString::Atoi(*PassPart);
			Summary.Failed = FCString::Atoi(*FailPart);
		}
		else if (Message.StartsWith(TEXT("[FAIL] ")))
		{
			Summary.FailLines.Add(Message.Mid(7));
			if (Summary.HangLine.IsEmpty() && Message.Contains(TEXT("응답 없음")))
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
	UE_LOG(LogBugHunt, Verbose, TEXT("실행: %s"), *Args);
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
		Result.Detail = TEXT("GPU 렌더링 결과가 필요한 랩 (-nullrhi 로는 판정 불가). grade.bat -gpu 로 채점하세요.");
		return Result;
	}

	const FString Project = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
	FString Common = TEXT(" -game -unattended -nosplash -NoVerifyGC -BugHuntExit");
	Common += Options.bUseGPU ? TEXT(" -windowed -ResX=640 -ResY=360") : TEXT(" -nullrhi");
	Common += FString::Printf(TEXT(" -BugHuntSeed=%d"), Options.Seed);

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
		// 3부·4부의 멀티플레이 랩: 리슨 서버 프로세스가 랩을 돌리고, 클라이언트 프로세스가 접속한다.
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
			FailLines.Add(Line.StartsWith(TEXT("[클라이언트]")) ? Line : TEXT("[클라이언트] ") + Line);
		}
		CrashLine = CrashLine.IsEmpty() && !Client->CrashLine.IsEmpty() ? TEXT("[클라이언트] ") + Client->CrashLine : CrashLine;
		HangLine = HangLine.IsEmpty() ? Client->HangLine : HangLine;
	}

	if (!bFinished && !Main.bHasResult)
	{
		Result.Result = TEXT("TIMEOUT");
		Result.Detail = FString::Printf(TEXT("%.0f초 안에 끝나지 않아 프로세스를 종료함 (진짜로 굳었나요?)"), Options.TimeoutSeconds);
	}
	else if (!CrashLine.IsEmpty())
	{
		Result.Result = TEXT("CRASH");
		Result.Detail = CrashLine;
	}
	else if (!Main.bHasResult)
	{
		Result.Result = TEXT("CRASH");
		Result.Detail = FString::Printf(TEXT("결과 줄 없이 종료됨 (종료 코드 %d). 로그: %s"), ExitCode, *ServerLog);
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
			Result.Detail += FString::Printf(TEXT(" 외 %d개"), FailLines.Num() - 1);
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
		Result.Detail = FString::Printf(TEXT("검사 %d개 통과"), Main.Passed);
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
	Out(FString::Printf(TEXT("BugHunt UE 채점: 랩 %d개, %s, 랩마다 새 프로세스 (%s)"), Labs.Num(),
		Options.Seed == 0 ? TEXT("공개 입력") : *FString::Printf(TEXT("숨은 입력 시드 %d"), Options.Seed),
		Options.bUseGPU ? TEXT("GPU 사용") : TEXT("-nullrhi")));
	Out(FString::Printf(TEXT("랩별 로그: %s"), *Options.LogDir));
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

		const FString Row = FString::Printf(TEXT(" %2d  %d부  %-7s %6.1fs  %s  - %s"),
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
	Report->SetStringField(TEXT("version"), GVersion);
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
	Summary.Add(TEXT("  #  부   결과       시간  랩  - 내용"));
	Summary.Append(Table);
	Summary.Add(TEXT(""));
	Summary.Add(FString::Printf(TEXT("점수: %d / %d  (SKIP %d개 제외%s)"), Passed, Graded, Skipped,
		Passed == Graded ? TEXT(", 만점!") : TEXT("")));
	Summary.Add(FString::Printf(TEXT("결과 파일: %s"), *Options.OutPath));
	for (const FString& Line : Summary)
	{
		Out(Line);
	}
	FFileHelper::SaveStringToFile(FString::Join(Summary, TEXT("\r\n")) + TEXT("\r\n"),
		*FPaths::Combine(Options.LogDir, TEXT("grade-summary.txt")), FFileHelper::EEncodingOptions::ForceUTF8);
	return Passed == Graded ? 0 : 1;
}
