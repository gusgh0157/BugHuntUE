# 解答 (ネタバレ注意!)

[한국어](SOLUTIONS.md) | **日本語**

> 注: この文書の出力例は日本語の出力です。ラボを日本語で実行するには `-BugHuntLang=ja`、コンソール変数 `BugHunt.Lang ja`、またはエディター/OS の言語を日本語にします (README.ja.md 参照)。

# 第1部: クラッシュの追跡

## ラボ 1 — Cast の失敗をチェックしない
```
止まった関数:      ABH_Hero::Heal (this == nullptr)
おかしな値:        GiveHealth の Hero == nullptr
その値が来たフレーム: OnTouched: Toucher == BH_Pet_0 → Cast<ABH_Hero>(Toucher) == nullptr
経由したエンジン機能: オーバーラップイベント (UPrimitiveComponent::UpdateOverlaps → BeginComponentOverlap → OnComponentBeginOverlap デリゲート → execHandleOverlap)
本当の原因:        アイテムに触れたアクターは英雄だと決めつけていた。ペット (StartLab の 2 つ目の SetActorLocation) が触れたので Cast が失敗した。
```
**修正:**
```cpp
void ABH_HealthPickup::OnTouched(AActor* Toucher)
{
    ABH_Hero* Hero = Cast<ABH_Hero>(Toucher);
    if (!Hero)
    {
        if (const ABH_Pet* Pet = Cast<ABH_Pet>(Toucher))
        {
            Hero = Pet->Master;     // 仕様: ペットが拾ったアイテムは飼い主に
        }
    }
    if (!Hero)
    {
        return;                     // 英雄でもペットでもなければ無視 (アイテムは残しておく)
    }
    GiveHealth(Hero);
}
```
**教訓:** `Cast<T>` は失敗すると nullptr を返すだけで、知らせてはくれません。「必ず T のはず」なら `CastChecked<T>` でその場で落とし、そうでなければ結果をチェックします。オーバーラップイベントには **思ったより多くの種類のアクター** が入ってきます。

## ラボ 2 — 他のアクターが使うメンバーを BeginPlay の後半で作る
```
止まった関数:      [エンジンのテンプレート] TMulticastDelegate::AddUObject ← ABH_ScoreHud::BeginPlay
おかしな値:        Manager->Scoreboard == nullptr
その値が来たフレーム: ABH_ScoreManager::BeginPlay — 緑の矢印が SpawnActor<ABH_ScoreHud> の行。Scoreboard はその次の行で作る。
経由したエンジン機能: UWorld::SpawnActor → AActor::FinishSpawning → PostActorConstruction → DispatchBeginPlay → BeginPlay
本当の原因:        すでに開始したワールドで SpawnActor すると、新しいアクターの BeginPlay が SpawnActor の中ですぐに実行される。
                 HUD の BeginPlay はマネージャーの BeginPlay が終わる前に実行された。
```
**修正 (どれか 1 つ):**
- `Scoreboard = NewObject<...>` を `SpawnActor` の上に移します。
- さらに良いのは、他人が使うメンバーを **コンストラクタ** で `CreateDefaultSubobject<UBH_Scoreboard>(TEXT("Scoreboard"))` で作るか、`PostInitializeComponents` で準備することです。どちらもどの BeginPlay よりも先に実行されます。

**追加の防御 (単独では FAIL):** HUD 側の `if (UBH_Scoreboard* Board = Manager->GetScoreboard()) { ... }`。クラッシュは防げますが、HUD がスコアボードを購読できないのでスコアが更新されません。上の修正と一緒に使います。

**教訓:** アンリアルのアクターの初期化順序は `コンストラクタ → (プロパティ値の適用) → PostInitializeComponents → BeginPlay` です。BeginPlay 同士の順序は「スポーン順/レベルのロード順」なので当てにしてはいけません。クラッシュ位置がエンジンのテンプレート (`FMRSWRecursiveAccessDetector`、`AddDelegateInstance`) でも、最初に出てくる自分のコードから見れば大丈夫です。

## ラボ 3 — 範囲ベース for の途中で Remove
```
Ensure:          "Array has changed during ranged-for iteration!" ← TCheckedPointerIterator::operator!= ← CleanupDeadEnemies
追ってみると:      [Slime, Bat, Orc, Wolf, Golem]
                 1 周目 Slime を削除 → [Bat, Orc, Wolf, Golem]  (Bat が 0 番に詰められる)
                 2 周目は 1 番 = Orc  → Bat はチェックされない
                 3 周目 Wolf を削除 → [Bat, Orc, Golem]
                 4・5 周目は配列の末尾を越えた古い要素 (Golem のコピー)
結果:            Bat (HP 0) が残り、削除数 2
```
**修正:**
```cpp
for (int32 i = Enemies.Num() - 1; i >= 0; --i)
{
    ABH_Enemy* Enemy = Enemies[i];
    if (Enemy->Health <= 0)
    {
        Enemies.RemoveAt(i);
        Enemy->Destroy();
        ++Removed;
    }
}
```
または「選ぶ」と「消す」を分けます: まず死んだ敵を別の配列に集め (`FilterByPredicate`)、`Enemies.RemoveAll(...)` で一度に取り除いてから、集めた配列を回って `Destroy()`。
**教訓:** コンソール版ラボ 13 (erase 後のイテレータ) のアンリアル版です。UE は Development ビルドで `ensure` で知らせてくれますが、**セッションにつき 1 回だけ** です。`ensure` は無視して先に進んではいけない「落ちないクラッシュ」です。

## ラボ 4 — UPROPERTY のない UObject ポインター (GC 後の解放済みメモリの使用)
```
止まった関数:      UnknownFunction (アドレスが 0x00000000000fxxxx のようなおかしな値) ← ABH_QuestGiver::AcceptQuest
おかしな値:        QuestLog のアドレスはそのままなのに中身 (名前、Entries) がゴミ。GC の前にメモしておいた値と違う。
本当の原因:        QuestLog メンバーに UPROPERTY がないので GC がこの参照を知らない。誰も参照していないオブジェクトとみなして回収した。
                 Outer (NewObject の第 1 引数 this) はオブジェクトを生かしておかない。
                 回収されたメモリで仮想関数 AddEntry を呼ぶと、壊れた vtable をたどってとんでもないアドレスにジャンプした。
```
**修正:** ヘッダーで
```cpp
UPROPERTY()
TObjectPtr<UBH_QuestLog> QuestLog;
```
**教訓:**
- UObject を指すメンバーには `UPROPERTY()` (値は `TObjectPtr<>`) が付いていてはじめて、GC が「生きている」とわかります。UObject ではないクラス (F クラス) が UObject を持つ必要があるなら、`FGCObject` を継承するか `TStrongObjectPtr` を使います。
- 持っているだけで生かしておいてはいけない場合 (他人のオブジェクトの観察) は `TWeakObjectPtr` + 使うときに `IsValid()`/`Get()` でチェック。
- 解放済みメモリの使用は毎回症状が違います。再現するには `gc.CollectGarbageEveryFrame 1`、確実に捕まえるには `-stomp2malloc` (解放と同時にアクセス禁止ページに変え、その行で止まります)。

---

# 第2部: 流れの追跡 (クラッシュなし)

## ラボ 5 — ウィンドウを閉じるときにデリゲートを解除しない
```
間違った結果:     記録 2 行、通知 4 回
止まった位置:     HandleDamaged のブレークポイント → Damage が 10, 5, 7, 7 の順で 4 回
期待 → 実際:      10, 7 → 10, 5 (ウィンドウが閉じているのに!), 7, 7
最初にずれた所:   Close() の後の TakeHit(5) が HandleDamaged を呼んだ。OnDamaged.InvocationList の数: Open 後 1 → Close 後 1 → 再 Open 後 2
本当の原因:       Open で AddUObject、Close では Remove なし。ウィンドウオブジェクトはプールされて生きているので呼ばれ続け、再び開くとまた追加される。
```
**修正:**
```cpp
void UBH_CombatLogPanel::Close()
{
    if (Target)
    {
        Target->OnDamaged.RemoveAll(this);
    }
    Target = nullptr;
    Lines.Reset();
}
```
(または `FDelegateHandle Handle = ...AddUObject(...)` をメンバーに保存して `Remove(Handle)`。)
**教訓:** `AddUObject`/`AddDynamic` はオブジェクトが **破棄されれば** 安全に無視されますが、生きているオブジェクトはいつまでも呼ばれます。**Add した場所ごとに対になる Remove** (EndPlay、Close、Deinitialize) が必要です。`AddDynamic` を同じ関数で 2 回行うと Development ビルドでは ensure が出ますが、**それでも 2 回登録されます** (UE 5.8 `ScriptDelegates.h` の `AddInternal`: `#if DO_ENSURE` のチェックの後でそのまま `Add`)。Shipping では ensure もなく静かに重複します。`AddUObject`/`AddLambda` はどのビルドでも静かに重複します。重複を防ぐには `AddUniqueDynamic`。

