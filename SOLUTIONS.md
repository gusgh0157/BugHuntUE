# 해답 (스포일러!)

# 1부: 크래시 추적

## Lab 1 — Cast 실패를 검사하지 않음
```
멈춘 함수:        ABH_Hero::Heal (this == nullptr)
이상한 값:        GiveHealth 의 Hero == nullptr
그 값이 온 프레임: OnTouched: Toucher == BH_Pet_0 → Cast<ABH_Hero>(Toucher) == nullptr
거쳐 온 엔진 기능: 겹침 이벤트 (UPrimitiveComponent::UpdateOverlaps → BeginComponentOverlap → OnComponentBeginOverlap 델리게이트 → execHandleOverlap)
진짜 원인:        아이템을 건드린 액터가 영웅이라고 가정했다. 펫(StartLab 의 두 번째 SetActorLocation)이 건드리자 Cast 가 실패했다.
```
**수정:**
```cpp
void ABH_HealthPickup::OnTouched(AActor* Toucher)
{
    ABH_Hero* Hero = Cast<ABH_Hero>(Toucher);
    if (!Hero)
    {
        if (const ABH_Pet* Pet = Cast<ABH_Pet>(Toucher))
        {
            Hero = Pet->Master;     // 기획: 펫이 주운 아이템은 주인에게
        }
    }
    if (!Hero)
    {
        return;                     // 영웅도 펫도 아니면 무시 (아이템은 남겨 둔다)
    }
    GiveHealth(Hero);
}
```
**교훈:** `Cast<T>`는 실패하면 nullptr를 돌려줄 뿐 알려 주지 않는다. "반드시 T여야 한다"면 `CastChecked<T>`로 그 자리에서 죽게 하고, 아니면 결과를 검사한다. 겹침 이벤트에는 **생각보다 많은 종류의 액터**가 들어온다.

## Lab 2 — 다른 액터가 쓸 멤버를 BeginPlay 뒤쪽에서 만듦
```
멈춘 함수:        [엔진 템플릿] TMulticastDelegate::AddUObject ← ABH_ScoreHud::BeginPlay
이상한 값:        Manager->Scoreboard == nullptr
그 값이 온 프레임: ABH_ScoreManager::BeginPlay — 초록 화살표가 SpawnActor<ABH_ScoreHud> 줄. Scoreboard 는 그 다음 줄에서 만든다.
거쳐 온 엔진 기능: UWorld::SpawnActor → AActor::FinishSpawning → PostActorConstruction → DispatchBeginPlay → BeginPlay
진짜 원인:        이미 시작된 월드에서 SpawnActor 하면 새 액터의 BeginPlay 가 SpawnActor 안에서 즉시 실행된다.
                 HUD 의 BeginPlay 는 매니저의 BeginPlay 가 끝나기 전에 돌았다.
```
**수정 (택1):**
- `Scoreboard = NewObject<...>` 를 `SpawnActor` 위로 옮긴다.
- 더 좋게는 남이 쓸 멤버를 **생성자**에서 `CreateDefaultSubobject<UBH_Scoreboard>(TEXT("Scoreboard"))`로 만들거나 `PostInitializeComponents`에서 준비한다. 둘 다 어떤 BeginPlay 보다도 먼저 실행된다.
- HUD 쪽 방어: `if (UBH_Scoreboard* Board = Manager->GetScoreboard()) { ... }`

**교훈:** 언리얼 액터의 초기화 순서는 `생성자 → (속성 값 적용) → PostInitializeComponents → BeginPlay`. 같은 BeginPlay 끼리의 순서는 "스폰 순서/레벨 로드 순서"라서 믿으면 안 된다. 크래시 위치가 엔진 템플릿(`FMRSWRecursiveAccessDetector`, `AddDelegateInstance`)이어도 처음 나오는 내 코드부터 보면 된다.

## Lab 3 — 범위 기반 for 도중 Remove
```
Ensure:          "Array has changed during ranged-for iteration!" ← TCheckedPointerIterator::operator!= ← CleanupDeadEnemies
따라가 보면:      [Slime, Bat, Orc, Wolf, Golem]
                 1바퀴 Slime 제거 → [Bat, Orc, Wolf, Golem]  (Bat 이 0번 칸으로 당겨짐)
                 2바퀴는 1번 칸 = Orc  → Bat 은 검사되지 않음
                 3바퀴 Wolf 제거 → [Bat, Orc, Golem]
                 4·5바퀴는 배열 끝을 넘어선 낡은 칸(Golem 사본)
결과:            Bat(HP 0) 이 남고 제거 수 2
```
**수정:**
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
또는 "고르기"와 "지우기"를 나눈다: 먼저 죽은 적을 다른 배열에 모으고(`FilterByPredicate`), `Enemies.RemoveAll(...)`로 한 번에 뺀 뒤, 모은 배열을 돌며 `Destroy()`.
**교훈:** 콘솔판 랩 13(erase 후 이터레이터)의 언리얼 버전. UE는 Development 빌드에서 `ensure`로 알려 주지만 **세션당 한 번만** 알려 준다. `ensure`는 무시하고 넘어가면 안 되는 "죽지 않은 크래시"다.

## Lab 4 — UPROPERTY 없는 UObject 포인터 (GC 후 해제된 메모리 사용)
```
멈춘 함수:        UnknownFunction (주소가 0x00000000000fxxxx 같은 이상한 값) ← ABH_QuestGiver::AcceptQuest
이상한 값:        QuestLog 주소는 그대로인데 내용(이름, Entries)이 쓰레기. GC 전에 적어 둔 값과 다르다.
진짜 원인:        QuestLog 멤버에 UPROPERTY 가 없어 GC 가 이 참조를 모른다. 아무도 참조하지 않는 객체로 보고 수거했다.
                 Outer(NewObject 의 첫 인자 this)는 객체를 살려 두지 않는다.
                 수거된 메모리에서 가상 함수 AddEntry 를 부르자 망가진 vtable 을 따라 엉뚱한 주소로 점프했다.
```
**수정:** 헤더에서
```cpp
UPROPERTY()
TObjectPtr<UBH_QuestLog> QuestLog;
```
**교훈:**
- UObject를 가리키는 멤버에는 `UPROPERTY()`(값은 `TObjectPtr<>`)가 붙어야 GC가 "살아 있다"고 안다. UObject가 아닌 클래스(F클래스)가 UObject를 들고 있어야 하면 `FGCObject`를 상속하거나 `TStrongObjectPtr`를 쓴다.
- 들고만 있고 살려 두면 안 되는 경우(남의 객체 관찰)는 `TWeakObjectPtr` + 사용할 때 `IsValid()`/`Get()` 검사.
- 해제된 메모리 사용은 증상이 매번 다르다. 재현하려면 `gc.CollectGarbageEveryFrame 1`, 확실하게 잡으려면 `-stomp2malloc`(해제 즉시 접근 금지 페이지로 바꿔 그 줄에서 멈춘다).

