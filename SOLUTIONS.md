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
