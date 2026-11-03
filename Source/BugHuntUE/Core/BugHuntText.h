#pragma once

#include "CoreMinimal.h"

// 실행 출력의 언어 (한국어 / 일본어). 이 파일에는 버그가 없다.
// 実行時の出力言語（韓国語 / 日本語）。このファイルにバグはありません。
//
// 언어 고르는 순서 / 言語を選ぶ順番:
//   1. 명령줄 -BugHuntLang=ja|ko (콘솔 변수 BugHunt.Lang 에 들어간다) / コマンドライン -BugHuntLang=ja|ko（コンソール変数 BugHunt.Lang に入ります）
//   2. 콘솔 변수 BugHunt.Lang ja|ko / コンソール変数 BugHunt.Lang ja|ko
//   3. 에디터/OS 언어 (FInternationalization 의 현재 언어가 일본어면 ja) / エディター／OS の言語（FInternationalization の現在の言語が日本語なら ja）
//   4. 기본 한국어 / 既定は韓国語
//
// LOCTEXT 대신 단순한 두 언어 표를 쓴다. LOCTEXT 는 .locres 를 모아 굽고 문화권을 설정해야 바뀌는데,
// 채점기의 헤드리스 자식 프로세스(-nullrhi)에서 그 준비가 늘 같다고 보장하기 어렵다. 표는 빌드에 들어가 있어서 어디서나 같다.
// LOCTEXT ではなく単純な 2 言語の表を使います。LOCTEXT は .locres の収集・ビルドとカルチャー設定が必要で、
// 採点機のヘッドレス子プロセス（-nullrhi）でその準備が常に同じとは限りません。表はビルドに含まれるので、どこでも同じです。
//
// 쓰는 법 / 使い方 (랩마다 파일 위쪽에 표를 하나 둔다 / ラボごとにファイル上部に表を 1 つ置きます):
//   namespace Lab18Text
//   {
//   BH_TEXT(Placed, "칸 %d개를 밝혔다.", "%d マスを明らかにしました。");
//   }
//   BugHunt::Note(BugHunt::Fmt<Lab18Text::Placed>(Count));   // Printf 형식 / Printf 形式
//   BugHunt::Note(BugHunt::Tr<Lab18Text::Placed>());          // 그대로 / そのまま
// 일본어 칸이 비어 있으면 컴파일 오류가 난다 (번역 빠짐을 빌드에서 잡는다).
// 日本語の欄が空だとコンパイルエラーになります（翻訳漏れをビルドで検出します）。
namespace BugHunt
{
enum class ELang : uint8
{
	Ko,
	Ja,
};

BUGHUNTUE_API ELang CurrentLang();
inline bool IsJa() { return CurrentLang() == ELang::Ja; }
// "ko" 또는 "ja". 채점기가 자식 프로세스에 넘길 때 쓴다. / "ko" または "ja"。採点機が子プロセスへ渡すときに使います。
BUGHUNTUE_API const TCHAR* LangCode();

template <typename T>
const TCHAR* Tr()
{
	return IsJa() ? T::JaText : T::KoText;
}

template <typename T, typename... ArgTypes>
FString Fmt(ArgTypes... Args)
{
	return IsJa() ? FString::Printf(T::JaText, Args...) : FString::Printf(T::KoText, Args...);
}

// 두 언어 중 어느 쪽과도 같으면 true (채점기가 자식 로그를 읽을 때 언어와 상관없이 알아보려고).
// どちらかの言語と一致すれば true（採点機が子プロセスのログを言語に関係なく読むため）。
template <typename T>
bool MatchesAny(const FString& Text, ESearchCase::Type Case = ESearchCase::CaseSensitive)
{
	return Text.Contains(T::KoText, Case) || Text.Contains(T::JaText, Case);
}
}

#define BH_TEXT(Name, Ko, Ja) \
	struct Name \
	{ \
		static constexpr TCHAR KoText[] = TEXT(Ko); \
		static constexpr TCHAR JaText[] = TEXT(Ja); \
		static_assert(sizeof(JaText) > sizeof(TCHAR), "일본어 번역이 비었습니다 / 日本語訳が空です: " #Name); \
	}