---

# 2부: 흐름 추적 (크래시 없음)

## Lab 5 — 창을 닫을 때 델리게이트를 풀지 않음
```
틀린 결과:     기록 2줄, 알림 4번
멈춘 위치:     HandleDamaged 중단점 → Damage 가 10, 5, 7, 7 순서로 4번
기대 → 실제:   10, 7 → 10, 5(창이 닫혀 있는데!), 7, 7
처음 어긋난 곳: Close() 뒤의 TakeHit(5) 가 HandleDamaged 를 불렀다. OnDamaged.InvocationList 개수: Open 뒤 1 → Close 뒤 1 → 재 Open 뒤 2
진짜 원인:     Open 에서 AddUObject, Close 에서는 Remove 없음. 창 객체가 풀링되어 살아 있으니 계속 불리고, 다시 열 때 또 추가된다.
```
**수정:**
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
(또는 `FDelegateHandle Handle = ...AddUObject(...)`를 멤버로 저장해 `Remove(Handle)`.)
**교훈:** `AddUObject`/`AddDynamic`은 객체가 **파괴되면** 안전하게 무시되지만, 살아 있는 객체는 언제까지나 불린다. **Add 한 곳마다 짝이 되는 Remove**(EndPlay, Close, Deinitialize)가 있어야 한다. `AddDynamic`을 같은 함수로 두 번 하면 ensure가 뜨지만 `AddUObject`/`AddLambda`는 조용히 중복된다.

## Lab 6 — 지역 변수에 받은 타이머 핸들
```
틀린 결과:     정화 후 피해 3번, HP 50
멈춘 위치:     Output Log 의 시각 / ListTimers / Cleanse 중단점
기대 → 실제:   0.4 0.8 1.4 1.8 → 0.4 0.8 1.2 1.4 1.6 1.8 2.0 2.2 2.6 3.0 (1.0초 뒤 간격이 0.2초로 줄어듦 = 타이머 두 개)
처음 어긋난 곳: 1.2초 피해 (원래 타이머가 RefreshPoison 뒤에도 살아 있음)
진짜 원인:     RefreshPoison 이 지역 FTimerHandle NewTimer 로 새 타이머를 만들었다.
              ① 기존 PoisonTimer 는 지워지지 않았고 ② 새 타이머의 핸들은 함수가 끝나며 사라져서 Cleanse 가 지울 방법이 없다.
              Cleanse 는 원래 타이머만 지우고, 새 타이머는 영원히 돈다.
```
**수정:**
```cpp
void ABH_PoisonVictim::RefreshPoison()
{
    // 같은 핸들로 SetTimer 하면 기존 타이머를 지우고 새로 시작한다.
    GetWorldTimerManager().SetTimer(PoisonTimer, this, &ABH_PoisonVictim::PoisonTick, PoisonInterval, true);
}
```
**교훈:** 나중에 멈추거나 바꿔야 하는 타이머의 핸들은 **멤버**로 들고 있어야 한다. "타이머가 두 개 도는지"는 `ListTimers`가 바로 보여 준다. 액터가 파괴되면 그 액터의 타이머는 정리되지만, 오래 사는 액터(GameState, 플레이어)라면 끝까지 돈다.

## Lab 7 — 읽는 쪽이 쓰는 쪽보다 먼저 Tick
```
틀린 결과:     30프레임 동안 HP 바가 1씩 늦고, 쓰러진 프레임에 HP 바 1
멈춘 위치:     두 Tick 에 추적점 → 출력 순서가 매 프레임 "Bar → Hero"
처음 어긋난 곳: 첫 프레임부터. HP 바(TG_PrePhysics)가 영웅(TG_PostUpdateWork)보다 먼저 돌아 지난 프레임의 HP 를 읽음
진짜 원인:     생성자의 PrimaryActorTick.TickGroup = TG_PostUpdateWork
```
**수정 (둘 다 하는 것이 좋다):**
```cpp
// ABH_BleedingHero 생성자: TickGroup 줄을 지운다 (기본 TG_PrePhysics)
// ABH_HealthBar::Attach:
Hero = InHero;
AddTickPrerequisiteActor(InHero);     // 영웅이 Tick 한 다음에 내가 Tick
DisplayedHealth = Hero->Health;
```
**교훈:** Tick 그룹이 다르면 그룹 순서대로, **같은 그룹 안에서는 순서가 정해져 있지 않다.** 순서가 중요하면 `AddTickPrerequisiteActor`/`AddTickPrerequisiteComponent`로 명시한다. 한 프레임 늦는 버그는 화면으로는 거의 안 보이지만, 판정(사망 처리, 히트 체크)에 쓰이면 진짜 버그가 된다. 생성자 변경이므로 **에디터를 닫고 빌드**해야 반영된다.

## Lab 8 — 3D 거리로 바닥 도착 판정
```
틀린 결과:     5초 동안 웨이포인트 0개
쓴 도구:       Visual Logger — 초록 점(순찰병)이 첫 노란 원 근처에서 앞뒤로 떨고, 로그의 거리는 항상 90 이상
기대 → 실제:   가로로 겹치면 거리 < 50 → 거리 최소 90.0
진짜 원인:     액터 위치 Z = 90(몸 중심), 웨이포인트 Z = 0(바닥). 이동은 2D 인데 판정은 3D.
```
**수정:**
```cpp
const float Distance = FVector::Dist2D(GetActorLocation(), Target);
```
(또는 웨이포인트를 액터 높이로 맞추거나, 도착 판정에 높이 허용치를 따로 둔다.)
**교훈:** 위치·경로 버그는 숫자만 보면 안 보이고 **그려 보면** 바로 보인다. Visual Logger는 프레임별 기록을 녹화해 되감을 수 있어서 "매 프레임 도는 코드"의 F10 대용이다. `UE_VLOG_*`는 녹화 중이 아닐 때 비용이 거의 없으니 AI/이동 코드에 미리 넣어 두면 좋다.

