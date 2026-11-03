// [주의] 이 랩은 소리를 냅니다. 실행 전에 시스템 볼륨을 낮추고, 헤드폰을 쓴다면 귀에서 떼고 시작하세요.
//        (실습용 소리는 볼륨 배율 0.5 로 재생합니다.)
// [JA] [注意] このラボは音が出ます。実行前にシステムの音量を下げ、ヘッドホンを使っている場合は耳から離してから始めてください。
// [JA]        （実習用の音は音量倍率 0.5 で再生します。）

#include "Labs/Lab29_FireSound.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

ABH_Blaster::ABH_Blaster()
{
	// (임시 사운드로 엔진 알림음을 쓴다. 다른 랩이 쓰지 않는 소리여야 처음에 메모리에 없다.)
	FireSound = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Engine/EditorSounds/Notifications/CompileFailed.CompileFailed")));
}

void ABH_Blaster::Fire()
{
	const uint64 FireFrame = GFrameCounter;
	const double FireTime = FPlatformTime::Seconds();
	UE_LOG(LogBugHunt, Log, TEXT("  프레임 %llu: 총구 섬광 (발사음 %s)"), FireFrame,
		FireSound.IsValid() ? TEXT("메모리에 있음") : TEXT("메모리에 없음"));

	if (USoundBase* Loaded = FireSound.Get())
	{
	// [JA] （仮のサウンドとしてエンジンの通知音を使う。他のラボが使わない音でないと、最初にメモリにない状態にならない。）
		PlayFireSound(Loaded, FireFrame, FireTime);
		return;
	}

	// 아직 메모리에 없으면 비동기로 불러서 재생한다. (동기 로드는 게임을 멈추게 하니까)
	UAssetManager::GetStreamableManager().RequestAsyncLoad(FireSound.ToSoftObjectPath(),
		FStreamableDelegate::CreateWeakLambda(this, [this, FireFrame, FireTime]
		{
			PlayFireSound(FireSound.Get(), FireFrame, FireTime);
		}));
}

void ABH_Blaster::PlayFireSound(USoundBase* Sound, uint64 FireFrame, double FireTime)
{
	UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation(), /*VolumeMultiplier*/ 0.5f);

	const int32 Frames = static_cast<int32>(GFrameCounter - FireFrame);
	// [JA] まだメモリにないなら非同期で読み込んで再生する。（同期ロードはゲームを止めてしまうので）
	const float Ms = static_cast<float>((FPlatformTime::Seconds() - FireTime) * 1000.0);
	SoundDelayFrames.Add(Frames);
	SoundDelayMs.Add(Ms);
	UE_LOG(LogBugHunt, Log, TEXT("  프레임 %llu: 발사음 재생 (%d프레임, %.1fms 늦음)"), GFrameCounter, Frames, Ms);
}

void ABH_Lab29::StartLab()
{
	BugHunt::Note(TEXT("[주의] 소리가 납니다. 볼륨을 낮추세요."));
	ABH_Blaster* Blaster = SpawnLabActor<ABH_Blaster>();
	BugHunt::Note(TEXT("블래스터를 세 번 쏜다. 두 번째와 세 번째 사이에 레벨 전환 등으로 GC가 돈다."));

	After(0.3f, [Blaster] { Blaster->Fire(); });
	After(0.6f, [Blaster] { Blaster->Fire(); });
	After(2.5f, [Blaster]
	{
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);   // 콘솔 명령 obj gc 와 같다
		Blaster->Fire();
	});

	After(3.2f, [this, Blaster]
	{
		for (int32 i = 0; i < 3; ++i)
		{
			const int32 Frames = Blaster->SoundDelayFrames.IsValidIndex(i) ? Blaster->SoundDelayFrames[i] : -1;
			BugHunt::Expect(FString::Printf(TEXT("%d번째 발사: 총구 섬광과 발사음 사이 프레임 수"), i + 1), 0, Frames);
		}
		FinishLab();
	});
		// [JA] コンソールコマンド obj gc と同じ
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// Output Log에서 "총구 섬광"과 "발사음 재생" 줄의 프레임 번호를 비교하세요. 몇 번째 발사가 늦나요?
// 첫 발만 늦다면 "처음 한 번만 하는 일"을 의심합니다. 두 번째는 빠른데 세 번째가 다시 늦다면,
// 두 번째와 세 번째 사이에 무엇이 있었나요?
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// Fire 에 중단점을 걸고 매번 FireSound.Get() 의 결과를 F10 으로 확인하세요. nullptr 이면 어느 길로 가나요?
// RequestAsyncLoad 의 콜백(람다)에도 중단점을 걸고, 호출 스택에서 누가 콜백을 불렀는지 보세요 (FStreamableHandle, 비동기 로딩 틱).
// 콘솔: obj list class=SoundWave 로 지금 메모리에 있는 사운드를 볼 수 있습니다. GC 전과 후를 비교하세요.
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// 실제 게임에서는 stat audio, stat soundwaves, 그리고 Unreal Insights(-trace=default,loadtime)의 Asset Loading 트랙으로
// "누가 언제 무엇을 로드했나"를 봅니다.
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// 소프트 레퍼런스는 "필요할 때 불러온다"는 뜻이고, 불러온 뒤에도 객체를 살려 두지 않습니다.

// [JA] アウトプットログで「銃口の閃光」と「発射音を再生」の行のフレーム番号を比べてください。何回目の発射が遅れていますか？
// [JA] 1 発目だけ遅れるなら「最初に 1 回だけやること」を疑います。2 回目は速いのに 3 回目がまた遅れるなら、
// [JA] 2 回目と 3 回目の間に何がありましたか？
// 비동기 로드는 적어도 다음 프레임에 끝나고, 핸들을 버리면 다음 GC에서 다시 내려갑니다.
// 바로 울려야 하는 소리는 언제 불러 두고, 무엇으로 붙잡아 둬야 할까요? (BeginPlay, UPROPERTY 하드 레퍼런스)
// 동기 로드(LoadSynchronous)로 바꾸면 늦지는 않지만, 그 프레임이 멈춥니다(히치). 그것도 정답이 아닙니다.
#pragma endregion

// [JA] Fire にブレークポイントを設定し、毎回 FireSound.Get() の結果を F10 で確認してください。nullptr ならどちらの道へ進みますか？
// [JA] RequestAsyncLoad のコールバック（ラムダ）にもブレークポイントを設定し、呼び出し履歴で誰がコールバックを呼んだかを見てください（FStreamableHandle、非同期ロードの Tick）。
// [JA] コンソール: obj list class=SoundWave で、今メモリにあるサウンドを見られます。GC の前と後を比べてください。
// [JA] 実際のゲームでは stat audio、stat soundwaves、そして Unreal Insights（-trace=default,loadtime）の Asset Loading トラックで
// [JA] 「誰がいつ何をロードしたか」を見ます。

// [JA] ソフト参照は「必要なときに読み込む」という意味で、読み込んだ後もオブジェクトを生かしておいてはくれません。
// [JA] 非同期ロードは早くても次のフレームに終わり、ハンドルを捨てると次の GC でまたアンロードされます。
// [JA] すぐに鳴らすべき音は、いつ読み込んでおき、何でつかまえておくべきでしょう？（BeginPlay、UPROPERTY のハード参照）
// [JA] 同期ロード（LoadSynchronous）に変えれば遅れはしませんが、そのフレームが止まります（ヒッチ）。それも正解ではありません。