## ラボ 6 — ローカル変数で受け取ったタイマーハンドル
```
間違った結果:     浄化後にダメージ 3 回、HP 50
止まった位置:     出力ログ (Output Log) の時刻 / ListTimers / Cleanse のブレークポイント
期待 → 実際:      0.4 0.8 1.4 1.8 → 0.4 0.8 1.2 1.4 1.6 1.8 2.0 2.2 2.6 3.0 (1.0 秒後から間隔が 0.2 秒に縮む = タイマー 2 つ)
最初にずれた所:   1.2 秒のダメージ (元のタイマーが RefreshPoison の後も生きている)
本当の原因:       RefreshPoison がローカルの FTimerHandle NewTimer で新しいタイマーを作った。
              ① 既存の PoisonTimer は消されず、② 新しいタイマーのハンドルは関数の終わりで消えるので Cleanse には消す方法がない。
              Cleanse は元のタイマーだけを消し、新しいタイマーは永遠に回り続ける。
```
**修正:**
```cpp
void ABH_PoisonVictim::RefreshPoison()
{
    // 同じハンドルで SetTimer すると、既存のタイマーを消して新しく開始する。
    GetWorldTimerManager().SetTimer(PoisonTimer, this, &ABH_PoisonVictim::PoisonTick, PoisonInterval, true);
}
```
**教訓:** 後で止めたり変えたりする必要のあるタイマーのハンドルは **メンバー** に持っておく必要があります。「タイマーが 2 つ回っているか」は `ListTimers` がすぐに見せてくれます。アクターが破棄されればそのアクターのタイマーは片付けられますが、長生きするアクター (GameState、プレイヤー) なら最後まで回り続けます。

## ラボ 7 — 読む側が書く側より先に Tick
```
間違った結果:     30 フレームの間 HP バーが 1 ずつ遅れ、倒れたフレームで HP バー 1
止まった位置:     2 つの Tick にトレースポイント → 出力順が毎フレーム「Bar → Hero」
最初にずれた所:   最初のフレームから。HP バー (TG_PrePhysics) が英雄 (TG_PostUpdateWork) より先に実行され、前のフレームの HP を読む
本当の原因:       コンストラクタの PrimaryActorTick.TickGroup = TG_PostUpdateWork
```
**修正 (両方行うのがおすすめ):**
```cpp
// ABH_BleedingHero のコンストラクタ: TickGroup の行を消す (デフォルトは TG_PrePhysics)
// ABH_HealthBar::Attach:
Hero = InHero;
AddTickPrerequisiteActor(InHero);     // 英雄が Tick した後に自分が Tick
DisplayedHealth = Hero->Health;
```
**教訓:** Tick グループが違えばグループの順に、**同じグループの中では順序が決まっていません。** 順序が重要なら `AddTickPrerequisiteActor`/`AddTickPrerequisiteComponent` で明示します。1 フレーム遅れるバグは画面ではほとんど見えませんが、判定 (死亡処理、ヒットチェック) に使われると本物のバグになります。コンストラクタの変更なので、**エディタを閉じてビルド** しないと反映されません。

## ラボ 8 — 3D の距離で床への到着を判定
```
間違った結果:     5 秒間でウェイポイント 0 個
使ったツール:     Visual Logger — 緑の点 (巡回兵) が最初の黄色い円の近くで前後に震え、ログの距離は常に 90 以上
期待 → 実際:      水平に重なれば距離 < 50 → 距離は最小 90.0
本当の原因:       アクターの位置 Z = 90 (体の中心)、ウェイポイント Z = 0 (床)。移動は 2D なのに判定は 3D。
```
**修正:**
```cpp
const float Distance = FVector::Dist2D(GetActorLocation(), Target);
```
(またはウェイポイントをアクターの高さに合わせるか、到着判定に高さの許容値を別に設けます。)
**教訓:** 位置・経路のバグは数字だけ見ていても見えず、**描いてみれば** すぐに見えます。Visual Logger はフレームごとの記録を録画して巻き戻せるので、「毎フレーム回るコード」の F10 の代わりになります。`UE_VLOG_*` は録画中でなければコストがほとんどないので、AI/移動のコードにあらかじめ入れておくとよいです。

## ラボ 9 — Destroy されたアクターを nullptr 比較でチェック
```
間違った結果:     スコア 700、かかし 2 体が立っている
止まった位置:     Score += のブレークポイント 2 回目のヒット → CurrentTarget = BH_Dummy_0、Health -10
最初にずれた所:   矢 4: if (!CurrentTarget) が false (ポインターはそのまま) → 倒れた BH_Dummy_0 をまた撃つ → Health <= 0 なのでまた「撃破」
本当の原因:       Destroy() はアクターを Garbage とマークするだけ。UPROPERTY ポインターが nullptr になるのは次の GC のとき
               (ガベージ除去が有効なときだけ。UE 5.8 の gc.GarbageEliminationEnabled、旧名 gc.PendingKillEnabled。
                無効なら GC は参照されている Garbage オブジェクトを消しも nullptr にもしない。)
```
**修正:**
```cpp
if (!IsValid(CurrentTarget))
{
    CurrentTarget = FindNearestTarget();
}
if (!IsValid(CurrentTarget))
{
    return;
}
```
**教訓:** UObject ポインターの「生きている」チェックは `IsValid()` です。`if (Ptr)` は「アドレスがある」という意味でしかありません。このバグは **GC がいつ回るかによって** ときどき消えます (ガベージ除去が有効なら GC の後でポインターが nullptr に片付けられます)。「たまにしか起きないバグ」の典型です。同じファイルの `FindNearestTarget` がすでに `IsValid` を使っていたことも手がかりでした。

## ラボ 10 — Config ファイルがコンストラクタのデフォルト値を上書き
```
間違った結果:     MaxHealth 100 (期待 150)
止まった位置:     コンストラクタのブレークポイント → 150 が入る。データブレークポイント → コンストラクタが終わった後でまた変わる
              getall BH_Slime MaxHealth → 100
最初にずれた所:   クラスのデフォルトオブジェクト (Default__BH_Slime) がコンストラクタを実行した後、Config/DefaultGame.ini の [/Script/BugHuntUE.BH_Slime] MaxHealth=100 を読む。
              新しくスポーンしたスライムはコンストラクタを実行した後、CDO の値を引き継ぐ。
本当の原因:       昔の調整値が ini に残っていた。コードだけ変えた「パッチ」には ini が勝つ。
```
**修正:** `Config/DefaultGame.ini` から `MaxHealth=100` の行を消すか、`MaxHealth=150` に変えます。(バランス値の持ち主を 1 か所に決めます: コードか、ini か、データアセットか。)
**教訓:** アンリアルのオブジェクトの値は `コンストラクタ → CDO のシリアライズ値 (Config、ブループリントのデフォルト値、アセット) → インスタンスのシリアライズ値 (レベルに配置したアクターの詳細 (Details) パネルの値)` の順で上書きされます。コンストラクタの値は **いちばん先**、つまりいちばん弱いのです。ブループリントも同じです: BP で変えて保存した値は C++ のコンストラクタの値に勝ち、変えていない値だけが C++ のデフォルト値に従います。「C++ のデフォルト値を変えたのに変わらない」なら、Config フォルダ、ブループリントのクラスのデフォルト (Class Defaults)、レベルに配置されたインスタンスの値を順に疑いましょう。

---

## ラボ 11 — 移動に DeltaSeconds を掛けない
```
間違った結果:     30fps で突進距離 160 cm、120fps で 610 cm (期待 300 cm)
止まった位置:     ABH_Dasher::Tick に条件付きブレークポイント bDashing。DeltaSeconds は 0.0333 / 0.0083、毎回 X が 10 ずつ増える。
               トレースポイントで数えると 0.5 秒の突進で Tick が 16 回 / 61 回
本当の原因:       DashSpeed が「1 フレームに 10cm」だった。60fps で合わせた数字なので 60fps でしか合わない。
               突進時間 (DashTimeLeft) は DeltaSeconds で減らしたので、移動回数だけがフレームレートに従った。
```
**修正:**
```cpp
float DashSpeed = 600.f;   // 秒速 cm
AddActorWorldOffset(FVector(DashSpeed * DeltaSeconds, 0.f, 0.f));
```
**教訓:** Tick の中で「どれだけ」を変えるすべての式 (移動、回転、クールダウン、ゲージ) には `DeltaSeconds` が掛けられている必要があります。単位を名前に書いておきます (`SpeedCmPerSec`)。テストは `t.MaxFPS 30` と `t.MaxFPS 120` の 2 回実行してみます。

## ラボ 12 — フレームごとに 0.9 倍の減速
```
間違った結果:     30fps で 400 cm、120fps で 100 cm (60fps 基準で 200 cm)
止まった位置:     Tick にトレースポイント {Velocity}。1 秒後の速度が 30fps で約 50、120fps でほぼ 0
本当の原因:       Velocity *= 0.9 は「Tick 1 回で 10%」の減速。1 秒に 30 回なら 0.9^30、120 回なら 0.9^120。
               移動に DeltaSeconds を掛けただけでは足りない。減速 (掛け算) も時間に合わせる必要がある。
```
**修正:**
```cpp
Velocity *= FMath::Pow(Friction, DeltaSeconds * 60.f);    // 「1/60 秒ごとに 0.9 倍」
// または最初から時間基準で: Velocity *= FMath::Exp(-Damping * DeltaSeconds);
```
**教訓:** 足し算は `* DeltaSeconds`、掛け算は `Pow(x, DeltaSeconds * 基準fps)` または `Exp(-k * DeltaSeconds)` です。`Lerp(Current, Target, 0.1f)` を毎 Tick 呼ぶカメラ/UI の補間も同じバグです (`FInterpTo` を使います)。位置と速度をどの順で更新するか (オイラー積分) によっても、フレームレートで結果が少し変わります。数値が正確である必要があるなら、固定タイムステップか物理のサブステップを使います。