## Lab 9 — Destroy 된 액터를 nullptr 비교로 검사
```
틀린 결과:     점수 700, 허수아비 2개가 서 있음
멈춘 위치:     Score += 중단점 두 번째 적중 → CurrentTarget = BH_Dummy_0, Health -10
처음 어긋난 곳: 화살 4: if (!CurrentTarget) 가 false (포인터는 그대로) → 쓰러진 BH_Dummy_0 을 다시 쏨 → Health <= 0 이라 또 "처치"
진짜 원인:     Destroy() 는 액터를 Garbage 로 표시만 한다. UPROPERTY 포인터가 nullptr 이 되는 건 다음 GC 때다.
```
**수정:**
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
**교훈:** UObject 포인터의 "살아 있음" 검사는 `IsValid()`. `if (Ptr)`은 "주소가 있다"는 뜻일 뿐이다. 이 버그는 **GC가 언제 도느냐에 따라** 가끔 사라진다(GC 뒤엔 포인터가 정리되어 nullptr). "가끔만 생기는 버그"의 전형. 같은 파일의 `FindNearestTarget`이 이미 `IsValid`를 쓰고 있었다는 점도 단서였다.

## Lab 10 — Config 파일이 생성자 기본값을 덮어씀
```
틀린 결과:     MaxHealth 100 (기대 150)
멈춘 위치:     생성자 중단점 → 150 이 들어감. 데이터 중단점 → 생성자가 끝난 뒤 다시 바뀜
              getall BH_Slime MaxHealth → 100
처음 어긋난 곳: 클래스 기본 객체(Default__BH_Slime)가 생성자를 돈 뒤 Config/DefaultGame.ini 의 [/Script/BugHuntUE.BH_Slime] MaxHealth=100 을 읽음.
              새로 스폰한 슬라임은 생성자를 돈 뒤 CDO 의 값을 물려받음.
진짜 원인:     예전 튜닝값이 ini 에 남아 있었다. 코드만 바꾼 "패치"는 ini 가 이긴다.
```
**수정:** `Config/DefaultGame.ini`에서 `MaxHealth=100` 줄을 지우거나 `MaxHealth=150`으로 바꾼다. (밸런스 값의 주인을 한 곳으로 정한다: 코드냐, ini 냐, 데이터 에셋이냐.)
**교훈:** 언리얼 객체의 값은 `생성자 → CDO 직렬화 값(Config, 블루프린트 기본값, 에셋) → 인스턴스 직렬화 값(레벨에 배치한 액터의 Details 값)` 순으로 덮인다. 생성자 값은 **가장 먼저**, 즉 가장 약하다. 블루프린트도 같다: BP에서 바꿔 저장한 값은 C++ 생성자 값을 이기고, 바꾸지 않은 값만 C++ 기본값을 따른다. "C++ 기본값을 바꿨는데 안 바뀐다"면 Config 폴더, 블루프린트 Class Defaults, 레벨에 배치된 인스턴스 값을 차례로 의심하자.

---

# 3부: 멀티플레이

## Lab 11 — C++ RepNotify 는 서버에서 불리지 않음
```
틀린 결과:     [리슨 서버] 화면의 HP 바 100 (기대 55). [클라이언트]는 PASS. 스탠드얼론도 FAIL.
멈춘 위치:     OnRep_Health 중단점 → GetNetMode() 가 항상 NM_Client. 서버에선 한 번도 안 멈춤.
진짜 원인:     ReplicatedUsing 함수는 "복제로 새 값을 받은 머신"에서만 불린다. 값을 직접 바꾼 서버(리슨 서버 호스트, 스탠드얼론)는 받는 쪽이 아니다.
```
**수정:**
```cpp
void ABH_NetHero::ApplyDamage(int32 Damage)
{
    check(HasAuthority());
    Health = FMath::Max(0, Health - Damage);
    OnRep_Health();     // 서버(호스트)도 화면이 있다. 직접 불러 준다.
}
```
**교훈:** 리슨 서버 호스트는 서버이면서 플레이어다. C++에서는 서버가 값을 바꾼 곳에서 OnRep을 직접 부르는 패턴이 일반적이다. (블루프린트 RepNotify는 서버에서도 자동으로 불려서, BP에서 C++로 옮길 때 자주 생기는 버그다.) PIE 리슨 서버 + 클라이언트로 돌리고 `GetNetMode()`로 누가 멈췄는지 확인하는 습관을 들이자.

## Lab 12 — 클라이언트에서 서버 상태를 직접 바꿈
```
틀린 결과:     [서버 기준] 원격 플레이어의 상자 열림 FAIL, 골드 0. 원격 클라이언트 로그엔 "내 화면의 골드: 100"
멈춘 위치:     ABH_Chest::Open 중단점 → 원격 플레이어 몫은 GetNetMode() == NM_Client, HasAuthority() == false
처음 어긋난 곳: Client_PressInteract → InteractWithChest → Chest->Open 이 클라이언트 머신에서 실행됨
진짜 원인:     복제는 서버 → 클라이언트 한 방향. 클라이언트가 자기 사본의 bOpened, Gold 를 바꿔도 서버로 가지 않는다.
              (게다가 서버 값이 바뀌지 않았으니 서버가 덮어쓰지도 않아서, 클라이언트 화면에는 계속 100 골드로 보인다.)
```
**수정:**
```cpp
// Lab12_Chest.h, ABH_PlayerController 에 추가
UFUNCTION(Server, Reliable)
void Server_OpenChest(ABH_Chest* Chest);

// Lab12_Chest.cpp
void ABH_PlayerController::InteractWithChest(ABH_Chest* Chest)
{
    if (!Chest)
    {
        return;
    }
    if (HasAuthority())
    {
        Chest->Open(this);          // 호스트: 이미 서버다
    }
    else
    {
        Server_OpenChest(Chest);    // 원격: 서버에게 부탁한다
    }
}

void ABH_PlayerController::Server_OpenChest_Implementation(ABH_Chest* Chest)
{
    // 서버에서 실행된다. 실제 게임이라면 거리, 이미 열렸는지 등을 여기서 검증한다.
    if (Chest)
    {
        Chest->Open(this);
    }
}
```
(`HasAuthority()` 분기 없이 항상 `Server_OpenChest`를 불러도 된다. 서버에서 부르면 바로 실행된다.)
**교훈:** "상태를 바꾸는 코드는 서버에서만"이 원칙이다. 상태를 바꾸는 함수에 `check(HasAuthority())`나 `if (!HasAuthority()) return;`을 두면 이런 버그가 바로 드러난다. Server RPC는 그 클라이언트가 **소유한 액터**(PlayerController, 조종하는 Pawn, PlayerState)에서만 서버에 도착한다. 상자 액터에 Server RPC를 만들면 클라이언트가 소유하지 않아서 조용히 버려진다(Output Log에 경고). UFUNCTION 추가는 Live Coding으로 안 되므로 에디터를 닫고 빌드한다.

