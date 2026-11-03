#include "Core/BugHuntText.h"
#include "HAL/IConsoleManager.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace BugHunt
{
namespace
{
TAutoConsoleVariable<FString> CVarLang(
	TEXT("BugHunt.Lang"),
	TEXT(""),
	TEXT("랩 출력 언어: ko 또는 ja. 비우면 에디터/OS 언어. 명령줄 -BugHuntLang= 이 먼저. / ラボ出力の言語: ko または ja。空ならエディター／OS の言語。コマンドライン -BugHuntLang= が優先。"));

bool GReadCommandLine = false;

ELang FromCode(const FString& Code, ELang Fallback)
{
	if (Code.StartsWith(TEXT("ja"), ESearchCase::IgnoreCase))
	{
		return ELang::Ja;
	}
	if (Code.StartsWith(TEXT("ko"), ESearchCase::IgnoreCase))
	{
		return ELang::Ko;
	}
	return Fallback;
}
}

ELang CurrentLang()
{
	if (!GReadCommandLine)
	{
		GReadCommandLine = true;
		FString Code;
		if (FParse::Value(FCommandLine::Get(), TEXT("BugHuntLang="), Code))
		{
			CVarLang->Set(*Code, ECVF_SetByCommandline);
		}
	}

	const FString Code = CVarLang.GetValueOnAnyThread();
	if (!Code.IsEmpty())
	{
		return FromCode(Code, ELang::Ko);
	}
	if (FInternationalization::IsAvailable())
	{
		const FCultureRef Culture = FInternationalization::Get().GetCurrentLanguage();
		return FromCode(Culture->GetTwoLetterISOLanguageName(), ELang::Ko);
	}
	return ELang::Ko;
}

const TCHAR* LangCode()
{
	return CurrentLang() == ELang::Ja ? TEXT("ja") : TEXT("ko");
}
}