## ラボ 13 — 構造体メンバーに初期値がない
```
間違った結果:     2 ラウンド目のキルが 1, 1, 1 のはずが -1527265117, 40405, ... (実行ごとに違うことがある)
止まった位置:     2 回目の StartRound の AddDefaulted_GetRef の次の行。Entry.Player は新しい名前、Kills/Deaths はおかしな値。
               エディタ起動時の出力ログ: LogClass: Error: IntProperty FBH_RoundStats::Kills is not initialized properly.
本当の原因:       FBH_RoundStats の Kills、Deaths に初期値がない。AddDefaulted はデフォルトコンストラクタを呼ぶが、初期値がないと int32 は
               その場所にあったバイトのまま。Stats.Reset() はメモリを残すので、その場所には 1 ラウンド目の記録 (とそれ以前のゴミ) があった。
               ABH_RoundBoard::Round は初期値がなくても 0 から始まった。UObject はメモリを 0 で埋めてから作られるからだ。
```
**修正:**
```cpp
UPROPERTY() int32 Kills = 0;
UPROPERTY() int32 Deaths = 0;
```
`Stats.AddZeroed_GetRef()` でもこの 1 か所は直りますが、ローカル変数 `FBH_RoundStats S;` や `FMemory::Malloc` で作った場所は相変わらずゴミです。
**教訓:** 構造体とローカル変数は宣言する行で初期化します (UE のコーディング規約)。UObject の「自動 0」に頼らず、UObject のメンバーにも初期値を書きます。「is not initialized properly」の行は無視しません。ゴミ値のバグは Debug (0xCD のようなパターン)、Development、別の PC でそれぞれ違って見えるので、「自分の PC では動くんですが」の常連です。

# 第3部: コンテナ

## ラボ 14 — 1 から数えるウェーブ番号をそのままインデックスに使う
```
間違った結果:     ウェーブ 4 の報酬まで受け取って Assertion failed: (Index >= 0) & (Index < ArrayNum)  [Array.h]
止まった位置:     呼び出し履歴 (Call Stack) の Array.h の RangeCheck → 1 行上の ABH_WaveRewards::ClaimRewards。ウォッチ式 Wave = 5、Rewards.Num() = 5
本当の原因:       for (Wave = 1; Wave <= NumCleared) で回して Rewards[Wave]。ウェーブ 1 に Rewards[1] (2 つ目の報酬) を与え、
               最後のウェーブで Rewards[5] で範囲を越えた。ログのゴールド (20, 50, 90...) も最初から 1 つずつずれている。
```
**修正:**
```cpp
Gold += Rewards[Wave - 1];     // または for (int32 i = 0; i < NumCleared; ++i) Gold += Rewards[i];
```
**教訓:** アンリアルの `TArray` は Development ビルドでも範囲をチェックします (`checkSlow` ではなく `RangeCheck`)。おかげで「たまにゴミ値」ではなく、その場で落ちます。assert が出たら、メッセージ中の式 (`Index < ArrayNum`) の 2 つの値をまずウォッチ式で見ます。「何番目」(1 から) と「インデックス」(0 から) が混ざる場所は、変数名で区別しておきます (`WaveNumber` / `WaveIndex`)。

## ラボ 15 — 存在しないキーで TMap::operator[]
```
間違った結果:     KingsLetter を売るときに Assertion failed: Pair != nullptr  [Map.h.inl, FindChecked]
止まった位置:     呼び出し履歴 FindChecked → operator[] → ABH_Shop::SellAll。ウォッチ式 Item = KingsLetter、Prices にそのキーなし
本当の原因:       TMap::operator[] は FindChecked。std::map のようにデフォルト値を入れてくれず、check で落ちる。
```
**修正:**
```cpp
const int32* Found = Prices.Find(Item);
if (!Found) { ++Refused; continue; }   // 価格表にない品物は返す
Total += *Found;
```
なければ 0 でよい場合は `Prices.FindRef(Item)`、ないときに追加したければ `Prices.FindOrAdd(Item)`。
**教訓:** キーが必ずあると **証明できるときだけ** `[]`/`FindChecked` を使います。データ (クエストアイテム、新しい DLC アイテム) が増えるとその証明が崩れます。`std::map` の癖 (`m[k]` で追加) はアンリアルでは正反対に動きます。

## ラボ 16 — TMap::Find で受け取ったポインターを持ったまま新しいキーを追加
```
間違った結果:     掲示板のオオカミ討伐数 期待 3、実際 2 (ログには "依頼対象 3 匹")
止まった位置:     RecordHunt の ++(*TargetKills) にブレークポイント。Goblin が入る前後で TargetKills のアドレスと
               Kills を展開した Wolf 要素のアドレスを比べる → Goblin (5 種類目) の後で食い違う
本当の原因:       Find がくれた int32* はマップ内部の配列の中のアドレス。FindOrAdd で新しいキーが入って配列が大きくなると (ここでは
               TInlineSetAllocator<4> を超えた瞬間) 要素が移動し、TargetKills は古い場所を指す。
               古い場所 (インラインバッファ) はそのまま残っているので落ちず、本物の要素だけが増えない。
```
**修正:**
```cpp
for (const FName& Monster : Killed)
{
    ++Kills.FindOrAdd(Monster);       // 使うたびに探す
}
```
**教訓:** ラボ 23 の `TArray` 要素の参照と同じ規則が `TMap`/`TSet` にもあります。`Find`/`FindOrAdd`/`Add` がくれたポインター・参照は **次の追加や削除までしか** 信用しません。削除は Compact されるまで場所を空けておきますが、それも保証されたものではありません。あらかじめ持っておきたいなら、キーを覚えておいて探し直します (ハッシュ 1 回は安いです)。

## ラボ 17 — 順序が重要な配列で RemoveAtSwap
```
間違った結果:     行動順 期待 盗賊 → 騎士 → 魔法使い → 僧侶、実際 盗賊 → 僧侶 → 騎士 → 魔法使い
止まった位置:     RemoveUnit の RemoveAtSwap の次の行。ウォッチ式で Order を展開すると最後の要素 (僧侶) が空いた場所 (1 番) に来ていた
本当の原因:       RemoveAtSwap/RemoveSwap は「最後の要素を空いた場所に移す」。O(1) だが順序を守らない。
```
**修正:**
```cpp
Order.RemoveAt(Index);        // 後ろを 1 つずつ詰める。順序を維持、O(n)
// 値で消すときは Order.Remove(Unit) (すべて消す) / RemoveSingle(Unit) (最初の 1 つ)
```
**教訓:** `RemoveAtSwap` は順序に意味のない集まり (スポーンした弾のリストなど) にだけ使います。ソート済みの配列、ターン順、UI のリストに使うと、どこかで「たまに順序がおかしくなる」として現れます。名前に `Swap` が入った関数は、何と何を入れ替えるのか一度考えます。5 人の配列で O(n) はコストではありません。

## ラボ 18 — operator== は座標だけ、GetTypeHash は Source まで見る
```
間違った結果:     判明したマス数 期待 7、実際 8 / 明かしたマス 8 つを問い合わせると「判明済み」は 2 つだけ (公開入力。いくつ合うかはハッシュ次第)
止まった位置:     ウォッチ式で Revealed を展開すると (2,0) が Source だけ違う状態で 2 回入っている。
               IsRevealed(0,0) は Source が空のキーで探す → ハッシュが違うので別のバケットを見る
本当の原因:       TSet/TMap は GetTypeHash でバケットを選んでから、その中だけで == で比較する。
               「a == b なら Hash(a) == Hash(b)」が崩れると、同じキーが別のバケットに入って重複し、見つけることもできない。
```
**修正:**
```cpp
friend uint32 GetTypeHash(const FBH_CellKey& Key)
{
    return HashCombine(GetTypeHash(Key.X), GetTypeHash(Key.Y));   // == が見るフィールドだけ
}
```
さらに良い方法は、キーから Source を外して `TMap<FIntPoint, FString>` に変えることです (`FIntPoint` はハッシュと == がすでに一致しています)。
**教訓:** ハッシュキーの構造体は `operator==` と `GetTypeHash` を **同じフィールドで、並べて** 書きます。キーのフィールドを入れた後で変えるのも同じバグです (古いハッシュのバケットに残ります)。変える必要があるなら、取り出してから入れ直します。検索が「たまに」失敗するならハッシュを疑います。

## ラボ 19 — UPROPERTY のない TArray<UObject*>
```
間違った結果:     GC の後も生きている袋の中のアイテム 期待 3、実際 0 / 最近見たリストの有効なアイテム 期待 3、実際 0
止まった位置:     CollectGarbage の後にブレークポイント。obj list class=BH_LootItem が GC 前 4 個、後 0 個。Bag->Items の要素はアドレスだけが残った
本当の原因:       Items に UPROPERTY がないので GC が配列の中のポインターを知らない。アイテムをつかんでおく参照が 1 つもないので全部回収された。
               (Outer が袋でもつかんでおいてはくれない。Outer は名前とパスにすぎず、参照ではない。)
               RecentlySeen (TWeakObjectPtr) は正しく動作した: オブジェクトが消えたので無効になっただけ。
```
**修正:**
```cpp
UPROPERTY()
TArray<TObjectPtr<UBH_LootItem>> Items;
```
**教訓:** ラボ 4 の規則はコンテナにもそのまま当てはまります。UObject を入れる `TArray`/`TMap`/`TSet` には `UPROPERTY()` を付けるか (UObject のメンバー)、`AddReferencedObjects`/`FGCObject` で知らせます (UObject ではないクラス)。つかんでおくつもりのないリストは `TWeakObjectPtr` で持ち、使うときに `IsValid`。要素の型は `TObjectPtr<>` で書くのが UE5 の規則です (エディタビルドでアクセスを追跡)。