---

# 4부: 사운드

## Lab 13 — 소프트 레퍼런스를 쏠 때 처음 불러옴
```
틀린 결과:     1번째 발사 3프레임(약 13ms) 늦음, 2번째 0, 3번째(GC 뒤) 다시 2프레임 늦음
멈춘 위치:     Fire 중단점 → FireSound.Get() == nullptr → RequestAsyncLoad 경로. 콜백은 몇 프레임 뒤 비동기 로딩 틱에서 불림
              로그의 "(발사음 메모리에 없음)" / obj list class=SoundWave 로 GC 전후 비교
처음 어긋난 곳: 1번째 Fire: 소리가 메모리에 없어서 그 자리에서 비동기 로드를 시작
진짜 원인:     ① 꼭 즉시 울려야 하는 소리를 "쏠 때" 불러왔다. 비동기 로드는 빨라도 다음 프레임 이후에 끝난다.
              ② 불러온 뒤에도 하드 레퍼런스가 없어서(소프트 포인터와 버려진 핸들뿐) GC가 다시 내렸다 → GC 뒤 첫 발이 또 늦다.
```
**수정:**
```cpp
// Lab13_FireSound.h
protected:
    virtual void BeginPlay() override;

    UPROPERTY()
    TObjectPtr<USoundBase> LoadedFireSound;     // 붙잡아 두는 하드 레퍼런스

// Lab13_FireSound.cpp
void ABH_Blaster::BeginPlay()
{
    Super::BeginPlay();
    LoadedFireSound = FireSound.LoadSynchronous();   // 무기를 들 때(로딩 화면, 장착 시점) 미리
}

void ABH_Blaster::Fire()
{
    if (LoadedFireSound)
    {
        PlayFireSound(LoadedFireSound, GFrameCounter, FPlatformTime::Seconds());
        return;
    }
    ... (기존 코드는 예비 경로로 남겨도 된다)
}
```
게임 중 멈춤(히치)까지 피하려면 `LoadSynchronous` 대신 장착 시점에 `RequestAsyncLoad`로 미리 불러 두고 반환된 `TSharedPtr<FStreamableHandle>`을 멤버로 보관한다. 아예 무기의 `UPROPERTY` 하드 레퍼런스로 바꾸면 무기와 함께 로드된다.
**교훈:** "첫 번째만 늦다"는 지연 로드를 1순위로 의심한다. 소프트 레퍼런스는 **로드 시점과 수명을 내가 책임진다**는 뜻이다. 쏠 때 동기 로드하면 늦지는 않지만 그 프레임이 멈추고(히치), 비동기 로드하면 멈추지 않지만 늦는다. 정답은 "쓰기 전에 미리, 그리고 붙잡아 두기". 실제 프로젝트에서는 Unreal Insights의 Asset Loading 트랙으로 "발사 순간에 로드가 있었나"를 확인한다.

## Lab 14 — 액터에 붙인 소리가 액터와 함께 사라짐
```
틀린 결과:     0.1초 뒤 폭발음 재생 중 아님 (폭발음 길이 0.28초)
멈춘 위치:     Explode 의 Destroy() 줄 → 조사식 Audio->GetOwner() == BH_ExplosiveBarrel_0 (로그의 경로도 ...BH_ExplosiveBarrel_0.AudioComponent_0)
진짜 원인:     SpawnSoundAttached 는 붙일 컴포넌트의 액터 안에 오디오 컴포넌트를 만든다. 바로 Destroy() 하면 그 컴포넌트도 파괴되고 소리가 멈춘다.
```
**수정:**
```cpp
UAudioComponent* Audio = UGameplayStatics::SpawnSoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());
Destroy();
```
⚠️ 첫 인자에 `this`를 넘기면 **여전히 잘린다.** UE 5.8의 `SpawnSoundAtLocation`은 WorldContextObject의 액터를 컴포넌트의 주인(Outer)으로 삼기 때문이다(로그 경로가 `...BH_ExplosiveBarrel_0.AudioComponent_0`). `GetWorld()`를 넘기면 주인이 `WorldSettings`가 되어(`...WorldSettings_1.AudioComponent_0`) 통이 사라져도 끝까지 재생된다. 실제로 이 실수로 한 번 더 FAIL을 보고 찾은 함정이다.
(따라 움직여야 하는 소리라면: 액터를 바로 지우지 말고 숨기고 충돌을 끈 뒤 `SetLifeSpan(소리 길이)`.)
**교훈:** 컴포넌트의 수명은 액터의 수명이다. "사라지면서 내는 소리·이펙트"는 액터에 붙이지 않는다. 같은 원리로 `SpawnEmitterAttached`/`SpawnSystemAttached`한 파티클도 잘린다. 참고: 엔진은 1초 이하의 짧은 소리를 들을 수 없는 위치에서 재생하면 컴포넌트를 아예 만들지 않는다(`au.SoundDistanceOptimizationLength`). 이 랩은 결과가 카메라 위치에 따라 흔들리지 않게 그 최적화를 끄고 검사한다.

## Lab 15 — 서버에서만 재생한 소리
```
틀린 결과:     [리슨 서버] 3번 PASS, [클라이언트] 0번 FAIL
멈춘 위치:     PlayRingSound 중단점 → 항상 GetNetMode() == NM_ListenServer. 클라이언트에선 한 번도 안 멈춤
진짜 원인:     PlaySoundAtLocation 은 부른 머신의 스피커에서만 울린다. 소리(와 파티클 등 연출)는 복제되지 않는다.
```
**수정:**
```cpp
// Lab15_AlarmBell.h
UFUNCTION(NetMulticast, Unreliable)
void Multicast_PlayRing();

// Lab15_AlarmBell.cpp
void ABH_AlarmBell::Ring()
{
    check(HasAuthority());
    Multicast_PlayRing();           // 서버 자신 + 모든 클라이언트에서 실행
}

void ABH_AlarmBell::Multicast_PlayRing_Implementation()
{
    PlayRingSound();
}
```
**교훈:** 랩 11과 같은 뿌리다. 상태(HP)는 복제 변수로, **일회성 연출**(소리, 이펙트)은 NetMulticast로 보낸다. 연출은 하나쯤 빠져도 게임이 망가지지 않으니 보통 `Unreliable`. 서버가 데디케이티드면 서버의 `PlaySound`는 아무도 듣지 못한다. 반대로 "늦게 접속한 사람도 들어야 하는" 지속음(경보가 계속 울리는 중)이라면 Multicast가 아니라 복제 변수(`bAlarmActive` + RepNotify)로 상태를 보내야 한다.

