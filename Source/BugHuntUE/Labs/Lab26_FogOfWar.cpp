#include "Labs/Lab26_FogOfWar.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab26Text
{
BH_TEXT(Player,   "플레이어", "プレイヤー");
BH_TEXT(Scout,    "정찰병",   "斥候");
BH_TEXT(Walked,   "플레이어와 정찰병이 칸 %d개씩 밝혔다. 겹치는 칸 (%d,0).", "プレイヤーと斥候が %d マスずつ明らかにした。重なるマスは (%d,0)。");
BH_TEXT(Count,    "밝혀진 칸 수 (겹친 칸은 한 번)", "明らかになったマスの数（重なったマスは 1 回）");
BH_TEXT(Asked,    "밝힌 칸을 물었을 때 밝혀졌다고 답한 수", "明らかにしたマスを問い合わせて「明らかになっている」と答えた数");
BH_TEXT(StillFog, "(9,9) 는 아직 안개", "(9,9) はまだ霧の中");
}

void ABH_FogOfWar::Reveal(int32 X, int32 Y, const FString& Source)
{
	Revealed.Add({ X, Y, Source });
}

bool ABH_FogOfWar::IsRevealed(int32 X, int32 Y) const
{
	return Revealed.Contains({ X, Y, FString() });
}

void ABH_Lab26::StartLab()
{
	ABH_FogOfWar* Fog = SpawnLabActor<ABH_FogOfWar>();

	// 플레이어는 (0,0)~(3,0) 을 걷고, 정찰병은 (2,0)~(2,3) 을 걷는다. 겹치는 칸은 (2,0) 하나.
	// [JA] プレイヤーは (0,0)〜(3,0) を、斥候は (2,0)〜(2,3) を歩く。重なるマスは (2,0) の 1 つ。
	const int32 Length = BugHunt::CaseInt(4, 3, 9);
	const int32 Cross = BugHunt::CaseInt(2, 0, Length - 1);
	TSet<FIntPoint> Unique;
	for (int32 i = 0; i < Length; ++i)
	{
		Fog->Reveal(i, 0, BugHunt::Tr<Lab26Text::Player>());
		Unique.Add(FIntPoint(i, 0));
	}
	for (int32 j = 0; j < Length; ++j)
	{
		Fog->Reveal(Cross, j, BugHunt::Tr<Lab26Text::Scout>());
		Unique.Add(FIntPoint(Cross, j));
	}
	BugHunt::Note(BugHunt::Fmt<Lab26Text::Walked>(Length, Cross));

	BugHunt::Expect(BugHunt::Tr<Lab26Text::Count>(), Unique.Num(), Fog->Revealed.Num());
	// 밝힌 칸 하나하나를 다시 물어본다 (칸 하나만 보면 운 좋게 같은 버킷에 들어가 통과할 수 있다).
	// [JA] 明らかにしたマスを 1 つずつ問い合わせ直す（1 マスだけだと、運よく同じバケットに入って通ってしまうことがある）。
	int32 Found = 0;
	for (int32 i = 0; i < Length; ++i)
	{
		Found += Fog->IsRevealed(i, 0) ? 1 : 0;
		Found += Fog->IsRevealed(Cross, i) ? 1 : 0;
	}
	BugHunt::Expect(BugHunt::Tr<Lab26Text::Asked>(), 2 * Length, Found);
	BugHunt::ExpectTrue(BugHunt::Tr<Lab26Text::StillFog>(), !Fog->IsRevealed(9, 9));
	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 밝힌 칸인데 IsRevealed 가 false 이고, 겹친 칸이 두 번 들어 있습니다.
// 조사식에 Revealed 를 넣고 펼쳐 겹친 칸 두 개의 X, Y, Source 를 보세요. operator== 로 비교하면 둘은 같은가요?
// 그런데 왜 TSet 은 둘 다 넣었을까요? TSet 이 원소를 찾을 때 operator== 보다 먼저 보는 것은?

// [JA] 明らかにしたマスなのに IsRevealed が false で、重なったマスが 2 回入っています。
// [JA] ウォッチに Revealed を入れて展開し、重なった 2 つのマスの X、Y、Source を見てください。operator== で比べると 2 つは同じですか？
// [JA] では、なぜ TSet は両方とも入れたのでしょう？ TSet が要素を探すとき、operator== より先に見るものは？
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// TSet/TMap 은 GetTypeHash 로 칸(버킷)을 고른 뒤, 그 칸 안에서만 operator== 로 비교합니다.
// "a == b 이면 GetTypeHash(a) == GetTypeHash(b)" 가 반드시 성립해야 합니다. 여기서는 == 이 보지 않는 Source 가 해시에 섞여서,
// 같은 칸인데 다른 버킷을 찾아갑니다. (운 나쁘게 같은 버킷이면 찾기도 하니 결과가 들쭉날쭉합니다.)

// [JA] TSet/TMap は GetTypeHash でバケットを選び、そのバケットの中だけで operator== で比べます。
// [JA] 「a == b なら GetTypeHash(a) == GetTypeHash(b)」が必ず成り立たなければなりません。ここでは == が見ない Source がハッシュに混ざっているため、
// [JA] 同じマスなのに別のバケットを探しに行きます。（運悪く同じバケットなら見つかることもあるので、結果がばらつきます。）
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// 해시는 == 에 쓰는 필드만으로 만듭니다.
//     return HashCombine(GetTypeHash(Key.X), GetTypeHash(Key.Y));
// 같은 함정: 키를 넣은 뒤에 키의 필드를 바꾸면(예: TSet 원소를 const_cast 로 고침) 그 원소는 엉뚱한 버킷에 남습니다.
// 키는 바꾸지 말고 Remove 한 뒤 새로 Add 하세요.

// [JA] ハッシュは == で使うフィールドだけで作ります。
// [JA]     return HashCombine(GetTypeHash(Key.X), GetTypeHash(Key.Y));
// [JA] 同じ落とし穴: キーを入れた後でキーのフィールドを変えると（例: TSet の要素を const_cast で書き換える）、その要素は見当違いのバケットに残ります。
// [JA] キーは書き換えず、Remove してから新しく Add してください。
#pragma endregion