## ラボ 20 — FastArraySerializer の要素を変えて MarkItemDirty しない
```
間違った結果:     [リッスンサーバー] 進行度 3, 5 PASS / [クライアント] 進行度 0, 0 FAIL (クエスト名 3 つはクライアントにある)
止まった位置:     クライアントプロセスで GetProgress にブレークポイント。Quests.Items は 3 個、Progress はすべて 0。
               サーバーで AddQuest と AddProgress を並べて見ると、MarkItemDirty が AddQuest にしかない
本当の原因:       FFastArraySerializer は以前の値と比較しない。要素ごとに ReplicationKey を見て「マークされた要素」だけを送る。
               AddQuest はマークしたので最初の状態 (進行度 0) は送られ、その後の変更はマークがないので永遠に送られない。
```
**修正:**
```cpp
Entry.Progress += Amount;
Quests.MarkItemDirty(Entry);       // 要素を消したなら Quests.MarkArrayDirty()
```
**教訓:** 普通の `UPROPERTY(Replicated) TArray` はエンジンが毎回比較して勝手に送ってくれます (その代わり大きな配列だと高くつきます)。`FFastArraySerializer` は比較を省く代わりに、**変えた人がマークする責任** を負います。要素を変える関数を 1 か所に集めて (このラボの AddProgress)、そこでだけ `MarkItemDirty` すれば漏れません。クライアント側の反応は要素の `PostReplicatedAdd`/`PostReplicatedChange`/`PreReplicatedRemove` で受け取ります。

# 第4部: ポインターと寿命

## ラボ 21 — UObject ではない場所に raw ポインターでアクターを覚える
```
間違った結果:     Bram のヘイト 60 FAIL (期待 10)。ボスが狙う対象 Bram FAIL (期待 Lia)
止まった位置:     Kael->Destroy() の行でウォッチ式 Kael = 0x00000xxxxxxx をメモ → Bram のスポーン後、Bram のアドレスがまったく同じ
               ThreatTable.Threat にキーが 2 つだけ: (Kael が使っていたアドレス = 今の Bram) 60、Lia 30
本当の原因:       FBH_ThreatTable は普通の C++ 構造体なので UPROPERTY を付けられない。だから AActor* でキーを持った。
               GC が Kael のメモリを返すと同じサイズの Bram がその場所に作られ、表はアドレスだけを見て Bram を Kael と勘違いした (ABA)。
               アドレスが再利用されなかったなら: 表に消えたメモリを指すキー (50) が残り、TopTarget がそれを選ぶ。
```
**修正:**
```cpp
// Lab21_ThreatTable.h
TMap<TWeakObjectPtr<AActor>, int32> Threat;

// Lab21_ThreatTable.cpp — TopTarget
for (const TPair<TWeakObjectPtr<AActor>, int32>& Pair : Threat)
{
    if (Pair.Key.IsValid() && Pair.Value > BestThreat)
    {
        Best = Pair.Key.Get();
        BestThreat = Pair.Value;
    }
}
```
`AddThreat`/`ThreatOf` はそのままでかまいません (`AActor*` が `TWeakObjectPtr` に変換されて探します)。死んだキーをときどき消したいなら、`Threat.Remove(nullptr)` ではなく `for (auto It = Threat.CreateIterator(); It; ++It) if (!It.Key().IsValid()) It.RemoveCurrent();`。
**教訓:** ラボ 4 (UPROPERTY のないメンバー) は「GC が知らない参照なので **消される**」でしたが、このラボは「消されるのは正しいのに **消されたことを知らない**」です。UObject を UObject ではない場所 (普通の構造体、ラムダ、別スレッド、Slate ウィジェット、グローバルキャッシュ) で覚えておくなら `TWeakObjectPtr`。オブジェクトを生かしておく必要があるなら、`TStrongObjectPtr` か `FGCObject` (AddReferencedObjects) で GC に知らせます。raw ポインターの「アドレスが同じ」は「同じオブジェクト」ではありません。

## ラボ 22 — タイマーのラムダが this をキャプチャしたがオブジェクトに結び付いていない
```
間違った結果:     標的に当たった回数 7 FAIL (期待 2)。"砲台破壊！" の後も発射 3, 4, 5... が出力される
止まった位置:     Fire の条件付きブレークポイント (ShotsFired >= 3) → 呼び出し履歴 FTimerManager::Tick → ラムダ → Fire
               ウォッチ式 this->bActorIsBeingDestroyed = true (すでに破棄された砲台)
本当の原因:       FTimerDelegate::CreateLambda はどの UObject にも結び付かない。アクターの EndPlay はそのアクターに結び付いた
               タイマーだけを消す (ClearAllTimersForObject)。ラムダの中の this はただのアドレス。
```
**修正 (どれか 1 つ):**
```cpp
GetWorldTimerManager().SetTimer(FireTimer, FTimerDelegate::CreateWeakLambda(this, [this] { Fire(); }), InInterval, true);
GetWorldTimerManager().SetTimer(FireTimer, FTimerDelegate::CreateUObject(this, &ABH_Turret::Fire), InInterval, true);
GetWorldTimerManager().SetTimer(FireTimer, this, &ABH_Turret::Fire, InInterval, true);
```
補助手段: `EndPlay` で `GetWorldTimerManager().ClearTimer(FireTimer)`。
**教訓:** デリゲート・タイマー・非同期コールバックに `[this]` を渡すときは、「このオブジェクトが先に消えたら?」を問います。`CreateLambda`/`AddLambda` + `[this]` は C++ のダングリングポインターとまったく同じです。今回は GC 前なので「撃つだけ」でしたが、GC が回った後ならラボ 4 のように解放済みメモリを読みます。アンリアルの答えは `CreateUObject`、`CreateWeakLambda`、`AddUObject`、`AddWeakLambda` (UObject)、`CreateSP`/`CreateSPLambda` (TSharedFromThis) です。

## ラボ 23 — 配列要素の参照を持ったまま Add (再割り当て)
```
記録表:          ポーション 4 (Gem の新しいスロット、4 スロット) → 3 → 2 (Herb の新しいスロット、5 スロット: インライン 4 スロットを超えてヒープへ引っ越し) → "1"
間違った結果:     残りのポーション 2 FAIL (期待 1)。ログには 1 と出るが本物のスロットは 2
止まった位置:     ウォッチ式 &Potions と &Slots[0] が Herb を入れる前は同じで、入れた後は違う
               データブレークポイント &Slots[0].Count → 最後のウェーブで止まらない (誰も本物のスロットを変えていない)
本当の原因:       FBH_BeltSlot& Potions は「その瞬間のメモリ位置」を指す。Add が配列を新しいメモリに移したので、古い位置に書くことになった。
```
**修正:**
```cpp
const int32 PotionIndex = FindSlot(Potion);
for (const FName& Drop : Drops)
{
    Slots[PotionIndex].Count -= 1;
    AddItem(Drop, 1);
}
```
`Slots.Reserve(...)` であらかじめ領域を確保しても今回は通りますが、「何スロットあれば十分か」をコードが保証できないなら根本的な解決ではありません。
**教訓:** `TArray` の `Add`、`Emplace`、`Insert`、`SetNum`、`Reserve`、`Append` は要素を移動させることがあります。その前に取り出した参照、ポインター、`GetData()` は無効です。デフォルトのアロケーターなら古い位置は **解放されたヒープ** なので、ときどき落ちたり他人のデータを壊したりし (-stomp2malloc で捕まります)、`TInlineAllocator` は古い位置が生きているので **落ちもせず値だけが消えます**。`TMap`/`TSet` の `FindOrAdd`/`Add` も同じです (`Find` で得たポインターは次の `Add` まで)。

## ラボ 24 — TSharedPtr の循環参照
```
間違った結果:     会話が終わった後に残ったノード 7 FAIL (期待 0)。デストラクタのブレークポイントが一度も止まらない
止まった位置:     EndConversation の Root.Reset() の直前にウォッチ式 Root → 参照カウント 4 (Root メンバー 1 つ + 選択肢 3 つの Parent。Current はすぐ前の行で Reset)
本当の原因:       親 → 子 (Choices) と子 → 親 (Parent) を両方 TSharedPtr で持った。輪の中の参照カウントは 0 にならない。
```
**修正:**
```cpp
// Lab24_DialogueTree.h
TWeakPtr<FBH_DialogueNode> Parent;

// Lab24_DialogueTree.cpp — GoBack
if (TSharedPtr<FBH_DialogueNode> Up = Current ? Current->Parent.Pin() : nullptr)
{
    Current = Up;
}
```
**教訓:** `TSharedPtr` は参照カウントを数えるだけです。GC のように「誰も届かない輪」を見つけてはくれません。所有の方向 1 つだけを `TSharedPtr`/`TSharedRef` にし、逆向きに指す側 (親、所有者、リスナー) は `TWeakPtr`。クラッシュも警告もないので、`stat memory`、`memreport -full`、LLM (`-llm`) で「増え続ける数字」として見つかることがよくあります。このラボのように生成/破棄のカウンターを置けばすぐに見えます。

# 第5部: 物理

## ラボ 25 — 速くて小さい物体のトンネリング (CCD 無効)
```
間違った結果:     弾丸が鉄板に当たらなかった (Hit 0 回)、弾丸の X が鉄板の向こう側
止まった位置:     Chaos Visual Debugger で録画してフレームを送ると、弾丸が鉄板の手前 → 鉄板の向こうへ一気に移る
本当の原因:       60fps で秒速 300m の弾丸は 1 物理ステップで 5m 進む。物理はステップごとに「今の位置」でしか重なりを見ないので
               厚さ 4cm の鉄板は 2 つの位置の間に挟まって見えない。
```
**修正:**
```cpp
Ball->SetUseCCD(true);    // 詳細パネル コリジョン → Use CCD
```
**教訓:** 1 ステップで進む距離 (速度 × ステップ時間) が相手の物体の厚さより大きいとトンネリングします。速いものにだけ CCD を有効にします (高くつきます)。弾丸は普通、物理の代わりにライントレース (ヒットスキャン) やスイープで移動する `UProjectileMovementComponent` を使います。サブステップはステップを小さくするだけなので、この速度では足りません。

