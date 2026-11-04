# BugHunt UE — 언리얼 엔진 C++ 버그 추적 실습

[BugHunt](https://github.com/gusgh0157/BugHunt)(C++ 콘솔판)의 언리얼 엔진 버전입니다.
버그가 **일부러 심어진** 작은 게임 코드 모음이고, 이번에는 **언리얼에서만 생기는** 버그들입니다.

> **UE6 안내 (2026년 11월 기준):** Epic은 UE6에서 **Actor, Blueprint, 그리고 C++로 쓰인 게임플레이 프레임워크를 폐지**하고, 게임플레이를 **Verse**와 **Scene Graph**(Entity + Component) 기반의 새 프레임워크로 옮긴다고 밝혔습니다. 당초 로드맵은 "비추천 뒤 단계적 폐지"였지만, Unreal Fest Tokyo 2026(11월 3일) 기조 강연에서 Tim Sweeney가 **UE6 얼리 액세스(2027년 말 예정)부터 전면 폐지**로 바뀌었다고 발표했고, AI 변환 지원 도구를 제공할 예정입니다. C++는 렌더링·물리 같은 엔진 하층에 남습니다. 출처: [Gamemakers.jp 기조 강연 보도](https://gamemakers.jp/article/2026_11_03_149187/), [Epic 로드맵 "The road to UE6"](https://www.unrealengine.com/news/the-road-to-ue-6).
> 이 저장소는 **UE 5.8의 C++ 게임플레이 프레임워크** 기준입니다. 여기서 익히는 것(엔진 프레임이 섞인 호출 스택 읽기, GC와 `UPROPERTY`, 복제와 권한, Tick 순서, 비동기 로드, 디버거 도구)은 UE6에서도 엔진 쪽 C++ 디버깅에 그대로 쓰이지만, Actor/Blueprint 기반의 랩 코드 자체는 UE6의 새 프레임워크로 옮겨 써야 할 수 있습니다.

- **1부: 크래시 추적 (랩 1~4)**: 에디터나 게임이 죽거나 `Ensure`가 뜬 순간, 호출 스택을 거슬러 올라가 원인을 찾습니다. 엔진 프레임이 섞인 긴 스택을 읽는 연습입니다.
- **2부: 흐름 추적 (랩 5~10)**: 게임은 멀쩡히 도는데 **결과가 틀립니다**. 델리게이트, 타이머, Tick 순서, GC, Config처럼 "언리얼이 대신 해 주는 일"의 순서와 수명을 직접 따라갑니다.
- **3부: 멀티플레이 (랩 11~12)**: 서버와 클라이언트가 **서로 다른 값**을 보는 버그. PIE로 여러 클라이언트를 띄워 한 디버거에서 같이 추적합니다.
- **4부: 사운드 (랩 13~15)**: 소리가 늦게 나고, 끊기고, 한 사람만 듣는 버그. 지연을 프레임/ms로 재서 판정합니다.
- **5부: 이미지 / 렌더링 (랩 16~18)**: 색이 안 바뀌고, 엉뚱한 아이콘이 뜨고, 색이 물 빠져 보이는 버그. 머티리얼 파라미터 값과 브러시에 들어간 텍스처를 조회해서 판정합니다.
- **6부: 애니메이션 (랩 19~21)**: 발소리가 안 나고, 연타하면 상태가 꼬이고, 끊긴 공격의 히트박스가 남는 버그. 노티파이 횟수와 상태값으로 판정합니다.
- **부록: Live Coding 함정**: "고쳤는데 그대로예요"의 단골 원인.

> 콘솔판과 같은 규칙입니다. 코드를 눈으로 훑어 정답을 찾지 마세요. 1부는 **죽게 만들고 디버거가 멈춘 곳에서부터**, 2·3부는 **직접 멈춘 곳에서부터 값을 확인하며** 추적하세요.
> 콘솔판 1·3부를 먼저 끝냈다면 좋습니다. 호출 스택 창, 지역 창, 조건부 중단점, 추적점, 데이터 중단점 사용법은 거기서 설명한 그대로 씁니다.

## BugHunt 시리즈

같은 방식(일부러 심은 버그를 디버거로 추적)으로 만든 네 가지 판입니다.

| 판 | 다루는 버그 | 저장소 |
|---|---|---|
| C++ 콘솔 (표준 C++20, CMake) | 호출 스택, 흐름 추적, 연산자, 컨테이너, 언어 함정, 인터페이스와 상속, 파일 입출력, 직렬화, 포인터와 메모리, 비동기와 동시성, 멈춤, 성능 (랩 98개 + 챌린지 11개) | [BugHunt](https://github.com/gusgh0157/BugHunt) |
| C# 콘솔 (.NET 10) | 예외, 흐름 추적, 컨테이너, 언어 함정, 인터페이스와 상속, 파일 입출력, 직렬화, 비동기와 동시성, 리소스와 메모리, unsafe, 멈춤, 성능 (랩 95개 + 챌린지 11개) | [BugHuntCS](https://github.com/gusgh0157/BugHuntCS) |
| 언리얼 엔진 (UE 5.8) | 🚧 **공사중** — 크래시, 흐름 추적, 멀티플레이, 사운드, 이미지, 애니메이션 (공개된 master는 랩 21개). 컨테이너, 포인터와 수명, 물리, 프레임레이트, 트랜스폼, 멈춤, 성능, 채점기, 일본어판은 작업 브랜치에서 진행 중 | [BugHuntUE](https://github.com/gusgh0157/BugHuntUE) **(지금 보는 저장소)** |
| 유니티 (Unity 6) | 🚧 **공사중** — 크래시/예외, 흐름 추적(프레임레이트 포함), 컨테이너, 참조와 네이티브 메모리, 물리, 사운드, 이미지, 애니메이션, 멈춤, 성능, 데이터 레이스 (랩 55개). 트랜스폼 부 추가와 재배치는 작업 브랜치에서 진행 중 | [BugHuntUnity](https://github.com/gusgh0157/BugHuntUnity) |

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
| `-stomp2malloc` | 해제된 메모리/범위 밖 접근을 그 자리에서 잡는 할당기 (언리얼판 ASan, 에디터 빌드 전용). 아주 느리고 메모리를 많이 씀. `-MallocStomp2MinSize=`, `-MallocStomp2MaxSize=`로 크기 범위를 좁힐 수 있음 |
| `-log` | 별도 로그 창 띄우기 |

## 다시 풀고 싶을 때

이 폴더는 git 저장소입니다. 고친 뒤 `git restore Source`로 버그 상태로 되돌릴 수 있습니다.
