# BugHunt UE — 언리얼 엔진 C++ 버그 추적 실습

[BugHunt](https://github.com/gusgh0157/BugHunt)(C++ 콘솔판)의 언리얼 엔진 버전입니다.
버그가 **일부러 심어진** 작은 게임 코드 모음이고, 이번에는 **언리얼에서만 생기는** 버그들입니다.

- **1부: 크래시 추적 (랩 1~4)**: 에디터나 게임이 죽거나 `Ensure`가 뜬 순간, 호출 스택을 거슬러 올라가 원인을 찾습니다. 엔진 프레임이 섞인 긴 스택을 읽는 연습입니다.
- **2부: 흐름 추적 (랩 5~10)**: 게임은 멀쩡히 도는데 **결과가 틀립니다**. 델리게이트, 타이머, Tick 순서, GC, Config처럼 "언리얼이 대신 해 주는 일"의 순서와 수명을 직접 따라갑니다.
- **3부: 멀티플레이 (랩 11~12)**: 서버와 클라이언트가 **서로 다른 값**을 보는 버그. PIE로 여러 클라이언트를 띄워 한 디버거에서 같이 추적합니다.
- **4부: 사운드 (랩 13~15)**: 소리가 늦게 나고, 끊기고, 한 사람만 듣는 버그. 지연을 프레임/ms로 재서 판정합니다.
- **5부: 이미지 / 렌더링 (랩 16~18)**: 색이 안 바뀌고, 엉뚱한 아이콘이 뜨고, 색이 물 빠져 보이는 버그. 머티리얼 파라미터 값과 브러시에 들어간 텍스처를 조회해서 판정합니다.
- **6부: 애니메이션 (랩 19~21)**: 발소리가 안 나고, 연타하면 상태가 꼬이고, 끊긴 공격의 히트박스가 남는 버그. 노티파이 횟수와 상태값으로 판정합니다.
- **7부: 잘못된 포인터 (랩 22~25)**: 죽지 않는 포인터 버그. 지워진 액터를 기억하는 raw 포인터, 객체보다 오래 사는 타이머 람다, `Add` 뒤의 배열 원소 참조, `TSharedPtr` 순환 참조를 주소와 참조 수로 추적합니다.
- **8부: 멈춤 (랩 26~28)**: 크래시 없이 게임이 굳는 무한루프와 데드락. "모두 중단", 스레드 창, 병렬 스택으로 굳은 순간을 세워 봅니다. (실습용 안전장치가 있어 에디터가 진짜로 굳지는 않습니다.)
- **9부: 컨테이너 (랩 29~35)**: `TArray`/`TMap`/`TSet` 의 범위 검사 assert, 없는 키, 재할당 뒤의 포인터, 순서를 바꾸는 제거, 해시와 `==` 불일치, GC 와 복제(`FFastArraySerializer`)를 만난 컨테이너.
- **10부: 프레임레이트와 초기화 (랩 36~38)**: 30fps 와 120fps 에서 다르게 움직이는 코드(`DeltaSeconds` 누락, 프레임당 감속)와 초기화 안 된 구조체 멤버.
- **11부: 물리 (랩 39~42)**: 터널링과 CCD, 채널 응답과 Overlap 이벤트, 스윕 없는 이동, 물리 물체를 Tick 에서 옮기기. Chaos Visual Debugger 로 프레임을 되감습니다.
- **12부: 데이터 레이스 (랩 43~44)**: 작업 스레드가 잠금 없이 같은 값을 고치거나 게임 스레드 전용 객체를 만지는 버그. 재현 확대경으로 매번 일어나게 했습니다.
- **13부: 성능 (랩 45~48)**: Tick 마다 액터 검색, 프레임마다 배열 재할당, 수백 개의 Tick, 게임 중 동기 로드. 시간 대신 횟수로 판정합니다.
- **채점기**: `grade.bat` 하나로 모든 랩을 헤드리스로 돌려 PASS/FAIL/TIMEOUT/CRASH/SKIP 과 점수를 냅니다. 숨은 입력으로 하드코딩을 막습니다.
- **부록: Live Coding 함정**: "고쳤는데 그대로예요"의 단골 원인.

> 콘솔판과 같은 규칙입니다. 코드를 눈으로 훑어 정답을 찾지 마세요. 1부는 **죽게 만들고 디버거가 멈춘 곳에서부터**, 2·3부는 **직접 멈춘 곳에서부터 값을 확인하며** 추적하세요.
> 콘솔판 1·3부를 먼저 끝냈다면 좋습니다. 호출 스택 창, 지역 창, 조건부 중단점, 추적점, 데이터 중단점 사용법은 거기서 설명한 그대로 씁니다.

## 다루는 버그 한눈에 보기

랩 하나가 여러 줄에 걸칠 수 있습니다. "미포함"은 아직 그 분류의 대표 사례를 다루는 랩이 없다는 뜻입니다.

| 분류 | 랩 | 대표 사례 |
|---|---|---|
| 포인터·수명·GC | 1, 4, 9, 22, 23, 24, 25, 34 | `Cast` 실패 nullptr, `UPROPERTY` 없는 포인터와 GC, `Destroy` 뒤 `IsValid`, `TWeakObjectPtr`, 람다의 `[this]`, `TSharedPtr` 순환 |
| 컨테이너 | 3, 24, 29, 30, 31, 32, 33, 34, 35 | 순회 중 제거, 범위 밖 인덱스, 없는 키 `operator[]`, `Add` 뒤 원소 참조/`Find` 포인터, `RemoveAtSwap` 순서, 해시/`==` 불일치, GC 와 `TArray<UObject*>`, FastArray 복제 |
| 흐름·순서 | 2, 5, 6, 7, 8, 10 | BeginPlay 순서, 델리게이트 해제 누락, 타이머 핸들, Tick 그룹, Visual Logger, CDO/Config 기본값 |
| 멀티플레이 | 11, 12, 15, 35 | RepNotify, 권한과 Server RPC, 소리는 복제되지 않음, FastArray `MarkItemDirty` |
| 사운드 | 13, 14, 15 | 소프트 레퍼런스 지연 로드, 붙인 컴포넌트의 수명, NetMulticast |
| 이미지·렌더링 | 16, 17, 18 | MID, 비동기 로드 콜백 순서, sRGB/Linear |
| 애니메이션 | 19, 20, 21 | 렌더 안 될 때 노티파이, 몽타주 interrupted, 노티파이 스테이트 |
| 멈춤 (무한루프·데드락) | 26, 27, 28 | 게임 스레드 while 대기, 방문 체크 누락, Future 동기 대기 데드락 |
| 물리 | 39, 40, 41, 42 | 터널링과 CCD, 채널 응답과 Generate Overlap Events, `bSweep=false` 순간이동, 시뮬레이션 물체를 Tick 에서 옮기기. (서브스테핑 차이는 설명만) |
| 프레임레이트 의존 | 36, 37 | `DeltaSeconds` 누락, 프레임당 감속(적분), `t.MaxFPS`, 고정 시간 간격 |
| 성능 | 45, 46, 47, 48 | Tick 마다 `GetAllActorsOfClass`, `Reserve` 누락, Tick 남발, 게임 중 동기 로드. (매 프레임 `FString` 만들기는 랩 46 힌트에서만 다룸) |
| 데이터 레이스 | 43, 44 | `ParallelFor` 읽기-계산-쓰기 레이스, 작업 스레드에서 UObject 만지기(`check(IsInGameThread())`) |
| 초기화 안 된 값 | 38 | 구조체 멤버 초기값 없음, `AddDefaulted` 와 남은 메모리. (지역 변수, `FMemory::Malloc` 은 설명만) |

## BugHunt 시리즈

같은 방식(일부러 심은 버그를 디버거로 추적)으로 만든 네 가지 판입니다.

| 판 | 다루는 버그 | 저장소 |
|---|---|---|
| C++ 콘솔 | 호출 스택, 메모리 오류, 크래시 없는 흐름 추적 (랩 28개) | [BugHunt](https://github.com/gusgh0157/BugHunt) |
| C# 콘솔 | 예외와 스택 트레이스, 흐름 추적, 비동기/동시성, 리소스와 메모리 (랩 28개) | [BugHuntCS](https://github.com/gusgh0157/BugHuntCS) |
| 언리얼 엔진 | 크래시, 흐름 추적, 멀티플레이, 사운드, 이미지, 애니메이션, 잘못된 포인터, 멈춤(무한루프·데드락), 컨테이너, 프레임레이트, 초기화, 물리, 데이터 레이스, 성능, 채점기 (랩 48개) | [BugHuntUE](https://github.com/gusgh0157/BugHuntUE) **(지금 보는 저장소)** |
| 유니티 | 크래시/예외, 흐름 추적, 사운드, 이미지, 애니메이션 (랩 23개) | [BugHuntUnity](https://github.com/gusgh0157/BugHuntUnity) |

## 준비

필요한 것: **Unreal Engine 5.8**, **Visual Studio 2022 이상**(워크로드 "C++를 사용한 게임 개발", 구성 요소 "Unreal Engine 디버거/IDE 지원") 또는 Rider.

1. `BugHuntUE.uproject`를 우클릭 → **Generate Visual Studio project files**. (`BugHuntUE.sln`이 생깁니다.)
   - `EngineAssociation`은 `5.8`입니다. 런처로 5.8을 설치했다면 uproject를 바로 열 수 있습니다. 소스 빌드 엔진을 쓴다면 uproject 우클릭 → **Switch Unreal Engine version...** 으로 그 엔진을 고르세요.
2. `BugHuntUE.sln`을 엽니다. 솔루션 탐색기에서 **BugHuntUE**를 우클릭 → **시작 프로젝트로 설정**.
3. 도구 모음에서 구성 **Development Editor**, 플랫폼 **Win64**.
4. **F5**. 빌드가 끝나면 디버거가 붙은 채로 에디터가 뜹니다. (첫 실행은 셰이더 컴파일로 몇 분 걸립니다.)

명령줄 빌드: 에디터를 닫고 `build.bat` (기본 Development) 또는 `build.bat DebugGame`.

> 💡 이 모듈은 `BugHuntUE.Build.cs`에서 `OptimizeCode = CodeOptimization.Never`로 최적화를 껐습니다. 그래서 Development Editor에서도 랩 코드의 변수가 "최적화되어 사용할 수 없음"으로 보이지 않고 F10이 줄을 건너뛰지 않습니다. 실제 프로젝트에서 같은 문제를 만나면 **DebugGame Editor** 구성을 쓰세요.

### 디버거를 붙이는 세 가지 방법

| 방법 | 하는 법 | 언제 |
|---|---|---|
| VS에서 F5 | 위 준비 4번 | 기본. 에디터 시작부터 디버거가 붙어 있다 |
| 실행 중인 에디터에 붙이기 | 에디터를 그냥 실행 → VS에서 **디버그 → 프로세스에 연결 (Ctrl+Alt+P)** → `UnrealEditor.exe` 선택 | 이미 에디터를 켜 둔 상태에서 버그가 났을 때. 재시작이 필요 없다 |
| Rider | **Run → Attach to Unreal Editor**, 또는 Run 구성 `BugHuntUE`로 Debug | Rider 사용자 |

> 디버거 없이 크래시가 나면 **Crash Reporter** 창에 호출 스택이 나오고, 같은 내용이 `Saved/Logs/BugHuntUE.log` 맨 아래에 남습니다. 하지만 지역 변수는 볼 수 없으니 **디버거를 붙이고 다시 재현**하세요.

## 랩 실행하기

1. 에디터 위쪽 **Play(▶)** 를 누릅니다. (PIE: Play In Editor)
2. 게임 화면을 클릭하고 **`~`(물결표)** 키로 콘솔을 엽니다.
3. `BugHunt.Lab 1` 입력. (`BugHunt.List`는 목록)
4. 결과는 화면 왼쪽 위와 **Output Log**(메뉴 Window → Output Log)에 나옵니다. Output Log 검색창에 `LogBugHunt`를 넣으면 랩 결과만 보입니다.

```
LogBugHunt: ==== 랩 5: 전투 기록 창 ====
LogBugHunt: [FAIL] 다시 연 창의 기록 줄 수 (기대: 1, 실제: 2)
LogBugHunt: [PASS] 기사 HP (값: 78)
LogBugHunt: ==== 랩 5 결과: PASS 1 / FAIL 2 ====
```

**같은 랩을 계속 돌릴 때 (콘솔판의 "명령 인수"와 같은 방법)**: 솔루션 탐색기에서 BugHuntUE 우클릭 → 속성 → 디버깅 → **명령 인수**를

```
"$(SolutionDir)BugHuntUE.uproject" -game -BugHuntLab=5
```

로 바꾸고 F5. 에디터 대신 게임 창이 바로 뜨고 랩 5가 자동 실행됩니다. 에디터 로딩을 기다리지 않아서 훨씬 빠릅니다. (원래대로 하려면 `-game -BugHuntLab=5`를 지우세요.)

### 고친 코드를 반영하는 법

| 바꾼 것 | 반영 방법 |
|---|---|
| `.cpp`의 함수 본문만 | 에디터를 켠 채로 **Ctrl+Alt+F11** (Live Coding) → 컴파일 끝나면 PIE를 다시 시작 |
| 헤더(`.h`), `UPROPERTY`/`UFUNCTION`, 생성자, 새 파일 | **에디터를 닫고** VS에서 F5 (다시 빌드) |

헷갈리면 항상 "에디터를 닫고 빌드"가 안전합니다. 이유는 [부록](#부록-live-coding-함정)에서 직접 겪어 봅니다.

---

# 1부: 크래시 추적

## 콘솔판과 다른 점: 엔진 프레임

언리얼 크래시의 호출 스택은 길고, 대부분 엔진 코드입니다.

```
UnrealEditor-BugHuntUE.dll!ABH_Hero::Heal(int Amount) 줄 21              ← 내 코드 (여기서 죽었다)
UnrealEditor-BugHuntUE.dll!ABH_HealthPickup::GiveHealth(ABH_Hero * Hero) 줄 63
UnrealEditor-BugHuntUE.dll!ABH_HealthPickup::OnTouched(AActor * Toucher) 줄 58
UnrealEditor-BugHuntUE.dll!ABH_HealthPickup::HandleOverlap(...) 줄 52
UnrealEditor-BugHuntUE.dll!ABH_HealthPickup::execHandleOverlap(...)      ← UHT가 만든 코드 (.gen.cpp)
UnrealEditor-CoreUObject.dll!UFunction::Invoke(...)                     ← 엔진
UnrealEditor-CoreUObject.dll!UObject::ProcessEvent(...)
UnrealEditor-Engine.dll!...FComponentBeginOverlapSignature...           ← 델리게이트 Broadcast
UnrealEditor-Engine.dll!UPrimitiveComponent::BeginComponentOverlap(...)
UnrealEditor-Engine.dll!UPrimitiveComponent::UpdateOverlapsImpl(...)
...
UnrealEditor-BugHuntUE.dll!ABH_Lab01::StartLab() 줄 82                   ← 다시 내 코드: "어디서 시작됐나"
```

- 엔진 프레임이 `UnrealEditor-Engine.dll!UnknownFunction`이나 주소로만 보이면 엔진 디버그 심볼이 없는 것입니다. Epic Games Launcher → 라이브러리 → 엔진 버전 옆 ▼ → **옵션** → **Editor symbols for debugging**을 설치하면 위처럼 함수 이름이 나옵니다. (수 GB. 없어도 랩은 풀 수 있습니다. 내 코드 프레임은 항상 이름과 줄 번호가 나옵니다.)
- 모듈 이름으로 구분하세요. **`UnrealEditor-BugHuntUE.dll!`** 이 내 코드, `UnrealEditor-Engine.dll!`, `UnrealEditor-CoreUObject.dll!` 등은 엔진입니다.
- 엔진 프레임은 건너뛰되, **어떤 엔진 기능을 거쳐 왔는지**(겹침 이벤트, SpawnActor, Tick, 타이머, 델리게이트...)는 이름만 읽어 두세요. 그게 "왜 이 시점에 불렸나"의 답입니다.
- `exec함수이름`은 UHT가 만든 접착 코드입니다. 블루프린트/델리게이트에서 C++ 함수로 들어오는 입구라고 보면 됩니다.

## 언리얼의 세 가지 검사: check, ensure, verify

| 매크로 | 실패하면 | 디버거가 붙어 있으면 | 배포(Shipping) 빌드 |
|---|---|---|---|
| `check(조건)` / `checkf` | **프로그램 종료** ("Assertion failed") | 그 줄에서 멈춤 | 검사 자체가 사라짐 |
| `ensure(조건)` / `ensureMsgf` | Output Log에 **"Ensure condition failed"** + 호출 스택을 찍고 **계속 실행** | 그 줄에서 멈춤 → F5로 계속 가능 | 검사 사라짐 |
| `verify(식)` | check와 같지만 Shipping에서도 **식은 실행됨** | 그 줄에서 멈춤 | 식만 실행 |

> `ensure`는 같은 줄에서 **세션당 한 번만** 보고합니다. 랩을 두 번 돌렸는데 두 번째에 안 뜨는 건 정상입니다. 에디터를 다시 켜면 또 뜹니다. (매번 보려면 `ensureAlways`)

## 추적 4단계 (콘솔판과 같음)

1. **어디서 멈췄나?** 호출 스택에서 **`BugHuntUE.dll!`이 처음 나오는 줄**을 찾는다.
2. **무엇이 이상한가?** 지역 창에서 nullptr, 이상한 이름, 쓰레기 값을 찾는다. 언리얼 타입(FString, TArray, UObject)은 natvis 덕분에 내용이 바로 보입니다.
3. **그 값은 어디서 왔나?** 아래 프레임으로 내려가며 출처를 찾는다. 엔진 프레임은 건너뛴다.
4. **진짜 원인은?** 이상한 값이 **처음 만들어진 곳**을 고친다.

```
기록표 (1부용)
멈춘 함수:              
이상한 값:              
그 값이 온 프레임:       
거쳐 온 엔진 기능:        
진짜 원인:              
수정:                   
```

## 따라 해 보기: 랩 1 (회복 아이템)

1. VS에서 **F5**로 에디터를 띄우고 Play → `~` → `BugHunt.Lab 1`.
2. 화면에 "시작 HP 10..."이 나온 뒤 VS가 앞으로 나오며 멈춥니다. **"예외가 throw됨: 읽기 액세스 위반. this이(가) nullptr였습니다."**
3. **Ctrl+Alt+C**로 호출 스택을 엽니다. 맨 위가 `ABH_Hero::Heal`입니다. → *멈춘 함수*
4. **Alt+4**로 지역 창을 엽니다. `this`가 `0x0000000000000000`입니다. 영웅이 없는데 회복을 했네요. → *이상한 값*
5. 두 번째 줄 `ABH_HealthPickup::GiveHealth`를 더블클릭. `Hero`가 nullptr입니다. 세 번째 줄 `OnTouched`를 더블클릭. 이번엔 `Toucher`가 nullptr가 **아닙니다**. 펼쳐서 이름을 보세요. 누구인가요? → *그 값이 온 프레임*
6. 아래로 더 내려가면 `UPrimitiveComponent::BeginComponentOverlap` 같은 엔진 프레임이 보입니다. 겹침 이벤트에서 불렸다는 뜻입니다. → *거쳐 온 엔진 기능*
7. 더 내려가 `ABH_Lab01::StartLab`을 더블클릭하면 초록 화살표가 **어느 이동**을 가리키는지 보입니다.
8. **Shift+F5**로 멈추고(에디터도 같이 꺼집니다), 고친 뒤 다시 F5 → 랩 1 → `[PASS]`를 확인합니다.
9. 보너스: `if (!Hero) return;`만 넣으면 크래시는 사라지지만 `[FAIL]`이 남습니다. 왜일까요? 파일 맨 위 주석을 읽으세요.

## 1부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 1 | 회복 아이템 | 펫이 아이템을 줍자 게임이 죽는다 | 디버거 붙이기, 엔진 프레임 건너뛰기, `Cast<T>` 실패 = nullptr | ★ |
| 2 | 점수판 HUD | 시작하자마자 죽는다 | 생성자 / PostInitializeComponents / **BeginPlay 순서**, SpawnActor 안에서 바로 불리는 BeginPlay | ★★ |
| 3 | 죽은 적 정리 | **Ensure**가 뜨고 죽은 적이 남아 있다 | `ensure` 메시지 읽기, **TArray 순회 중 제거** | ★★ |
| 4 | 퀘스트 로그 | 두 번째 퀘스트를 받을 때 죽는다 (가끔 안 죽음) | **GC**, `UPROPERTY` 없는 포인터, `obj gc`, `gc.CollectGarbageEveryFrame`, `-stomp2malloc` | ★★★ |

## 힌트 보는 법

콘솔판과 같습니다. 각 랩 `.cpp` 파일 **맨 아래**에 `#pragma region`으로 접혀 있습니다.

1. 랩 파일을 처음 열면 **Ctrl+M, Ctrl+O**로 전부 접습니다.
2. 막힐 때 **힌트 1**만 펼칩니다. 그래도 막히면 다음 힌트를 하나씩.

랩 코드는 `Source/BugHuntUE/Labs/LabNN_*.cpp` 입니다. `Source/BugHuntUE/Core/`(랩 실행기)에는 버그가 없습니다.
다 풀면 [SOLUTIONS.md](SOLUTIONS.md)로 맞춰 보세요.

---

# 2부: 흐름 추적 (크래시 없음)

콘솔판 3부와 같은 4단계입니다. 언리얼에서는 **"내가 부르지 않았는데 불리는 코드"**(델리게이트, 타이머, Tick, GC, Config 로딩)가 많아서, "언제, 누가, 몇 번 불렀나"를 보는 도구가 특히 중요합니다.

## 흐름 추적 4단계

1. **기대를 먼저 적는다.** "독 피해는 0.4, 0.8, 1.4, 1.8초에 들어와야 한다"처럼.
2. **틀린 결과에서 가장 가까운 곳부터 멈춘다.**
3. **기대와 처음 달라지는 곳을 찾는다.** 매 프레임 도는 코드는 F10 대신 추적점, `UE_LOG`, Visual Logger로 기록한다.
4. **"처음 달라진 곳"을 고친다.**

```
기록표 (2·3부용)
틀린 결과:              
멈춘 위치 / 쓴 도구:     
기대값 → 실제값:         
처음 어긋난 곳:          
진짜 원인:              
수정:                   
```

## 2부에서 쓰는 언리얼 도구

| 도구 | 하는 법 | 언제 |
|---|---|---|
| **Output Log** | Window → Output Log. 검색창에 카테고리(`LogBugHunt`) 입력, 오른쪽 Filters로 Warning/Error만 | 모든 랩. "몇 초에, 몇 번" 찍혔는지 보기 |
| **`UE_LOG`** | `UE_LOG(LogBugHunt, Log, TEXT("HP %d, 프레임 %llu"), Health, GFrameCounter);` | 추적점보다 오래 남기고 싶을 때. Live Coding으로 넣고 빼기 쉽다 |
| 로그 상세도 바꾸기 | 콘솔 `log LogBugHunt Verbose` | `Verbose` 로그까지 보고 싶을 때 |
| **추적점** | 중단점 우클릭 → 작업, `{Damage}` 같은 식, "실행 계속" 체크 | 코드를 고치지 않고 호출 순서·횟수 기록 |
| **적중 횟수 / 조건** | 중단점 우클릭 → 조건 | 매 프레임 불리는 함수의 N번째만 |
| **`ListTimers`** | 콘솔 | 지금 살아 있는 타이머와 남은 시간 (랩 6) |
| **`dumpticks`** | 콘솔 | Tick 함수 목록과 Tick 그룹 (랩 7) |
| **Visual Logger** | Tools → Debug → **Visual Logger** → 녹화 → PIE → 정지 → 타임라인 끌기 | 위치·경로처럼 "공간"에서 일어나는 버그를 **되감아** 보기 (랩 8) |
| **`getall` / `obj`** | `getall BH_Slime MaxHealth`, `obj list class=BH_Enemy`, `obj gc`, `obj refs name=...` | 살아 있는 객체 목록과 값, 누가 참조하는지 (랩 4, 10) |
| **`stat` 명령** | `stat fps`, `stat unit`(게임/렌더/GPU 스레드 ms), `stat game`(Tick 시간), `stat uobjects` | 성능이 이상하거나 객체 수가 늘어날 때 |
| **데이터 중단점** | 디버그 → 새 중단점 → 데이터 중단점, 주소 `&MaxHealth`, 4바이트 | "누가 이 값을 바꿨지?" (랩 10) |
| **Gameplay Debugger** | PIE 중 **`'`(작은따옴표)** 키, 숫자 키로 카테고리 | AI·지각·내비게이션 상태를 화면에 겹쳐 보기. 이 랩들은 AI 컨트롤러를 쓰지 않아 직접 쓰진 않지만, AI 버그라면 Visual Logger와 함께 1순위 도구입니다 |

## 따라 해 보기: 랩 5 (전투 기록 창)

1. Play → `BugHunt.Lab 5`. 아무것도 멈추지 않고 `[FAIL] 다시 연 창의 기록 줄 수 (기대: 1, 실제: 2)`.
2. **기대를 적습니다.** 기사는 10, 5, 7 피해를 받습니다. 창은 10을 받을 때와 7을 받을 때만 열려 있으므로 `HandleDamaged`는 **두 번**(10, 7) 불려야 합니다.
3. `Lab05_CombatLog.cpp`의 `HandleDamaged` 첫 줄에 **F9**.
4. `BugHunt.Lab 5`를 다시 입력. 멈출 때마다 지역 창의 `Damage`를 적고 F5. 몇 번 멈추나요? 값은?
5. 기대(10, 7)와 다르게 **처음** 멈춘 값이 무엇인지 보세요. 그때 창은 열려 있었나요? → *처음 어긋난 곳*
6. 호출 스택에서 `ABH_Fighter::TakeHit` 프레임을 더블클릭 → 조사식에 `OnDamaged`를 넣고 펼칩니다. `InvocationList`에 몇 개가 들어 있나요?
7. 고치고 (`.cpp`만 바꿨다면 Ctrl+Alt+F11 Live Coding으로 충분합니다) PIE를 다시 시작해 [PASS] 세 개를 확인합니다.

## 2부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 5 | 전투 기록 창 | 창을 다시 열면 기록이 두 줄씩 찍힌다 | **델리게이트 바인딩 해제 누락**, 추적점, InvocationList 보기 | ★ |
| 6 | 독 정화 | 정화했는데 독 피해가 계속 들어온다 | **FTimerHandle** 수명, `ListTimers` | ★★ |
| 7 | HP 바 | HP 바가 한 프레임씩 늦다 | **Tick 그룹과 순서**, `dumpticks`, `GFrameCounter`, `AddTickPrerequisiteActor` | ★★ |
| 8 | 순찰병 | 순찰병이 첫 웨이포인트에서 떨고만 있다 | **Visual Logger**로 녹화·되감기 | ★★ |
| 9 | 궁수 | 쓰러진 허수아비로 점수를 계속 번다 | `Destroy()` 뒤의 포인터, **`IsValid` vs nullptr** | ★★ |
| 10 | 밸런스 패치 | HP를 150으로 올렸는데 게임에선 100이다 | **CDO**, Config/블루프린트 기본값이 생성자 값을 덮어쓰는 순서, `getall`, 데이터 중단점 | ★★★ |

> 랩 10의 블루프린트 버전도 직접 만들어 보세요: Content Browser에서 `BH_Slime`을 부모로 블루프린트 `BP_Slime`을 만들고 Class Defaults에서 MaxHealth를 120으로 바꾼 뒤 저장 → C++ 생성자 값을 200으로 바꾸고 에디터를 다시 켭니다. `BP_Slime`의 MaxHealth는 몇인가요? 블루프린트에서 **바꾸지 않은** MoveSpeed는요? (블루프린트는 "부모와 다르게 바꾼 값"만 저장합니다.)

---

# 3부: 멀티플레이

## PIE로 서버와 클라이언트 띄우기

1. 플레이 버튼 옆 **⋮** → **Number of Players: 2**, **Net Mode: Play As Listen Server**.
2. Play를 누르면 창이 두 개 뜹니다. 하나는 서버(리슨 서버 = 호스트 플레이어), 하나는 클라이언트입니다. 창 제목과 Output Log 줄 앞의 꼬리표로 구분합니다.
3. **서버 창**을 클릭하고 `~` → `BugHunt.Lab 11`.
4. 랩 출력에는 `[리슨 서버]`, `[클라이언트]` 꼬리표가 붙습니다.

- PIE의 서버와 클라이언트는 **같은 프로세스**입니다. 중단점 하나가 양쪽 모두에서 걸리므로, 멈출 때마다 조사식에 `GetNetMode()`(`NM_ListenServer` / `NM_Client`)나 `HasAuthority()`를 넣어 **지금 누구의 코드인지** 먼저 확인하세요.
- 이 랩들은 **Play As Listen Server**로 하세요. (Play As Client는 창이 없는 데디케이티드 서버를 띄워서, 서버 쪽 콘솔에 랩 명령을 넣기 번거롭습니다.)
- 네트워크 로그: 콘솔 `log LogNet Verbose`, `log LogNetTraffic Verbose`. 복제 상태: `net.ListActors`, `stat net`.

## 3부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 11 | 호스트 HP 바 | 리슨 서버 호스트 화면만 HP 바가 안 바뀐다 (스탠드얼론도) | **RepNotify**가 불리는 곳, `GetNetMode()`로 머신 구분 | ★★ |
| 12 | 보물 상자 | 클라이언트 화면엔 100골드인데 서버엔 0골드 | **권한(Authority)**, 복제 방향, **Server RPC** | ★★★ |

> 랩 12는 스탠드얼론에서는 통과합니다. 버그는 원격 클라이언트에만 있습니다. "내 PC에선 되는데요"의 멀티플레이 버전입니다.

---

# 4부: 사운드

"소리가 늦게 나요", "소리가 끊겨요", "저만 안 들려요"는 게임 개발에서 아주 흔한 신고입니다. 소리 자체는 귀로 들리지만, 버그를 **재현하고 판정**하려면 숫자가 필요합니다. 그래서 이 랩들은 "섬광과 소리 사이의 프레임 수", "0.1초 뒤에도 재생 중인가", "각 머신에서 몇 번 울렸나"를 재서 `[PASS]`/`[FAIL]`을 냅니다.

## 4부에서 쓰는 도구

| 도구 | 하는 법 | 언제 |
|---|---|---|
| `stat audio` | 콘솔 | 활성 소리 수, 오디오 스레드 시간 |
| `stat soundwaves` / `stat soundcues` | 콘솔 | 지금 재생 중인 소리 목록과 거리, 볼륨 |
| `au.Debug.Sounds 1` | 콘솔 | 화면에 재생 중인 소리를 글자로 표시 |
| `au.3dVisualize.Enabled 1` | 콘솔 | 3D 소리의 위치를 뷰포트에 표시 |
| `obj list class=SoundWave` | 콘솔 | 메모리에 올라와 있는 사운드 에셋 (로드/언로드 확인) |
| Unreal Insights | 명령줄 `-trace=default,loadtime` → Tools → Unreal Insights | "누가 언제 무엇을 로드했나", 그 프레임이 얼마나 걸렸나 (첫 재생 히치) |
| 로그 시각·프레임 번호 | `UE_LOG`에 `GFrameCounter`, `FPlatformTime::Seconds()` | 지연을 ms/프레임 숫자로 남기기 |

> 소리가 "Sound Concurrency"(동시 재생 한도) 때문에 묻히는 버그도 흔합니다. 사운드 에셋의 Concurrency 설정(Max Count, Resolution Rule)과 `au.Debug.Sounds`로 확인합니다. 이 랩 세트에는 넣지 않았습니다. Concurrency 판정은 실제 오디오 렌더링 상태에 따라 달라져서 자동으로 PASS/FAIL을 안정적으로 낼 수 없었기 때문입니다.

## 4부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 13 | 블래스터 발사음 | 첫 발(그리고 가끔)만 발사음이 섬광보다 늦다 | **소프트 레퍼런스** 지연 로드, 비동기 로드는 다음 프레임, GC가 언로드, `obj list class=SoundWave` | ★★ |
| 14 | 폭발 통 | 폭발음이 터지자마자 끊긴다 | `SpawnSoundAttached` 컴포넌트의 수명 = 액터의 수명, `au.Debug.Sounds` | ★ |
| 15 | 경보 종 | 리슨 서버 호스트만 종소리를 듣는다 | 소리는 **복제되지 않는다**, 연출용 NetMulticast | ★★ |

- 랩 14는 오디오 장치가 필요합니다. `-nosound`로 실행했거나 출력 장치가 없으면 `[SKIP]`이 나옵니다.
- 랩 15는 랩 11·12처럼 **Number of Players 2, Play As Listen Server**로 실행하고 서버 창에서 `BugHunt.Lab 15`.

---

# 5부: 이미지 / 렌더링

"색이 안 바뀌어요", "아이콘이 다른 아이템 거예요", "색이 기획서랑 달라요"는 눈으로는 바로 보이지만, 눈대중으로는 "조금 다른 것 같다"에서 멈추기 쉽습니다. 이 랩들은 **메시에 끼워진 머티리얼의 파라미터 값**, **이미지 위젯 브러시에 들어간 텍스처**, **8비트 색 값**을 직접 조회해서 `[PASS]`/`[FAIL]`을 냅니다.

## 5부에서 쓰는 도구

| 도구 | 하는 법 | 언제 |
|---|---|---|
| Details 패널의 Materials | PIE 중 F8(Eject) → 액터 선택 | 지금 메시에 **끼워진** 머티리얼이 공유 에셋인지 `MID_...`인지 |
| 머티리얼 인스턴스 에디터 | 머티리얼 더블클릭 → Parameter 목록 | 파라미터 이름(대소문자까지)과 기본값 |
| Widget Reflector | Tools → Debug → Widget Reflector, Pick Hit-Testable Widgets | 화면의 위젯이 어떤 브러시·텍스처를 그리고 있나 |
| `obj list class=Texture2D` | 콘솔 | 텍스처가 메모리에 올라와 있나 (비동기 로드 확인) |
| 텍스처 에디터의 sRGB 체크 | 텍스처 더블클릭 → Details → Texture → sRGB | 색 텍스처는 켜고, 노멀·마스크는 끈다 |
| `stat streaming`, `r.Streaming.*` | 콘솔 | 텍스처 스트리밍 풀, 밉 레벨 (아래 참고) |
| RenderDoc / `ProfileGPU` | 플러그인 RenderDoc 켜기 / 콘솔 | 실제로 그려진 픽셀, 드로 콜 |

> **GPU가 필요해서 넣지 않은 랩:** "텍스처가 흐릿하게 나온다(밉 스트리밍)"와 "렌더 타깃 픽셀 값이 틀리다(readback)"는 흔한 버그지만, 둘 다 실제 GPU 렌더링 결과를 읽어야 판정할 수 있습니다. 이 프로젝트의 자동 검증은 `-nullrhi`(GPU 없이 실행)로 돌기 때문에 이 두 종류는 넣지 않았습니다. 랩 16~18은 GPU 없이 조회되는 값(파라미터, 브러시, 색 변환)으로 판정하므로 `-nullrhi`에서도 결과가 같습니다. 화면으로 직접 확인하고 싶다면 에디터의 PIE에서 실행하세요.

## 5부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 16 | 상자 강조 | 마우스를 올려도 상자가 빨개지지 않는다 (경고도 없음) | **MID**는 만들기만 하면 끼워지지 않는다, `CreateDynamicMaterialInstance`, 공유 머티리얼 | ★ |
| 17 | 인벤토리 아이콘 | 스크롤하면 칸 이름은 맞는데 아이콘이 다른 아이템 것이다 | 비동기 로드 콜백의 **늦은 도착**, 재사용되는 위젯, 프레임 번호로 순서 적기 | ★★ |
| 18 | 팀 색 | 팀 색이 기획서보다 물 빠지고 밝다 (0x00, 0xFF는 맞음) | **sRGB와 Linear**, `FColor` → `FLinearColor` 변환, 텍스처의 sRGB 체크 | ★★ |

- 셋 다 GPU 없이 판정됩니다(`-nullrhi` 검증 완료). 랩 17은 아이콘이 이미 메모리에 있으면 재현되지 않습니다. 에디터에서 해당 텍스처를 열어 본 직후라면 에디터를 다시 켜고 실행하세요.

---

# 6부: 애니메이션

애니메이션 버그는 "가끔 공격이 안 맞아요", "연타하면 캐릭터가 미끄러져요", "죽은 적의 칼에 맞았어요"처럼 **게임 판정**으로 신고됩니다. 공격 판정, 발소리, 상태 전환이 애니메이션 노티파이와 몽타주 이벤트에 걸려 있기 때문입니다. 이 랩들은 화면의 포즈가 아니라 **노티파이가 몇 번 왔나**, **이벤트가 왔을 때 상태값이 무엇인가**로 `[PASS]`/`[FAIL]`을 냅니다.

에셋은 엔진 기본 콘텐츠만 씁니다. 캐릭터는 `/Engine/Tutorial/SubEditors/TutorialAssets/Character/TutorialTPP`, 애니메이션은 같은 폴더의 `Tutorial_Walk_Fwd`(1.83초)입니다. 몽타주와 노티파이는 에디터에서 에셋에 다는 대신 코드로 만듭니다(`Core/BugHuntAnim.h`, 버그 없음). 애님 블루프린트 없이 기본 `UAnimInstance`를 쓰므로 PIE 화면의 캐릭터는 걷는 포즈를 취하지 않습니다. 몽타주 재생, 노티파이, 종료 이벤트는 정상적으로 돕니다.

## 6부에서 쓰는 도구

| 도구 | 하는 법 | 언제 |
|---|---|---|
| `showdebug animation` | 콘솔 (PIE에서 캐릭터를 뷰 타깃으로) | 재생 중인 몽타주, 위치, 가중치, 노티파이 |
| Rewind Debugger | Tools → Debug → Rewind Debugger (PIE 녹화) | 노티파이가 언제 왔는지 타임라인으로 |
| Animation Insights | 명령줄 `-trace=default,animation` → Unreal Insights | 프레임별 몽타주 상태, 노티파이, 포즈 틱 여부 |
| 노티파이에 중단점 | `Notify`, `NotifyBegin/End`, `OnMontageEnded` 핸들러 | 누가 불렀나 (호출 스택), 언제 안 불리나 |
| 프레임 번호 로그 | `UE_LOG`에 `GFrameCounter` | 이벤트 순서를 기록표로 |
| Details → Mesh → Optimization | PIE 중 캐릭터의 스켈레탈 메시 컴포넌트 | `Visibility Based Anim Tick Option` 값 |

## 6부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 19 | 발자국 | 카메라 밖에서 걷는 NPC의 발소리가 0번 난다 | `VisibilityBasedAnimTickOption`, 렌더되지 않으면 포즈 틱과 노티파이가 멈춤 (서버, `-nullrhi`, 카메라 밖) | ★★ |
| 20 | 연타 콤보 | 연타하면 공격 중인데 움직여지고, 콤보가 한 번 더 오른다 | `OnMontageEnded`의 `bInterrupted`, 같은 몽타주를 다시 재생하면 이전 것이 interrupted로 끝남 | ★★ |
| 21 | 히트박스 | 공격이 경직으로 끊긴 뒤에도 칼 히트박스가 켜져 있다 | 노티파이 두 개(On/Off) vs **노티파이 스테이트**, 끊긴 몽타주는 뒤쪽 노티파이를 지나가지 않는다 | ★★ |

- 셋 다 GPU 없이 판정됩니다(`-nullrhi` 검증 완료). 랩 19는 헤드리스 실행에서 "렌더되지 않음"이 확실하므로 늘 재현됩니다. PIE에서는 NPC가 카메라 뒤 4m에 생기므로, 카메라를 돌려 NPC를 화면에 넣으면 그때부터 발자국이 찍히는 것까지 볼 수 있습니다.

---

# 7부: 잘못된 포인터

1부의 크래시는 "죽은 자리"가 있었습니다. 이번 포인터 버그들은 **대부분 죽지 않습니다**. 가리키는 메모리가 아직 남아 있거나(GC 전, 인라인 버퍼), 다른 객체가 그 자리를 다시 쓰고 있기 때문입니다. 그래서 값이 이상하게 나오는 곳에서 거꾸로 "이 포인터는 지금 무엇을 가리키나"를 확인해야 합니다.

1부와 2부에서 이미 다룬 것은 다시 다루지 않습니다: `Cast` 실패 nullptr(랩 1), `UPROPERTY` 없는 멤버의 GC(랩 4), `Destroy` 뒤의 포인터와 `IsValid`(랩 9).

## 7부에서 쓰는 도구

| 도구 | 하는 법 | 언제 |
|---|---|---|
| 조사식에 **주소** 적어 두기 | 조사식 `Kael` → 값 칸의 `0x...` 를 메모장에 | "같은 객체인가?"를 주소로 비교 (랩 22, 24) |
| `&변수` 조사식 | `&Potions`, `&Slots[0]` | 참조가 가리키는 위치가 배열의 진짜 위치와 같은가 (랩 24) |
| **데이터 중단점** | 디버그 → 새 중단점 → 데이터 중단점, 주소 `&Slots[0].Count`, 4바이트 | 진짜 값을 누가 바꾸는지 / 아무도 안 바꾸는지 (랩 24) |
| natvis | 조사식에서 `TWeakObjectPtr`, `TSharedPtr` 를 펼침 | 약한 포인터가 가리키는 객체, 공유 포인터의 참조 수 (랩 22, 25) |
| 생성자/소멸자 중단점 | 소멸자 첫 줄 F9 | 지워져야 할 것이 지워지나 (랩 25) |
| `obj list class=...`, `obj refs name=...` | 콘솔 | UObject 가 아직 살아 있나, 누가 붙잡나 |
| `-stomp2malloc` | 명령줄 (명령줄 옵션 표 참고) | 해제된 힙에 쓰는 순간 그 자리에서 멈추게 하기 (랩 24 를 기본 `TArray`로 바꿔서 해 보세요) |

## 7부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 22 | 어그로 표 | 새로 온 영웅이 죽은 영웅의 어그로를 물려받는다 | UObject 가 아닌 구조체에서 액터 기억하기, **`TWeakObjectPtr`**, 주소 재사용(ABA) | ★★ |
| 23 | 포탑 | 부순 포탑이 계속 쏜다 | 타이머/델리게이트 람다의 `[this]`, **`CreateWeakLambda` / `CreateUObject`**, EndPlay 가 지우는 타이머 | ★★ |
| 24 | 물약 벨트 | 전리품을 주운 뒤로 물약이 줄지 않는다 (로그에는 줄어든다고 찍힘) | **`TArray` 원소 참조 + `Add` 재할당**, `TInlineAllocator`, 데이터 중단점 | ★★ |
| 25 | 대화 트리 | 대화가 끝나도 대화 노드가 메모리에 남는다 | **`TSharedPtr` 순환 참조**, `TWeakPtr`, 참조 수 보기 | ★★ |

- 넷 다 GPU 없이 판정됩니다(`-nullrhi` 검증 완료). 랩 22 는 할당기가 같은 주소를 다시 쓰면 "Bram 어그로 60", 다른 주소를 주면 "보스가 지워진 영웅을 노림"으로 나타납니다. 어느 쪽이든 FAIL 입니다.

---

# 8부: 멈춤 (무한루프와 데드락)

크래시도 오류 메시지도 없이 **게임이 굳는** 버그입니다. 신고는 "멈췄어요" 한 마디뿐이고, 로그도 굳기 직전에서 끊깁니다. 이럴 때는 굳은 프로그램을 **디버거로 멈춰 세워서** 각 스레드가 지금 어디서 무엇을 하는지 봅니다.

## 안전장치: 이 랩들은 진짜로 굳지 않습니다

진짜로 굳으면 에디터를 강제 종료해야 하므로, 8부의 루프와 대기에는 **시간 예산**이 붙어 있습니다(`Core/BugHuntHang.h`, 버그 없음).

```
LogBugHunt: [FAIL] 응답 없음 (2003 ms / 213회 초과) - 엘리베이터 기다리기
```

- 예산(기본 **2초**)을 넘기면 위 줄을 찍고 그 루프나 대기에서 **빠져나옵니다**. 그 뒤 코드는 버그가 만든 상태 그대로 계속 돕니다. 채점기는 이 랩을 TIMEOUT 으로 매깁니다.
- 안전장치는 버그를 가리지 않습니다. 정상 코드는 예산의 1%도 쓰지 않고, 걸리면 다른 검사가 어떻게 나오든 그 랩은 실패입니다.
- `BH_HANG_CHECK(Guard)` 줄이나 `BugHunt::WaitWithBudget(...)`(= `Future.Get()`)을 지우지 마세요. 지우면 에디터가 진짜로 굳습니다. 버그는 그 줄이 아니라 루프 조건과 기다리는 방식에 있습니다.
- 예산은 콘솔 변수 **`BugHunt.HangBudgetSeconds`** 로 바꿉니다. "모두 중단" 실습을 하려면 `BugHunt.HangBudgetSeconds 30` 으로 늘린 뒤 랩을 실행하세요. 30초 동안 진짜로 굳어 있습니다.

## 굳은 프로그램을 추적하는 법

1. 콘솔 `BugHunt.HangBudgetSeconds 30` → `BugHunt.Lab 26`. 게임 화면이 굳습니다.
2. VS 에서 **디버그 → 모두 중단 (Ctrl+Alt+Break)**. 프로그램 전체가 그 순간에 멈춥니다. (에디터 창이 아니라 VS 창에서 누르세요.)
3. **디버그 → 창 → 스레드 (Ctrl+Alt+H)**. 수십 개의 스레드 중 **GameThread** 를 더블클릭하면 호출 스택이 게임 스레드로 바뀝니다. 모두 중단 직후의 "현재 스레드"는 게임 스레드가 아닐 수 있습니다.
4. 호출 스택에서 처음 나오는 `BugHuntUE.dll!` 줄을 더블클릭. 지역 창/조사식으로 루프 조건에 쓰인 값을 봅니다.
5. **F5 → 다시 모두 중단**을 몇 번 반복합니다. 매번 같은 루프 안이고, 조건에 쓰인 값이 **변하지 않거나 끝없이 커지면** 무한루프입니다.
6. 스레드가 둘 이상 얽혔다면 **디버그 → 창 → 병렬 스택 (Ctrl+Shift+D, S)**. 모든 스레드의 호출 스택이 한 그림에 나옵니다. 두 스레드가 서로 상대의 결과(잠금, Future, 이벤트)를 기다리고 있으면 데드락입니다.

| 도구 | 하는 법 | 언제 |
|---|---|---|
| **모두 중단** | VS: 디버그 → 모두 중단 (Ctrl+Alt+Break). Rider: Pause | 굳은 순간을 세우기 |
| **스레드 창** | Ctrl+Alt+H, GameThread 더블클릭, 우클릭 → 플래그 | 어느 스레드가 어디서 돌거나 기다리나 |
| **병렬 스택** | Ctrl+Shift+D, S | 데드락: 누가 누구를 기다리나 (랩 28) |
| 다음 문 설정 | 루프 뒤 줄에서 우클릭 → 다음 문 설정 (Ctrl+Shift+F10) | 굳은 루프에서 강제로 빠져나와 계속 관찰하기 (위험: 상태가 꼬일 수 있음) |
| **Unreal Insights** | 명령줄 `-trace=default` → Tools → Unreal Insights → Timing | 몇 번째 프레임이 몇 ms 걸렸나, 그 프레임 안에서 무엇이 시간을 먹었나. 굳기 전후의 긴 프레임을 찾는다 |
| `stat unit`, `stat game` | 콘솔 | 게임 스레드 ms 가 갑자기 튀는가 (히치) |
| 행 감지기 (`FThreadHeartBeat`) | `DefaultEngine.ini` `[Core.System]` `HangDuration=초` | **패키징한 게임**에서 게임 스레드가 N초 동안 하트비트를 안 보내면 그 스레드의 호출 스택을 로그에 남기고 크래시 리포트를 만든다. 에디터 빌드와 디버그 빌드에서는 꺼져 있고(엔진 `Build.h`의 `USE_HANG_DETECTION`), 엔진 기본값도 `HangDuration=0`(끔)이다. 디버거가 붙어 있으면 동작하지 않는다 |
| 히치 감지 | 패키징한 게임에서 명령줄 `-hitchdetection=0.1` (`ALLOW_HITCH_DETECTION`로 빌드했을 때) | 한 프레임이 0.1초를 넘으면 "Hitch detected on gamethread" 로그 |

> 8부 랩은 에디터 빌드에서 돌기 때문에 엔진의 행 감지기가 동작하지 않습니다. `Core/BugHuntHang.h`가 같은 일을 랩 안에서 대신합니다. 실제 프로젝트에서는 QA 빌드(Development, 패키징)에 `HangDuration`을 켜 두면 "멈췄어요" 신고에 호출 스택이 따라옵니다.

## 8부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 26 | 엘리베이터 | 엘리베이터를 부르면 게임이 굳는다 | 게임 스레드에서 **다른 액터의 Tick 이 바꿔 줄 값을 while 로 기다리기**, 모두 중단, 스레드 창, 델리게이트로 바꾸기 | ★ |
| 27 | 길찾기 | 끊긴 다리 너머로 보내면 게임이 굳는다 (갈 수 있는 곳은 잘 감) | 너비 우선 탐색의 **방문 체크 누락**(지역 변수 수명), 도달 불가능한 입력으로 테스트하기 | ★★ |
| 28 | 자동 저장 | 자동 저장할 때 게임이 굳는다 | 게임 스레드에서 **비동기 결과를 동기로 기다림** → 그 작업이 게임 스레드를 기다리는 **데드락**, 병렬 스택, `AsyncTask(GameThread)` 로 결과 받기 | ★★★ |

- 셋 다 GPU 없이 판정됩니다(`-nullrhi` 검증 완료).
- 랩 28 에서 데드락이 풀려도 "게임이 멈춘 시간 < 50 ms" 검사가 남습니다. 결과를 기다리느라 게임 스레드를 세우는 것 자체가 히치이기 때문입니다.

---

# 9부: 컨테이너

`TArray`, `TMap`, `TSet` 은 표준 라이브러리 컨테이너와 비슷해 보이지만 규칙이 조금씩 다릅니다. 범위 검사가 **개발 빌드에서도 켜져 있어서** 잘못된 인덱스는 조용히 넘어가지 않고 assert 로 죽고(랩 29, 30), 반대로 "원소의 주소"나 "순서"나 "해시"에 기댄 코드는 죽지 않고 값만 틀립니다(랩 31~33). 마지막 두 랩은 컨테이너가 **GC**와 **복제**를 만났을 때입니다.

이미 다룬 것은 다시 다루지 않습니다: 순회 중 제거(랩 3), `TArray` 원소 참조를 쥔 채 `Add`(랩 24).

## 9부에서 쓰는 도구

| 도구 | 하는 법 | 언제 |
|---|---|---|
| assert 메시지 읽기 | `Assertion failed: (Index >= 0) & (Index < ArrayNum)` → 호출 스택에서 `Array.h` 아래 첫 `BugHuntUE.dll!` 줄 | 범위 밖 인덱스 (랩 29). 조사식에 `Wave`, `Rewards.Num()` 을 넣어 비교 |
| `Pair != nullptr` | `Map.h.inl` 의 `FindChecked` → 호출 스택을 한 줄 올라가면 `operator[]` | 없는 키로 `Map[Key]` (랩 30) |
| natvis 로 `TMap`/`TSet` 펼치기 | 조사식 `Kills`, `Revealed` | 원소 수, 키와 값 |
| 주소 비교 | 조사식 `TargetKills` 의 주소와, `Kills` 를 펼쳐 본 Wolf 원소의 주소를 나란히 | 미리 받아 둔 포인터가 아직 진짜 원소를 가리키나 (랩 31) |
| 중복 키 찾기 | 조사식 `Revealed` 를 펼쳐 같은 (X,Y) 가 두 번 들어 있는지 본다 | `==` 와 `GetTypeHash` 가 어긋남 (랩 33) |
| `obj list class=BH_LootItem` | 콘솔. GC 전후 비교 | 컨테이너 속 UObject 가 살아 있나 (랩 34) |
| `obj refs name=...` | 콘솔 | 그 객체를 누가 붙잡고 있나 (UPROPERTY 컨테이너만 보임) |
| Networking Insights | 명령줄 `-NetTrace=1 -trace=net` → Unreal Insights 에서 열기 | 어떤 프로퍼티가 몇 바이트로 언제 복제됐나 (랩 35) |

## 9부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 29 | 웨이브 보상 | 보상을 받으면 크래시한다 | `TArray` **범위 검사 assert**, 1부터 세는 번호와 0부터 세는 인덱스 | ★ |
| 30 | 상점 시세표 | 가격표에 없는 물건을 팔면 크래시한다 | **`TMap::operator[]` = `FindChecked`** (없는 키를 만들지 않음, `std::map` 과 다름), `Find`/`FindRef`/`Contains` | ★ |
| 31 | 현상금 게시판 | 새 종류의 몬스터를 잡은 뒤로 의뢰 대상 집계가 늘지 않는다 | **`TMap::Find` 포인터 + `Add`/`FindOrAdd` 재할당**, `TInlineSetAllocator` | ★★ |
| 32 | 턴 순서 | 유닛 하나가 쓰러지면 행동 순서가 뒤섞인다 | **`RemoveAtSwap`/`RemoveSwap`** 은 순서를 지키지 않는다, `RemoveAt`/`Remove` 와 비용 차이 | ★ |
| 33 | 전장의 안개 | 같은 칸이 두 번 밝혀지고, 밝힌 칸을 물으면 아니라고 한다 | **`GetTypeHash` 와 `operator==` 가 같은 필드를 봐야 한다**, 키를 넣은 뒤 바꾸면 안 되는 이유 | ★★ |
| 34 | 전리품 가방 | 시간이 지나면 가방 속 아이템이 사라진다 | `UPROPERTY` 없는 **`TArray<UObject*>`** 와 GC, `TArray<TObjectPtr<>>`, `TArray<TWeakObjectPtr<>>` | ★★ |
| 35 | 퀘스트 게시판 | 클라이언트의 퀘스트 진행도가 0 에서 멈춘다 (퀘스트 이름은 보임) | **`FFastArraySerializer`** 는 비교하지 않는다, `MarkItemDirty`/`MarkArrayDirty`, 일반 복제 `TArray` 와 차이 | ★★★ |

- 일곱 개 다 GPU 없이 판정됩니다(`-nullrhi` 검증 완료). 랩 29, 30 은 CRASH 가 버그 상태입니다.
- 랩 31 은 `TInlineSetAllocator<4>` 로 "다섯 번째 종류가 들어오는 순간"에 원소가 옮겨지게 해 두었습니다. 기본 할당기라면 언제 옮겨질지 정해져 있지 않아서, 같은 버그가 몇 시간 뒤에 엉뚱한 값으로 나타납니다.
- 랩 33 의 키를 넣은 뒤 `X` 를 바꾸는 것도 같은 버그입니다. 원소는 옛 해시의 버킷에 남아 있어서 새 좌표로 찾으면 없고, 옛 좌표로 찾아도 `==` 가 틀려서 없습니다. 키를 바꿔야 하면 빼고 다시 넣으세요.
- 랩 35 는 랩 11·12·15 처럼 **Number of Players 2, Play As Listen Server** 로 실행하고 서버 창에서 `BugHunt.Lab 35`. 스탠드얼론에서는 통과합니다.

---

# 10부: 프레임레이트와 초기화

"내 PC 에선 되는데요"의 두 가지 단골 원인입니다. 하나는 **프레임레이트**(30fps 노트북과 144Hz 모니터에서 게임이 다르게 움직임), 하나는 **초기화 안 된 값**(빌드나 순서에 따라 쓰레기 값이 달라짐)입니다.

## 프레임레이트를 바꿔 가며 실행하기

| 도구 | 하는 법 | 언제 |
|---|---|---|
| `t.MaxFPS N` | 콘솔. `t.MaxFPS 30` → 실행 → `t.MaxFPS 120` → 실행 → `t.MaxFPS 0`(제한 없음) | 프레임레이트를 바꿔 같은 장면을 비교 |
| 고정 시간 간격 | 명령줄 `-benchmark -fps=30` (코드: `FApp::SetUseFixedTimeStep(true)`, `FApp::SetFixedDeltaTime(1.0/30)`) | 실제 시간과 상관없이 매 프레임 `DeltaSeconds` 를 정확히 1/N 으로. 느린 PC 에서도 결과가 같다. 랩 36, 37 이 이 방식 (`Core/BugHuntProbe.h`) |
| `stat fps`, `stat unit` | 콘솔 | 지금 실제 프레임레이트와 프레임 시간 |
| 추적점 `{DeltaSeconds}` | Tick 첫 줄 | 프레임 간격이 얼마인지, Tick 이 1초에 몇 번 불리는지 |

## 초기화 안 된 값

| 무엇 | 처음 값 |
|---|---|
| UObject(액터, 컴포넌트)의 멤버 | **0** (메모리를 0 으로 채운 뒤 생성자를 부름). 그래도 초기값을 적는 것이 규칙 |
| `USTRUCT`/일반 구조체의 멤버, 지역 변수 | **아무 값** (초기값을 안 적으면) |
| `FMemory::Malloc`, `TArray::AddUninitialized`/`SetNumUninitialized` | **아무 값** |
| `TArray::AddDefaulted` | 구조체의 기본 생성자대로. 초기값이 없는 멤버는 **아무 값** (`AddZeroed` 는 0) |

- "아무 값"은 그 메모리를 직전에 누가 썼느냐에 달려 있어서, Debug 와 Development 빌드, 에디터와 패키징, 실행 순서에 따라 달라집니다. 그래서 "가끔", "내 PC 에서만" 나타납니다.
- 언리얼은 시작할 때 USTRUCT 의 `UPROPERTY` 기본값을 검사해서, 초기값이 빠진 것을 Output Log 에 빨간 줄로 남깁니다:
  `LogClass: Error: IntProperty FBH_RoundStats::Kills is not initialized properly. Module:BugHuntUE File:Labs/Lab38_RoundStats.h`
  이 저장소에서는 랩 38 이 고쳐질 때까지 에디터를 켤 때마다 이 줄이 보입니다. `UPROPERTY` 가 없는 멤버와 지역 변수는 검사하지 않습니다.
- 데이터 중단점(2부)으로 "이 값을 처음 쓴 게 누구인가"를 찾으면, 아무도 쓰지 않았다는 것을 확인할 수 있습니다.

## 10부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 36 | 돌진 | 30fps 에서는 반만, 120fps 에서는 두 배로 돌진한다 | 이동에 **`DeltaSeconds`** 빠짐, `t.MaxFPS`, 고정 시간 간격 | ★ |
| 37 | 얼음 미끄럼 | 30fps 에서는 멀리, 120fps 에서는 짧게 미끄러진다 (이동에는 `DeltaSeconds` 를 곱했는데도) | **프레임당 감속**(`Velocity *= 0.9`), `FMath::Pow`/`Exp` 로 시간 기준 감속, `FInterpTo` | ★★ |
| 38 | 라운드 점수판 | 새 라운드인데 지난 라운드 킬 수가 남아 있다 | 구조체 멤버 **초기값 없음**, UObject 는 0, `AddDefaulted` vs `AddZeroed`, `Reset` 이 남긴 메모리 | ★★ |

- 셋 다 GPU 없이 판정됩니다(`-nullrhi` 검증 완료). 랩 36, 37 은 한 번 실행에 30fps 와 120fps 를 차례로 돌립니다.

---

# 11부: 물리

물리 버그는 "가끔 뚫고 지나간다", "가끔 안 눌린다"로 신고됩니다. 같은 장면을 프레임 단위로 되감아 보는 도구가 핵심입니다. 이 부의 랩은 모두 **고정 60fps**로 돌아서 결과가 늘 같습니다.

## 11부에서 쓰는 도구

| 도구 | 하는 법 | 언제 |
|---|---|---|
| `show Collision` | 콘솔 (PIE 중) | 충돌 모양이 실제로 어디 있는지 |
| `p.Chaos.DebugDraw.Enabled 1` | 콘솔. `p.Chaos.DebugDraw.ShowCollisionParticles 1` 등 하위 옵션 | 물리 엔진이 보는 물체와 접촉점을 화면에 그림 (예전 PhysX 의 `pxvis collision` 에 해당) |
| **Chaos Visual Debugger** | Tools → Debug → Chaos Visual Debugger → 녹화 → PIE → 정지 → 프레임 넘기기 | 물리 프레임마다 위치, 속도, 충돌을 되감아 보기 (랩 39, 42) |
| **Visual Logger** | 2부와 같음. `UE_VLOG_LOCATION`, `UE_VLOG_SEGMENT` 로 위치와 이동 경로를 남김 | "이 프레임에 어디서 어디로 갔나" |
| 디테일 패널 **Collision** | PIE 일시 정지 → World Outliner 에서 선택 | 채널 응답(Ignore/Overlap/Block), Generate Overlap Events, Simulation Generates Hit Events (랩 40) |
| `Collision` 프로필 | Project Settings → Collision → Preset | `BlockAll`, `OverlapAll`, `Trigger`, `PhysicsActor` 가 채널별로 어떻게 응답하는지 |
| 서브스테핑 | Project Settings → Physics → Substepping, Max Substep Delta Time | 낮은 프레임레이트에서도 물리 한 단계를 작게. 프레임레이트에 따라 물리 결과가 달라질 때 |

Block 과 Overlap 규칙: 두 물체의 서로에 대한 응답 중 **약한 쪽**이 이깁니다(Ignore < Overlap < Block). 둘 다 Block 이면 Hit, 하나라도 Overlap 이면 Overlap(그리고 **둘 다** Generate Overlap Events 가 켜져 있어야 이벤트), 하나라도 Ignore 면 아무 일도 없습니다.

## 11부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 39 | 저격탄 | 탄환이 얇은 철판을 뚫고 지나간다 | **터널링**, 연속 충돌 검출 **`bUseCCD`**, 히트스캔/스윕과 비교 | ★ |
| 40 | 압력판 | 상자를 올려 놓아도 압력판이 눌리지 않는다 | 채널 응답, **Generate Overlap Events 는 양쪽 다**, `AStaticMeshActor` 가 끄는 기본값 | ★★ |
| 41 | 넉백 | 밀려난 거한이 성벽 너머에 서 있다 | **`SetActorLocation` 의 `bSweep=false`(순간이동)**, 스윕과 `FHitResult` | ★ |
| 42 | 투석기 | 투석기가 던진 바위가 그 자리에 뚝 떨어진다 | 물리 시뮬레이션 중인 물체를 **Tick 에서 위치로 옮기기**, 물리 속도, `SetPhysicsLinearVelocity`/힘/PhysicsHandle | ★★ |

- 넷 다 GPU 없이 판정됩니다(`-nullrhi` 에서 Chaos 물리가 그대로 돈다는 것을 확인했습니다).

---

# 12부: 데이터 레이스

작업 스레드(`AsyncTask`, `ParallelFor`, `UE::Tasks`)가 같은 데이터를 잠금 없이 만지는 버그입니다. 진짜 레이스는 "백 번에 한 번"이라 실습으로 재현하기 어려우므로, 이 부의 랩에는 레이스가 일어날 틈을 넓히는 **확대경** `BugHunt::RaceWindow()` 가 있습니다(`Core/BugHuntProbe.h`, 버그 없음). 이 줄에 도착한 스레드는 다른 스레드가 같은 줄에 올 때까지 잠깐 기다리므로, 보호되지 않은 코드에서는 **매번** 겹칩니다. 지워도 버그는 남고 "가끔"이 될 뿐입니다.

## 12부에서 쓰는 도구

| 도구 | 하는 법 | 언제 |
|---|---|---|
| **스레드 창** | 디버그 → 창 → 스레드 (Ctrl+Alt+H). 스레드 이름(GameThread, Foreground Worker, Background Worker...) | 지금 멈춘 코드가 어느 스레드에서 도는가 |
| **병렬 스택** | Ctrl+Shift+D, S | 같은 함수에 동시에 서 있는 스레드가 몇 개인가 |
| 추적점 `{FPlatformTLS::GetCurrentThreadId()}` | 중단점 → 작업 | 스레드 번호와 값을 같이 찍어 순서 보기 |
| **Unreal Insights** | 명령줄 `-trace=default,task` → Timing 창의 스레드 트랙 | 어떤 작업이 어느 스레드에서 언제 겹쳐 돌았나 |
| `check(IsInGameThread())` | 게임 스레드 전용 코드 맨 위 | 다른 스레드에서 들어오면 그 자리에서 멈춤 (엔진 곳곳에 있음) |
| `FCriticalSection` + `FScopeLock` | `FScopeLock Lock(&CS);` 블록 끝에서 자동 해제 | 읽기-계산-쓰기를 한 덩어리로 |
| `std::atomic` / `TAtomic` | `Count.fetch_add(1)` | 숫자 하나를 여러 스레드가 더할 때 |
| `AsyncTask(ENamedThreads::GameThread, ...)` | 작업 스레드에서 게임 스레드로 결과 보내기 | UObject 를 고쳐야 할 때 (랩 28, 44) |

## 12부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 43 | 길드 금고 | 동시에 입금하면 골드가 모자란다 | `ParallelFor` 와 **읽기-계산-쓰기 레이스(lost update)**, `FCriticalSection`/`FScopeLock`, 원자 연산, 결과를 칸별로 모으기 | ★★ |
| 44 | 하이라이트 점수 | 리플레이 분석이 끝나면 `IsInGameThread` assert 로 죽는다 | **게임 스레드 전용 객체를 작업 스레드에서 만짐**, 스레드 창, `AsyncTask(GameThread)` | ★ |

- 둘 다 GPU 없이 판정됩니다. 랩 44 는 버그 상태에서 CRASH 입니다.
- 8부의 랩 28(데드락)도 같은 스레드 도구를 씁니다.

---

# 13부: 성능

성능 버그는 결과가 맞습니다. "느려요", "멈칫해요"만 있습니다. 그래서 이 부의 랩은 **시간 대신 횟수**(검색 횟수, 메모리 할당 횟수, Tick 호출 횟수, 동기 로드 횟수)로 판정합니다. 시간은 PC 마다 다르지만 횟수는 같기 때문입니다. 실제 게임에서는 아래 도구로 시간을 먼저 보고, 그 안에서 횟수를 셉니다.

## 13부에서 쓰는 도구

| 도구 | 하는 법 | 언제 |
|---|---|---|
| `stat unit` | 콘솔 | Frame / Game / Draw / GPU 중 어느 스레드가 느린가 |
| `stat game` | 콘솔 | 게임 스레드 안에서 Tick, 타이머 등이 쓰는 시간 |
| `stat startfile` / `stat stopfile` | 콘솔. `Saved/Profiling/` 에 파일 | 몇 초 동안의 통계를 기록해 나중에 보기 |
| **Unreal Insights** | 명령줄 `-trace=cpu` (할당까지: `-trace=cpu,memalloc`, 로드까지: `-trace=cpu,loadtime`) → Tools → Unreal Insights | 긴 프레임을 골라 그 안의 함수별 시간을 보기. 가장 먼저 쓰는 도구 |
| `dumpticks` | 콘솔 | 등록된 틱 함수 수와 목록 (랩 47) |
| `obj list class=...` | 콘솔 | 무엇이 메모리에 올라와 있나 (랩 48) |
| `log LogStreaming Verbose` | 콘솔 | 에셋 로드가 언제 일어나는지 로그로 |

## 13부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 45 | 미니맵 | 아이콘이 많으면 게임이 느려진다 | Tick 마다 **`GetAllActorsOfClass`**, BeginPlay 에서 찾아 기억하기 | ★ |
| 46 | 조준선 | 조준하는 동안 메모리 할당이 잦다 | 프레임마다 지역 `TArray` 재할당, **`Reserve`**, 멤버 + `Reset`, `TInlineAllocator` | ★ |
| 47 | 횃불 | 횃불 수백 개를 세우면 느려진다 | 할 일 없는 **Tick 끄기**, 타이머, `TickInterval`, Significance Manager | ★ |
| 48 | 보스 등장 | 보스가 나오는 순간 화면이 멈칫한다 | 게임 중 **동기 로드**(`LoadSynchronous`/`LoadObject`), `RequestAsyncLoad` 로 미리 불러 두기 | ★★ |

- 넷 다 GPU 없이 판정됩니다. 판정은 횟수로만 합니다(로그의 ms 는 참고용).

---

# 채점기

모든 랩을 헤드리스(GPU 없이 `-nullrhi`)로 돌려 랩별 결과와 총점을 냅니다. 에디터를 닫고 프로젝트 폴더에서:

```bat
grade.bat
```

먼저 `build.bat`으로 빌드한 뒤, 랩마다 **새 게임 프로세스**를 띄워 `-BugHuntLab=N -BugHuntExit`로 실행하고 그 로그를 읽습니다. 랩 하나에 프로세스 시작을 포함해 25~35초(멀티플레이 랩은 45초 안팎), 48개 전체는 25분 안팎 걸립니다.

```
  #  부   결과       시간  랩  - 내용
  1  1부  CRASH     35.8s  회복 아이템  - Unhandled Exception: EXCEPTION_ACCESS_VIOLATION reading address 0x0000000000000470
  5  2부  FAIL      27.6s  전투 기록 창  - 다시 연 창의 기록 줄 수 (기대: 1, 실제: 2) 외 1개
 11  3부  FAIL      47.1s  호스트 HP 바  - [리슨 서버] 화면의 HP 바 (기대: 79, 실제: 100)
 22  7부  FAIL      25.3s  어그로 표  - Bram 의 어그로 (Bram 이 준 피해) (기대: 23, 실제: 68) 외 1개
 26  8부  TIMEOUT   36.1s  엘리베이터  - 응답 없음 (2005 ms / 212회 초과) - 엘리베이터 기다리기
 ...
점수: 0 / 28  (SKIP 0개 제외)
```

| 결과 | 뜻 |
|---|---|
| PASS | 랩의 검사가 모두 통과 |
| FAIL | `[FAIL]` 이 하나라도 있음 (내용 칸은 첫 번째 FAIL) |
| TIMEOUT | 8부 안전장치가 "응답 없음"을 보고했거나, 랩 프로세스가 제한 시간(기본 180초) 안에 끝나지 않아 채점기가 종료함 |
| CRASH | 프로세스가 크래시(`Unhandled Exception`, `Assertion failed`, `Fatal error`)했거나 결과 줄 없이 끝남 |
| SKIP | 이 환경에서 판정할 수 없음 (예: 오디오 장치가 없는데 랩 14). GPU 렌더링 결과가 필요한 랩도 SKIP 하지만 지금 48개 중에는 없습니다 |

점수는 `PASS 수 / (전체 - SKIP)` 입니다.

- **숨은 입력:** 채점기는 실행할 때마다 무작위 시드를 정해 `-BugHuntSeed=시드`로 넘깁니다. 이를 지원하는 랩(5, 11, 22~48 대부분)은 README 에 적힌 공개 입력과 **다른 값**(피해량, 개수, 그래프 모양, 시간)으로 돕니다. 기대값을 코드에 박아 넣은 "수정"은 여기서 걸립니다. 같은 입력을 PIE 에서 재현하려면 표 위에 찍힌 시드로 `BugHunt.Lab 22 시드`.
- **멀티플레이 랩(11, 12, 15, 35)** 은 3부에서 설명한 자동 검증 방식 그대로 리슨 서버 프로세스(`?listen -BugHuntWaitForClient`)와 클라이언트 프로세스(`127.0.0.1:포트`)를 띄웁니다. 클라이언트 쪽 `[FAIL]` 도 그 랩의 FAIL 로 셉니다.
- **결과 파일:** `grade-report.json` (프로젝트 폴더, 다른 BugHunt 판과 같은 형식). 랩별 로그는 `Saved/Grade/LabNN.log`, 클라이언트는 `LabNN_client.log`.

```json
{ "edition": "unreal", "version": "1.0", "total": 48, "passed": 0,
  "labs": [ { "id": 1, "title": "회복 아이템", "part": 1, "result": "CRASH", "ms": 35800, "detail": "Unhandled Exception: EXCEPTION_ACCESS_VIOLATION ..." } ] }
```

| 옵션 | 하는 일 |
|---|---|
| `grade.bat -labs=22-28` | 일부 랩만 (예: `-labs=1-4,22`) |
| `grade.bat -seed=1234` | 숨은 입력 시드 고정 (같은 결과를 다시 보기) |
| `grade.bat -public` | 숨은 입력 대신 공개 입력으로 |
| `grade.bat -nobuild` | 빌드 건너뛰기 |
| `grade.bat -timeout=300` | 랩 하나의 제한 시간(초) |
| `grade.bat -gpu` | `-nullrhi` 없이 실행 (GPU 가 필요한 랩도 채점) |
| `set UE_ROOT=D:\Epic\UE_5.8` | 엔진 위치 바꾸기 (기본 `F:\Program Files\UE_5.8`, `build.bat`도 같음) |

채점기 자체는 커맨드렛입니다(`Core/BugHuntGradeCommandlet.cpp`). `grade.bat` 없이 직접 부르려면 `UnrealEditor-Cmd.exe BugHuntUE.uproject -run=BugHuntGrade -labs=22-28`.

> 채점기는 정답을 알지 못합니다. 랩이 스스로 찍는 `[PASS]`/`[FAIL]`을 모을 뿐입니다. 랩의 검사 코드(`StartLab`의 `Expect` 줄)나 `Core/`를 고쳐서 통과시키는 것은 반칙입니다.

---

# 부록: Live Coding 함정

Live Coding(Ctrl+Alt+F11)은 **실행 중인 에디터의 함수 코드를 바꿔치기**합니다. 이미 만들어진 객체, 이미 등록된 클래스 정보는 바꾸지 못합니다. 직접 겪어 보세요.

1. 에디터를 켠 채로 `Lab01_Pickup.h`에서 `int32 Health = 10;`을 `int32 Health = 40;`으로 바꾸고 Ctrl+Alt+F11.
   → 헤더 변경이라 Live Coding이 거부하거나 경고합니다. Output Log의 **LogLiveCoding** 줄을 읽어 보세요.
2. 이번엔 `Lab09_Archer.cpp`에서 `constexpr int32 KillReward = 100;`을 `50`으로 바꾸고 Ctrl+Alt+F11 → PIE에서 랩 9. 반영되나요? (함수 본문/`.cpp` 상수는 대체로 반영됩니다.)
3. `Lab12_Chest.h`에 `UFUNCTION(Server, Reliable)`을 추가하고 Ctrl+Alt+F11. 무슨 일이 생기나요? → 랩 12의 정답은 **에디터를 닫고 빌드**해야만 적용됩니다.
4. 생성자 안의 값(예: 랩 7의 `PrimaryActorTick.TickGroup`)을 바꾸고 Live Coding → 랩 7. 반영되나요? 에디터를 껐다 켜면요?
   생성자는 **클래스 기본 객체(CDO)** 를 만들 때 이미 한 번 실행됐고, 새 객체는 그 CDO에서 값을 복사해 옵니다.

**규칙:** 헤더, `UPROPERTY`/`UFUNCTION`/`UCLASS`, 생성자, 새 파일 → **에디터 닫고 빌드**. "고쳤는데 안 바뀌어요"면 먼저 이걸 의심하세요.
(Hot Reload, 즉 에디터의 옛 "Compile" 버튼은 UE5에서 Live Coding으로 대체됐습니다. 예전 자료에 나오는 Hot Reload 문제도 원인은 같습니다.)

---

## 콘솔 명령 모음

| 명령 | 하는 일 |
|---|---|
| `BugHunt.List` / `BugHunt.Lab N` | 랩 목록 / 랩 실행 |
| `BugHunt.Lab N 시드` | 랩 N 을 채점기의 숨은 입력(시드)으로 실행. 시드 0 은 공개 입력 |
| `BugHunt.HangBudgetSeconds 초` | 8부 안전장치의 시간 예산 (기본 2). "모두 중단" 실습 때 30 |
| `ListTimers` | 살아 있는 타이머 |
| `dumpticks` | Tick 함수와 그룹 |
| `getall 클래스 속성` | 그 클래스 모든 객체의 속성 값 |
| `obj list class=클래스` | 살아 있는 객체 목록 |
| `obj gc` | 지금 GC 실행 |
| `obj refs name=객체이름` | 그 객체를 참조하는 경로 (왜 안 지워지나) |
| `gc.CollectGarbageEveryFrame 1` | 매 프레임 GC → GC 버그를 바로 드러냄 (느려짐) |
| `stat fps` / `stat unit` / `stat game` | 프레임, 스레드별 ms, Tick 시간 |
| `log 카테고리 Verbose` | 그 카테고리 로그 상세도 올리기 |
| `showdebug` | 화면에 디버그 정보 (카메라, 입력 등) |

## 명령줄 옵션

| 옵션 | 하는 일 |
|---|---|
| `-game` | 에디터 대신 게임으로 바로 실행 |
| `-BugHuntLab=N` | 월드가 시작되면 랩 N 자동 실행 |
| `-BugHuntExit` | 랩이 끝나면 종료 (자동 검증용) |
| `-BugHuntSeed=S` | 숨은 입력(시드 S)으로 실행 (채점기가 쓴다) |
| `-stomp2malloc` | 해제된 메모리/범위 밖 접근을 그 자리에서 잡는 할당기 (언리얼판 ASan, 에디터 빌드 전용). 아주 느리고 메모리를 많이 씀. `-MallocStomp2MinSize=`, `-MallocStomp2MaxSize=`로 크기 범위를 좁힐 수 있음 |
| `-log` | 별도 로그 창 띄우기 |

## 다시 풀고 싶을 때

이 폴더는 git 저장소입니다. 고친 뒤 `git restore Source`로 버그 상태로 되돌릴 수 있습니다.