# 5부: 이미지 / 렌더링

## Lab 16 — 만든 MID를 메시에 끼우지 않음
```
틀린 결과:     왼쪽 상자 (0.90, 0.90, 0.90) FAIL, 머티리얼 이름이 MID_... 가 아니라 BasicShapeMaterial
멈춘 위치:     SetHighlighted 의 SetVectorParameterValue 줄 → 조사식 MID 와 Mesh->GetMaterial(0) 의 주소가 다름
진짜 원인:     UMaterialInstanceDynamic::Create 는 새 인스턴스를 만들기만 한다. 메시는 계속 공유 머티리얼을 쓰므로, 아무도 안 쓰는 MID의 색만 바뀐다.
```
**수정:**
```cpp
void ABH_Crate::SetHighlighted(bool bOn)
{
    UMaterialInstanceDynamic* MID = Mesh->CreateDynamicMaterialInstance(0);   // 만들고, 끼우고, 이미 MID면 재사용
    MID->SetVectorParameterValue(ColorParam, bOn ? FLinearColor::Red : FLinearColor::White);
}
```
`Create`를 쓰고 싶다면 `Mesh->SetMaterial(0, MID)`를 덧붙이면 된다. 다만 그러면 부를 때마다 새 MID가 생기므로 처음 한 번만 만들어 멤버에 들고 있어야 한다.
**교훈:** "값은 바꿨는데 화면이 그대로"라면 **바꾼 객체와 그리는 객체가 같은가**부터 본다. 그렇다고 공유 머티리얼 에셋을 직접 고치면 같은 머티리얼을 쓰는 상자가 전부 바뀐다(오른쪽 상자 검사가 그걸 잡는다). 상자마다 다른 색 = 상자마다 MID.

## Lab 17 — 늦게 도착한 비동기 로드 콜백이 재사용된 칸을 덮어씀
```
틀린 결과:     칸 이름 '회복 물약' PASS, 아이콘 AICON-Red FAIL
기록표:        프레임 0 칸 ← 루비 반지 (아이콘 없음 → 비동기 요청)
               프레임 1 칸 ← 회복 물약 (아이콘 있음 → 바로 넣음)
               프레임 2 루비 반지 아이콘 로드 완료 → 칸에 넣음   ← 범인
멈춘 위치:     람다 안 SetBrushFromTexture 중단점 → Item.Name '루비 반지', this->ShownName '회복 물약'
진짜 원인:     콜백은 "요청할 때의 아이템"을 기억한 채, 칸이 이미 다른 아이템으로 바뀐 뒤에 실행된다.
```
**수정 (둘 중 하나):**
```cpp
// 1) 콜백에서 아직 같은 아이템인지 확인
[this, Item]
{
    if (ShownName != Item.Name)
    {
        return;     // 이미 다른 아이템을 보여 주는 칸
    }
    IconImage->SetBrushFromTexture(Item.Icon.Get());
}

// 2) 새 아이템을 넣을 때 이전 요청을 취소 (헤더에 TSharedPtr<FStreamableHandle> PendingLoad;)
if (PendingLoad.IsValid()) { PendingLoad->CancelHandle(); PendingLoad.Reset(); }
...
PendingLoad = UAssetManager::GetStreamableManager().RequestAsyncLoad(...);
```
(2)를 쓸 때도 **바로 넣는 경로(이미 로드된 아이콘)에서 이전 요청을 취소**해야 한다. 이 랩에서 덮어쓰는 쪽이 바로 그 경로 다음에 도착한다. 실제 프로젝트에서는 이름보다 아이템 ID나 요청 번호로 비교한다.
**교훈:** 비동기 콜백은 "요청한 때의 세상"을 기억하고 "완료된 때의 세상"에서 실행된다. 리스트뷰·타일뷰처럼 **위젯을 재사용**하는 UI에서 가장 흔하다. 이런 버그는 순서가 핵심이므로 로그에 프레임 번호를 찍어 기록표로 만든다.

## Lab 18 — sRGB 색을 Linear 변환 없이 /255
```
틀린 결과:     #FF8000 → 화면 #FFBC00 (G 128 → 188 FAIL), #008080 → #00BCBC (G·B FAIL). 0x00과 0xFF는 PASS
멈춘 위치:     ToMaterialColor 중단점 → 조사식 Color.G / 255.f = 0.502, FLinearColor(Color).G = 0.216
진짜 원인:     FColor(디자인 툴 값)는 sRGB, 머티리얼 파라미터는 Linear다. /255 는 변환이 아니라 재해석이라, 화면에 나갈 때 sRGB로 한 번 더 밝아진다.
```
**수정:**
```cpp
FLinearColor ToMaterialColor(const FColor& Color)
{
    return FLinearColor(Color);     // sRGB → Linear 감마 변환 (= FLinearColor::FromSRGBColor)
}
```
**교훈:** 0과 1(0x00, 0xFF)은 감마 변환해도 그대로라서 순수한 빨강·검정으로만 테스트하면 이 버그를 못 잡는다. 중간값(0x80)을 꼭 넣는다. "물 빠지고 밝다" = Linear 자리에 sRGB 값, "너무 어둡고 진하다" = 그 반대. 텍스처라면 같은 문제가 sRGB 체크박스로 나타난다(색 텍스처 켬, 노멀·마스크·러프니스 끔). 엔진 함수 이름: `FLinearColor(FColor)`/`FromSRGBColor`는 변환, `FColor::ReinterpretAsLinear`는 변환 없음, `ToFColor(true)`/`ToFColorSRGB`는 Linear → sRGB.

# 6부: 애니메이션

## Lab 19 — 렌더되지 않으면 애니메이션이 진행되지 않음
```
틀린 결과:     발자국 0번 FAIL (기대 4). 재생 시간이 지났는데 "몽타주 아직 재생 중: 예, 위치 0.00초"
멈춘 위치:     OnAnimEvent 중단점에 한 번도 안 멈춤 → 조사식 Walker->Mesh->ShouldTickPose() = false, bRecentlyRendered = false
진짜 원인:     생성자의 VisibilityBasedAnimTickOption = OnlyTickPoseWhenRendered. 최근 1초 안에 그려지지 않은 메시는 TickPose 를 건너뛴다.
               포즈가 진행되지 않으니 몽타주 위치가 0에 멈추고, 노티파이도 종료 이벤트도 오지 않는다.
```
**수정:**
```cpp
// 안 보일 때도 몽타주(와 몽타주 노티파이)는 진행, 나머지 애님 그래프는 아낀다.
Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickMontagesWhenNotRendered;
```
`AlwaysTickPose`도 통과한다(최적화는 덜 됨). 발자국이 애님 그래프의 시퀀스(로코모션)에서 나온다면 `OnlyTickMontagesWhenNotRendered`로는 부족하고 `AlwaysTickPose` 이상이 필요하다.
**교훈:** "화면 밖에서만", "서버에서만", "헤드리스에서만" 안 일어나는 애니메이션 이벤트는 이 옵션부터 본다. 데디케이티드 서버는 아무것도 그리지 않으므로 서버의 공격 판정 노티파이가 통째로 사라진다. `ACharacter`의 메시 기본값은 `AlwaysTickPoseAndRefreshBones`인데, 최적화하면서 바꾼 값이 원인인 경우가 많다. 반대로 꼭 필요한 판정은 노티파이 대신 서버의 타이머나 게임 코드로 돌리는 것도 방법이다.