## ラボ 26 — AStaticMeshActor はオーバーラップイベントの生成 (Generate Overlap Events) が無効
```
間違った結果:     木箱は感圧板の中に置かれたのに押された回数 0、扉が開かない
止まった位置:     PIE を一時停止 → 木箱の詳細パネル コリジョン (Collision): オーバーラップイベントの生成 (Generate Overlap Events) のチェックが外れた状態。
               ABH_WoodCrate のコンストラクタで F12 → AStaticMeshActor のコンストラクタ: StaticMeshComponent->SetGenerateOverlapEvents(false)
本当の原因:       Overlap イベントは、2 つのコンポーネントが互いに Overlap で応答し、「両方とも」オーバーラップイベントの生成が有効でないと起きない。
               感圧板だけ有効にして、木箱は親クラスのデフォルト値 (無効) をそのまま使っていた。
```
**修正:**
```cpp
Mesh->SetGenerateOverlapEvents(true);   // ABH_WoodCrate のコンストラクタ
```
**教訓:** トリガーが呼ばれないときは、① チャンネルの応答 (両側)、② オーバーラップイベントの生成 (両側)、③ 本当に重なったか (`show Collision`) の順に見ます。両方 Block なら Overlap ではなく Hit が来ます (そして Hit は Simulation Generates Hit Events が有効でないと物理衝突からは来ません)。親クラスのコンストラクタが変えるデフォルト値を疑います (`AStaticMeshActor`、`ACharacter` のカプセルなど)。

## ラボ 27 — SetActorLocation のデフォルトは瞬間移動 (bSweep=false)
```
間違った結果:     大男の背面 X=540、城壁の前面 X=280
止まった位置:     Knockback の SetActorLocation で F12 → bSweep = false がデフォルト値
本当の原因:       Block のコリジョンはスイープする移動しか止めない。瞬間移動は目的地に置くだけで、途中の道は見ない。
```
**修正:**
```cpp
FHitResult Hit;
SetActorLocation(Target, /*bSweep*/ true, &Hit);   // 城壁の前で止まる。Hit.bBlockingHit で「壁ドン」処理
```
**教訓:** ゲームプレイの移動 (ノックバック、ダッシュ、押し出し) はスイープで行います。瞬間移動が正しい場合 (リスポーン、テレポート) にだけ `bSweep=false`。スイープはルートコンポーネントの形状でしか行いません。キャラクターは `LaunchCharacter`/`CharacterMovement` が代わりにやってくれます。

## ラボ 28 — 物理シミュレーション中の物体を位置で動かす
```
間違った結果:     放した直後の岩の速度 (0, 0, -635) (アームの速度 X 1000、Z 1000) → その場で落ちる
止まった位置:     Tick にトレースポイント {Carried->Ball->GetPhysicsLinearVelocity()}: 持っている間 X は 0、Z は重力のせいでどんどん大きくなる負の数
本当の原因:       SetActorLocation は物理オブジェクトを「その場所に移して置く」だけで、物理の速度には触れない。
               毎フレーム移して置いても、物理エンジンは岩が前に動いているとは知らず、放した瞬間に物理が知っている速度で落ちる。
```
**修正:**
```cpp
void ABH_Catapult::Release()
{
    Carried->Ball->SetPhysicsLinearVelocity(ArmVelocity);   // 放す瞬間の速度を物理に教える
    Carried = nullptr;
}
```
持っている間から物理で動かしたいなら、Tick で `SetPhysicsLinearVelocity((ArmTip - 現在位置) / DeltaSeconds)` か `UPhysicsHandleComponent`。
**教訓:** シミュレーション中の物体は、位置ではなく速度、力、インパルスで動かします。Tick で位置を上書きすると、投げるときに速度が消え、他の物体とぶつかるときにガタガタ震えたり突き抜けたりします。どうしても位置で動かす必要があるなら、その間シミュレーションを切る (キネマティック) か、物理ハンドルを使います。

# 第6部: サウンド

## ラボ 29 — ソフト参照を撃つときに初めてロードする
```
間違った結果:     1 発目は 3 フレーム (約 13ms) 遅れ、2 発目は 0、3 発目 (GC の後) は再び 2 フレーム遅れ
止まった位置:     Fire のブレークポイント → FireSound.Get() == nullptr → RequestAsyncLoad の経路。コールバックは数フレーム後の非同期ロードのティックで呼ばれる
                  ログの「(発射音がメモリにない)」/ obj list class=SoundWave で GC の前後を比較
最初にずれた所:   1 発目の Fire: 音がメモリになかったので、その場で非同期ロードを開始
本当の原因:       ① すぐに鳴らすべき音を「撃つとき」にロードした。非同期ロードはどんなに速くても次のフレーム以降に終わる。
                  ② ロードした後もハード参照がなく (ソフトポインタと捨てたハンドルだけ)、GC が再びアンロードした → GC の後の 1 発目がまた遅れる。
```
**修正:**
```cpp
// Lab29_FireSound.h
protected:
    virtual void BeginPlay() override;

    UPROPERTY()
    TObjectPtr<USoundBase> LoadedFireSound;     // 保持しておくハード参照

// Lab29_FireSound.cpp
void ABH_Blaster::BeginPlay()
{
    Super::BeginPlay();
    LoadedFireSound = FireSound.LoadSynchronous();   // 武器を持つとき (ロード画面、装備した時点) に前もって
}

void ABH_Blaster::Fire()
{
    if (LoadedFireSound)
    {
        PlayFireSound(LoadedFireSound, GFrameCounter, FPlatformTime::Seconds());
        return;
    }
    ... (既存のコードは予備の経路として残してもよい)
}
```
ゲーム中の停止 (ヒッチ) まで避けたいなら、`LoadSynchronous` の代わりに装備した時点で `RequestAsyncLoad` で前もってロードしておき、返された `TSharedPtr<FStreamableHandle>` をメンバーに保持します。ただし、この方法は **時間に頼る** 解決策です。装備してすぐ撃つとロードがまだ終わっておらず 1 発目が依然として遅れることがあるので、ロード完了前に撃ったら同期ロードに切り替える予備の経路 (上のコード) も一緒に置きます。このラボのテストは装備直後に撃つので、予備の経路がないと FAIL になることがあります。いっそ武器の `UPROPERTY` のハード参照に変えれば、武器と一緒にロードされます。
**教訓:** 「1 回目だけ遅い」なら遅延ロードを真っ先に疑います。ソフト参照とは **ロードのタイミングと寿命を自分が責任を持つ** という意味です。撃つときに同期ロードすれば遅れはしませんがそのフレームが止まり (ヒッチ)、非同期ロードすれば止まりませんが遅れます。正解は「使う前に前もって、そして保持しておく」です。実際のプロジェクトでは Unreal Insights の Asset Loading トラックで「発射の瞬間にロードがあったか」を確認します。

## ラボ 30 — アクターにアタッチした音がアクターと一緒に消える
```
間違った結果:     0.1 秒後に爆発音が再生中ではない (爆発音の長さ 0.28 秒)
止まった位置:     Explode の Destroy() の行 → ウォッチ式 Audio->GetOwner() == BH_ExplosiveBarrel_0 (ログのパスも ...BH_ExplosiveBarrel_0.AudioComponent_0)
本当の原因:       SpawnSoundAttached はアタッチ先のコンポーネントのアクターの中にオーディオコンポーネントを作る。すぐに Destroy() するとそのコンポーネントも破棄され、音が止まる。
```
**修正:**
```cpp
UAudioComponent* Audio = UGameplayStatics::SpawnSoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());
Destroy();
```
⚠️ 第 1 引数に `this` を渡すと **やはり途切れます。** UE 5.8 のエンジンソースでは、`SpawnSoundAtLocation` が `GetActorOwnerFromWorldContextObject` で WorldContextObject のアクターを探して `FCreateComponentParams(World, Actor)` に渡し、そのアクターがコンポーネントの持ち主 (Outer) になるからです。以前のエンジンバージョン (5.0〜5.4) は動作が異なる可能性があるので、ログのコンポーネントのパスで確認します (ログのパスが `...BH_ExplosiveBarrel_0.AudioComponent_0`)。`GetWorld()` を渡すと持ち主が `WorldSettings` になり (`...WorldSettings_1.AudioComponent_0`)、樽が消えても最後まで再生されます。実際にこのミスでもう一度 FAIL を見て見つけた落とし穴です。
(一緒に動く必要がある音なら: アクターをすぐに消さず、非表示にしてコリジョンを切ってから `SetLifeSpan(音の長さ)`。)
**教訓:** コンポーネントの寿命はアクターの寿命です。「消えるときに出す音・エフェクト」はアクターにアタッチしません。同じ原理で `SpawnEmitterAttached`/`SpawnSystemAttached` したパーティクルも途切れます。参考: エンジンは 1 秒以下の短い音を聞こえない位置で再生する場合、コンポーネントをそもそも作りません (`au.SoundDistanceOptimizationLength`)。このラボは結果がカメラの位置によって揺れないよう、その最適化を切ってテストします。

## ラボ 31 — サーバーでだけ再生した音
```
間違った結果:     [リッスンサーバー] 3 回 PASS、[クライアント] 0 回 FAIL
止まった位置:     PlayRingSound のブレークポイント → 常に GetNetMode() == NM_ListenServer。クライアントでは一度も止まらない
本当の原因:       PlaySoundAtLocation は呼んだマシンのスピーカーでしか鳴らない。音 (やパーティクルなどの演出) はレプリケートされない。
```
**修正:**
```cpp
// Lab31_AlarmBell.h
UFUNCTION(NetMulticast, Unreliable)
void Multicast_PlayRing();

// Lab31_AlarmBell.cpp
void ABH_AlarmBell::Ring()
{
    check(HasAuthority());
    Multicast_PlayRing();           // サーバー自身 + すべてのクライアントで実行
}

void ABH_AlarmBell::Multicast_PlayRing_Implementation()
{
    PlayRingSound();
}
```
**教訓:** ラボ 38 と同じ根っこです。状態 (HP) はレプリケートされる変数で、**一回限りの演出** (音、エフェクト) は NetMulticast で送ります。演出は 1 つくらい抜けてもゲームが壊れないので、普通は `Unreliable` です。サーバーが専用サーバー (デディケイテッド) なら、サーバーの `PlaySound` は誰にも聞こえません。逆に「後から接続した人にも聞こえるべき」持続音 (警報が鳴り続けている最中) なら、Multicast ではなくレプリケートされる変数 (`bAlarmActive` + RepNotify) で状態を送る必要があります。