## Lab 20 — interrupted 로 끝난 이전 몽타주의 이벤트를 "완료"로 처리
```
기록표:        프레임 0    공격 → 휘두르기 시작
               (0.3초)     공격 → 휘두르기 시작      ← 같은 몽타주 다시 재생, 이전 인스턴스는 블렌드 아웃
               (0.55초쯤)  휘두르기 끝 → 콤보 +1     ← bInterrupted == true, 이전 인스턴스가 사라짐
               (2.1초쯤)   휘두르기 끝 → 콤보 +1     ← bInterrupted == false, 진짜 끝
틀린 결과:     두 번째 휘두르기 도중 bAttacking == false FAIL, 콤보 2 FAIL (기대 1)
멈춘 위치:     HandleMontageEnded 첫 번째 멈춤 → 지역 창 bInterrupted = true
진짜 원인:     OnMontageEnded 는 몽타주 인스턴스가 사라질 때마다 불린다. 이유(끝까지 재생 / 밀려남 / Stop)를 무시했다.
```
**수정:**
```cpp
void ABH_Swordsman::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage != SwingMontage || bInterrupted)
    {
        return;     // 밀려난 이전 휘두르기. 지금 휘두르는 중인 새 인스턴스가 상태를 책임진다.
    }
    bAttacking = false;
    ++CompletedSwings;
}
```
주의: 이렇게만 하면 **다른 몽타주(회피, 피격)에 밀려난** 경우에도 `bAttacking`이 true로 남는다. 실제 게임이라면 interrupted 일 때는 콤보는 올리지 않고, `bAttacking`은 "지금도 이 몽타주가 재생 중인가"(`!GetAnim()->Montage_IsPlaying(SwingMontage)`)로 정한다. 또는 `Montage_Play` 직후 `Montage_SetEndDelegate`로 **이번 인스턴스 전용** 델리게이트를 건다.
**교훈:** 몽타주 종료 이벤트는 "끝났다"가 아니라 "사라졌다"다. 연타·캔슬이 있는 공격은 이전 인스턴스의 종료 이벤트가 **새 인스턴스가 재생되는 도중에** 도착한다. 블렌드 아웃 시간만큼 늦게 오기 때문에 테스트에서 놓치기 쉽다.

## Lab 21 — 켜고 끄는 일을 노티파이 두 개에 나눠 맡김
```
기록표:        0.55초 HitOn → 히트박스 켬
               0.82초 맞음 → 경직, Montage_Stop(0.1)
               (1.10초 HitOff 는 몽타주가 그 전에 멈춰서 영영 오지 않음)
틀린 결과:     경직 1초 뒤 bHitboxOn == true FAIL
멈춘 위치:     OnAnimEvent 중단점이 경직 뒤로는 안 멈춤. 몽타주 재생 중: 아니오
진짜 원인:     노티파이는 몽타주가 그 시각을 지나갈 때만 온다. 끝을 알리는 노티파이는 몽타주가 끊기면 오지 않는다.
```
**수정 (검증한 것): 구간을 노티파이 스테이트 하나로**
```cpp
// Lab21_HitWindow.h
virtual void OnAnimStateBegin(FName EventName) override;
virtual void OnAnimStateEnd(FName EventName) override;

// Lab21_HitWindow.cpp — BeginPlay
BugHunt::AddNotify(SlashMontage, Length * 0.3f, TEXT("HitWindow"), Length * 0.3f);   // [30%, 60%] 스테이트

void ABH_Knight::OnAnimStateBegin(FName EventName) { bHitboxOn = true; }
void ABH_Knight::OnAnimStateEnd(FName EventName)   { bHitboxOn = false; }
```
검증 로그: 경직 프레임 바로 다음 프레임에 `HitWindow 끝 → 끔`. 노티파이 스테이트는 시작한 것이 사라지면(끊김, 정지, 다른 몽타주) 엔진이 `NotifyEnd`를 불러 준다. 헤더를 바꿨으므로 Live Coding이 아니라 에디터를 닫고 빌드해야 한다.
또 다른 방법: `OnMontageBlendingOut`/`OnMontageEnded`에서 히트박스를 끄는 안전장치(이 랩에서는 검증하지 않았다).
**교훈:** "켜고 → 끈다"처럼 구간에 걸친 상태를 서로 다른 시각의 두 이벤트에 맡기면, 중간에 끊기는 순간 끄는 쪽이 사라진다. 사망, 피격, 회피, 렉으로 프레임이 크게 뛸 때 재현된다. 구간 = 노티파이 스테이트, 그리고 상태를 켜는 코드에는 항상 "끊겼을 때 끄는 길"을 같이 만든다.

# 7부: 잘못된 포인터

## Lab 22 — UObject 가 아닌 곳에 raw 포인터로 액터를 기억함
```
틀린 결과:     Bram 의 어그로 60 FAIL (기대 10). 보스가 노리는 대상 Bram FAIL (기대 Lia)
멈춘 위치:     Kael->Destroy() 줄에서 조사식 Kael = 0x00000xxxxxxx 를 적음 → Bram 스폰 뒤 Bram 의 주소가 똑같다
               ThreatTable.Threat 에 키가 2개뿐: (Kael 이 쓰던 주소 = 지금의 Bram) 60, Lia 30
진짜 원인:     FBH_ThreatTable 은 보통 C++ 구조체라 UPROPERTY 를 못 단다. 그래서 AActor* 로 키를 잡았다.
               GC 가 Kael 의 메모리를 돌려주자 같은 크기의 Bram 이 그 자리에 생겼고, 표는 주소만 보고 Bram 을 Kael 로 착각했다 (ABA).
               주소가 재사용되지 않았다면: 표에 지워진 메모리를 가리키는 키(50)가 남아 TopTarget 이 그것을 고른다.
```
**수정:**
```cpp
// Lab22_ThreatTable.h
TMap<TWeakObjectPtr<AActor>, int32> Threat;

// Lab22_ThreatTable.cpp — TopTarget
for (const TPair<TWeakObjectPtr<AActor>, int32>& Pair : Threat)
{
    if (Pair.Key.IsValid() && Pair.Value > BestThreat)
    {
        Best = Pair.Key.Get();
        BestThreat = Pair.Value;
    }
}
```
`AddThreat`/`ThreatOf`는 그대로 둬도 된다 (`AActor*`가 `TWeakObjectPtr`로 바뀌어 찾는다). 죽은 키를 가끔 지우고 싶으면 `Threat.Remove(nullptr)` 대신 `for (auto It = Threat.CreateIterator(); It; ++It) if (!It.Key().IsValid()) It.RemoveCurrent();`.
**교훈:** 랩 4(UPROPERTY 없는 멤버)는 "GC 가 모르는 참조라서 **지워진다**"였고, 이 랩은 "지워지는 게 맞는데 **지워진 줄 모른다**"다. UObject 를 UObject 가 아닌 곳(보통 구조체, 람다, 다른 스레드, Slate 위젯, 전역 캐시)에서 기억하려면 `TWeakObjectPtr`. 객체를 살려 둬야 한다면 `TStrongObjectPtr`이나 `FGCObject`(AddReferencedObjects)로 GC 에 알린다. raw 포인터의 "주소가 같다"는 "같은 객체다"가 아니다.

## Lab 23 — 타이머 람다가 this 를 캡처했지만 객체에 묶이지 않음
```
틀린 결과:     표적이 맞은 횟수 7 FAIL (기대 2). "포탑 파괴!" 뒤에도 발사 3, 4, 5... 가 찍힌다
멈춘 위치:     Fire 조건부 중단점 (ShotsFired >= 3) → 호출 스택 FTimerManager::Tick → 람다 → Fire
               조사식 this->bActorIsBeingDestroyed = true (이미 파괴된 포탑)
진짜 원인:     FTimerDelegate::CreateLambda 는 어떤 UObject 에도 묶이지 않는다. 액터의 EndPlay 는 그 액터에 묶인
               타이머만 지운다 (ClearAllTimersForObject). 람다 안의 this 는 그냥 주소다.
```
**수정 (택1):**
```cpp
GetWorldTimerManager().SetTimer(FireTimer, FTimerDelegate::CreateWeakLambda(this, [this] { Fire(); }), InInterval, true);
GetWorldTimerManager().SetTimer(FireTimer, FTimerDelegate::CreateUObject(this, &ABH_Turret::Fire), InInterval, true);
GetWorldTimerManager().SetTimer(FireTimer, this, &ABH_Turret::Fire, InInterval, true);
```
보조 수단: `EndPlay`에서 `GetWorldTimerManager().ClearTimer(FireTimer)`.
**교훈:** 델리게이트·타이머·비동기 콜백에 `[this]`를 넘길 때는 "이 객체가 먼저 사라지면?"을 묻는다. `CreateLambda`/`AddLambda` + `[this]`는 C++ 의 댕글링 포인터와 똑같다. 지금은 GC 전이라 "쏘기만" 했지만, GC 가 돈 뒤였다면 랩 4 처럼 해제된 메모리를 읽는다. 언리얼의 답은 `CreateUObject`, `CreateWeakLambda`, `AddUObject`, `AddWeakLambda`(UObject), `CreateSP`/`CreateSPLambda`(TSharedFromThis).

## Lab 24 — 배열 원소 참조를 든 채로 Add (재할당)
```
기록표:        물약 4 (Gem 새 칸, 4칸) → 3 → 2 (Herb 새 칸, 5칸: 인라인 4칸을 넘어 힙으로 이사) → "1"
틀린 결과:     남은 물약 2 FAIL (기대 1). 로그에는 1 로 찍히는데 진짜 칸은 2
멈춘 위치:     조사식 &Potions 와 &Slots[0] 이 Herb 를 넣기 전엔 같고, 넣은 뒤엔 다르다
               데이터 중단점 &Slots[0].Count → 마지막 웨이브에서 안 걸린다 (아무도 진짜 칸을 안 바꿈)
진짜 원인:     FBH_BeltSlot& Potions 는 "그 순간의 메모리 위치"를 가리킨다. Add 가 배열을 새 메모리로 옮기자 옛 위치에 쓰게 됐다.
```
**수정:**
```cpp
const int32 PotionIndex = FindSlot(Potion);
for (const FName& Drop : Drops)
{
    Slots[PotionIndex].Count -= 1;
    AddItem(Drop, 1);
}
```
`Slots.Reserve(...)`로 미리 공간을 잡아도 이번에는 통과하지만, "몇 칸이면 충분한가"를 코드가 장담할 수 없으면 근본 해결이 아니다.
**교훈:** `TArray`의 `Add`, `Emplace`, `Insert`, `SetNum`, `Reserve`, `Append`는 원소를 옮길 수 있다. 그 전에 꺼낸 참조, 포인터, `GetData()`는 무효다. 기본 할당자라면 옛 위치는 **해제된 힙**이라 가끔 죽거나 남의 데이터를 망가뜨리고(-stomp2malloc 으로 잡힌다), `TInlineAllocator`는 옛 위치가 살아 있어서 **죽지도 않고 값만 사라진다**. `TMap`/`TSet`의 `FindOrAdd`/`Add`도 같다 (`Find`로 얻은 포인터는 다음 `Add` 전까지만).

## Lab 25 — TSharedPtr 순환 참조
```
틀린 결과:     대화가 끝난 뒤 남은 노드 7 FAIL (기대 0). 소멸자 중단점이 한 번도 안 걸린다
멈춘 위치:     EndConversation 의 Root.Reset() 직전 조사식 Root → 참조 수 4 (Root 멤버 하나 + 선택지 3개의 Parent. Current 는 바로 앞 줄에서 Reset)
진짜 원인:     부모 → 자식(Choices)과 자식 → 부모(Parent)를 둘 다 TSharedPtr 로 잡았다. 고리 안의 참조 수는 0 이 되지 않는다.
```
**수정:**
```cpp
// Lab25_DialogueTree.h
TWeakPtr<FBH_DialogueNode> Parent;

// Lab25_DialogueTree.cpp — GoBack
if (TSharedPtr<FBH_DialogueNode> Up = Current ? Current->Parent.Pin() : nullptr)
{
    Current = Up;
}
```
**교훈:** `TSharedPtr`는 참조 수만 센다. GC 처럼 "아무도 안 닿는 고리"를 찾아 주지 않는다. 소유 방향 하나만 `TSharedPtr`/`TSharedRef`, 거꾸로 가리키는 쪽(부모, 소유자, 리스너)은 `TWeakPtr`. 크래시도 경고도 없어서 `stat memory`, `memreport -full`, LLM(`-llm`)에서 "계속 느는 숫자"로 발견되곤 한다. 이 랩처럼 생성/소멸 카운터를 두면 바로 보인다.