# 第7部: 画像 / レンダリング

## ラボ 32 — 作った MID をメッシュに設定していない
```
間違った結果:     左の木箱 (0.90, 0.90, 0.90) FAIL、マテリアル名が MID_... ではなく BasicShapeMaterial
止まった位置:     SetHighlighted の SetVectorParameterValue の行 → ウォッチ式 MID と Mesh->GetMaterial(0) のアドレスが違う
本当の原因:       UMaterialInstanceDynamic::Create は新しいインスタンスを作るだけ。メッシュは共有マテリアルを使い続けるので、誰も使っていない MID の色だけが変わる。
```
**修正:**
```cpp
void ABH_Crate::SetHighlighted(bool bOn)
{
    UMaterialInstanceDynamic* MID = Mesh->CreateDynamicMaterialInstance(0);   // 作って、設定して、すでに MID なら再利用
    MID->SetVectorParameterValue(ColorParam, bOn ? FLinearColor::Red : FLinearColor::White);
}
```
`Create` を使いたいなら `Mesh->SetMaterial(0, MID)` を付け加えれば済みます。ただしそうすると呼ぶたびに新しい MID ができるので、最初の 1 回だけ作ってメンバーに持っておく必要があります。
**教訓:** 「値は変えたのに画面がそのまま」なら、まず **変えたオブジェクトと描画しているオブジェクトが同じか** を見ます。だからといって共有マテリアルのアセットを直接書き換えると、同じマテリアルを使う木箱が全部変わります (右の木箱のテストがそれを捕まえます)。木箱ごとに違う色 = 木箱ごとに MID。

## ラボ 33 — 遅れて届いた非同期ロードのコールバックが再利用された枠を上書きする
```
間違った結果:     枠の名前「回復ポーション」PASS、アイコン AICON-Red FAIL
記録表:           フレーム 0 枠 ← ルビーの指輪 (アイコンなし → 非同期リクエスト)
                   フレーム 1 枠 ← 回復ポーション (アイコンあり → すぐに設定)
                   フレーム 2 ルビーの指輪のアイコンのロード完了 → 枠に設定   ← 犯人
止まった位置:     ラムダ内の SetBrushFromTexture のブレークポイント → Item.Name「ルビーの指輪」、this->ShownName「回復ポーション」
本当の原因:       コールバックは「リクエストしたときのアイテム」を覚えたまま、枠がすでに別のアイテムに変わった後で実行される。
```
**修正 (いずれか 1 つ):**
```cpp
// 1) コールバックでまだ同じアイテムか確認
[this, Item]
{
    if (ShownName != Item.Name)
    {
        return;     // すでに別のアイテムを表示している枠
    }
    IconImage->SetBrushFromTexture(Item.Icon.Get());
}

// 2) 新しいアイテムを設定するときに前のリクエストをキャンセル (ヘッダーに TSharedPtr<FStreamableHandle> PendingLoad;)
if (PendingLoad.IsValid()) { PendingLoad->CancelHandle(); PendingLoad.Reset(); }
...
PendingLoad = UAssetManager::GetStreamableManager().RequestAsyncLoad(...);
```
(2) を使う場合も、**すぐに設定する経路 (ロード済みのアイコン) で前のリクエストをキャンセル** する必要があります。このラボで上書きする側は、まさにその経路の後に届きます。実際のプロジェクトでは名前よりもアイテム ID やリクエスト番号で比較します。
**教訓:** 非同期コールバックは「リクエストしたときの世界」を覚えていて、「完了したときの世界」で実行されます。リストビュー・タイルビューのように **ウィジェットを再利用する** UI で最もよく起こります。この種のバグは順序が鍵なので、ログにフレーム番号を出して記録表を作ります。

## ラボ 34 — sRGB の色を Linear 変換なしで /255
```
間違った結果:     #FF8000 → 画面 #FFBC00 (G 128 → 188 FAIL)、#008080 → #00BCBC (G・B FAIL)。0x00 と 0xFF は PASS
止まった位置:     ToMaterialColor のブレークポイント → ウォッチ式 Color.G / 255.f = 0.502、FLinearColor(Color).G = 0.216
本当の原因:       FColor (デザインツールの値) は sRGB、マテリアルパラメータは Linear。/255 は変換ではなく再解釈なので、画面に出るときに sRGB でもう一度明るくなる。
```
**修正:**
```cpp
FLinearColor ToMaterialColor(const FColor& Color)
{
    return FLinearColor(Color);     // sRGB → Linear のガンマ変換 (= FLinearColor::FromSRGBColor)
}
```
**教訓:** 0 と 1 (0x00、0xFF) はガンマ変換してもそのままなので、純粋な赤・黒だけでテストするとこのバグは捕まりません。中間値 (0x80) を必ず入れます。「色あせて明るい」= Linear の場所に sRGB の値、「暗すぎて濃い」= その逆です。テクスチャなら同じ問題が sRGB チェックボックスとして現れます (色のテクスチャはオン、ノーマル・マスク・ラフネスはオフ)。エンジンの関数名: `FLinearColor(FColor)`/`FromSRGBColor` は変換、`FColor::ReinterpretAsLinear` は変換なし、`ToFColor(true)`/`ToFColorSRGB` は Linear → sRGB。

# 第8部: アニメーション

## ラボ 35 — 描画されないとアニメーションが進まない
```
間違った結果:     足音 0 回 FAIL (期待 4)。再生時間が過ぎたのに「モンタージュはまだ再生中: はい、位置 0.00 秒」
止まった位置:     OnAnimEvent のブレークポイントで一度も止まらない → ウォッチ式 Walker->Mesh->ShouldTickPose() = false、bRecentlyRendered = false
本当の原因:       コンストラクタの VisibilityBasedAnimTickOption = OnlyTickPoseWhenRendered。直近 1 秒以内に描画されていないメッシュは TickPose をスキップする。
                   ポーズが進まないのでモンタージュの位置が 0 で止まり、通知も終了イベントも来ない。
```
**修正:**
```cpp
// 見えないときもモンタージュ (とモンタージュの通知) は進め、それ以外のアニムグラフは節約する。
Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickMontagesWhenNotRendered;
```
`AlwaysTickPose` でも通ります (最適化は弱くなります)。足音がアニムグラフのシーケンス (ロコモーション) から出ているなら、`OnlyTickMontagesWhenNotRendered` では足りず `AlwaysTickPose` 以上が必要です。
**教訓:** 「画面外でだけ」「サーバーでだけ」「ヘッドレスでだけ」起きないアニメーションイベントは、まずこのオプションを見ます。専用サーバーは何も描画しないので、サーバーの攻撃判定の通知がまるごと消えます。スケルタルメッシュコンポーネントのデフォルト値は `AlwaysTickPoseAndRefreshBones`、`ACharacter` がコンストラクタでメッシュに設定する値は `AlwaysTickPose` (UE 5.8 `Character.cpp`) ですが、最適化の途中で変えた値が原因であることがよくあります。逆に、どうしても必要な判定は通知の代わりにサーバーのタイマーやゲームコードで回すのも一つの方法です。

## ラボ 36 — interrupted で終わった前のモンタージュのイベントを「完了」として処理する
```
記録表:           フレーム 0    攻撃 → 振り始め
                   (0.3 秒)     攻撃 → 振り始め          ← 同じモンタージュを再び再生、前のインスタンスはブレンドアウト
                   (0.55 秒頃)  振り終わり → コンボ +1   ← bInterrupted == true、前のインスタンスが消えた
                   (2.1 秒頃)   振り終わり → コンボ +1   ← bInterrupted == false、本当の終わり
間違った結果:     2 回目の振りの途中で bAttacking == false FAIL、コンボ 2 FAIL (期待 1)
止まった位置:     HandleMontageEnded で最初に止まったとき → ローカル ウィンドウ (Locals) で bInterrupted = true
本当の原因:       OnMontageEnded はモンタージュのインスタンスが消えるたびに呼ばれる。理由 (最後まで再生 / 押し出された / Stop) を無視した。
```
**修正:**
```cpp
void ABH_Swordsman::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage != SwingMontage || bInterrupted)
    {
        return;     // 押し出された前の振り。今振っている最中の新しいインスタンスが状態の責任を持つ。
    }
    bAttacking = false;
    ++CompletedSwings;
}
```
注意: これだけだと、**別のモンタージュ (回避、被弾) に押し出された** 場合にも `bAttacking` が true のまま残ります。実際のゲームなら、interrupted のときはコンボを上げず、`bAttacking` は「今もこのモンタージュが再生中か」(`!GetAnim()->Montage_IsPlaying(SwingMontage)`) で決めます。または `Montage_Play` の直後に `Montage_SetEndDelegate` で **今回のインスタンス専用** のデリゲートを設定します。
**教訓:** モンタージュの終了イベントは「終わった」ではなく「消えた」です。連打・キャンセルのある攻撃では、前のインスタンスの終了イベントが **新しいインスタンスの再生中に** 届きます。ブレンドアウトの時間分だけ遅れて来るので、テストで見逃しやすいです。

## ラボ 37 — オンとオフを 2 つの通知に分けて任せる
```
記録表:           0.55 秒 HitOn → ヒットボックスをオン
                   0.82 秒 被弾 → のけぞり、Montage_Stop(0.1)
                   (1.10 秒の HitOff は、モンタージュがその前に止まったので永遠に来ない)
間違った結果:     のけぞりの 1 秒後に bHitboxOn == true FAIL
止まった位置:     OnAnimEvent のブレークポイントがのけぞりの後は止まらない。モンタージュ再生中: いいえ
本当の原因:       通知はモンタージュがその時刻を通過するときにだけ来る。終わりを知らせる通知は、モンタージュが途切れると来ない。
```
**修正 (検証済み): 区間を通知ステート 1 つに**
```cpp
// Lab37_HitWindow.h
virtual void OnAnimStateBegin(FName EventName) override;
virtual void OnAnimStateEnd(FName EventName) override;

// Lab37_HitWindow.cpp — BeginPlay
BugHunt::AddNotify(SlashMontage, Length * 0.3f, TEXT("HitWindow"), Length * 0.3f);   // [30%, 60%] のステート

void ABH_Knight::OnAnimStateBegin(FName EventName) { bHitboxOn = true; }
void ABH_Knight::OnAnimStateEnd(FName EventName)   { bHitboxOn = false; }
```
検証ログ: のけぞりのフレームのすぐ次のフレームに「HitWindow 終了 → オフ」。通知ステートは、始まったものが消えると (途切れ、停止、別のモンタージュ) エンジンが `NotifyEnd` を呼んでくれます。ヘッダーを変えたので、ライブコーディング (Live Coding) ではなくエディタを閉じてビルドする必要があります。
別の方法: `OnMontageBlendingOut`/`OnMontageEnded` でヒットボックスをオフにする安全装置 (このラボでは検証していません)。
**教訓:** 「オンにして → オフにする」のように区間にまたがる状態を別々の時刻の 2 つのイベントに任せると、途中で途切れた瞬間にオフにする側が消えます。死亡、被弾、回避、ラグでフレームが大きく飛んだときに再現します。区間 = 通知ステート、そして状態をオンにするコードには常に「途切れたときにオフにする道」も一緒に作ります。

# 第9部: マルチプレイ

## ラボ 38 — C++ の RepNotify はサーバーでは呼ばれない
```
間違った結果:     [リッスンサーバー] 画面の HP バー 100 (期待 55)。[クライアント] は PASS。スタンドアロンも FAIL。
止まった位置:     OnRep_Health のブレークポイント → GetNetMode() が常に NM_Client。サーバーでは一度も止まらない。
本当の原因:       ReplicatedUsing の関数は「レプリケーションで新しい値を受け取ったマシン」でだけ呼ばれる。値を直接変えたサーバー (リッスンサーバーのホスト、スタンドアロン) は受け取る側ではない。
```
**修正:**
```cpp
void ABH_NetHero::ApplyDamage(int32 Damage)
{
    check(HasAuthority());
    Health = FMath::Max(0, Health - Damage);
    OnRep_Health();     // サーバー (ホスト) にも画面がある。直接呼んであげる。
}
```
**教訓:** リッスンサーバーのホストはサーバーであると同時にプレイヤーです。C++ では、サーバーが値を変えた場所で OnRep を直接呼ぶパターンが一般的です。(ブループリントの RepNotify はサーバーでも自動的に呼ばれるので、BP から C++ に移すときによく起きるバグです。) プレイインエディタ (PIE) をリッスンサーバー + クライアントで実行し、`GetNetMode()` で誰が止まったかを確認する習慣をつけましょう。

## ラボ 39 — クライアントでサーバーの状態を直接変える
```
間違った結果:     [サーバー基準] リモートプレイヤーの宝箱が開いた FAIL、ゴールド 0。リモートクライアントのログには「自分の画面のゴールド: 100」
止まった位置:     ABH_Chest::Open のブレークポイント → リモートプレイヤーの分は GetNetMode() == NM_Client、HasAuthority() == false
最初にずれた所:   Client_PressInteract → InteractWithChest → Chest->Open がクライアントのマシンで実行された
本当の原因:       レプリケーションはサーバー → クライアントの一方向。クライアントが自分のコピーの bOpened、Gold を変えてもサーバーには行かない。
                  (しかもサーバーの値が変わっていないのでサーバーが上書きもせず、クライアントの画面にはずっと 100 ゴールドと表示される。)
```
**修正:**
```cpp
// Lab39_Chest.h、ABH_PlayerController に追加
UFUNCTION(Server, Reliable)
void Server_OpenChest(ABH_Chest* Chest);

// Lab39_Chest.cpp
void ABH_PlayerController::InteractWithChest(ABH_Chest* Chest)
{
    if (!Chest)
    {
        return;
    }
    if (HasAuthority())
    {
        Chest->Open(this);          // ホスト: すでにサーバーである
    }
    else
    {
        Server_OpenChest(Chest);    // リモート: サーバーに頼む
    }
}

void ABH_PlayerController::Server_OpenChest_Implementation(ABH_Chest* Chest)
{
    // サーバーで実行される。実際のゲームなら、距離やすでに開いているかなどをここで検証する。
    if (Chest)
    {
        Chest->Open(this);
    }
}
```
(`HasAuthority()` の分岐なしで常に `Server_OpenChest` を呼んでも構いません。サーバーで呼べばその場で実行されます。)
**教訓:** 「状態を変えるコードはサーバーでだけ」が原則です。状態を変える関数に `check(HasAuthority())` や `if (!HasAuthority()) return;` を置いておけば、この種のバグはすぐに表に出ます。Server RPC は、そのクライアントが **所有するアクター** (PlayerController、操作している Pawn、PlayerState) からしかサーバーに届きません。宝箱のアクターに Server RPC を作ると、クライアントが所有していないので黙って捨てられます (出力ログ (Output Log) に警告)。UFUNCTION の追加は Live Coding ではできないので、エディタを閉じてビルドします。

---

# 第10部: 並行処理とフリーズ (データ競合、無限ループ、デッドロック)

## ラボ 40 — 別のアクターの Tick が変えてくれる値をゲームスレッドで while で待つ
```
間違った結果:     応答なし (2000 ms) TIMEOUT。乗るときにエレベーターが来ていた FAIL、ゲームが止まった時間 2000 ms FAIL
止まった位置:     HangBudgetSeconds 30 → すべて中断 (Break All) → スレッド ウィンドウ (Threads) で GameThread → RideElevator の while
                  ウォッチ式 Elevator->Height = 0、TicksWhileMoving = 0。F5 → すべて中断を繰り返しても同じまま
本当の原因:       Tick はゲームスレッドが次のフレームで呼ぶ。ゲームスレッドが while で待っている間、次のフレームは来ない。
```
**修正:**
```cpp
void ABH_Passenger::RideElevator(ABH_Elevator* Elevator, float TargetHeight, float Speed)
{
    Elevator->Call(TargetHeight, Speed);
    Elevator->OnArrived.AddUObject(this, &ABH_Passenger::Board, Elevator);
}
```
タイマーで 0.05 秒ごとに `bArrived` を確認しても構いません。すでに到着している可能性があるなら `if (Elevator->bArrived) Board(Elevator); else AddUObject(...)`。
**教訓:** ゲームスレッドのコードでは「待つ」コードを書きません。待つべきことは、潜在アクション (`FPendingLatentAction`、ブループリントのディレイ (Delay) ノード)、タイマー、デリゲート、Tick、`UE::Tasks` で「後で呼んでほしい」に変えます。`Sleep` を入れても CPU 使用率が減るだけで、固まるのは同じです。

## ラボ 41 — 幅優先探索の訪問記録がループ 1 周分しか生きていない
```
間違った結果:     城門 → 港 PASS、城門 → 灯台 応答なし TIMEOUT (226,000 周以上回った)
止まった位置:     HangBudgetSeconds 30 → すべて中断 → FindPath。ウォッチ式 Frontier.Num() が数十万、Visited.Num() は 1〜4
本当の原因:       TSet<int32> Visited を while の中で作った。毎周新しく空になるので、すでに入れた地点を何度も入れ直す。
                  行ける場所は偶然たどり着いて止まるが、行けない場所はキューが際限なく膨らむ。
```
**修正:**
```cpp
TArray<int32> Frontier = { From };
TMap<int32, int32> CameFrom = { { From, INDEX_NONE } };
TSet<int32> Visited = { From };          // 探索全体で 1 つ
while (Frontier.Num() > 0)
{
    ...
    for (int32 Next : Nodes[Current].Links)
    {
        if (!Visited.Contains(Next)) { Visited.Add(Next); CameFrom.FindOrAdd(Next, Current); Frontier.Add(Next); }
    }
}
```
`CameFrom.Contains(Next)` で調べても構いません (CameFrom がそのまま訪問リストです)。
**教訓:** グラフ探索が固まるのは、ほぼ常に「行けない目的地 + 訪問チェックのミス」です。テストには **到達できない目的地** を必ず入れます。ローカル変数がどの中括弧の中で作られるか (寿命) も一緒に見ます。実際のゲームでは探索ノード数に上限を設け (Unreal のナビメッシュの `MaxSearchNodes` のように)、超えたら「経路なし」で終えるのが普通です。