# 8부: 멈춤 (무한루프와 데드락)

## Lab 26 — 다른 액터의 Tick 이 바꿔 줄 값을 게임 스레드에서 while 로 기다림
```
틀린 결과:     응답 없음 (2000 ms) TIMEOUT. 탈 때 엘리베이터가 와 있었다 FAIL, 게임이 멈춘 시간 2000 ms FAIL
멈춘 위치:     HangBudgetSeconds 30 → 모두 중단 → 스레드 창 GameThread → RideElevator 의 while
               조사식 Elevator->Height = 0, TicksWhileMoving = 0. F5 → 모두 중단을 반복해도 그대로
진짜 원인:     Tick 은 게임 스레드가 다음 프레임에 부른다. 게임 스레드가 while 에서 기다리는 동안 다음 프레임은 오지 않는다.
```
**수정:**
```cpp
void ABH_Passenger::RideElevator(ABH_Elevator* Elevator, float TargetHeight, float Speed)
{
    Elevator->Call(TargetHeight, Speed);
    Elevator->OnArrived.AddUObject(this, &ABH_Passenger::Board, Elevator);
}
```
타이머로 0.05초마다 `bArrived`를 확인해도 된다. 이미 도착해 있을 수도 있다면 `if (Elevator->bArrived) Board(Elevator); else AddUObject(...)`.
**교훈:** 게임 스레드 코드는 "기다리는" 코드를 쓰지 않는다. 기다릴 일은 델리게이트, 타이머, Tick, 잠재 액션(Delay 노드), 코루틴(UE5 의 `UE::Tasks`/`co_await` 계열)으로 "나중에 불러 달라"로 바꾼다. `Sleep`을 넣어도 CPU 사용률만 줄 뿐 굳는 것은 같다.

## Lab 27 — 너비 우선 탐색의 방문 기록이 루프 한 바퀴만 산다
```
틀린 결과:     성문 → 항구 PASS, 성문 → 등대 응답 없음 TIMEOUT (226,000 바퀴 넘게 돌았다)
멈춘 위치:     HangBudgetSeconds 30 → 모두 중단 → FindPath. 조사식 Frontier.Num() 이 수십만, Visited.Num() 은 1~4
진짜 원인:     TSet<int32> Visited 를 while 안에서 만들었다. 매 바퀴 새로 비어 있으니 이미 넣은 지점을 계속 다시 넣는다.
               갈 수 있는 곳은 우연히 도착해서 멈추지만, 못 가는 곳은 큐가 끝없이 불어난다.
```
**수정:**
```cpp
TArray<int32> Frontier = { From };
TMap<int32, int32> CameFrom = { { From, INDEX_NONE } };
TSet<int32> Visited = { From };          // 탐색 전체에 하나
while (Frontier.Num() > 0)
{
    ...
    for (int32 Next : Nodes[Current].Links)
    {
        if (!Visited.Contains(Next)) { Visited.Add(Next); CameFrom.FindOrAdd(Next, Current); Frontier.Add(Next); }
    }
}
```
`CameFrom.Contains(Next)`로 검사해도 된다 (CameFrom 이 곧 방문 목록).
**교훈:** 그래프 탐색이 굳는 것은 거의 항상 "못 가는 목적지 + 방문 체크 실수"다. 테스트에 **도달 불가능한 목적지**를 꼭 넣는다. 지역 변수가 어느 중괄호 안에서 만들어지는지(수명)도 같이 본다. 실제 게임에서는 탐색 노드 수에 상한을 두고(언리얼 내비메시의 `MaxSearchNodes`처럼) 넘으면 "경로 없음"으로 끝내는 것이 보통이다.

## Lab 28 — 게임 스레드가 작업 스레드 결과를 동기로 기다리는데, 작업 스레드는 게임 스레드를 기다림
```
틀린 결과:     응답 없음 (2000 ms 기다림, 데드락 의심) TIMEOUT. 게임이 멈춘 시간 2000 ms, 저장 완료 FAIL
멈춘 위치:     HangBudgetSeconds 30 → 모두 중단 → 병렬 스택:
                 GameThread:  ABH_SaveSystem::AutoSave → WaitWithBudget → TFuture::WaitFor
                 작업 스레드: AutoSave 의 람다 → Snapshot.Get() → TFuture::Get
진짜 원인:     작업 스레드가 AsyncTask(GameThread) 로 보낸 일은 게임 스레드가 다음 프레임에 처리한다.
               그 게임 스레드는 작업 스레드가 끝나기를 기다리고 있다. 서로를 기다린다 (데드락).
```
**수정:**
```cpp
void ABH_SaveSystem::AutoSave(ABH_SaveHero* Hero)
{
    const float Seconds = WriteSeconds;
    const FString Text = Describe(Hero);                 // 게임 스레드에서 지금 상태를 복사
    TWeakObjectPtr<ABH_SaveSystem> WeakThis(this);
    Async(EAsyncExecution::ThreadPool, [Text, Seconds, WeakThis]
    {
        FPlatformProcess::Sleep(Seconds);                 // 디스크 쓰기
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
스냅샷만 먼저 뜨고 `Done.Get()`을 남기면 데드락은 풀리지만 "게임이 멈춘 시간 < 50 ms"가 FAIL 이다 (디스크 쓰기 0.3초만큼 히치). 작업 스레드에서 `Hero->Gold`를 직접 읽으면 데이터 경쟁이고, 저장 직후의 +50 이 섞여 들어가 "저장 내용"이 FAIL 일 수 있다.
**교훈:** 게임 스레드에서 `Future.Get()`, `Event->Wait()`, `FScopeLock`으로 오래 기다리지 않는다. 특히 기다리는 상대가 게임 스레드에 일을 보내는 구조(로딩, 세이브, 에셋 스트리밍 콜백)라면 그대로 데드락이다. 잠금 두 개를 서로 반대 순서로 잡는 데드락(A→B / B→A)도 병렬 스택에서 같은 모양으로 보인다: 두 스레드가 서로 상대가 쥔 것을 기다린다.