## ラボ 42 — ロックなしの読み取り・計算・書き込み (lost update)
```
間違った結果:     金庫のゴールド 期待 1000、実際 500 (公開入力: 100+200+300+400。どの 2 つが重なるかによって 500〜700)
止まった位置:     Gold = Before + Amount の行にブレークポイント → 並列スタック (Parallel Stacks): 2 つのスレッドが同じ行に止まっていて、どちらも Before = 0
本当の原因:       Gold += Amount は読み取り、加算、書き込みの 3 段階。2 つのスレッドが同じ値を読んでそれぞれ書いたので、片方の入金が消えた。
                   (RaceWindow はこの隙間を広げて毎回重なるようにしているだけ。)
```
**修正:**
```cpp
struct FBH_GuildVault
{
    int32 Gold = 0;
    FCriticalSection Lock;
    void Deposit(int32 Amount)
    {
        FScopeLock ScopeLock(&Lock);
        const int32 Before = Gold;
        BugHunt::RaceWindow();
        Gold = Before + Amount;
    }
};
```
数値 1 つなら `std::atomic<int32> Gold; Gold.fetch_add(Amount);` でも構いません。
**教訓:** 複数のスレッドが同じデータを書き換えるなら、読み取りから書き込みまでをひとかたまりとして保護します。いちばん良いのは共有しないことです: タスクごとに自分の枠 (`Results[Index]`) に書き、終わった後に 1 つのスレッドでまとめます。競合は「たまに」起きるので、テストが通ったことは証拠になりません。疑わしいときはわざと隙間を広げて (`Sleep`、このラボの RaceWindow) 再現します。

## ラボ 43 — ワーカースレッドでゲームスレッド専用のオブジェクトに触れる
```
間違った結果:     Assertion failed: IsInGameThread()  (CRASH)
止まった位置:     呼び出し履歴 (Call Stack) のいちばん上は ABH_ScoreFeed::AddScore、いちばん下は Async が立ち上げたスレッドの実行関数。スレッド ウィンドウの現在のスレッドは GameThread ではない
本当の原因:       Async(EAsyncExecution::Thread, ...) のラムダの中でアクターの AddScore を呼んだ。アクターの TMap はゲームスレッドがロックなしで使うデータ。
```
**修正:**
```cpp
Async(EAsyncExecution::Thread, [WeakFeed, Points]
{
    const FString Mvp = TEXT("Aria");                     // 重い計算だけをここで
    AsyncTask(ENamedThreads::GameThread, [WeakFeed, Mvp, Points]
    {
        if (ABH_ScoreFeed* Target = WeakFeed.Get()) { Target->AddScore(Mvp, Points); }
    });
});
```
**教訓:** UObject/アクター/コンポーネント/ワールドはゲームスレッドでだけ。ワーカースレッドには計算に必要な値だけをコピーして渡し、結果は `AsyncTask(GameThread)` (または `UE::Tasks` の後続タスク、`FTSTicker`) で戻します。`TWeakObjectPtr::Get()` もゲームスレッドで呼びます。自分で作ったゲームスレッド専用の関数にも `check(IsInGameThread())` を入れておけば、この種のミスがその場で捕まります。

## ラボ 44 — ゲームスレッドがワーカースレッドの結果を同期で待ち、ワーカースレッドはゲームスレッドを待つ
```
間違った結果:     応答なし (2000 ms 待機、デッドロックの疑い) TIMEOUT。ゲームが止まった時間 2000 ms、保存完了 FAIL
止まった位置:     HangBudgetSeconds 30 → すべて中断 → 並列スタック:
                     GameThread:         ABH_SaveSystem::AutoSave → WaitWithBudget → TFuture::WaitFor
                     ワーカースレッド:   AutoSave のラムダ → Snapshot.Get() → TFuture::Get
本当の原因:       ワーカースレッドが AsyncTask(GameThread) で送った仕事は、ゲームスレッドが次のフレームで処理する。
                   そのゲームスレッドはワーカースレッドが終わるのを待っている。お互いを待っている (デッドロック)。
```
**修正:**
```cpp
void ABH_SaveSystem::AutoSave(ABH_SaveHero* Hero)
{
    const float Seconds = WriteSeconds;
    const FString Text = Describe(Hero);                 // ゲームスレッドで今の状態をコピー
    TWeakObjectPtr<ABH_SaveSystem> WeakThis(this);
    Async(EAsyncExecution::ThreadPool, [Text, Seconds, WeakThis]
    {
        FPlatformProcess::Sleep(Seconds);                 // ディスクへの書き込み
        AsyncTask(ENamedThreads::GameThread, [Text, WeakThis]
        {
            if (ABH_SaveSystem* Self = WeakThis.Get())
            {
                Self->OnSaved(Text);
            }
        });
    });
}
```
スナップショットだけ先に取って `Done.Get()` を残すと、デッドロックは解けますが「ゲームが止まった時間 < 50 ms」が FAIL です (ディスク書き込みの 0.3 秒分のヒッチ)。ワーカースレッドで `Hero->Gold` を直接読むとデータ競合になり、保存直後の +50 が混ざって「保存内容」が FAIL になることがあります。
**教訓:** ゲームスレッドで `Future.Get()`、`Event->Wait()`、`FScopeLock` で長く待ちません。特に、待つ相手がゲームスレッドに仕事を送る構造 (ロード、セーブ、アセットストリーミングのコールバック) なら、そのままデッドロックです。2 つのロックを互いに逆の順序で取るデッドロック (A→B / B→A) も、並列スタックでは同じ形に見えます: 2 つのスレッドがお互いに相手の握っているものを待っています。

# 第11部: パフォーマンス

## ラボ 45 — Tick のたびにワールド全体を検索
```
間違った結果:     1 秒間にアクター全体の検索 12,000 回 (アイコン 200 個 × 60 フレーム)
止まった位置:     stat game で Tick の時間が大きい。Insights の長いフレームの中に ABH_MapMarker::Tick → GetAllActorsOfClass が 200 回
本当の原因:       変わらない対象 (プレイヤー) を毎フレーム、アイコンごとに探し直した。アクター N 個 × アイコン M 個 × フレーム。
```
**修正:**
```cpp
TWeakObjectPtr<ABH_MapHero> Hero;                         // ヘッダー
void ABH_MapMarker::BeginPlay() { Super::BeginPlay(); Hero = FindHero(GetWorld()); }
void ABH_MapMarker::Tick(float DeltaSeconds) { ...; if (const ABH_MapHero* Found = Hero.Get()) { ... } }
```
**教訓:** Tick の中の `GetAllActorsOfClass`、`FindComponentByClass`、`TActorIterator` は、ほぼ常に BeginPlay に移せます。対象が変わりうるなら弱いポインタで覚えておき、変わる瞬間 (スポーン、破棄) に通知を受け取ります。

## ラボ 46 — 毎フレーム、ローカルの TArray を伸ばしながら埋める
```
間違った結果:     60 フレームの間に配列のメモリ確保 240 回 (1 フレームあたり 4 回、点 64 個のとき)
止まった位置:     Points.Add に条件付きブレークポイント Points.Num() == Points.Max(): 1 フレームに何度も、Max が段階的に増える
本当の原因:       個数 (NumPoints) がわかっているのに教えなかったので、TArray が足りなくなるたびにより大きなメモリを確保して移した。
                   ローカル配列なので Tick が終わると返却し、次の Tick で最初からやり直し。
```
**修正:**
```cpp
Points.Reserve(NumPoints);       // 1 フレームあたり 1 回
// さらに減らすなら: メンバー配列 + Points.Reset() → 最初の 1 回の後は 0 回
```
**教訓:** 個数がわかるなら `Reserve`。毎フレーム使う配列はメンバーにして `Reset` (メモリは保持)、小さくて個数が決まっているなら `TInlineAllocator`。文字列も同じです: 毎フレーム `FString::Printf` で UI の文字列を作ると、フレームごとに確保が起きます。値が変わったときだけ作ります。

## ラボ 47 — 何もすることのない Tick が数百個
```
間違った結果:     2 秒間にたいまつの Tick 36,900 回 (300 個 × 約 123 フレーム)、実際に揺らめいたのは 1,200 回
止まった位置:     dumpticks に BH_Torch のティック関数が 300 個。stat game の Tick Time がたいまつの数に比例
本当の原因:       0.5 秒ごとの仕事を、毎フレームの Tick で時間を足しながら待っていた。
```
**修正:**
```cpp
PrimaryActorTick.TickInterval = 0.5f;          // 方法 1: その周期でだけ Tick
// 方法 2: bCanEverTick = false + BeginPlay で SetTimer(FlickerHandle, this, &ABH_Torch::Flicker, 0.5f, true)
```
**教訓:** アクターの Tick はデフォルトでオフにしておき、必要なときだけオンにします (`SetActorTickEnabled`)。周期的な仕事はタイマーか `TickInterval`。数千個ならマネージャー 1 つが配列を回し、遠くにあるものは Significance Manager で頻度を下げます。

## ラボ 48 — ゲーム中の同期ロード
```
間違った結果:     扉を開けた瞬間に同期ロード 1 回 (/Engine/BasicShapes/Cone)、そのフレームが数 ms〜数百 ms 止まる
止まった位置:     -trace=cpu,loadtime → Insights の Timing: そのフレームに LoadPackage。Asset Loading Insights に Cone
本当の原因:       ソフト参照を SpawnBoss の中で LoadSynchronous した。メモリになければ、ディスクから全部読み終わるまでゲームスレッドが止まる。
```
**修正:**
```cpp
TSharedPtr<FStreamableHandle> LoadHandle;                  // ヘッダー
void ABH_BossArena::BeginPlay()
{
    Super::BeginPlay();
    LoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(BossMesh.ToSoftObjectPath());
}
UStaticMesh* Mesh = BossMesh.Get();                         // SpawnBoss: すでに届いている
```
**教訓:** ソフト参照とは「いつロードするかを自分で決める」という意味です。使う直前ではなく前もって (部屋に入るとき、ロード画面で) 非同期でロードし、ハンドルを持っておいて GC に片付けられないようにします。`LoadObject`/`LoadSynchronous`/`StaticLoadObject` をゲームプレイのコードで見かけたら疑います。ラボ 29 の発射音は逆に「非同期で遅すぎた」ケースです。

