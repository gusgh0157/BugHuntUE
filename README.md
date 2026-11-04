# BugHunt UE — 언리얼 엔진 C++ 버그 추적 실습 / Unreal Engine C++ バグ追跡ラボ

한국어 또는 日本語를 펼쳐 읽으세요. / 韓国語または日本語を開いてお読みください。

<details>
<summary><b>한국어</b></summary>

[BugHunt](https://github.com/gusgh0157/BugHunt)(C++ 콘솔판)의 언리얼 엔진 버전입니다.
버그가 **일부러 심어진** 작은 게임 코드 모음이고, 이번에는 **언리얼에서만 생기는** 버그들입니다.

> **UE6 안내 (2026년 11월 기준):** Epic은 UE6에서 **Actor, Blueprint, 그리고 C++로 쓰인 게임플레이 프레임워크를 폐지**하고, 게임플레이를 **Verse**와 **Scene Graph**(Entity + Component) 기반의 새 프레임워크로 옮긴다고 밝혔습니다. 당초 로드맵은 "비추천 뒤 단계적 폐지"였지만, Unreal Fest Tokyo 2026(11월 3일) 기조 강연에서 Tim Sweeney가 **UE6 얼리 액세스(2027년 말 예정)부터 전면 폐지**로 바뀌었다고 발표했고, AI 변환 지원 도구를 제공할 예정입니다. C++는 렌더링·물리 같은 엔진 하층에 남습니다. 출처: [Gamemakers.jp 기조 강연 보도](https://gamemakers.jp/article/2026_11_03_149187/), [Epic 로드맵 "The road to UE6"](https://www.unrealengine.com/news/the-road-to-ue-6).
> 이 저장소는 **UE 5.8의 C++ 게임플레이 프레임워크** 기준입니다. 여기서 익히는 것(엔진 프레임이 섞인 호출 스택 읽기, GC와 `UPROPERTY`, 복제와 권한, Tick 순서, 비동기 로드, 디버거 도구)은 UE6에서도 엔진 쪽 C++ 디버깅에 그대로 쓰이지만, Actor/Blueprint 기반의 랩 코드 자체는 UE6의 새 프레임워크로 옮겨 써야 할 수 있습니다.

- **1부: 크래시 추적 (랩 1~4)**: 에디터나 게임이 죽거나 `Ensure`가 뜬 순간, 호출 스택을 거슬러 올라가 원인을 찾습니다. 엔진 프레임이 섞인 긴 스택을 읽는 연습입니다.
- **2부: 흐름 추적 (랩 5~13)**: 게임은 멀쩡히 도는데 **결과가 틀립니다**. 델리게이트, 타이머, Tick 순서, GC, Config처럼 "언리얼이 대신 해 주는 일"의 순서와 수명을 직접 따라갑니다. 30fps 와 120fps 에서 다르게 움직이는 코드(`DeltaSeconds` 누락, 프레임당 감속)와 초기화 안 된 구조체 멤버도 여기서 다룹니다.
- **3부: 트랜스폼과 공간 (랩 14~21)**: 위치·회전·스케일이 엉뚱한 곳에 나타나는 버그. 상대/월드 좌표, Attach 규칙, 쿼터니언 곱 순서, 월드 축과 액터 축, 점과 방향의 변환, 비균일 스케일과 콜리전, Mobility, 소켓 이름. 원점에서 떨어진 곳에서 위치·방향으로 판정합니다.
- **4부: 컨테이너 (랩 22~28)**: `TArray`/`TMap`/`TSet` 의 범위 검사 assert, 없는 키, 재할당 뒤의 포인터, 순서를 바꾸는 제거, 해시와 `==` 불일치, GC 와 복제(`FFastArraySerializer`)를 만난 컨테이너.
- **5부: 포인터와 수명 (랩 29~32)**: 죽지 않는 포인터 버그. 지워진 액터를 기억하는 raw 포인터, 객체보다 오래 사는 타이머 람다, `Add` 뒤의 배열 원소 참조, `TSharedPtr` 순환 참조를 주소와 참조 수로 추적합니다.
- **6부: 물리 (랩 33~36)**: 터널링과 CCD, 채널 응답과 Overlap 이벤트, 스윕 없는 이동, 물리 물체를 Tick 에서 옮기기. Chaos Visual Debugger 로 프레임을 되감습니다.
- **7부: 사운드 (랩 37~39)**: 소리가 늦게 나고, 끊기고, 한 사람만 듣는 버그. 지연을 프레임/ms로 재서 판정합니다.
- **8부: 이미지 / 렌더링 (랩 40~42)**: 색이 안 바뀌고, 엉뚱한 아이콘이 뜨고, 색이 물 빠져 보이는 버그. 머티리얼 파라미터 값과 브러시에 들어간 텍스처를 조회해서 판정합니다.
- **9부: 애니메이션 (랩 43~45)**: 발소리가 안 나고, 연타하면 상태가 꼬이고, 끊긴 공격의 히트박스가 남는 버그. 노티파이 횟수와 상태값으로 판정합니다.
- **10부: 멀티플레이 (랩 46~47)**: 서버와 클라이언트가 **서로 다른 값**을 보는 버그. PIE로 여러 클라이언트를 띄워 한 디버거에서 같이 추적합니다.
- **11부: 동시성과 멈춤 (랩 48~52)**: 크래시 없이 게임이 굳는 무한루프와 데드락, 작업 스레드가 잠금 없이 같은 값을 고치거나 게임 스레드 전용 객체를 만지는 데이터 레이스. "모두 중단", 스레드 창, 병렬 스택으로 그 순간을 세워 봅니다. (실습용 안전장치가 있어 에디터가 진짜로 굳지는 않고, 레이스는 재현 확대경으로 매번 일어나게 했습니다.)
- **12부: 성능 (랩 53~56)**: Tick 마다 액터 검색, 프레임마다 배열 재할당, 수백 개의 Tick, 게임 중 동기 로드. 시간 대신 횟수로 판정합니다.
- **채점기**: `grade.bat` 하나로 모든 랩을 헤드리스로 돌려 PASS/FAIL/TIMEOUT/CRASH/SKIP 과 점수를 냅니다. 숨은 입력으로 하드코딩을 막습니다.
- **부록: Live Coding 함정**: "고쳤는데 그대로예요"의 단골 원인.

> 콘솔판과 같은 규칙입니다. 코드를 눈으로 훑어 정답을 찾지 마세요. 1부는 **죽게 만들고 디버거가 멈춘 곳에서부터**, 2부부터는 **직접 멈춘 곳에서부터 값을 확인하며** 추적하세요.
> 콘솔판 1·4부를 먼저 끝냈다면 좋습니다. 호출 스택 창, 지역 창, 조건부 중단점, 추적점, 데이터 중단점 사용법은 거기서 설명한 그대로 씁니다.

<details>
<summary>랩 번호가 바뀌었습니다 (옛 번호 → 새 번호)</summary>

랩이 56개로 늘면서 비슷한 주제끼리 모이도록 부와 번호를 다시 매겼습니다. 랩 1~10 은 그대로이고, 3부(트랜스폼과 공간, 랩 14~21)는 새로 생긴 랩입니다. 예전 번호로 적어 둔 메모나 `-BugHuntLab=` 인수가 있다면 아래 표로 바꾸세요.

| 옛 번호 | 새 번호 | 랩 |
|---|---|---|
| 11, 12 | 46, 47 | 호스트 HP 바, 보물 상자 (10부 멀티플레이) |
| 13~15 | 37~39 | 블래스터 발사음, 폭발 통, 경보 종 (7부 사운드) |
| 16~18 | 40~42 | 상자 강조, 인벤토리 아이콘, 팀 색 (8부 이미지) |
| 19~21 | 43~45 | 발자국, 연타 콤보, 히트박스 (9부 애니메이션) |
| 22~25 | 29~32 | 어그로 표, 포탑, 물약 벨트, 대화 트리 (5부 포인터와 수명) |
| 26, 27 | 48, 49 | 엘리베이터, 길찾기 (11부 동시성과 멈춤) |
| 28 | 52 | 자동 저장 (11부) |
| 29~35 | 22~28 | 웨이브 보상 … 퀘스트 게시판 (4부 컨테이너) |
| 36~38 | 11~13 | 돌진, 얼음 미끄럼, 라운드 점수판 (2부) |
| 39~42 | 33~36 | 저격탄, 압력판, 넉백, 투석기 (6부 물리) |
| 43, 44 | 50, 51 | 길드 금고, 하이라이트 점수 (11부) |
| 45~48 | 53~56 | 미니맵, 조준선, 횃불, 보스 등장 (12부 성능) |

</details>

## 다루는 버그 한눈에 보기

랩 하나가 여러 줄에 걸칠 수 있습니다. "미포함"은 아직 그 분류의 대표 사례를 다루는 랩이 없다는 뜻입니다.

| 분류 | 랩 | 대표 사례 |
|---|---|---|
| 포인터·수명·GC | 1, 4, 9, 27, 29, 30, 31, 32 | `Cast` 실패 nullptr, `UPROPERTY` 없는 포인터와 GC, `Destroy` 뒤 `IsValid`, `TWeakObjectPtr`, 람다의 `[this]`, `TSharedPtr` 순환 |
| 흐름·순서 | 2, 5, 6, 7, 8, 10 | BeginPlay 순서, 델리게이트 해제 누락, 타이머 핸들, Tick 그룹, Visual Logger, CDO/Config 기본값 |
| 프레임레이트 의존 | 11, 12 | `DeltaSeconds` 누락, 프레임당 감속(적분), `t.MaxFPS`, 고정 시간 간격 |
| 초기화 안 된 값 | 13 | 구조체 멤버 초기값 없음, `AddDefaulted` 와 남은 메모리. (지역 변수, `FMemory::Malloc` 은 설명만) |
| 트랜스폼·공간 | 14, 15, 16, 17, 18, 19, 20, 21 | 상대/월드 좌표, `FAttachmentTransformRules`, `FQuat` 곱 순서(로컬/월드 회전), `FVector::ForwardVector` vs `GetActorForwardVector`, `TransformPosition`/`TransformVector`, 비균일 스케일과 콜리전, Mobility Static, 없는 소켓 이름. (피벗 오프셋, 짐벌 락은 설명만) |
| 컨테이너 | 3, 22, 23, 24, 25, 26, 27, 28, 31 | 순회 중 제거, 범위 밖 인덱스, 없는 키 `operator[]`, `Add` 뒤 원소 참조/`Find` 포인터, `RemoveAtSwap` 순서, 해시/`==` 불일치, GC 와 `TArray<UObject*>`, FastArray 복제 |
| 물리 | 33, 34, 35, 36 | 터널링과 CCD, 채널 응답과 Generate Overlap Events, `bSweep=false` 순간이동, 시뮬레이션 물체를 Tick 에서 옮기기. (서브스테핑 차이는 설명만) |
| 사운드 | 37, 38, 39 | 소프트 레퍼런스 지연 로드, 붙인 컴포넌트의 수명, NetMulticast |
| 이미지·렌더링 | 40, 41, 42 | MID, 비동기 로드 콜백 순서, sRGB/Linear |
| 애니메이션 | 43, 44, 45 | 렌더 안 될 때 노티파이, 몽타주 interrupted, 노티파이 스테이트 |
| 멀티플레이 | 28, 39, 46, 47 | RepNotify, 권한과 Server RPC, 소리는 복제되지 않음, FastArray `MarkItemDirty` |
| 멈춤 (무한루프·데드락) | 48, 49, 52 | 게임 스레드 while 대기, 방문 체크 누락, Future 동기 대기 데드락 |
| 데이터 레이스 | 50, 51 | `ParallelFor` 읽기-계산-쓰기 레이스, 작업 스레드에서 UObject 만지기(`check(IsInGameThread())`) |
| 성능 | 53, 54, 55, 56 | Tick 마다 `GetAllActorsOfClass`, `Reserve` 누락, Tick 남발, 게임 중 동기 로드. (매 프레임 `FString` 만들기는 랩 54 힌트에서만 다룸) |

## BugHunt 시리즈

같은 방식(일부러 심은 버그를 디버거로 추적)으로 만든 네 가지 판입니다.

| 판 | 다루는 버그 | 저장소 |
|---|---|---|
| C++ 콘솔 (표준 C++20, CMake) | 호출 스택, 흐름 추적, 연산자, 컨테이너, 언어 함정, 인터페이스와 상속, 파일 입출력, 직렬화, 포인터와 메모리, 비동기와 동시성, 멈춤, 성능 (랩 98개 + 챌린지 11개) | [BugHunt](https://github.com/gusgh0157/BugHunt) |
| C# 콘솔 (.NET 10) | 예외, 흐름 추적, 컨테이너, 언어 함정, 인터페이스와 상속, 파일 입출력, 직렬화, 비동기와 동시성, 리소스와 메모리, unsafe, 멈춤, 성능 (랩 95개 + 챌린지 11개) | [BugHuntCS](https://github.com/gusgh0157/BugHuntCS) |
| 언리얼 엔진 (UE 5.8) | 크래시, 흐름 추적(프레임레이트·초기화 포함), 트랜스폼, 컨테이너, 포인터와 수명, 물리, 사운드, 이미지, 애니메이션, 멀티플레이, 동시성과 멈춤, 성능 (랩 56개) | [BugHuntUE](https://github.com/gusgh0157/BugHuntUE) **(지금 보는 저장소)** |
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
LogBugHunt: Display: ==== 랩 5: 전투 기록 창 ====
LogBugHunt: Display: [FAIL] 다시 연 창의 기록 줄 수 (기대: 1, 실제: 2)
LogBugHunt: Display: [PASS] 기사 HP (값: 78)
LogBugHunt: Display: ==== 랩 5 결과: PASS 1 / FAIL 2 ====
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
| 2 | 점수판 HUD | 게임 시작하자마자 죽는다 | 생성자 / PostInitializeComponents / **BeginPlay 순서**, SpawnActor 안에서 바로 불리는 BeginPlay | ★★ |
| 3 | 죽은 적 정리 | Ensure가 뜨고 죽은 적이 남아 있다 | `ensure` 메시지 읽기, **TArray 순회 중 제거** | ★★ |
| 4 | 퀘스트 로그 | 두 번째 퀘스트를 받을 때 죽는다 (가끔 안 죽음) | **GC**, `UPROPERTY` 없는 포인터, `obj gc`, `gc.CollectGarbageEveryFrame`, `-stomp2malloc` | ★★★ |

## 힌트 보는 법

콘솔판과 같습니다. 각 랩 `.cpp` 파일 **맨 아래**에 `#pragma region`으로 접혀 있습니다.

1. 랩 파일을 처음 열면 **Ctrl+M, Ctrl+O**로 전부 접습니다.
2. 막힐 때 **힌트 1**만 펼칩니다. 그래도 막히면 다음 힌트를 하나씩.

랩 코드는 `Source/BugHuntUE/Labs/LabNN_*.cpp` 입니다. `Source/BugHuntUE/Core/`(랩 실행기)에는 버그가 없습니다.
다 풀면 [SOLUTIONS.md](SOLUTIONS.md)로 맞춰 보세요.

---

# 2부: 흐름 추적 (크래시 없음)

콘솔판 4부와 같은 4단계입니다. 언리얼에서는 **"내가 부르지 않았는데 불리는 코드"**(델리게이트, 타이머, Tick, GC, Config 로딩)가 많아서, "언제, 누가, 몇 번 불렀나"를 보는 도구가 특히 중요합니다.

## 흐름 추적 4단계

1. **기대를 먼저 적는다.** "독 피해는 0.4, 0.8, 1.4, 1.8초에 들어와야 한다"처럼.
2. **틀린 결과에서 가장 가까운 곳부터 멈춘다.**
3. **기대와 처음 달라지는 곳을 찾는다.** 매 프레임 도는 코드는 F10 대신 추적점, `UE_LOG`, Visual Logger로 기록한다.
4. **"처음 달라진 곳"을 고친다.**

```
기록표 (2부부터)
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

## 프레임레이트와 초기화 (랩 11~13)

"내 PC 에선 되는데요"의 두 가지 단골 원인입니다. 하나는 **프레임레이트**(30fps 노트북과 144Hz 모니터에서 게임이 다르게 움직임), 하나는 **초기화 안 된 값**(빌드나 순서에 따라 쓰레기 값이 달라짐)입니다.

### 프레임레이트를 바꿔 가며 실행하기

| 도구 | 하는 법 | 언제 |
|---|---|---|
| `t.MaxFPS N` | 콘솔. `t.MaxFPS 30` → 실행 → `t.MaxFPS 120` → 실행 → `t.MaxFPS 0`(제한 없음) | 프레임레이트를 바꿔 같은 장면을 비교 |
| 고정 시간 간격 | 명령줄 `-benchmark -fps=30` (코드: `FApp::SetUseFixedTimeStep(true)`, `FApp::SetFixedDeltaTime(1.0/30)`) | 실제 시간과 상관없이 매 프레임 `DeltaSeconds` 를 정확히 1/N 으로. 느린 PC 에서도 결과가 같다. 랩 11, 12 가 이 방식 (`Core/BugHuntProbe.h`) |
| `stat fps`, `stat unit` | 콘솔 | 지금 실제 프레임레이트와 프레임 시간 |
| 추적점 `{DeltaSeconds}` | Tick 첫 줄 | 프레임 간격이 얼마인지, Tick 이 1초에 몇 번 불리는지 |

### 초기화 안 된 값

| 무엇 | 처음 값 |
|---|---|
| UObject(액터, 컴포넌트)의 멤버 | **0** (메모리를 0 으로 채운 뒤 생성자를 부름). 그래도 초기값을 적는 것이 규칙 |
| `USTRUCT`/일반 구조체의 멤버, 지역 변수 | **아무 값** (초기값을 안 적으면) |
| `FMemory::Malloc`, `TArray::AddUninitialized`/`SetNumUninitialized` | **아무 값** |
| `TArray::AddDefaulted` | 구조체의 기본 생성자대로. 초기값이 없는 멤버는 **아무 값** (`AddZeroed` 는 0) |

- "아무 값"은 그 메모리를 직전에 누가 썼느냐에 달려 있어서, Debug 와 Development 빌드, 에디터와 패키징, 실행 순서에 따라 달라집니다. 그래서 "가끔", "내 PC 에서만" 나타납니다.
- 언리얼은 시작할 때 USTRUCT 의 `UPROPERTY` 기본값을 검사해서, 초기값이 빠진 것을 Output Log 에 빨간 줄로 남깁니다:
  `LogClass: Error: IntProperty FBH_RoundStats::Kills is not initialized properly. Module:BugHuntUE File:Labs/Lab13_RoundStats.h`
  이 저장소에서는 랩 13 이 고쳐질 때까지 에디터를 켤 때마다 이 줄이 보입니다. `UPROPERTY` 가 없는 멤버와 지역 변수는 검사하지 않습니다.
- 데이터 중단점(위 도구 표)으로 "이 값을 처음 쓴 게 누구인가"를 찾으면, 아무도 쓰지 않았다는 것을 확인할 수 있습니다.

## 2부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 5 | 전투 기록 창 | 창을 다시 열면 기록이 두 줄씩 찍힌다 | **델리게이트 바인딩 해제 누락**, 추적점, InvocationList 보기 | ★ |
| 6 | 독 정화 | 정화했는데 독 피해가 계속 들어온다 | **FTimerHandle** 수명, `ListTimers` | ★★ |
| 7 | HP 바 | HP 바가 한 프레임씩 늦다 | **Tick 그룹과 순서**, `dumpticks`, `GFrameCounter`, `AddTickPrerequisiteActor` | ★★ |
| 8 | 순찰병 | 순찰병이 첫 웨이포인트에서 떨고만 있다 | **Visual Logger**로 녹화·되감기 | ★★ |
| 9 | 궁수 | 쓰러진 허수아비로 점수를 계속 번다 | `Destroy()` 뒤의 포인터, **`IsValid` vs nullptr** | ★★ |
| 10 | 밸런스 패치 | HP를 150으로 올렸는데 게임에선 100이다 | **CDO**, Config/블루프린트 기본값이 생성자 값을 덮어쓰는 순서, `getall`, 데이터 중단점 | ★★★ |
| 11 | 돌진 | 프레임레이트에 따라 돌진 거리가 다르다 | 이동에 **`DeltaSeconds`** 빠짐, `t.MaxFPS`, 고정 시간 간격 | ★ |
| 12 | 얼음 미끄럼 | 프레임레이트에 따라 미끄러지는 거리가 다르다 | **프레임당 감속**(`Velocity *= 0.9`), `FMath::Pow`/`Exp` 로 시간 기준 감속, `FInterpTo` | ★★ |
| 13 | 라운드 점수판 | 새 라운드에 지난 라운드 점수가 남아 있다 | 구조체 멤버 **초기값 없음**, UObject 는 0, `AddDefaulted` vs `AddZeroed`, `Reset` 이 남긴 메모리 | ★★ |

> 랩 10의 블루프린트 버전도 직접 만들어 보세요: Content Browser에서 `BH_Slime`을 부모로 블루프린트 `BP_Slime`을 만들고 Class Defaults에서 MaxHealth를 120으로 바꾼 뒤 저장 → C++ 생성자 값을 200으로 바꾸고 에디터를 다시 켭니다. `BP_Slime`의 MaxHealth는 몇인가요? 블루프린트에서 **바꾸지 않은** MoveSpeed는요? (블루프린트는 "부모와 다르게 바꾼 값"만 저장합니다.)

- 랩 11~13 은 GPU 없이 판정됩니다(`-nullrhi` 검증 완료). 랩 11, 12 는 한 번 실행에 30fps 와 120fps 를 차례로 돌립니다.

---

# 3부: 트랜스폼과 공간

위치·회전·스케일은 엔진에서 가장 많이 쓰는 값이고, 버그도 가장 흔합니다. 이 부의 버그는 크래시도 에러도 없이 **엉뚱한 곳에** 나타납니다. 대부분 "월드 원점에 회전 없이 놓고 테스트할 때는 맞는다"는 공통점이 있습니다. 원점에서는 상대 좌표와 월드 좌표, 액터 축과 월드 축이 같기 때문입니다. 그래서 이 부의 랩은 액터를 원점에서 떨어진 곳에 돌려 놓고 판정합니다(숨은 입력이 위치·각도·스케일을 바꿉니다).

추적할 때 먼저 물어볼 것은 하나입니다. **"이 값은 어느 좌표계인가?"** 월드, 부모(Relative), 액터 자신(로컬), 소켓. 함수 이름(`SetRelativeLocation`, `SetWorldLocation`, `TransformPosition`, `GetActorForwardVector`)이 기대하는 좌표계와 넣는 값의 좌표계가 같은지 확인합니다.

## 3부에서 쓰는 도구

| 도구 | 하는 법 | 언제 |
|---|---|---|
| 디테일 패널 **Transform** | PIE 일시 정지 → World Outliner 에서 액터 선택 → 컴포넌트 선택. Location/Rotation/Scale 이름 옆 메뉴로 **상대(Relative)/월드(World)** 표시 전환 | 같은 컴포넌트의 상대 값과 월드 값을 나란히 (랩 14, 15, 19) |
| 뷰포트 좌표계 전환 | 뷰포트 오른쪽 위 지구본/큐브 아이콘 (Ctrl+`) | 기즈모 축을 월드 축/로컬 축으로. 액터의 "앞"이 어디인지 눈으로 (랩 16, 17) |
| 조사식 | `GetActorTransform()`, `Comp->GetComponentTransform()`, `Comp->GetRelativeTransform()`, `GetActorQuat()` | 트랜스폼을 펼쳐 Translation/Rotation/Scale3D 를 직접 보기. `.ToHumanReadableString()` 도 됨 |
| **`DrawDebug*`** | 코드: `DrawDebugCoordinateSystem(World, Loc, Rot, 100, false, 5)`, `DrawDebugDirectionalArrow`, `DrawDebugSphere` (`DrawDebugHelpers.h`) | 계산한 점·방향·반경을 화면에 그려 실제 물체와 비교 (랩 17, 18, 19) |
| `show Collision` | 콘솔 (PIE 중) | 충돌 범위가 스케일 때문에 실제로 얼마나 커졌나/작아졌나 (랩 19) |
| `ShowDebug` | 콘솔 (PIE 중). `ShowDebug Collision`, `ShowDebug Camera` 등 | 플레이어 쪽 정보(카메라 위치·회전, 캡슐)를 화면에 |
| 소켓 매니저 | 스켈레탈 메시 에디터 → 스켈레톤 트리 → 뼈 우클릭 → Add Socket. 코드에서는 `Mesh->DoesSocketExist(Name)`, `GetAllSocketNames()` | 소켓·뼈 이름 확인, 손잡이 위치 맞추기 (랩 21) |
| 메시지 로그 | Window → Message Log → PIE 탭 | Static 컴포넌트를 옮기려 할 때의 경고 (랩 20) |

## 따라 해 보기: 랩 14 (갑판 깃발)

1. PIE → 콘솔 `BugHunt.Lab 14`. `[PASS] 조선소: 깃발이 돛대 꼭대기에 있다 (어긋난 거리 0 cm)` 다음에 `[FAIL] 항구: 깃발이 돛대 꼭대기에 있다 (어긋난 거리 4804 cm)`. 조선소(월드 원점)에서는 맞고 항구에서 틀립니다.
2. 로그의 `Harbor 깃발을 올림 → 돛대 꼭대기 (5000, 200, 800)` 는 맞는 좌표입니다(항구 X=5000, 뱃머리 90도, 돛대는 배 기준 (200, 0, 800)). 계산은 맞았는데 깃발은 엉뚱한 곳에 있습니다. → *값은 맞는데 결과가 틀리다*
3. `ABH_Ship::RaiseFlag` 의 `SetRelativeLocation` 줄에 중단점을 걸고 다시 실행합니다. 두 번째로 멈췄을 때(항구) F10 으로 한 줄 넘기고 조사식에 `Flag->GetRelativeLocation()` 과 `Flag->GetComponentLocation()` 을 넣습니다. Relative 가 (5000, 200, 800), 월드는 (4800, 5000, 800) 입니다.
4. Relative 에 월드 좌표를 넣었습니다. 자식의 월드 위치 = 부모 트랜스폼 × Relative 이니 배의 위치와 회전이 한 번 더 곱해졌습니다. 조선소에서는 배가 원점에 회전 없이 있어서 한 번 더 곱해도 같았습니다. → *최초 이상 지점*
5. PIE 를 일시 정지하고 World Outliner 에서 배를 골라 Flag 컴포넌트의 Location 표시를 상대/월드로 바꿔 보세요. 같은 내용을 에디터에서 확인하는 방법입니다.

## 3부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 14 | 갑판 깃발 | 항구에 들어오자 깃발이 배를 떠나 엉뚱한 곳에 걸린다 | **상대 좌표와 월드 좌표**, `SetRelativeLocation` 에 월드 좌표를 넣으면 부모 트랜스폼이 두 번 곱해짐 | ★ |
| 15 | 검 줍기 | 주운 검이 손이 아니라 옆 허공에 떠서 따라온다 | **`FAttachmentTransformRules`**: KeepRelative / KeepWorld / SnapToTarget(스케일 포함 여부) | ★ |
| 16 | 글라이더 | 기울인 채 조종간을 당기면 엉뚱한 쪽으로 돈다 | **`FQuat` 곱 순서**(월드 회전 vs 로컬 회전), `AddActorLocalRotation`, `FRotator` 더하기와 짐벌 락 | ★★ |
| 17 | 마법 대포 | 표적 쪽으로 돌아선 대포가 정면으로만 쏜다 | **`FVector::ForwardVector`(월드 +X) vs `GetActorForwardVector()`**(액터 기준) | ★ |
| 18 | 레이저 조준선 | 전차가 원점에서 멀어지면 조준선이 엉뚱한 곳에서 나간다 | **`TransformPosition` vs `TransformVector` / `TransformVectorNoScale`**: 점과 방향, 평행이동과 스케일 | ★★ |
| 19 | 감시탑 | 탑을 높게 만들자 경보 범위가 반으로 줄었다 | 부모의 **비균일 스케일**이 자식 콜리전에 미치는 영향, 구는 가장 작은 축 스케일, 스케일은 메시에만 | ★★ |
| 20 | 움직이는 발판 | 발판이 꼼짝하지 않는다 (에러 없음) | **Mobility Static** 컴포넌트는 게임 중 이동 무시(경고만), 이동 함수의 반환값 | ★ |
| 21 | 광부의 랜턴 | 손에 든 랜턴이 발밑에 붙어 있다 | **없는 소켓 이름**은 조용히 컴포넌트 원점, `DoesSocketExist`, 뼈 이름을 소켓으로, 소켓 매니저 | ★ |

- 여덟 개 모두 GPU 없이 판정됩니다. 판정은 위치·방향·거리(cm, 내적)로 하고, 프레임을 쓰는 랩(52, 55)은 고정 60fps 로 돕니다.
- 미포함: 피벗 오프셋(메시의 원점이 모양 가운데가 아닌 경우), 스켈레탈 메시의 뼈 스케일, 월드 원점 재배치(World Origin Rebasing), 큰 좌표의 float 정밀도.

---

# 4부: 컨테이너

`TArray`, `TMap`, `TSet` 은 표준 라이브러리 컨테이너와 비슷해 보이지만 규칙이 조금씩 다릅니다. 범위 검사가 **개발 빌드에서도 켜져 있어서** 잘못된 인덱스는 조용히 넘어가지 않고 assert 로 죽고(랩 22, 23), 반대로 "원소의 주소"나 "순서"나 "해시"에 기댄 코드는 죽지 않고 값만 틀립니다(랩 24~26). 마지막 두 랩은 컨테이너가 **GC**와 **복제**를 만났을 때입니다.

이미 다룬 것은 다시 다루지 않습니다: 순회 중 제거(랩 3), `TArray` 원소 참조를 쥔 채 `Add`(랩 31).

## 4부에서 쓰는 도구

| 도구 | 하는 법 | 언제 |
|---|---|---|
| assert 메시지 읽기 | `Assertion failed: (Index >= 0) & (Index < ArrayNum)` → 호출 스택에서 `Array.h` 아래 첫 `BugHuntUE.dll!` 줄 | 범위 밖 인덱스 (랩 22). 조사식에 `Wave`, `Rewards.Num()` 을 넣어 비교 |
| `Pair != nullptr` | `Map.h.inl` 의 `FindChecked` → 호출 스택을 한 줄 올라가면 `operator[]` | 없는 키로 `Map[Key]` (랩 23) |
| natvis 로 `TMap`/`TSet` 펼치기 | 조사식 `Kills`, `Revealed` | 원소 수, 키와 값 |
| 주소 비교 | 조사식 `TargetKills` 의 주소와, `Kills` 를 펼쳐 본 Wolf 원소의 주소를 나란히 | 미리 받아 둔 포인터가 아직 진짜 원소를 가리키나 (랩 24) |
| 중복 키 찾기 | 조사식 `Revealed` 를 펼쳐 같은 (X,Y) 가 두 번 들어 있는지 본다 | `==` 와 `GetTypeHash` 가 어긋남 (랩 26) |
| `obj list class=BH_LootItem` | 콘솔. GC 전후 비교 | 컨테이너 속 UObject 가 살아 있나 (랩 27) |
| `obj refs name=...` | 콘솔 | 그 객체를 누가 붙잡고 있나 (UPROPERTY 컨테이너만 보임) |
| Networking Insights | 명령줄 `-NetTrace=1 -trace=net` → Unreal Insights 에서 열기 | 어떤 프로퍼티가 몇 바이트로 언제 복제됐나 (랩 28) |

## 4부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 22 | 웨이브 보상 | 보상을 받으면 크래시한다 | `TArray` **범위 검사 assert**, 1부터 세는 번호와 0부터 세는 인덱스 | ★ |
| 23 | 상점 시세표 | 모르는 물건을 팔면 크래시한다 | **`TMap::operator[]` = `FindChecked`** (없는 키를 만들지 않음, `std::map` 과 다름), `Find`/`FindRef`/`Contains` | ★ |
| 24 | 현상금 게시판 | 새 몬스터를 잡으면 현상금 집계가 틀린다 | **`TMap::Find` 포인터 + `Add`/`FindOrAdd` 재할당**, `TInlineSetAllocator` | ★★ |
| 25 | 턴 순서 | 유닛이 쓰러지면 턴 순서가 뒤섞인다 | **`RemoveAtSwap`/`RemoveSwap`** 은 순서를 지키지 않는다, `RemoveAt`/`Remove` 와 비용 차이 | ★ |
| 26 | 전장의 안개 | 같은 칸이 여러 번 밝혀진다 | **`GetTypeHash` 와 `operator==` 가 같은 필드를 봐야 한다**, 키를 넣은 뒤 바꾸면 안 되는 이유 | ★★ |
| 27 | 전리품 가방 | 시간이 지나면 가방 속 아이템이 사라진다 | `UPROPERTY` 없는 **`TArray<UObject*>`** 와 GC, `TArray<TObjectPtr<>>`, `TArray<TWeakObjectPtr<>>` | ★★ |
| 28 | 퀘스트 게시판 | 클라이언트의 퀘스트 진행도가 0에서 멈춘다 | **`FFastArraySerializer`** 는 비교하지 않는다, `MarkItemDirty`/`MarkArrayDirty`, 일반 복제 `TArray` 와 차이 | ★★★ |

- 일곱 개 다 GPU 없이 판정됩니다(`-nullrhi` 검증 완료). 랩 22, 23 는 CRASH 가 버그 상태입니다.
- 랩 24 은 `TInlineSetAllocator<4>` 로 "다섯 번째 종류가 들어오는 순간"에 원소가 옮겨지게 해 두었습니다. 기본 할당기라면 언제 옮겨질지 정해져 있지 않아서, 같은 버그가 몇 시간 뒤에 엉뚱한 값으로 나타납니다.
- 랩 26 의 키를 넣은 뒤 `X` 를 바꾸는 것도 같은 버그입니다. 원소는 옛 해시의 버킷에 남아 있어서 새 좌표로 찾으면 없고, 옛 좌표로 찾아도 `==` 가 틀려서 없습니다. 키를 바꿔야 하면 빼고 다시 넣으세요.
- 랩 28 은 랩 46·47·39 처럼 **Number of Players 2, Play As Listen Server** 로 실행하고 서버 창에서 `BugHunt.Lab 28`. 스탠드얼론에서는 통과합니다.

---

# 5부: 포인터와 수명

1부의 크래시는 "죽은 자리"가 있었습니다. 이번 포인터 버그들은 **대부분 죽지 않습니다**. 가리키는 메모리가 아직 남아 있거나(GC 전, 인라인 버퍼), 다른 객체가 그 자리를 다시 쓰고 있기 때문입니다. 그래서 값이 이상하게 나오는 곳에서 거꾸로 "이 포인터는 지금 무엇을 가리키나"를 확인해야 합니다.

1부와 2부에서 이미 다룬 것은 다시 다루지 않습니다: `Cast` 실패 nullptr(랩 1), `UPROPERTY` 없는 멤버의 GC(랩 4), `Destroy` 뒤의 포인터와 `IsValid`(랩 9).

## 5부에서 쓰는 도구

| 도구 | 하는 법 | 언제 |
|---|---|---|
| 조사식에 **주소** 적어 두기 | 조사식 `Kael` → 값 칸의 `0x...` 를 메모장에 | "같은 객체인가?"를 주소로 비교 (랩 29, 31) |
| `&변수` 조사식 | `&Potions`, `&Slots[0]` | 참조가 가리키는 위치가 배열의 진짜 위치와 같은가 (랩 31) |
| **데이터 중단점** | 디버그 → 새 중단점 → 데이터 중단점, 주소 `&Slots[0].Count`, 4바이트 | 진짜 값을 누가 바꾸는지 / 아무도 안 바꾸는지 (랩 31) |
| natvis | 조사식에서 `TWeakObjectPtr`, `TSharedPtr` 를 펼침 | 약한 포인터가 가리키는 객체, 공유 포인터의 참조 수 (랩 29, 32) |
| 생성자/소멸자 중단점 | 소멸자 첫 줄 F9 | 지워져야 할 것이 지워지나 (랩 32) |
| `obj list class=...`, `obj refs name=...` | 콘솔 | UObject 가 아직 살아 있나, 누가 붙잡나 |
| `-stomp2malloc` | 명령줄 (명령줄 옵션 표 참고) | 해제된 힙에 쓰는 순간 그 자리에서 멈추게 하기 (랩 31 을 기본 `TArray`로 바꿔서 해 보세요) |

## 5부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 29 | 어그로 표 | 새로 온 영웅이 죽은 영웅의 어그로를 물려받는다 | UObject 가 아닌 구조체에서 액터 기억하기, **`TWeakObjectPtr`**, 주소 재사용(ABA) | ★★ |
| 30 | 포탑 | 부순 포탑이 계속 쏜다 | 타이머/델리게이트 람다의 `[this]`, **`CreateWeakLambda` / `CreateUObject`**, EndPlay 가 지우는 타이머 | ★★ |
| 31 | 물약 벨트 | 전리품을 주운 뒤로 물약이 줄지 않는다 | **`TArray` 원소 참조 + `Add` 재할당**, `TInlineAllocator`, 데이터 중단점 | ★★ |
| 32 | 대화 트리 | 대화가 끝나도 대화 노드가 메모리에 남는다 | **`TSharedPtr` 순환 참조**, `TWeakPtr`, 참조 수 보기 | ★★ |

- 넷 다 GPU 없이 판정됩니다(`-nullrhi` 검증 완료). 랩 29 은 할당기가 같은 주소를 다시 쓰면 "Bram 어그로 60", 다른 주소를 주면 "보스가 지워진 영웅을 노림"으로 나타납니다. 어느 쪽이든 FAIL 입니다.

---

# 6부: 물리

물리 버그는 "가끔 뚫고 지나간다", "가끔 안 눌린다"로 신고됩니다. 같은 장면을 프레임 단위로 되감아 보는 도구가 핵심입니다. 이 부의 랩은 모두 **고정 60fps**로 돌아서 결과가 늘 같습니다.

## 6부에서 쓰는 도구

| 도구 | 하는 법 | 언제 |
|---|---|---|
| `show Collision` | 콘솔 (PIE 중) | 충돌 모양이 실제로 어디 있는지 |
| `p.Chaos.DebugDraw.Enabled 1` | 콘솔. `p.Chaos.DebugDraw.ShowCollisionParticles 1` 등 하위 옵션 | 물리 엔진이 보는 물체와 접촉점을 화면에 그림 (예전 PhysX 의 `pxvis collision` 에 해당) |
| **Chaos Visual Debugger** | Tools → Debug → Chaos Visual Debugger → 녹화 → PIE → 정지 → 프레임 넘기기 | 물리 프레임마다 위치, 속도, 충돌을 되감아 보기 (랩 33, 36) |
| **Visual Logger** | 2부와 같음. `UE_VLOG_LOCATION`, `UE_VLOG_SEGMENT` 로 위치와 이동 경로를 남김 | "이 프레임에 어디서 어디로 갔나" |
| 디테일 패널 **Collision** | PIE 일시 정지 → World Outliner 에서 선택 | 채널 응답(Ignore/Overlap/Block), Generate Overlap Events, Simulation Generates Hit Events (랩 34) |
| `Collision` 프로필 | Project Settings → Collision → Preset | `BlockAll`, `OverlapAll`, `Trigger`, `PhysicsActor` 가 채널별로 어떻게 응답하는지 |
| 서브스테핑 | Project Settings → Physics → Substepping, Max Substep Delta Time | 낮은 프레임레이트에서도 물리 한 단계를 작게. 프레임레이트에 따라 물리 결과가 달라질 때 |

Block 과 Overlap 규칙: 두 물체의 서로에 대한 응답 중 **약한 쪽**이 이깁니다(Ignore < Overlap < Block). 둘 다 Block 이면 Hit, 하나라도 Overlap 이면 Overlap(그리고 **둘 다** Generate Overlap Events 가 켜져 있어야 이벤트), 하나라도 Ignore 면 아무 일도 없습니다.

## 6부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 33 | 저격탄 | 탄환이 얇은 철판을 뚫고 지나간다 | **터널링**, 연속 충돌 검출 **`bUseCCD`**, 히트스캔/스윕과 비교 | ★ |
| 34 | 압력판 | 상자를 올려도 압력판이 눌리지 않는다 | 채널 응답, **Generate Overlap Events 는 양쪽 다**, `AStaticMeshActor` 가 끄는 기본값 | ★★ |
| 35 | 넉백 | 밀려난 적이 성벽 너머에 서 있다 | **`SetActorLocation` 의 `bSweep=false`(순간이동)**, 스윕과 `FHitResult` | ★ |
| 36 | 투석기 | 투석기가 던진 바위가 그 자리에 떨어진다 | 물리 시뮬레이션 중인 물체를 **Tick 에서 위치로 옮기기**, 물리 속도, `SetPhysicsLinearVelocity`/힘/PhysicsHandle | ★★ |

- 넷 다 GPU 없이 판정됩니다(`-nullrhi` 에서 Chaos 물리가 그대로 돈다는 것을 확인했습니다).

---

# 7부: 사운드

> ⚠️ 이 부의 랩은 소리를 냅니다. 실행 전에 시스템 볼륨을 낮추고, 헤드폰을 쓴다면 귀에서 떼고 시작하세요.

"소리가 늦게 나요", "소리가 끊겨요", "저만 안 들려요"는 게임 개발에서 아주 흔한 신고입니다. 소리 자체는 귀로 들리지만, 버그를 **재현하고 판정**하려면 숫자가 필요합니다. 그래서 이 랩들은 "섬광과 소리 사이의 프레임 수", "0.1초 뒤에도 재생 중인가", "각 머신에서 몇 번 울렸나"를 재서 `[PASS]`/`[FAIL]`을 냅니다.

## 7부에서 쓰는 도구

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

## 7부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 37 | 블래스터 발사음 | 발사음이 섬광보다 늦게 난다 (첫 발, 그리고 가끔) | **소프트 레퍼런스** 지연 로드, 비동기 로드는 다음 프레임, GC가 언로드, `obj list class=SoundWave` | ★★ |
| 38 | 폭발 통 | 폭발음이 터지자마자 끊긴다 | `SpawnSoundAttached` 컴포넌트의 수명 = 액터의 수명, `au.Debug.Sounds` | ★ |
| 39 | 경보 종 | 호스트만 종소리를 듣는다 | 소리는 **복제되지 않는다**, 연출용 NetMulticast | ★★ |

- 랩 37는 발사음(`/Engine/EditorSounds/Notifications/CompileFailed`)이 이미 메모리에 있으면 재현되지 않습니다. 이 소리는 에디터 알림음이라, 에디터에서 컴파일이 실패해 알림음이 울린 직후라면 에디터를 다시 켜고 실행하세요(`obj list class=SoundWave` 로 확인). 채점기와 `-game` 실행에서는 늘 재현됩니다.
- 랩 38은 오디오 장치가 필요합니다. `-nosound`로 실행했거나 출력 장치가 없으면 `[SKIP]`이 나옵니다.
- 랩 39은 랩 46·47처럼 **Number of Players 2, Play As Listen Server**로 실행하고 서버 창에서 `BugHunt.Lab 39`. 실행하기 전에 볼륨을 낮추세요(이 부의 랩은 소리를 냅니다).

---

# 8부: 이미지 / 렌더링

"색이 안 바뀌어요", "아이콘이 다른 아이템 거예요", "색이 기획서랑 달라요"는 눈으로는 바로 보이지만, 눈대중으로는 "조금 다른 것 같다"에서 멈추기 쉽습니다. 이 랩들은 **메시에 끼워진 머티리얼의 파라미터 값**, **이미지 위젯 브러시에 들어간 텍스처**, **8비트 색 값**을 직접 조회해서 `[PASS]`/`[FAIL]`을 냅니다.

## 8부에서 쓰는 도구

| 도구 | 하는 법 | 언제 |
|---|---|---|
| Details 패널의 Materials | PIE 중 F8(Eject) → 액터 선택 | 지금 메시에 **끼워진** 머티리얼이 공유 에셋인지 `MID_...`인지 |
| 머티리얼 인스턴스 에디터 | 머티리얼 더블클릭 → Parameter 목록 | 파라미터 이름(대소문자까지)과 기본값 |
| Widget Reflector | Tools → Debug → Widget Reflector, Pick Hit-Testable Widgets | 화면의 위젯이 어떤 브러시·텍스처를 그리고 있나 |
| `obj list class=Texture2D` | 콘솔 | 텍스처가 메모리에 올라와 있나 (비동기 로드 확인) |
| 텍스처 에디터의 sRGB 체크 | 텍스처 더블클릭 → Details → Texture → sRGB | 색 텍스처는 켜고, 노멀·마스크는 끈다 |
| `stat streaming`, `r.Streaming.*` | 콘솔 | 텍스처 스트리밍 풀, 밉 레벨 (아래 참고) |
| RenderDoc / `ProfileGPU` | 플러그인 RenderDoc 켜기 / 콘솔 | 실제로 그려진 픽셀, 드로 콜 |

> **GPU가 필요해서 넣지 않은 랩:** "텍스처가 흐릿하게 나온다(밉 스트리밍)"와 "렌더 타깃 픽셀 값이 틀리다(readback)"는 흔한 버그지만, 둘 다 실제 GPU 렌더링 결과를 읽어야 판정할 수 있습니다. 이 프로젝트의 자동 검증은 `-nullrhi`(GPU 없이 실행)로 돌기 때문에 이 두 종류는 넣지 않았습니다. 랩 40~42는 GPU 없이 조회되는 값(파라미터, 브러시, 색 변환)으로 판정하므로 `-nullrhi`에서도 결과가 같습니다. 화면으로 직접 확인하고 싶다면 에디터의 PIE에서 실행하세요.

## 8부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 40 | 상자 강조 | 마우스를 올려도 상자가 빨개지지 않는다 | **MID**는 만들기만 하면 끼워지지 않는다, `CreateDynamicMaterialInstance`, 공유 머티리얼 | ★ |
| 41 | 인벤토리 아이콘 | 스크롤하면 엉뚱한 아이템 아이콘이 뜬다 | 비동기 로드 콜백의 **늦은 도착**, 재사용되는 위젯, 프레임 번호로 순서 적기 | ★★ |
| 42 | 팀 색 | 팀 색이 기획서보다 밝고 물 빠져 보인다 | **sRGB와 Linear**, `FColor` → `FLinearColor` 변환, 텍스처의 sRGB 체크 | ★★ |

- 셋 다 GPU 없이 판정됩니다(`-nullrhi` 검증 완료). 랩 41은 아이콘이 이미 메모리에 있으면 재현되지 않습니다. 에디터에서 해당 텍스처를 열어 본 직후라면 에디터를 다시 켜고 실행하세요.

---

# 9부: 애니메이션

애니메이션 버그는 "가끔 공격이 안 맞아요", "연타하면 캐릭터가 미끄러져요", "죽은 적의 칼에 맞았어요"처럼 **게임 판정**으로 신고됩니다. 공격 판정, 발소리, 상태 전환이 애니메이션 노티파이와 몽타주 이벤트에 걸려 있기 때문입니다. 이 랩들은 화면의 포즈가 아니라 **노티파이가 몇 번 왔나**, **이벤트가 왔을 때 상태값이 무엇인가**로 `[PASS]`/`[FAIL]`을 냅니다.

에셋은 엔진 기본 콘텐츠만 씁니다. 캐릭터는 `/Engine/Tutorial/SubEditors/TutorialAssets/Character/TutorialTPP`, 애니메이션은 같은 폴더의 `Tutorial_Walk_Fwd`(1.83초)입니다. 몽타주와 노티파이는 에디터에서 에셋에 다는 대신 코드로 만듭니다(`Core/BugHuntAnim.h`, 버그 없음). 애님 블루프린트 없이 기본 `UAnimInstance`를 쓰므로 PIE 화면의 캐릭터는 걷는 포즈를 취하지 않습니다. 몽타주 재생, 노티파이, 종료 이벤트는 정상적으로 돕니다.

## 9부에서 쓰는 도구

| 도구 | 하는 법 | 언제 |
|---|---|---|
| `showdebug animation` | 콘솔 (PIE에서 캐릭터를 뷰 타깃으로) | 재생 중인 몽타주, 위치, 가중치, 노티파이 |
| Rewind Debugger | Tools → Debug → Rewind Debugger (PIE 녹화) | 노티파이가 언제 왔는지 타임라인으로 |
| Animation Insights | 명령줄 `-trace=default,animation` → Unreal Insights | 프레임별 몽타주 상태, 노티파이, 포즈 틱 여부 |
| 노티파이에 중단점 | `Notify`, `NotifyBegin/End`, `OnMontageEnded` 핸들러 | 누가 불렀나 (호출 스택), 언제 안 불리나 |
| 프레임 번호 로그 | `UE_LOG`에 `GFrameCounter` | 이벤트 순서를 기록표로 |
| Details → Mesh → Optimization | PIE 중 캐릭터의 스켈레탈 메시 컴포넌트 | `Visibility Based Anim Tick Option` 값 |

## 9부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 43 | 발자국 | 카메라 밖 NPC의 발소리를 적이 못 듣는다 | `VisibilityBasedAnimTickOption`, 렌더되지 않으면 포즈 틱과 노티파이가 멈춤 (서버, `-nullrhi`, 카메라 밖) | ★★ |
| 44 | 연타 콤보 | 연타하면 공격 중인데 움직여지고 콤보가 더 오른다 | `OnMontageEnded`의 `bInterrupted`, 같은 몽타주를 다시 재생하면 이전 것이 interrupted로 끝남 | ★★ |
| 45 | 히트박스 | 공격이 끊긴 뒤에도 칼에 닿으면 맞는다 | 노티파이 두 개(On/Off) vs **노티파이 스테이트**, 끊긴 몽타주는 뒤쪽 노티파이를 지나가지 않는다 | ★★ |

- 셋 다 GPU 없이 판정됩니다(`-nullrhi` 검증 완료). 랩 43는 헤드리스 실행에서 "렌더되지 않음"이 확실하므로 늘 재현됩니다. PIE에서는 NPC가 카메라 뒤 4m에 생기므로, 카메라를 돌려 NPC를 화면에 넣으면 그때부터 발자국이 찍히는 것까지 볼 수 있습니다.

---

# 10부: 멀티플레이

## PIE로 서버와 클라이언트 띄우기

1. 플레이 버튼 옆 **⋮** → **Number of Players: 2**, **Net Mode: Play As Listen Server**.
2. Play를 누르면 창이 두 개 뜹니다. 하나는 서버(리슨 서버 = 호스트 플레이어), 하나는 클라이언트입니다. 창 제목과 Output Log 줄 앞의 꼬리표로 구분합니다.
3. **서버 창**을 클릭하고 `~` → `BugHunt.Lab 46`.
4. 랩 출력에는 `[리슨 서버]`, `[클라이언트]` 꼬리표가 붙습니다.

- PIE의 서버와 클라이언트는 **같은 프로세스**입니다. 중단점 하나가 양쪽 모두에서 걸리므로, 멈출 때마다 조사식에 `GetNetMode()`(`NM_ListenServer` / `NM_Client`)나 `HasAuthority()`를 넣어 **지금 누구의 코드인지** 먼저 확인하세요.
- 이 랩들은 **Play As Listen Server**로 하세요. (Play As Client는 창이 없는 데디케이티드 서버를 띄워서, 서버 쪽 콘솔에 랩 명령을 넣기 번거롭습니다.)
- 네트워크 로그: 콘솔 `log LogNet Verbose`, `log LogNetTraffic Verbose`. 복제 상태: `net.ListActors`, `stat net`.

## 10부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 46 | 호스트 HP 바 | 리슨 서버 호스트 화면만 HP 바가 안 바뀐다 | **RepNotify**가 불리는 곳, `GetNetMode()`로 머신 구분 | ★★ |
| 47 | 보물 상자 | 클라이언트가 연 상자의 골드가 서버에 없다 | **권한(Authority)**, 복제 방향, **Server RPC** | ★★★ |

> 랩 47는 스탠드얼론에서는 통과합니다. 버그는 원격 클라이언트에만 있습니다. "내 PC에선 되는데요"의 멀티플레이 버전입니다.

- 다른 부에도 멀티플레이 랩이 있습니다: 랩 28(컨테이너, FastArray 복제), 랩 39(사운드, NetMulticast). 실행 방법은 같습니다.

---

# 11부: 동시성과 멈춤

이 부는 두 종류의 버그입니다. **멈춤**(랩 48, 49, 52)과 **데이터 레이스**(랩 50, 51). 둘 다 "모두 중단", 스레드 창, 병렬 스택으로 그 순간의 스레드들을 세워 봅니다.

## 멈춤 (무한루프와 데드락)

크래시도 오류 메시지도 없이 **게임이 굳는** 버그입니다. 신고는 "멈췄어요" 한 마디뿐이고, 로그도 굳기 직전에서 끊깁니다. 이럴 때는 굳은 프로그램을 **디버거로 멈춰 세워서** 각 스레드가 지금 어디서 무엇을 하는지 봅니다.

### 안전장치: 이 랩들은 진짜로 굳지 않습니다

진짜로 굳으면 에디터를 강제 종료해야 하므로, 멈춤 랩(40, 41, 44)의 루프와 대기에는 **시간 예산**이 붙어 있습니다(`Core/BugHuntHang.h`, 버그 없음).

```
LogBugHunt: Display: [FAIL] 응답 없음 (2003 ms / 213회 초과) - 엘리베이터 기다리기
```

- 예산(기본 **2초**)을 넘기면 위 줄을 찍고 그 루프나 대기에서 **빠져나옵니다**. 그 뒤 코드는 버그가 만든 상태 그대로 계속 돕니다. 채점기는 이 랩을 TIMEOUT 으로 매깁니다.
- 안전장치는 버그를 가리지 않습니다. 정상 코드는 예산의 1%도 쓰지 않고, 걸리면 다른 검사가 어떻게 나오든 그 랩은 실패입니다.
- `BH_HANG_CHECK(Guard)` 줄이나 `BugHunt::WaitWithBudget(...)`(= `Future.Get()`)을 지우지 마세요. 지우면 에디터가 진짜로 굳습니다. 버그는 그 줄이 아니라 루프 조건과 기다리는 방식에 있습니다.
- 예산은 콘솔 변수 **`BugHunt.HangBudgetSeconds`** 로 바꿉니다. "모두 중단" 실습을 하려면 `BugHunt.HangBudgetSeconds 30` 으로 늘린 뒤 랩을 실행하세요. 30초 동안 진짜로 굳어 있습니다.

### 굳은 프로그램을 추적하는 법

1. 콘솔 `BugHunt.HangBudgetSeconds 30` → `BugHunt.Lab 48`. 게임 화면이 굳습니다.
2. VS 에서 **디버그 → 모두 중단 (Ctrl+Alt+Break)**. 프로그램 전체가 그 순간에 멈춥니다. (에디터 창이 아니라 VS 창에서 누르세요.)
3. **디버그 → 창 → 스레드 (Ctrl+Alt+H)**. 수십 개의 스레드 중 **GameThread** 를 더블클릭하면 호출 스택이 게임 스레드로 바뀝니다. 모두 중단 직후의 "현재 스레드"는 게임 스레드가 아닐 수 있습니다.
4. 호출 스택에서 처음 나오는 `BugHuntUE.dll!` 줄을 더블클릭. 지역 창/조사식으로 루프 조건에 쓰인 값을 봅니다.
5. **F5 → 다시 모두 중단**을 몇 번 반복합니다. 매번 같은 루프 안이고, 조건에 쓰인 값이 **변하지 않거나 끝없이 커지면** 무한루프입니다.
6. 스레드가 둘 이상 얽혔다면 **디버그 → 창 → 병렬 스택 (Ctrl+Shift+D, S)**. 모든 스레드의 호출 스택이 한 그림에 나옵니다. 두 스레드가 서로 상대의 결과(잠금, Future, 이벤트)를 기다리고 있으면 데드락입니다.

| 도구 | 하는 법 | 언제 |
|---|---|---|
| **모두 중단** | VS: 디버그 → 모두 중단 (Ctrl+Alt+Break). Rider: Pause | 굳은 순간을 세우기 |
| **스레드 창** | Ctrl+Alt+H, GameThread 더블클릭, 우클릭 → 플래그 | 어느 스레드가 어디서 돌거나 기다리나 |
| **병렬 스택** | Ctrl+Shift+D, S | 데드락: 누가 누구를 기다리나 (랩 52) |
| 다음 문 설정 | 루프 뒤 줄에서 우클릭 → 다음 문 설정 (Ctrl+Shift+F10) | 굳은 루프에서 강제로 빠져나와 계속 관찰하기 (위험: 상태가 꼬일 수 있음) |
| **Unreal Insights** | 명령줄 `-trace=default` → Tools → Unreal Insights → Timing | 몇 번째 프레임이 몇 ms 걸렸나, 그 프레임 안에서 무엇이 시간을 먹었나. 굳기 전후의 긴 프레임을 찾는다 |
| `stat unit`, `stat game` | 콘솔 | 게임 스레드 ms 가 갑자기 튀는가 (히치) |
| 행 감지기 (`FThreadHeartBeat`) | `DefaultEngine.ini` `[Core.System]` `HangDuration=초` | **패키징한 게임**에서 게임 스레드가 N초 동안 하트비트를 안 보내면 그 스레드의 호출 스택을 로그에 남기고 크래시 리포트를 만든다. 에디터 빌드와 디버그 빌드에서는 꺼져 있고(엔진 `Build.h`의 `USE_HANG_DETECTION`), 엔진 기본값도 `HangDuration=0`(끔)이다. 디버거가 붙어 있으면 동작하지 않는다 |
| 히치 감지 | 패키징한 게임에서 명령줄 `-hitchdetection=0.1` (`ALLOW_HITCH_DETECTION`로 빌드했을 때) | 한 프레임이 0.1초를 넘으면 "Hitch detected on gamethread" 로그 |

> 이 랩들은 에디터 빌드에서 돌기 때문에 엔진의 행 감지기가 동작하지 않습니다. `Core/BugHuntHang.h`가 같은 일을 랩 안에서 대신합니다. 실제 프로젝트에서는 QA 빌드(Development, 패키징)에 `HangDuration`을 켜 두면 "멈췄어요" 신고에 호출 스택이 따라옵니다.

## 데이터 레이스

작업 스레드(`AsyncTask`, `ParallelFor`, `UE::Tasks`)가 같은 데이터를 잠금 없이 만지는 버그입니다. 진짜 레이스는 "백 번에 한 번"이라 실습으로 재현하기 어려우므로, 이 부의 랩에는 레이스가 일어날 틈을 넓히는 **확대경** `BugHunt::RaceWindow()` 가 있습니다(`Core/BugHuntProbe.h`, 버그 없음). 이 줄에 도착한 스레드는 다른 스레드가 같은 줄에 올 때까지 잠깐 기다리므로, 보호되지 않은 코드에서는 **매번** 겹칩니다. 지워도 버그는 남고 "가끔"이 될 뿐입니다.

### 데이터 레이스에서 쓰는 도구

| 도구 | 하는 법 | 언제 |
|---|---|---|
| **스레드 창** | 디버그 → 창 → 스레드 (Ctrl+Alt+H). 스레드 이름(GameThread, Foreground Worker, Background Worker...) | 지금 멈춘 코드가 어느 스레드에서 도는가 |
| **병렬 스택** | Ctrl+Shift+D, S | 같은 함수에 동시에 서 있는 스레드가 몇 개인가 |
| 추적점 `{FPlatformTLS::GetCurrentThreadId()}` | 중단점 → 작업 | 스레드 번호와 값을 같이 찍어 순서 보기 |
| **Unreal Insights** | 명령줄 `-trace=default,task` → Timing 창의 스레드 트랙 | 어떤 작업이 어느 스레드에서 언제 겹쳐 돌았나 |
| `check(IsInGameThread())` | 게임 스레드 전용 코드 맨 위 | 다른 스레드에서 들어오면 그 자리에서 멈춤 (엔진 곳곳에 있음) |
| `FCriticalSection` + `FScopeLock` | `FScopeLock Lock(&CS);` 블록 끝에서 자동 해제 | 읽기-계산-쓰기를 한 덩어리로 |
| `std::atomic` / `TAtomic` | `Count.fetch_add(1)` | 숫자 하나를 여러 스레드가 더할 때 |
| `AsyncTask(ENamedThreads::GameThread, ...)` | 작업 스레드에서 게임 스레드로 결과 보내기 | UObject 를 고쳐야 할 때 (랩 51, 52) |

## 11부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 48 | 엘리베이터 | 엘리베이터를 부르면 게임이 굳는다 | 게임 스레드에서 **다른 액터의 Tick 이 바꿔 줄 값을 while 로 기다리기**, 모두 중단, 스레드 창, 델리게이트로 바꾸기 | ★ |
| 49 | 길찾기 | 끊긴 다리 너머로 보내면 게임이 굳는다 | 너비 우선 탐색의 **방문 체크 누락**(지역 변수 수명), 도달 불가능한 입력으로 테스트하기 | ★★ |
| 50 | 길드 금고 | 동시에 입금하면 골드가 모자란다 | `ParallelFor` 와 **읽기-계산-쓰기 레이스(lost update)**, `FCriticalSection`/`FScopeLock`, 원자 연산, 결과를 칸별로 모으기 | ★★ |
| 51 | 하이라이트 점수 | 리플레이 분석이 끝나면 크래시한다 | **게임 스레드 전용 객체를 작업 스레드에서 만짐**, 스레드 창, `AsyncTask(GameThread)` | ★ |
| 52 | 자동 저장 | 자동 저장할 때 게임이 굳는다 | 게임 스레드에서 **비동기 결과를 동기로 기다림** → 그 작업이 게임 스레드를 기다리는 **데드락**, 병렬 스택, `AsyncTask(GameThread)` 로 결과 받기 | ★★★ |

- 다섯 다 GPU 없이 판정됩니다(`-nullrhi` 검증 완료). 랩 51 은 버그 상태에서 CRASH 입니다.
- 랩 52 에서 데드락이 풀려도 "자동 저장이 게임을 세우지 않았다" 검사가 남습니다(저장이 요청한 프레임보다 뒤에 끝났는지 프레임으로 셉니다). 결과를 기다리느라 게임 스레드를 세우는 것 자체가 히치이기 때문입니다.

---

# 12부: 성능

성능 버그는 결과가 맞습니다. "느려요", "멈칫해요"만 있습니다. 그래서 이 부의 랩은 **시간 대신 횟수**(검색 횟수, 메모리 할당 횟수, Tick 호출 횟수, 동기 로드 횟수)로 판정합니다. 시간은 PC 마다 다르지만 횟수는 같기 때문입니다. 실제 게임에서는 아래 도구로 시간을 먼저 보고, 그 안에서 횟수를 셉니다.

## 12부에서 쓰는 도구

| 도구 | 하는 법 | 언제 |
|---|---|---|
| `stat unit` | 콘솔 | Frame / Game / Draw / GPU 중 어느 스레드가 느린가 |
| `stat game` | 콘솔 | 게임 스레드 안에서 Tick, 타이머 등이 쓰는 시간 |
| `stat startfile` / `stat stopfile` | 콘솔. `Saved/Profiling/` 에 파일 | 몇 초 동안의 통계를 기록해 나중에 보기 |
| **Unreal Insights** | 명령줄 `-trace=cpu` (할당까지: `-trace=cpu,memalloc`, 로드까지: `-trace=cpu,loadtime`) → Tools → Unreal Insights | 긴 프레임을 골라 그 안의 함수별 시간을 보기. 가장 먼저 쓰는 도구 |
| `dumpticks` | 콘솔 | 등록된 틱 함수 수와 목록 (랩 55) |
| `obj list class=...` | 콘솔 | 무엇이 메모리에 올라와 있나 (랩 56) |
| `log LogStreaming Verbose` | 콘솔 | 에셋 로드가 언제 일어나는지 로그로 |

## 12부 랩 목록

| # | 이름 | 증상 | 새로 배우는 것 | 난이도 |
|---|---|---|---|---|
| 53 | 미니맵 | 아이콘이 많으면 게임이 느려진다 | Tick 마다 **`GetAllActorsOfClass`**, BeginPlay 에서 찾아 기억하기 | ★ |
| 54 | 조준선 | 조준하는 동안 메모리 할당이 잦다 | 프레임마다 지역 `TArray` 재할당, **`Reserve`**, 멤버 + `Reset`, `TInlineAllocator` | ★ |
| 55 | 횃불 | 횃불이 많으면 게임이 느려진다 | 할 일 없는 **Tick 끄기**, 타이머, `TickInterval`, Significance Manager | ★ |
| 56 | 보스 등장 | 보스가 나올 때 화면이 멈칫한다 | 게임 중 **동기 로드**(`LoadSynchronous`/`LoadObject`), `RequestAsyncLoad` 로 미리 불러 두기 | ★★ |

- 넷 다 GPU 없이 판정됩니다. 판정은 횟수로만 합니다(로그의 ms 는 참고용).

---

# 채점기

모든 랩을 헤드리스(GPU 없이 `-nullrhi`)로 돌려 랩별 결과와 총점을 냅니다. 에디터를 닫고 프로젝트 폴더에서:

```bat
grade.bat
```

먼저 `build.bat`으로 빌드한 뒤, 랩마다 **새 게임 프로세스**를 띄워 `-BugHuntLab=N -BugHuntExit`로 실행하고 그 로그를 읽습니다. 랩 하나에 프로세스 시작을 포함해 25~35초(멀티플레이 랩은 45초 안팎), 48개 전체는 25분 안팎 걸립니다.

> ⚠️ 채점기는 사운드 랩(29~31)을 오디오 장치로 실제 재생합니다. 돌리기 전에 시스템 볼륨을 낮추고, 헤드폰을 쓴다면 귀에서 떼 두세요.

```
  #  부   결과       시간  랩  - 내용
  1  1부  CRASH     44.5s  회복 아이템  - Unhandled Exception: EXCEPTION_ACCESS_VIOLATION reading address 0x0000000000000470
  5  2부  FAIL      52.3s  전투 기록 창  - 다시 연 창의 기록 줄 수 (기대: 1, 실제: 2) 외 1개
 21  5부  FAIL      46.5s  어그로 표  - Bram 의 어그로 (Bram 이 준 피해) (기대: 15, 실제: 83) 외 1개
 38  10부  FAIL      49.1s  호스트 HP 바  - [리슨 서버] 화면의 HP 바 (기대: 43, 실제: 100)
 40  11부  TIMEOUT   28.1s  엘리베이터  - 응답 없음 (2008 ms / 212회 초과) - 엘리베이터 기다리기
 ...
점수: 0 / 56  (SKIP 0개 제외)
```

| 결과 | 뜻 |
|---|---|
| PASS | 랩의 검사가 모두 통과 |
| FAIL | `[FAIL]` 이 하나라도 있음 (내용 칸은 첫 번째 FAIL) |
| TIMEOUT | 11부 안전장치가 "응답 없음"을 보고했거나, 랩 프로세스가 제한 시간(기본 180초) 안에 끝나지 않아 채점기가 종료함 |
| CRASH | 프로세스가 크래시(`Unhandled Exception`, `Assertion failed`, `Fatal error`)했거나 결과 줄 없이 끝남 |
| SKIP | 이 환경에서 판정할 수 없음 (예: 오디오 장치가 없는데 랩 38). GPU 렌더링 결과가 필요한 랩도 SKIP 하지만 지금 56개 중에는 없습니다 |

점수는 `PASS 수 / (전체 - SKIP)` 입니다.

- **숨은 입력:** 채점기는 실행할 때마다 무작위 시드를 정해 `-BugHuntSeed=시드`로 넘깁니다. 이를 지원하는 랩(5, 11~36, 46, 48~55)은 README 에 적힌 공개 입력과 **다른 값**(피해량, 개수, 그래프 모양, 시간)으로 돕니다. 기대값을 코드에 박아 넣은 "수정"은 여기서 걸립니다. 같은 입력을 PIE 에서 재현하려면 표 위에 찍힌 시드로 `BugHunt.Lab 29 시드`.
- **멀티플레이 랩(20, 31, 38, 39)** 은 리슨 서버 프로세스(`?listen -BugHuntWaitForClient`)와 클라이언트 프로세스(`127.0.0.1:포트`)를 띄웁니다. 클라이언트 쪽 `[FAIL]` 도 그 랩의 FAIL 로 셉니다.
- **결과 파일:** `grade-report.json` (프로젝트 폴더, 다른 BugHunt 판과 같은 형식). 랩별 로그는 `Saved/Grade/LabNN.log`, 클라이언트는 `LabNN_client.log`.

```json
{ "edition": "unreal", "version": 1, "total": 56, "passed": 0,
  "labs": [ { "id": 1, "title": "회복 아이템", "part": 1, "result": "CRASH", "ms": 35800, "detail": "Unhandled Exception: EXCEPTION_ACCESS_VIOLATION ..." } ] }
```

| 옵션 | 하는 일 |
|---|---|
| `grade.bat -labs=29-32` | 일부 랩만 (예: `-labs=1-4,29`) |
| `grade.bat -seed=1234` | 숨은 입력 시드 고정 (같은 결과를 다시 보기) |
| `grade.bat -public` | 숨은 입력 대신 공개 입력으로 |
| `grade.bat -nobuild` | 빌드 건너뛰기 |
| `grade.bat -timeout=300` | 랩 하나의 제한 시간(초) |
| `grade.bat -gpu` | `-nullrhi` 없이 실행 (GPU 가 필요한 랩도 채점) |
| `set UE_ROOT=D:\Epic\UE_5.8` | 엔진 위치 바꾸기 (기본 `F:\Program Files\UE_5.8`, `build.bat`도 같음) |

채점기 자체는 커맨드렛입니다(`Core/BugHuntGradeCommandlet.cpp`). `grade.bat` 없이 직접 부르려면 `UnrealEditor-Cmd.exe BugHuntUE.uproject -run=BugHuntGrade -labs=29-32`.

> 채점기는 정답을 알지 못합니다. 랩이 스스로 찍는 `[PASS]`/`[FAIL]`을 모을 뿐입니다. 랩의 검사 코드(`StartLab`의 `Expect` 줄)나 `Core/`를 고쳐서 통과시키는 것은 반칙입니다.

---

# 부록: Live Coding 함정

Live Coding(Ctrl+Alt+F11)은 **실행 중인 에디터의 함수 코드를 바꿔치기**합니다. 이미 만들어진 객체, 이미 등록된 클래스 정보는 바꾸지 못합니다. 직접 겪어 보세요.

1. 에디터를 켠 채로 `Lab01_Pickup.h`에서 `int32 Health = 10;`을 `int32 Health = 40;`으로 바꾸고 Ctrl+Alt+F11.
   → 헤더 변경이라 Live Coding이 거부하거나 경고합니다. Output Log의 **LogLiveCoding** 줄을 읽어 보세요.
2. 이번엔 `Lab09_Archer.cpp`에서 `constexpr int32 KillReward = 100;`을 `50`으로 바꾸고 Ctrl+Alt+F11 → PIE에서 랩 9. 반영되나요? (함수 본문/`.cpp` 상수는 대체로 반영됩니다.)
3. `Lab47_Chest.h`에 `UFUNCTION(Server, Reliable)`을 추가하고 Ctrl+Alt+F11. 무슨 일이 생기나요? → 랩 47의 정답은 **에디터를 닫고 빌드**해야만 적용됩니다.
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
| `BugHunt.HangBudgetSeconds 초` | 11부 멈춤 랩 안전장치의 시간 예산 (기본 2). "모두 중단" 실습 때 30 |
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
| `BugHunt.Lang ko` / `BugHunt.Lang ja` | 랩·채점기 출력 언어를 실행 중에 바꾸기. 값이 비어 있으면(기본) 에디터/OS 언어를 따른다 |

## 명령줄 옵션

| 옵션 | 하는 일 |
|---|---|
| `-game` | 에디터 대신 게임으로 바로 실행 |
| `-BugHuntLab=N` | 월드가 시작되면 랩 N 자동 실행 |
| `-BugHuntExit` | 랩이 끝나면 종료 (자동 검증용) |
| `-BugHuntSeed=S` | 숨은 입력(시드 S)으로 실행 (채점기가 쓴다) |
| `-stomp2malloc` | 해제된 메모리/범위 밖 접근을 그 자리에서 잡는 할당기 (언리얼판 ASan. UE 5.8 기본으로는 **Windows 에디터 빌드 전용**: `MallocStomp2.h` 의 `WITH_MALLOC_STOMP2 = WITH_EDITOR && PLATFORM_WINDOWS`. 게임 빌드에서는 명령줄을 줘도 아무 일도 없다). 아주 느리고 메모리를 많이 씀. `-MallocStomp2MinSize=`, `-MallocStomp2MaxSize=`로 크기 범위를 좁힐 수 있음 |
| `-log` | 별도 로그 창 띄우기 |
| `-BugHuntLang=ko` / `-BugHuntLang=ja` | 랩·채점기 출력 언어 (`grade.bat -BugHuntLang=ja` 도 같음). 정하는 순서: `BugHunt.Lang` 값(이 옵션이 넣고, 실행 중 콘솔로 바꾸면 그 값) → 에디터/OS 언어 → 한국어. 헤드리스 채점에서도 똑같이 나오도록 LOCTEXT 대신 소스 안의 한국어/일본어 표(`Core/BugHuntText.h`)를 쓴다. `grade-report.json` 의 키와 `result` 는 언어와 상관없이 같고 `title`/`detail` 만 바뀐다 |

## 다시 풀고 싶을 때

이 폴더는 git 저장소입니다. 고친 뒤 `git restore Source`로 버그 상태로 되돌릴 수 있습니다.

</details>

<details>
<summary><b>日本語</b></summary>

> 注: ラボの実行出力と採点ツールのメッセージは、コマンドライン `-BugHuntLang=ja`(採点は `grade.bat -BugHuntLang=ja`)、コンソール変数 `BugHunt.Lang ja`、エディター/OS の言語が日本語、のいずれかで日本語になります (実行中にコンソールで変えた値が最優先、次にコマンドライン、最後にエディター/OS の言語)。それ以外は韓国語です。

[BugHunt](https://github.com/gusgh0157/BugHunt)(C++ コンソール版)の Unreal Engine 版です。
バグが**わざと仕込まれた**小さなゲームコードの集まりで、今回は **Unreal でしか起きない**バグを集めています。

> **UE6 について (2026 年 11 月時点):** Epic は UE6 で **Actor、Blueprint、そして C++ で書かれたゲームプレイ フレームワークを廃止**し、ゲームプレイを **Verse** と **Scene Graph** (Entity + Component) ベースの新しいフレームワークに移すと明らかにしました。当初のロードマップは「非推奨にしたあと段階的に廃止」でしたが、Unreal Fest Tokyo 2026 (11 月 3 日) の基調講演で Tim Sweeney が **UE6 アーリーアクセス (2027 年末予定) から全面廃止**に変わったと発表し、AI による変換支援ツールを提供する予定です。C++ はレンダリング・物理のようなエンジンの下層に残ります。出典: [Gamemakers.jp 基調講演の記事](https://gamemakers.jp/article/2026_11_03_149187/)、[Epic のロードマップ "The road to UE6"](https://www.unrealengine.com/news/the-road-to-ue-6)。
> このリポジトリは **UE 5.8 の C++ ゲームプレイ フレームワーク**を基準にしています。ここで身につけること (エンジンのフレームが混ざったコールスタックの読み方、GC と `UPROPERTY`、レプリケーションと権限、Tick の順序、非同期ロード、デバッガーのツール) は UE6 でもエンジン側の C++ デバッグにそのまま使えますが、Actor/Blueprint ベースのラボのコード自体は UE6 の新しいフレームワークに移して書き直す必要があるかもしれません。

- **第1部: クラッシュの追跡 (ラボ 1~4)**: エディターやゲームが落ちたり `Ensure` が出たりした瞬間から、コールスタックをさかのぼって原因を探します。エンジンのフレームが混ざった長いスタックを読む練習です。
- **第2部: 処理の流れの追跡 (ラボ 5~13)**: ゲームは普通に動いているのに**結果が間違っています**。デリゲート、タイマー、Tick の順序、GC、Config のように「Unreal が代わりにやってくれること」の順序と寿命を自分で追いかけます。30fps と 120fps で動きが変わるコード(抜けている `DeltaSeconds`、フレームごとの減速)と、初期化されていない構造体メンバーもここで扱います。
- **第3部: トランスフォームと空間 (ラボ 14~21)**: 位置・回転・スケールが見当違いの場所に現れるバグ。相対/ワールド座標、アタッチのルール、クォータニオンの積の順序、ワールドの軸とアクターの軸、点と方向の変換、非均一スケールとコリジョン、Mobility、ソケット名。原点から離れた場所で位置・方向で判定します。
- **第4部: コンテナ (ラボ 22~28)**: `TArray`/`TMap`/`TSet` の範囲チェックの assert、存在しないキー、再割り当て後のポインター、順序を変える削除、ハッシュと `==` の不一致、GC やレプリケーション(`FFastArraySerializer`)と組み合わさったコンテナ。
- **第5部: ポインターと寿命 (ラボ 29~32)**: 落ちないポインターのバグ。削除されたアクターを覚えている raw ポインター、オブジェクトより長生きするタイマーのラムダ、`Add` の後の配列要素への参照、`TSharedPtr` の循環参照を、アドレスと参照カウントで追跡します。
- **第6部: 物理 (ラボ 33~36)**: トンネリングと CCD、チャンネルのレスポンスと Overlap イベント、スイープなしの移動、物理オブジェクトを Tick で動かすこと。Chaos Visual Debugger でフレームを巻き戻します。
- **第7部: サウンド (ラボ 37~39)**: 音が遅れて鳴る、途切れる、1 人にしか聞こえないバグ。遅れをフレーム/ms で測って判定します。
- **第8部: 画像 / レンダリング (ラボ 40~42)**: 色が変わらない、関係ないアイコンが出る、色が薄く見えるバグ。マテリアルのパラメータ値とブラシに入ったテクスチャを調べて判定します。
- **第9部: アニメーション (ラボ 43~45)**: 足音が鳴らない、連打すると状態がおかしくなる、中断された攻撃のヒットボックスが残るバグ。通知(ノーティファイ)の回数と状態の値で判定します。
- **第10部: マルチプレイ (ラボ 46~47)**: サーバーとクライアントが**違う値**を見ているバグ。PIE で複数のクライアントを立ち上げ、1 つのデバッガーで一緒に追跡します。
- **第11部: 並行処理とフリーズ (ラボ 48~52)**: クラッシュせずにゲームが固まる無限ループとデッドロック、ワーカースレッドがロックなしで同じ値を書き換えたりゲームスレッド専用のオブジェクトに触ったりするデータレース。「すべて中断」、スレッド ウィンドウ、並列スタックでその瞬間を止めて見ます。(練習用の安全装置があるのでエディターが本当に固まることはなく、レースは再現用の拡大鏡で毎回起きるようにしてあります。)
- **第12部: パフォーマンス (ラボ 53~56)**: Tick ごとのアクター検索、フレームごとの配列の再割り当て、数百個の Tick、ゲーム中の同期ロード。時間ではなく回数で判定します。
- **採点ツール**: `grade.bat` 1 つで全ラボをヘッドレスで実行し、PASS/FAIL/TIMEOUT/CRASH/SKIP と点数を出します。隠し入力でハードコードを防ぎます。
- **付録: Live Coding の落とし穴**: 「直したのに変わりません」のよくある原因。

> コンソール版と同じルールです。コードを目で流し読みして答えを探さないでください。第1部は**落ちるようにしてデバッガーが止まった場所から**、第2部以降は**自分で止めた場所から値を確認しながら**追跡してください。
> コンソール版の第1部・第4部を先に終えているとよいです。呼び出し履歴 (Call Stack) ウィンドウ、ローカル ウィンドウ、条件付きブレークポイント、トレースポイント、データ ブレークポイントの使い方は、そこで説明したとおりに使います。

<details>
<summary>ラボ番号が変わりました (以前の番号 → 新しい番号)</summary>

ラボが 56 個に増えたので、似たテーマ同士が集まるように部と番号を振り直しました。ラボ 1~10 はそのままで、第3部 (トランスフォームと空間、ラボ 14~21) は新しく加わったラボです。以前の番号で書いたメモや `-BugHuntLab=` 引数があれば、下の表で読み替えてください。

| 以前の番号 | 新しい番号 | ラボ |
|---|---|---|
| 11, 12 | 46, 47 | ホストの HP バー、宝箱 (第10部 マルチプレイ) |
| 13~15 | 37~39 | ブラスターの発射音、爆発する樽、警報の鐘 (第7部 サウンド) |
| 16~18 | 40~42 | 箱のハイライト、インベントリのアイコン、チームカラー (第8部 画像) |
| 19~21 | 43~45 | 足跡、連打コンボ、ヒットボックス (第9部 アニメーション) |
| 22~25 | 29~32 | ヘイト表、砲台、ポーションベルト、会話ツリー (第5部 ポインターと寿命) |
| 26, 27 | 48, 49 | エレベーター、経路探索 (第11部 並行処理とフリーズ) |
| 28 | 52 | オートセーブ (第11部) |
| 29~35 | 22~28 | ウェーブ報酬 … クエスト掲示板 (第4部 コンテナ) |
| 36~38 | 11~13 | ダッシュ、氷の滑り、ラウンドスコアボード (第2部) |
| 39~42 | 33~36 | 狙撃弾、感圧板、ノックバック、投石機 (第6部 物理) |
| 43, 44 | 50, 51 | ギルド金庫、ハイライトスコア (第11部) |
| 45~48 | 53~56 | ミニマップ、照準線、たいまつ、ボス登場 (第12部 パフォーマンス) |

</details>

## 扱うバグの一覧

1 つのラボが複数の行にまたがることがあります。「未収録」は、そのカテゴリーの代表的な例を扱うラボがまだないという意味です。

| カテゴリー | ラボ | 代表的な例 |
|---|---|---|
| ポインター・寿命・GC | 1, 4, 9, 27, 29, 30, 31, 32 | `Cast` の失敗による nullptr、`UPROPERTY` のないポインターと GC、`Destroy` の後の `IsValid`、`TWeakObjectPtr`、ラムダの `[this]`、`TSharedPtr` の循環 |
| 流れ・順序 | 2, 5, 6, 7, 8, 10 | BeginPlay の順序、デリゲートの解除漏れ、タイマーハンドル、Tick グループ、Visual Logger、CDO/Config のデフォルト値 |
| フレームレート依存 | 11, 12 | 抜けている `DeltaSeconds`、フレームごとの減速(積分)、`t.MaxFPS`、固定タイムステップ |
| 初期化されていない値 | 13 | 構造体メンバーの初期値なし、`AddDefaulted` と残ったメモリ。(ローカル変数、`FMemory::Malloc` は説明のみ) |
| トランスフォーム・空間 | 14, 15, 16, 17, 18, 19, 20, 21 | 相対/ワールド座標、`FAttachmentTransformRules`、`FQuat` の積の順序 (ローカル/ワールド回転)、`FVector::ForwardVector` vs `GetActorForwardVector`、`TransformPosition`/`TransformVector`、非均一スケールとコリジョン、Mobility Static、存在しないソケット名。(ピボットのオフセット、ジンバルロックは説明のみ) |
| コンテナ | 3, 22, 23, 24, 25, 26, 27, 28, 31 | ループ中の削除、範囲外のインデックス、存在しないキーの `operator[]`、`Add` 後の要素参照/`Find` のポインター、`RemoveAtSwap` の順序、ハッシュ/`==` の不一致、GC と `TArray<UObject*>`、FastArray のレプリケーション |
| 物理 | 33, 34, 35, 36 | トンネリングと CCD、チャンネルのレスポンスとオーバーラップイベントの生成 (Generate Overlap Events)、`bSweep=false` の瞬間移動、シミュレーション中のオブジェクトを Tick で動かすこと。(サブステップの違いは説明のみ) |
| サウンド | 37, 38, 39 | ソフト参照の遅延ロード、アタッチしたコンポーネントの寿命、NetMulticast |
| 画像・レンダリング | 40, 41, 42 | MID、非同期ロードのコールバックの順序、sRGB/Linear |
| アニメーション | 43, 44, 45 | レンダリングされないときの通知、モンタージュの interrupted、通知ステート |
| マルチプレイ | 28, 39, 46, 47 | RepNotify、権限と Server RPC、音はレプリケートされない、FastArray の `MarkItemDirty` |
| フリーズ (無限ループ・デッドロック) | 48, 49, 52 | ゲームスレッドの while 待ち、訪問チェックの漏れ、Future を同期で待つデッドロック |
| データレース | 50, 51 | `ParallelFor` の読み取り-計算-書き込みのレース、ワーカースレッドで UObject に触る(`check(IsInGameThread())`) |
| パフォーマンス | 53, 54, 55, 56 | Tick ごとの `GetAllActorsOfClass`、`Reserve` の漏れ、Tick の乱用、ゲーム中の同期ロード。(毎フレームの `FString` 生成はラボ 54 のヒントでのみ扱う) |

## BugHunt シリーズ

同じ方式(わざと仕込んだバグをデバッガーで追跡する)で作った 4 つの版です。

| 版 | 扱うバグ | リポジトリ |
|---|---|---|
| C++ コンソール (標準 C++20、CMake) | 呼び出し履歴、フロー追跡、演算子、コンテナ、言語の落とし穴、インターフェースと継承、ファイル入出力、シリアライズ、ポインターとメモリ、非同期と並行性、ハング、パフォーマンス (ラボ 98 個 + チャレンジ 11 個) | [BugHunt](https://github.com/gusgh0157/BugHunt) |
| C# コンソール (.NET 10) | 例外、フロー追跡、コンテナ、言語の落とし穴、インターフェースと継承、ファイル入出力、シリアライズ、非同期と並行性、リソースとメモリ、unsafe、ハング、パフォーマンス (ラボ 95 個 + チャレンジ 11 個) | [BugHuntCS](https://github.com/gusgh0157/BugHuntCS) |
| Unreal Engine (UE 5.8) | クラッシュ、フロー追跡(フレームレート・初期化を含む)、トランスフォーム、コンテナ、ポインターと寿命、物理、サウンド、画像、アニメーション、マルチプレイ、並行性とハング、パフォーマンス (ラボ 56 個) | [BugHuntUE](https://github.com/gusgh0157/BugHuntUE) **(今見ているリポジトリ)** |
| Unity (Unity 6) | 🚧 **工事中** — クラッシュ/例外、フロー追跡(フレームレート依存を含む)、コンテナ、参照とネイティブメモリ、物理、サウンド、画像、アニメーション、ハング、パフォーマンス、データレース (ラボ 55 個)。トランスフォーム部の追加と再配置は作業ブランチで進行中 | [BugHuntUnity](https://github.com/gusgh0157/BugHuntUnity) |

## 準備

必要なもの: **Unreal Engine 5.8**、**Visual Studio 2022 以降**(ワークロード「C++ によるゲーム開発」、コンポーネント「Unreal Engine 用 IDE サポート」)または Rider。

1. `BugHuntUE.uproject` を右クリック → **Generate Visual Studio project files**。(`BugHuntUE.sln` ができます。)
   - `EngineAssociation` は `5.8` です。ランチャーで 5.8 をインストールしていれば、uproject をそのまま開けます。ソースビルドのエンジンを使う場合は、uproject を右クリック → **Switch Unreal Engine version...** でそのエンジンを選んでください。
2. `BugHuntUE.sln` を開きます。ソリューション エクスプローラーで **BugHuntUE** を右クリック → **スタートアップ プロジェクトに設定**。
3. ツールバーで構成を **Development Editor**、プラットフォームを **Win64** にします。
4. **F5**。ビルドが終わると、デバッガーがアタッチされた状態でエディターが起動します。(初回はシェーダーのコンパイルで数分かかります。)

コマンドラインでのビルド: エディターを閉じて `build.bat` (デフォルトは Development) または `build.bat DebugGame`。

> 💡 このモジュールは `BugHuntUE.Build.cs` で `OptimizeCode = CodeOptimization.Never` にして最適化を切っています。そのため Development Editor でもラボコードの変数が「最適化されているため使用できません」と表示されず、F10 で行が飛ばされません。実際のプロジェクトで同じ問題に出会ったら **DebugGame Editor** 構成を使ってください。

### デバッガーをアタッチする 3 つの方法

| 方法 | やり方 | いつ |
|---|---|---|
| VS で F5 | 上の準備の 4 番 | 基本。エディターの起動時からデバッガーがアタッチされている |
| 実行中のエディターにアタッチ | エディターを普通に起動 → VS で **デバッグ → プロセスにアタッチ (Ctrl+Alt+P)** → `UnrealEditor.exe` を選ぶ | すでにエディターを起動した状態でバグが起きたとき。再起動が要らない |
| Rider | **Run → Attach to Unreal Editor**、または Run 構成 `BugHuntUE` で Debug | Rider ユーザー |

> デバッガーなしでクラッシュすると**クラッシュレポーター (Crash Reporter)** ウィンドウにコールスタックが出て、同じ内容が `Saved/Logs/BugHuntUE.log` の一番下に残ります。ただしローカル変数は見られないので、**デバッガーをアタッチしてもう一度再現**してください。

## ラボを実行する

1. エディター上部の**再生 (Play) (▶)** を押します。(PIE: プレイインエディタ / Play In Editor)
2. ゲーム画面をクリックし、**`~`(チルダ)** キーでコンソールを開きます。
3. `BugHunt.Lab 1` と入力します。(`BugHunt.List` は一覧)
4. 結果は画面の左上と**出力ログ (Output Log)**(メニューの ウィンドウ → 出力ログ / Window → Output Log)に出ます。出力ログの検索欄に `LogBugHunt` と入れると、ラボの結果だけが表示されます。

```
LogBugHunt: Display: ==== ラボ 5: 戦闘ログウィンドウ ====
LogBugHunt: Display: [FAIL] 開き直したウィンドウの記録行数 (期待: 1, 実際: 2)
LogBugHunt: Display: [PASS] 騎士の HP (値: 78)
LogBugHunt: Display: ==== ラボ 5 結果: PASS 1 / FAIL 2 ====
```

**同じラボを繰り返し実行するとき (コンソール版の「コマンド引数」と同じ方法)**: ソリューション エクスプローラーで BugHuntUE を右クリック → プロパティ → デバッグ → **コマンド引数**を

```
"$(SolutionDir)BugHuntUE.uproject" -game -BugHuntLab=5
```

に変えて F5。エディターの代わりにゲームウィンドウがすぐ開き、ラボ 5 が自動で実行されます。エディターの読み込みを待たないので、ずっと速いです。(元に戻すには `-game -BugHuntLab=5` を消してください。)

### 直したコードを反映する方法

| 変えたもの | 反映方法 |
|---|---|
| `.cpp` の関数本体だけ | エディターを起動したまま **Ctrl+Alt+F11** (ライブコーディング / Live Coding) → コンパイルが終わったら PIE を開始し直す |
| ヘッダー(`.h`)、`UPROPERTY`/`UFUNCTION`、コンストラクター、新しいファイル | **エディターを閉じて** VS で F5 (ビルドし直す) |

迷ったら、いつでも「エディターを閉じてビルド」が安全です。理由は[付録](#付録-live-coding-の落とし穴)で実際に体験します。

---

# 第1部: クラッシュの追跡

## コンソール版との違い: エンジンのフレーム

Unreal のクラッシュのコールスタックは長く、ほとんどがエンジンのコードです。

```
UnrealEditor-BugHuntUE.dll!ABH_Hero::Heal(int Amount) 行 21              ← 自分のコード (ここで落ちた)
UnrealEditor-BugHuntUE.dll!ABH_HealthPickup::GiveHealth(ABH_Hero * Hero) 行 63
UnrealEditor-BugHuntUE.dll!ABH_HealthPickup::OnTouched(AActor * Toucher) 行 58
UnrealEditor-BugHuntUE.dll!ABH_HealthPickup::HandleOverlap(...) 行 52
UnrealEditor-BugHuntUE.dll!ABH_HealthPickup::execHandleOverlap(...)      ← UHT が生成したコード (.gen.cpp)
UnrealEditor-CoreUObject.dll!UFunction::Invoke(...)                     ← エンジン
UnrealEditor-CoreUObject.dll!UObject::ProcessEvent(...)
UnrealEditor-Engine.dll!...FComponentBeginOverlapSignature...           ← デリゲートの Broadcast
UnrealEditor-Engine.dll!UPrimitiveComponent::BeginComponentOverlap(...)
UnrealEditor-Engine.dll!UPrimitiveComponent::UpdateOverlapsImpl(...)
...
UnrealEditor-BugHuntUE.dll!ABH_Lab01::StartLab() 行 82                   ← また自分のコード: 「どこから始まったか」
```

- エンジンのフレームが `UnrealEditor-Engine.dll!UnknownFunction` やアドレスだけで表示される場合は、エンジンのデバッグシンボルがありません。Epic Games Launcher → ライブラリ → エンジンバージョンの横の ▼ → **オプション** → **Editor symbols for debugging** をインストールすると、上のように関数名が出ます。(数 GB。なくてもラボは解けます。自分のコードのフレームは常に名前と行番号が出ます。)
- モジュール名で見分けてください。**`UnrealEditor-BugHuntUE.dll!`** が自分のコード、`UnrealEditor-Engine.dll!`、`UnrealEditor-CoreUObject.dll!` などはエンジンです。
- エンジンのフレームは飛ばしてかまいませんが、**どのエンジン機能を経由してきたか**(オーバーラップイベント、SpawnActor、Tick、タイマー、デリゲート...)は名前だけ読んでおいてください。それが「なぜこのタイミングで呼ばれたか」の答えです。
- `exec関数名` は UHT が生成したつなぎのコードです。ブループリント/デリゲートから C++ 関数に入ってくる入り口だと考えてください。

## Unreal の 3 つのチェック: check、ensure、verify

| マクロ | 失敗すると | デバッガーがアタッチされていると | 配布 (Shipping) ビルド |
|---|---|---|---|
| `check(条件)` / `checkf` | **プログラム終了** ("Assertion failed") | その行で止まる | チェック自体が消える |
| `ensure(条件)` / `ensureMsgf` | 出力ログに **"Ensure condition failed"** + コールスタックを出して**実行を続ける** | その行で止まる → F5 で続行できる | チェックが消える |
| `verify(式)` | check と同じだが、Shipping でも**式は実行される** | その行で止まる | 式だけ実行 |

> `ensure` は同じ行について**セッションごとに 1 回だけ**報告します。ラボを 2 回実行して 2 回目に出ないのは正常です。エディターを起動し直すとまた出ます。(毎回見たいなら `ensureAlways`)

## 追跡の 4 ステップ (コンソール版と同じ)

1. **どこで止まったか?** コールスタックで **`BugHuntUE.dll!` が最初に出てくる行**を探す。
2. **何がおかしいか?** ローカル ウィンドウで nullptr、おかしな名前、ゴミ値を探す。Unreal の型(FString、TArray、UObject)は natvis のおかげで中身がすぐに見えます。
3. **その値はどこから来たか?** 下のフレームに下りながら出どころを探す。エンジンのフレームは飛ばす。
4. **本当の原因は?** おかしな値が**最初に作られた場所**を直す。

```
記録表 (第1部用)
止まった関数:              
おかしな値:              
その値が来たフレーム:       
経由したエンジン機能:        
本当の原因:              
修正:                   
```

## やってみよう: ラボ 1 (回復アイテム)

1. VS で **F5** を押してエディターを起動し、再生 → `~` → `BugHunt.Lab 1`。
2. 画面に「開始 HP 10...」と出たあと、VS が前面に出て止まります。**「例外がスローされました: 読み取りアクセス違反。 this が nullptr でした。」**
3. **Ctrl+Alt+C** で呼び出し履歴を開きます。一番上が `ABH_Hero::Heal` です。→ *止まった関数*
4. **Alt+4** でローカル ウィンドウを開きます。`this` が `0x0000000000000000` です。ヒーローがいないのに回復しています。→ *おかしな値*
5. 2 行目の `ABH_HealthPickup::GiveHealth` をダブルクリック。`Hero` が nullptr です。3 行目の `OnTouched` をダブルクリック。今度は `Toucher` が nullptr では**ありません**。展開して名前を見てください。誰でしょうか? → *その値が来たフレーム*
6. さらに下りると `UPrimitiveComponent::BeginComponentOverlap` のようなエンジンのフレームが見えます。オーバーラップイベントから呼ばれたという意味です。→ *経由したエンジン機能*
7. さらに下りて `ABH_Lab01::StartLab` をダブルクリックすると、緑の矢印が**どの移動**を指しているかが見えます。
8. **Shift+F5** で止め(エディターも一緒に終了します)、直してからもう一度 F5 → ラボ 1 → `[PASS]` を確認します。
9. ボーナス: `if (!Hero) return;` だけを入れるとクラッシュは消えますが、`[FAIL]` が残ります。なぜでしょうか? ファイルの一番上のコメントを読んでください。

## 第1部 ラボ一覧

| # | 名前 | 症状 | 新しく学ぶこと | 難易度 |
|---|---|---|---|---|
| 1 | 回復アイテム | ペットがアイテムを拾うとゲームが落ちる | デバッガーのアタッチ、エンジンのフレームを飛ばす、`Cast<T>` の失敗 = nullptr | ★ |
| 2 | スコアボード HUD | ゲーム開始直後に落ちる | コンストラクター / PostInitializeComponents / **BeginPlay の順序**、SpawnActor の中ですぐ呼ばれる BeginPlay | ★★ |
| 3 | 倒した敵の片付け | Ensure が出て、倒した敵が残っている | `ensure` のメッセージを読む、**TArray のループ中の削除** | ★★ |
| 4 | クエストログ | 2 つ目のクエストを受けると落ちる（たまに落ちない） | **GC**、`UPROPERTY` のないポインター、`obj gc`、`gc.CollectGarbageEveryFrame`、`-stomp2malloc` | ★★★ |

## ヒントの見方

コンソール版と同じです。各ラボの `.cpp` ファイルの**一番下**に `#pragma region` で折りたたまれています。

1. ラボのファイルを初めて開いたら、**Ctrl+M, Ctrl+O** で全部折りたたみます。
2. 行き詰まったら**ヒント 1** だけを開きます。それでも行き詰まったら次のヒントを 1 つずつ。

ラボのコードは `Source/BugHuntUE/Labs/LabNN_*.cpp` です。`Source/BugHuntUE/Core/`(ラボの実行器)にはバグはありません。
全部解いたら [SOLUTIONS.ja.md](SOLUTIONS.ja.md) で答え合わせをしてください。

---

# 第2部: 処理の流れの追跡 (クラッシュなし)

コンソール版の第4部と同じ 4 ステップです。Unreal には**「自分が呼んでいないのに呼ばれるコード」**(デリゲート、タイマー、Tick、GC、Config の読み込み)が多いので、「いつ、誰が、何回呼んだか」を見るツールが特に重要です。

## 処理の流れの追跡 4 ステップ

1. **期待を先に書く。** 「毒のダメージは 0.4、0.8、1.4、1.8 秒に入るはず」のように。
2. **間違った結果に一番近いところから止める。**
3. **期待と最初に食い違う場所を探す。** 毎フレーム回るコードは F10 ではなく、トレースポイント、`UE_LOG`、Visual Logger で記録する。
4. **「最初に食い違った場所」を直す。**

```
記録表 (第2部以降)
間違った結果:              
止めた場所 / 使ったツール:     
期待値 → 実際の値:         
最初に食い違った場所:          
本当の原因:              
修正:                   
```

## 第2部で使う Unreal のツール

| ツール | やり方 | いつ |
|---|---|---|
| **出力ログ** | ウィンドウ → 出力ログ。検索欄にカテゴリー(`LogBugHunt`)を入力、右の フィルタ (Filters) で Warning/Error だけに | すべてのラボ。「何秒に、何回」出たかを見る |
| **`UE_LOG`** | `UE_LOG(LogBugHunt, Log, TEXT("HP %d, フレーム %llu"), Health, GFrameCounter);` | トレースポイントより長く残したいとき。Live Coding で入れたり外したりしやすい |
| ログの詳細度を変える | コンソール `log LogBugHunt Verbose` | `Verbose` のログまで見たいとき |
| **トレースポイント** | ブレークポイントを右クリック → アクション、`{Damage}` のような式、「実行の継続」にチェック | コードを変えずに呼び出しの順序・回数を記録 |
| **ヒット カウント / 条件** | ブレークポイントを右クリック → 条件 | 毎フレーム呼ばれる関数の N 回目だけ |
| **`ListTimers`** | コンソール | 今生きているタイマーと残り時間 (ラボ 6) |
| **`dumpticks`** | コンソール | Tick 関数の一覧と Tick グループ (ラボ 7) |
| **Visual Logger** | ツール → デバッグ → **Visual Logger** (Tools → Debug) → 録画 → PIE → 停止 → タイムラインをドラッグ | 位置・経路のように「空間」で起きるバグを**巻き戻して**見る (ラボ 8) |
| **`getall` / `obj`** | `getall BH_Slime MaxHealth`、`obj list class=BH_Enemy`、`obj gc`、`obj refs name=...` | 生きているオブジェクトの一覧と値、誰が参照しているか (ラボ 4, 10) |
| **`stat` コマンド** | `stat fps`、`stat unit`(ゲーム/描画/GPU スレッドの ms)、`stat game`(Tick の時間)、`stat uobjects` | パフォーマンスがおかしいとき、オブジェクトの数が増えるとき |
| **データ ブレークポイント** | デバッグ → 新しいブレークポイント → データ ブレークポイント、アドレス `&MaxHealth`、4 バイト | 「誰がこの値を変えた?」 (ラボ 10) |
| **ゲームプレイデバッガ (Gameplay Debugger)** | PIE 中に **`'`(アポストロフィ)** キー、数字キーでカテゴリー | AI・知覚・ナビゲーションの状態を画面に重ねて見る。これらのラボは AI コントローラーを使わないので直接は使いませんが、AI のバグなら Visual Logger と並んで最優先のツールです |

## やってみよう: ラボ 5 (戦闘ログウィンドウ)

1. 再生 → `BugHunt.Lab 5`。何も止まらずに `[FAIL] 開き直したウィンドウのログの行数 (期待: 1, 実際: 2)`。
2. **期待を書きます。** 騎士は 10、5、7 のダメージを受けます。ウィンドウは 10 を受けるときと 7 を受けるときだけ開いているので、`HandleDamaged` は **2 回**(10、7)呼ばれるはずです。
3. `Lab05_CombatLog.cpp` の `HandleDamaged` の最初の行で **F9**。
4. もう一度 `BugHunt.Lab 5` と入力。止まるたびにローカル ウィンドウの `Damage` を書き留めて F5。何回止まりますか? 値は?
5. 期待(10、7)と違って**最初に**止まった値が何かを見てください。そのときウィンドウは開いていましたか? → *最初に食い違った場所*
6. 呼び出し履歴で `ABH_Fighter::TakeHit` のフレームをダブルクリック → ウォッチに `OnDamaged` を入れて展開します。`InvocationList` にはいくつ入っていますか?
7. 直して(`.cpp` だけを変えたなら Ctrl+Alt+F11 の Live Coding で十分です)PIE を開始し直し、[PASS] 3 つを確認します。

## フレームレートと初期化 (ラボ 11~13)

「自分の PC では動くんですけど」の 2 つのよくある原因です。1 つは**フレームレート**(30fps のノート PC と 144Hz のモニターでゲームの動きが変わる)、もう 1 つは**初期化されていない値**(ビルドや順序によってゴミ値が変わる)です。

### フレームレートを変えながら実行する

| ツール | やり方 | いつ |
|---|---|---|
| `t.MaxFPS N` | コンソール。`t.MaxFPS 30` → 実行 → `t.MaxFPS 120` → 実行 → `t.MaxFPS 0`(制限なし) | フレームレートを変えて同じ場面を比べる |
| 固定タイムステップ | コマンドライン `-benchmark -fps=30` (コード: `FApp::SetUseFixedTimeStep(true)`、`FApp::SetFixedDeltaTime(1.0/30)`) | 実時間に関係なく毎フレームの `DeltaSeconds` を正確に 1/N に。遅い PC でも結果が同じ。ラボ 11、12 がこの方式 (`Core/BugHuntProbe.h`) |
| `stat fps`、`stat unit` | コンソール | 今の実際のフレームレートとフレーム時間 |
| トレースポイント `{DeltaSeconds}` | Tick の最初の行 | フレーム間隔がどれくらいか、Tick が 1 秒に何回呼ばれるか |

### 初期化されていない値

| 何か | 最初の値 |
|---|---|
| UObject(アクター、コンポーネント)のメンバー | **0** (メモリを 0 で埋めてからコンストラクターを呼ぶ)。それでも初期値を書くのがルール |
| `USTRUCT`/普通の構造体のメンバー、ローカル変数 | **不定の値** (初期値を書かなければ) |
| `FMemory::Malloc`、`TArray::AddUninitialized`/`SetNumUninitialized` | **不定の値** |
| `TArray::AddDefaulted` | 構造体のデフォルトコンストラクターのとおり。初期値のないメンバーは**不定の値** (`AddZeroed` は 0) |

- 「不定の値」はそのメモリを直前に誰が使っていたかで決まるので、Debug と Development のビルド、エディターとパッケージ版、実行順序によって変わります。そのため「たまに」「自分の PC でだけ」現れます。
- Unreal は起動時に USTRUCT の `UPROPERTY` のデフォルト値をチェックして、初期値が抜けているものを出力ログに赤い行で残します:
  `LogClass: Error: IntProperty FBH_RoundStats::Kills is not initialized properly. Module:BugHuntUE File:Labs/Lab13_RoundStats.h`
  このリポジトリでは、ラボ 13 が直るまでエディターを起動するたびにこの行が見えます。`UPROPERTY` のないメンバーとローカル変数はチェックされません。
- データ ブレークポイント(上のツールの表)で「この値を最初に書いたのは誰か」を探すと、誰も書いていないことを確認できます。

## 第2部 ラボ一覧

| # | 名前 | 症状 | 新しく学ぶこと | 難易度 |
|---|---|---|---|---|
| 5 | 戦闘ログウィンドウ | ウィンドウを開き直すとログが 2 行ずつ出る | **デリゲートのバインド解除漏れ**、トレースポイント、InvocationList を見る | ★ |
| 6 | 毒の浄化 | 浄化したのに毒ダメージが続く | **FTimerHandle** の寿命、`ListTimers` | ★★ |
| 7 | HP バー | HP バーが 1 フレームずつ遅れる | **Tick グループと順序**、`dumpticks`、`GFrameCounter`、`AddTickPrerequisiteActor` | ★★ |
| 8 | 巡回兵 | 巡回兵が最初のウェイポイントで震えているだけ | **Visual Logger** で録画・巻き戻し | ★★ |
| 9 | アーチャー | 倒れたカカシでスコアを稼ぎ続ける | `Destroy()` の後のポインター、**`IsValid` と nullptr の違い** | ★★ |
| 10 | バランス調整 | HP を 150 に上げたのにゲームでは 100 | **CDO**、Config/ブループリントのデフォルト値がコンストラクターの値を上書きする順序、`getall`、データ ブレークポイント | ★★★ |
| 11 | ダッシュ | フレームレートによってダッシュ距離が変わる | 移動に **`DeltaSeconds`** が抜けている、`t.MaxFPS`、固定タイムステップ | ★ |
| 12 | 氷の滑り | フレームレートによって滑る距離が変わる | **フレームごとの減速**(`Velocity *= 0.9`)、`FMath::Pow`/`Exp` による時間基準の減速、`FInterpTo` | ★★ |
| 13 | ラウンドスコアボード | 新しいラウンドに前のラウンドのスコアが残っている | 構造体メンバーの**初期値なし**、UObject は 0、`AddDefaulted` と `AddZeroed`、`Reset` が残したメモリ | ★★ |

> ラボ 10 のブループリント版も自分で作ってみてください: コンテンツブラウザ (Content Browser) で `BH_Slime` を親にしてブループリント `BP_Slime` を作り、クラスのデフォルト (Class Defaults) で MaxHealth を 120 に変えて保存 → C++ のコンストラクターの値を 200 に変えてエディターを起動し直します。`BP_Slime` の MaxHealth はいくつですか? ブループリントで**変えていない** MoveSpeed は? (ブループリントは「親と違う値に変えたもの」だけを保存します。)

- ラボ 11~13 は GPU なしで判定されます(`-nullrhi` で検証済み)。ラボ 11、12 は 1 回の実行で 30fps と 120fps を順に実行します。

---

# 第3部: トランスフォームと空間

位置・回転・スケールはエンジンで最もよく使う値で、バグも最もよく起きます。この部のバグは、クラッシュもエラーもなく**見当違いの場所に**現れます。ほとんどに「ワールド原点に回転なしで置いてテストすると正しい」という共通点があります。原点では相対座標とワールド座標、アクターの軸とワールドの軸が同じだからです。そこで、この部のラボはアクターを原点から離れた場所に回して置いて判定します (隠し入力が位置・角度・スケールを変えます)。

追跡するとき最初に問うことは 1 つです。**「この値はどの座標系か?」** ワールド、親 (Relative)、アクター自身 (ローカル)、ソケット。関数名 (`SetRelativeLocation`、`SetWorldLocation`、`TransformPosition`、`GetActorForwardVector`) が期待する座標系と、入れる値の座標系が同じかを確認します。

## 第3部で使うツール

| ツール | やり方 | いつ |
|---|---|---|
| 詳細パネルの **Transform** | PIE を一時停止 → アウトライナでアクターを選択 → コンポーネントを選択。Location/Rotation/Scale の名前の横のメニューで**相対 (Relative)/ワールド (World)** 表示を切り替え | 同じコンポーネントの相対値とワールド値を並べて (ラボ 14、15、19) |
| ビューポートの座標系切り替え | ビューポート右上の地球儀/立方体アイコン (Ctrl+`) | ギズモの軸をワールド軸/ローカル軸に。アクターの「前」がどこかを目で (ラボ 16、17) |
| ウォッチ | `GetActorTransform()`、`Comp->GetComponentTransform()`、`Comp->GetRelativeTransform()`、`GetActorQuat()` | トランスフォームを展開して Translation/Rotation/Scale3D を直接見る。`.ToHumanReadableString()` も使える |
| **`DrawDebug*`** | コード: `DrawDebugCoordinateSystem(World, Loc, Rot, 100, false, 5)`、`DrawDebugDirectionalArrow`、`DrawDebugSphere` (`DrawDebugHelpers.h`) | 計算した点・方向・半径を画面に描いて実際の物体と比べる (ラボ 17、18、19) |
| `show Collision` | コンソール (PIE 中) | コリジョンの範囲がスケールのせいで実際にどれくらい大きく/小さくなったか (ラボ 19) |
| `ShowDebug` | コンソール (PIE 中)。`ShowDebug Collision`、`ShowDebug Camera` など | プレイヤー側の情報 (カメラの位置・回転、カプセル) を画面に |
| ソケットマネージャー | スケルタルメッシュ エディター → スケルトンツリー → ボーンを右クリック → Add Socket。コードでは `Mesh->DoesSocketExist(Name)`、`GetAllSocketNames()` | ソケット・ボーン名の確認、持ち手の位置合わせ (ラボ 21) |
| メッセージログ | ウィンドウ → メッセージログ → PIE タブ | Static のコンポーネントを動かそうとしたときの警告 (ラボ 20) |

## やってみよう: ラボ 14 (甲板の旗)

1. PIE → コンソールで `BugHunt.Lab 14`。`[PASS] 造船所: 旗がマストの先にある（ずれ 0 cm）` の次に `[FAIL] 港: 旗がマストの先にある（ずれ 4804 cm）`。造船所 (ワールド原点) では正しく、港で間違います。
2. ログの `Harbor の旗を掲げた → マストの先 (5000, 200, 800)` は正しい座標です (港 X=5000、船首 90 度、マストは船基準 (200, 0, 800))。計算は正しいのに、旗は見当違いの場所にあります。→ *値は正しいのに結果が間違う*
3. `ABH_Ship::RaiseFlag` の `SetRelativeLocation` の行にブレークポイントを置いてもう一度実行します。2 回目に止まったとき (港) F10 で 1 行進め、ウォッチに `Flag->GetRelativeLocation()` と `Flag->GetComponentLocation()` を入れます。Relative が (5000, 200, 800)、ワールドは (4800, 5000, 800) です。
4. Relative にワールド座標を入れていました。子のワールド位置 = 親のトランスフォーム × Relative なので、船の位置と回転がもう一度掛かりました。造船所では船が原点に回転なしで置かれていたので、もう一度掛けても同じでした。→ *最初に食い違った場所*
5. PIE を一時停止し、アウトライナで船を選んで、Flag コンポーネントの Location の表示を相対/ワールドに切り替えてみてください。同じことをエディターで確かめる方法です。

## 第3部 ラボ一覧

| # | 名前 | 症状 | 新しく学ぶこと | 難易度 |
|---|---|---|---|---|
| 14 | 甲板の旗 | 港に入ると旗が船を離れて見当違いの場所に掲げられる | **相対座標とワールド座標**、`SetRelativeLocation` にワールド座標を入れると親のトランスフォームが 2 回掛かる | ★ |
| 15 | 剣を拾う | 拾った剣が手ではなく横の空中に浮いてついてくる | **`FAttachmentTransformRules`**: KeepRelative / KeepWorld / SnapToTarget (スケールを含むかどうか) | ★ |
| 16 | グライダー | 傾けたまま操縦桿を引くと見当違いの方向へ曲がる | **`FQuat` の積の順序** (ワールド回転 vs ローカル回転)、`AddActorLocalRotation`、`FRotator` の足し算とジンバルロック | ★★ |
| 17 | 魔法の大砲 | 標的のほうへ向き直った大砲が正面にしか撃たない | **`FVector::ForwardVector` (ワールド +X) vs `GetActorForwardVector()`** (アクター基準) | ★ |
| 18 | レーザー照準線 | 戦車が原点から離れると照準線が見当違いの場所から出る | **`TransformPosition` vs `TransformVector` / `TransformVectorNoScale`**: 点と方向、平行移動とスケール | ★★ |
| 19 | 監視塔 | 塔を高くしたら警報範囲が半分になった | 親の**非均一スケール**が子のコリジョンに与える影響、球は最も小さい軸のスケール、スケールはメッシュだけに | ★★ |
| 20 | 動く足場 | 足場がびくともしない（エラーなし） | **Mobility が Static** のコンポーネントはゲーム中の移動を無視 (警告だけ)、移動関数の戻り値 | ★ |
| 21 | 鉱夫のランタン | 手に持ったランタンが足元に付いている | **存在しないソケット名**は黙ってコンポーネントの原点、`DoesSocketExist`、ボーン名をソケットとして、ソケットマネージャー | ★ |

- 8 つとも GPU なしで判定されます。判定は位置・方向・距離 (cm、内積) で行い、フレームを使うラボ (52、55) は固定 60fps で動きます。
- 含まないもの: ピボットのオフセット (メッシュの原点が形の中心ではない場合)、スケルタルメッシュのボーンのスケール、ワールド原点の再配置 (World Origin Rebasing)、大きな座標の float 精度。

---

# 第4部: コンテナ

`TArray`、`TMap`、`TSet` は標準ライブラリのコンテナに似ていますが、ルールが少しずつ違います。範囲チェックが**開発ビルドでも有効なので**、間違ったインデックスは黙って通り過ぎずに assert で落ち(ラボ 22、23)、逆に「要素のアドレス」や「順序」や「ハッシュ」に頼ったコードは落ちずに値だけが間違います(ラボ 24~26)。最後の 2 つのラボは、コンテナが **GC** や**レプリケーション**と組み合わさったときです。

すでに扱ったものはもう扱いません: ループ中の削除(ラボ 3)、`TArray` の要素への参照を持ったままの `Add`(ラボ 31)。

## 第4部で使うツール

| ツール | やり方 | いつ |
|---|---|---|
| assert のメッセージを読む | `Assertion failed: (Index >= 0) & (Index < ArrayNum)` → コールスタックで `Array.h` の下にある最初の `BugHuntUE.dll!` の行 | 範囲外のインデックス (ラボ 22)。ウォッチに `Wave`、`Rewards.Num()` を入れて比べる |
| `Pair != nullptr` | `Map.h.inl` の `FindChecked` → コールスタックを 1 行上がると `operator[]` | 存在しないキーで `Map[Key]` (ラボ 23) |
| natvis で `TMap`/`TSet` を展開 | ウォッチ `Kills`、`Revealed` | 要素数、キーと値 |
| アドレスの比較 | ウォッチ `TargetKills` のアドレスと、`Kills` を展開して見た Wolf の要素のアドレスを並べる | 先に受け取っておいたポインターがまだ本物の要素を指しているか (ラボ 24) |
| 重複キーを探す | ウォッチ `Revealed` を展開して、同じ (X,Y) が 2 回入っていないか見る | `==` と `GetTypeHash` が食い違う (ラボ 26) |
| `obj list class=BH_LootItem` | コンソール。GC の前後で比べる | コンテナの中の UObject が生きているか (ラボ 27) |
| `obj refs name=...` | コンソール | そのオブジェクトを誰がつかんでいるか (UPROPERTY のコンテナだけが見える) |
| Networking Insights | コマンドライン `-NetTrace=1 -trace=net` → Unreal Insights で開く | どのプロパティが何バイトでいつレプリケートされたか (ラボ 28) |

## 第4部 ラボ一覧

| # | 名前 | 症状 | 新しく学ぶこと | 難易度 |
|---|---|---|---|---|
| 22 | ウェーブ報酬 | 報酬を受け取るとクラッシュする | `TArray` の**範囲チェックの assert**、1 から数える番号と 0 から数えるインデックス | ★ |
| 23 | ショップの相場表 | 知らない品物を売るとクラッシュする | **`TMap::operator[]` = `FindChecked`** (存在しないキーを作らない、`std::map` とは違う)、`Find`/`FindRef`/`Contains` | ★ |
| 24 | 賞金掲示板 | 新しいモンスターを倒すと賞金の集計がずれる | **`TMap::Find` のポインター + `Add`/`FindOrAdd` の再割り当て**、`TInlineSetAllocator` | ★★ |
| 25 | ターン順 | ユニットが倒れるとターン順が入れ替わる | **`RemoveAtSwap`/`RemoveSwap`** は順序を守らない、`RemoveAt`/`Remove` とのコストの違い | ★ |
| 26 | 戦場の霧 | 同じマスが何度も明らかになる | **`GetTypeHash` と `operator==` は同じフィールドを見る必要がある**、キーを入れた後に変えてはいけない理由 | ★★ |
| 27 | 戦利品バッグ | 時間がたつとバッグの中のアイテムが消える | `UPROPERTY` のない **`TArray<UObject*>`** と GC、`TArray<TObjectPtr<>>`、`TArray<TWeakObjectPtr<>>` | ★★ |
| 28 | クエスト掲示板 | クライアントのクエスト進行度が 0 のまま止まる | **`FFastArraySerializer`** は比較しない、`MarkItemDirty`/`MarkArrayDirty`、普通にレプリケートされる `TArray` との違い | ★★★ |

- 7 つとも GPU なしで判定されます(`-nullrhi` で検証済み)。ラボ 22、23 は CRASH がバグのある状態です。
- ラボ 24 は `TInlineSetAllocator<4>` で「5 種類目が入ってきた瞬間」に要素が移動するようにしてあります。デフォルトのアロケーターだと、いつ移動するかが決まっていないので、同じバグが何時間も後にとんでもない値として現れます。
- ラボ 26 のキーを入れた後に `X` を変えるのも同じバグです。要素は古いハッシュのバケットに残っているので、新しい座標で探すと見つからず、古い座標で探しても `==` が一致しないので見つかりません。キーを変える必要があるなら、取り出してから入れ直してください。
- ラボ 28 はラボ 46・47・39 と同じく **プレイヤー数 (Number of Players) 2、Play As Listen Server** で実行し、サーバーのウィンドウで `BugHunt.Lab 28`。スタンドアロンでは通過します。

---

# 第5部: ポインターと寿命

第1部のクラッシュには「落ちた場所」がありました。今回のポインターのバグは**ほとんど落ちません**。指しているメモリがまだ残っている(GC の前、インラインバッファ)か、別のオブジェクトがその場所を再利用しているからです。そのため、値がおかしく出ている場所から逆に「このポインターは今何を指しているか」を確認する必要があります。

第1部と第2部ですでに扱ったものはもう扱いません: `Cast` の失敗による nullptr(ラボ 1)、`UPROPERTY` のないメンバーの GC(ラボ 4)、`Destroy` の後のポインターと `IsValid`(ラボ 9)。

## 第5部で使うツール

| ツール | やり方 | いつ |
|---|---|---|
| ウォッチで**アドレス**を書き留める | ウォッチ `Kael` → 値の欄の `0x...` をメモ帳に | 「同じオブジェクトか?」をアドレスで比べる (ラボ 29, 31) |
| `&変数` のウォッチ | `&Potions`、`&Slots[0]` | 参照が指している場所が配列の本当の場所と同じか (ラボ 31) |
| **データ ブレークポイント** | デバッグ → 新しいブレークポイント → データ ブレークポイント、アドレス `&Slots[0].Count`、4 バイト | 本物の値を誰が変えているか / 誰も変えていないか (ラボ 31) |
| natvis | ウォッチで `TWeakObjectPtr`、`TSharedPtr` を展開 | 弱いポインターが指すオブジェクト、共有ポインターの参照カウント (ラボ 29, 32) |
| コンストラクター/デストラクターのブレークポイント | デストラクターの最初の行で F9 | 消えるべきものが消えているか (ラボ 32) |
| `obj list class=...`、`obj refs name=...` | コンソール | UObject がまだ生きているか、誰がつかんでいるか |
| `-stomp2malloc` | コマンドライン (コマンドラインオプションの表を参照) | 解放されたヒープに書き込んだ瞬間にその場で止める (ラボ 31 をデフォルトの `TArray` に変えて試してみてください) |

## 第5部 ラボ一覧

| # | 名前 | 症状 | 新しく学ぶこと | 難易度 |
|---|---|---|---|---|
| 29 | ヘイト表 | 新しく来たヒーローが倒れたヒーローのヘイトを引き継ぐ | UObject ではない構造体でアクターを覚える、**`TWeakObjectPtr`**、アドレスの再利用(ABA) | ★★ |
| 30 | 砲台 | 壊した砲台が撃ち続ける | タイマー/デリゲートのラムダの `[this]`、**`CreateWeakLambda` / `CreateUObject`**、EndPlay が消すタイマー | ★★ |
| 31 | ポーションベルト | 戦利品を拾ってからポーションが減らない | **`TArray` の要素への参照 + `Add` の再割り当て**、`TInlineAllocator`、データ ブレークポイント | ★★ |
| 32 | 会話ツリー | 会話が終わっても会話ノードがメモリに残る | **`TSharedPtr` の循環参照**、`TWeakPtr`、参照カウントを見る | ★★ |

- 4 つとも GPU なしで判定されます(`-nullrhi` で検証済み)。ラボ 29 は、アロケーターが同じアドレスを再利用すると「Bram のヘイト 60」、別のアドレスを渡すと「ボスが削除されたヒーローを狙う」として現れます。どちらでも FAIL です。

---

# 第6部: 物理

物理のバグは「たまにすり抜ける」「たまに押されない」として報告されます。同じ場面をフレーム単位で巻き戻して見るツールが鍵です。この部のラボはすべて**固定 60fps** で動くので、結果はいつも同じです。

## 第6部で使うツール

| ツール | やり方 | いつ |
|---|---|---|
| `show Collision` | コンソール (PIE 中) | コリジョンの形状が実際にどこにあるか |
| `p.Chaos.DebugDraw.Enabled 1` | コンソール。`p.Chaos.DebugDraw.ShowCollisionParticles 1` などの下位オプション | 物理エンジンが見ているオブジェクトと接触点を画面に描く (以前の PhysX の `pxvis collision` に相当) |
| **Chaos Visual Debugger** | ツール → デバッグ → Chaos Visual Debugger → 録画 → PIE → 停止 → フレームを送る | 物理フレームごとの位置、速度、衝突を巻き戻して見る (ラボ 33, 36) |
| **Visual Logger** | 第2部と同じ。`UE_VLOG_LOCATION`、`UE_VLOG_SEGMENT` で位置と移動経路を残す | 「このフレームでどこからどこへ行ったか」 |
| 詳細 (Details) パネルの **コリジョン (Collision)** | PIE を一時停止 → アウトライナー (Outliner) で選択 | チャンネルのレスポンス(Ignore/Overlap/Block)、オーバーラップイベントの生成 (Generate Overlap Events)、Simulation Generates Hit Events (ラボ 34) |
| `Collision` プロファイル | プロジェクト設定 (Project Settings) → コリジョン → プリセット (Preset) | `BlockAll`、`OverlapAll`、`Trigger`、`PhysicsActor` がチャンネルごとにどうレスポンスするか |
| サブステップ (Substepping) | プロジェクト設定 → 物理 → サブステップ、最大サブステップデルタ時間 (Max Substep Delta Time) | 低いフレームレートでも物理の 1 ステップを小さく。フレームレートによって物理の結果が変わるとき |

Block と Overlap のルール: 2 つのオブジェクトの互いに対するレスポンスのうち、**弱いほう**が勝ちます(Ignore < Overlap < Block)。両方 Block なら Hit、どちらか一方でも Overlap なら Overlap(そして**両方**でオーバーラップイベントの生成がオンになっていないとイベントは来ない)、どちらか一方でも Ignore なら何も起きません。

## 第6部 ラボ一覧

| # | 名前 | 症状 | 新しく学ぶこと | 難易度 |
|---|---|---|---|---|
| 33 | 狙撃弾 | 弾丸が薄い鉄板をすり抜ける | **トンネリング**、連続衝突検出 **`bUseCCD`**、ヒットスキャン/スイープとの比較 | ★ |
| 34 | 感圧板 | 箱を載せても感圧板が押されない | チャンネルのレスポンス、**オーバーラップイベントの生成は両方に必要**、`AStaticMeshActor` がオフにしているデフォルト値 | ★★ |
| 35 | ノックバック | 吹き飛ばされた敵が城壁の向こうに立っている | **`SetActorLocation` の `bSweep=false`(瞬間移動)**、スイープと `FHitResult` | ★ |
| 36 | 投石機 | 投石機が投げた岩がその場に落ちる | 物理シミュレーション中のオブジェクトを **Tick で位置を変えて動かす**、物理の速度、`SetPhysicsLinearVelocity`/力/PhysicsHandle | ★★ |

- 4 つとも GPU なしで判定されます(`-nullrhi` でも Chaos 物理がそのまま動くことを確認しました)。

---

# 第7部: サウンド

> ⚠️ この部のラボは音を出します。実行する前にシステムの音量を下げ、ヘッドホンを使っている場合は耳から外してから始めてください。

「音が遅れて鳴ります」「音が途切れます」「自分だけ聞こえません」は、ゲーム開発でとてもよくある報告です。音そのものは耳で聞こえますが、バグを**再現して判定**するには数字が必要です。そこでこれらのラボは「閃光と音の間のフレーム数」「0.1 秒後もまだ再生中か」「各マシンで何回鳴ったか」を測って `[PASS]`/`[FAIL]` を出します。

## 第7部で使うツール

| ツール | やり方 | いつ |
|---|---|---|
| `stat audio` | コンソール | アクティブな音の数、オーディオスレッドの時間 |
| `stat soundwaves` / `stat soundcues` | コンソール | 今再生中の音の一覧と距離、音量 |
| `au.Debug.Sounds 1` | コンソール | 再生中の音を画面に文字で表示 |
| `au.3dVisualize.Enabled 1` | コンソール | 3D サウンドの位置をビューポートに表示 |
| `obj list class=SoundWave` | コンソール | メモリに読み込まれているサウンドアセット (ロード/アンロードの確認) |
| Unreal Insights | コマンドライン `-trace=default,loadtime` → ツール → Unreal Insights | 「誰がいつ何をロードしたか」、そのフレームにどれくらいかかったか (初回再生のヒッチ) |
| ログの時刻・フレーム番号 | `UE_LOG` に `GFrameCounter`、`FPlatformTime::Seconds()` | 遅れを ms/フレームの数字で残す |

> 音が「Sound Concurrency」(同時再生の上限)のせいで埋もれるバグもよくあります。サウンドアセットの Concurrency 設定(Max Count、Resolution Rule)と `au.Debug.Sounds` で確認します。このラボセットには入れていません。Concurrency の判定は実際のオーディオレンダリングの状態によって変わるので、自動で PASS/FAIL を安定して出せなかったからです。

## 第7部 ラボ一覧

| # | 名前 | 症状 | 新しく学ぶこと | 難易度 |
|---|---|---|---|---|
| 37 | ブラスターの発射音 | 発射音がマズルフラッシュより遅れて鳴る（初弾、そしてたまに） | **ソフト参照**の遅延ロード、非同期ロードは次のフレーム、GC によるアンロード、`obj list class=SoundWave` | ★★ |
| 38 | 爆発する樽 | 爆発音が鳴った瞬間に途切れる | `SpawnSoundAttached` のコンポーネントの寿命 = アクターの寿命、`au.Debug.Sounds` | ★ |
| 39 | 警報の鐘 | ホストにしか鐘の音が聞こえない | 音は**レプリケートされない**、演出用の NetMulticast | ★★ |

- ラボ 37 は、発射音(`/Engine/EditorSounds/Notifications/CompileFailed`)がすでにメモリにあると再現しません。この音はエディターの通知音なので、エディターでコンパイルが失敗して通知音が鳴った直後なら、エディターを起動し直してから実行してください(`obj list class=SoundWave` で確認)。採点ツールと `-game` での実行では必ず再現します。
- ラボ 38 にはオーディオデバイスが必要です。`-nosound` で実行したか出力デバイスがない場合は `[SKIP]` が出ます。
- ラボ 39 はラボ 46・47 と同じく **プレイヤー数 2、Play As Listen Server** で実行し、サーバーのウィンドウで `BugHunt.Lab 39`。実行する前に音量を下げてください(この部のラボは音を出します)。

---

# 第8部: 画像 / レンダリング

「色が変わりません」「アイコンが別のアイテムのものです」「色が企画書と違います」は目ではすぐ見えますが、目分量では「ちょっと違う気がする」で止まりがちです。これらのラボは**メッシュにはめ込まれたマテリアルのパラメータ値**、**画像ウィジェットのブラシに入ったテクスチャ**、**8 ビットの色の値**を直接調べて `[PASS]`/`[FAIL]` を出します。

## 第8部で使うツール

| ツール | やり方 | いつ |
|---|---|---|
| 詳細パネルの マテリアル (Materials) | PIE 中に F8 (イジェクト / Eject) → アクターを選択 | 今メッシュに**はめ込まれている**マテリアルが共有アセットか `MID_...` か |
| マテリアルインスタンスエディタ | マテリアルをダブルクリック → パラメータの一覧 | パラメータ名(大文字・小文字まで)とデフォルト値 |
| ウィジェットリフレクタ (Widget Reflector) | ツール → デバッグ → ウィジェットリフレクタ、Pick Hit-Testable Widgets | 画面のウィジェットがどのブラシ・テクスチャを描いているか |
| `obj list class=Texture2D` | コンソール | テクスチャがメモリに読み込まれているか (非同期ロードの確認) |
| テクスチャエディタの sRGB のチェック | テクスチャをダブルクリック → 詳細 → テクスチャ → sRGB | 色のテクスチャではオン、ノーマル・マスクではオフ |
| `stat streaming`、`r.Streaming.*` | コンソール | テクスチャストリーミングのプール、ミップレベル (下を参照) |
| RenderDoc / `ProfileGPU` | プラグイン RenderDoc を有効にする / コンソール | 実際に描かれたピクセル、ドローコール |

> **GPU が必要なので入れなかったラボ:** 「テクスチャがぼやけて出る(ミップのストリーミング)」と「レンダーターゲットのピクセル値が間違っている(readback)」はよくあるバグですが、どちらも実際の GPU レンダリングの結果を読まないと判定できません。このプロジェクトの自動検証は `-nullrhi`(GPU なしで実行)で動くため、この 2 種類は入れていません。ラボ 40~42 は GPU なしで調べられる値(パラメータ、ブラシ、色の変換)で判定するので、`-nullrhi` でも結果は同じです。画面で直接確認したい場合は、エディターの PIE で実行してください。

## 第8部 ラボ一覧

| # | 名前 | 症状 | 新しく学ぶこと | 難易度 |
|---|---|---|---|---|
| 40 | 箱のハイライト | マウスを乗せても箱が赤くならない | **MID** は作るだけでははめ込まれない、`CreateDynamicMaterialInstance`、共有マテリアル | ★ |
| 41 | インベントリのアイコン | スクロールすると別のアイテムのアイコンが出る | 非同期ロードのコールバックの**遅れた到着**、再利用されるウィジェット、フレーム番号で順序を書き留める | ★★ |
| 42 | チームカラー | チームカラーが仕様書より明るく色あせて見える | **sRGB と Linear**、`FColor` → `FLinearColor` の変換、テクスチャの sRGB のチェック | ★★ |

- 3 つとも GPU なしで判定されます(`-nullrhi` で検証済み)。ラボ 41 は、アイコンがすでにメモリにあると再現しません。エディターでそのテクスチャを開いた直後なら、エディターを起動し直してから実行してください。

---

# 第9部: アニメーション

アニメーションのバグは「たまに攻撃が当たりません」「連打するとキャラクターが滑ります」「倒した敵の剣に当たりました」のように**ゲームの判定**として報告されます。攻撃判定、足音、状態の遷移がアニメーション通知とモンタージュのイベントにひも付いているからです。これらのラボは画面のポーズではなく、**通知が何回来たか**、**イベントが来たときの状態の値が何か**で `[PASS]`/`[FAIL]` を出します。

アセットはエンジンの標準コンテンツだけを使います。キャラクターは `/Engine/Tutorial/SubEditors/TutorialAssets/Character/TutorialTPP`、アニメーションは同じフォルダーの `Tutorial_Walk_Fwd`(1.83 秒)です。モンタージュと通知はエディターでアセットに付ける代わりにコードで作ります(`Core/BugHuntAnim.h`、バグなし)。アニメーション ブループリントなしで標準の `UAnimInstance` を使うので、PIE の画面のキャラクターは歩くポーズを取りません。モンタージュの再生、通知、終了イベントは正常に動きます。

## 第9部で使うツール

| ツール | やり方 | いつ |
|---|---|---|
| `showdebug animation` | コンソール (PIE でキャラクターをビューターゲットにして) | 再生中のモンタージュ、位置、ウェイト、通知 |
| Rewind Debugger | ツール → デバッグ → Rewind Debugger (PIE を録画) | 通知がいつ来たかをタイムラインで |
| Animation Insights | コマンドライン `-trace=default,animation` → Unreal Insights | フレームごとのモンタージュの状態、通知、ポーズのティックの有無 |
| 通知にブレークポイント | `Notify`、`NotifyBegin/End`、`OnMontageEnded` のハンドラー | 誰が呼んだか (コールスタック)、いつ呼ばれないか |
| フレーム番号のログ | `UE_LOG` に `GFrameCounter` | イベントの順序を記録表に |
| 詳細 → メッシュ → 最適化 (Details → Mesh → Optimization) | PIE 中のキャラクターのスケルタルメッシュ コンポーネント | `Visibility Based Anim Tick Option` の値 |

## 第9部 ラボ一覧

| # | 名前 | 症状 | 新しく学ぶこと | 難易度 |
|---|---|---|---|---|
| 43 | 足跡 | カメラ外の NPC の足音を敵が聞き取れない | `VisibilityBasedAnimTickOption`、レンダリングされないとポーズのティックと通知が止まる (サーバー、`-nullrhi`、カメラの外) | ★★ |
| 44 | 連打コンボ | 連打すると攻撃中なのに動けて、コンボが余計に増える | `OnMontageEnded` の `bInterrupted`、同じモンタージュを再生し直すと前のものが interrupted で終わる | ★★ |
| 45 | ヒットボックス | 攻撃が中断された後も剣に触れるとダメージを受ける | 通知 2 つ(On/Off)と**通知ステート**の違い、中断されたモンタージュは後ろの通知を通過しない | ★★ |

- 3 つとも GPU なしで判定されます(`-nullrhi` で検証済み)。ラボ 43 はヘッドレス実行では「レンダリングされない」が確実なので、必ず再現します。PIE では NPC がカメラの 4m 後ろに出るので、カメラを回して NPC を画面に入れると、そこから足音が記録され始めるところまで見られます。

---

# 第10部: マルチプレイ

## PIE でサーバーとクライアントを立ち上げる

1. 再生ボタンの横の **⋮** → **プレイヤー数 (Number of Players): 2**、**ネットモード (Net Mode): Play As Listen Server**。
2. 再生を押すとウィンドウが 2 つ開きます。1 つはサーバー(リッスンサーバー = ホストプレイヤー)、もう 1 つはクライアントです。ウィンドウのタイトルと出力ログの行頭のタグで見分けます。
3. **サーバーのウィンドウ**をクリックして `~` → `BugHunt.Lab 46`。
4. ラボの出力には `[リッスンサーバー]`、`[クライアント]` のタグが付きます。

- PIE のサーバーとクライアントは**同じプロセス**です。1 つのブレークポイントが両方で止まるので、止まるたびにウォッチに `GetNetMode()`(`NM_ListenServer` / `NM_Client`)や `HasAuthority()` を入れて、**今誰のコードか**をまず確認してください。
- これらのラボは **Play As Listen Server** で行ってください。(Play As Client はウィンドウのない専用サーバーを立ち上げるので、サーバー側のコンソールにラボのコマンドを入れるのが面倒です。)
- ネットワークのログ: コンソール `log LogNet Verbose`、`log LogNetTraffic Verbose`。レプリケーションの状態: `net.ListActors`、`stat net`。

## 第10部 ラボ一覧

| # | 名前 | 症状 | 新しく学ぶこと | 難易度 |
|---|---|---|---|---|
| 46 | ホストの HP バー | リッスンサーバーのホスト画面だけ HP バーが変わらない | **RepNotify** が呼ばれる場所、`GetNetMode()` でマシンを見分ける | ★★ |
| 47 | 宝箱 | クライアントが開けた宝箱のゴールドがサーバーにない | **権限 (Authority)**、レプリケーションの方向、**Server RPC** | ★★★ |

> ラボ 47 はスタンドアロンでは通過します。バグはリモートのクライアントにだけあります。「自分の PC では動くんですけど」のマルチプレイ版です。

- ほかの部にもマルチプレイのラボがあります: ラボ 28(コンテナ、FastArray のレプリケーション)、ラボ 39(サウンド、NetMulticast)。実行方法は同じです。

---

# 第11部: 並行処理とフリーズ

この部は 2 種類のバグです。**フリーズ**(ラボ 48、49、52)と**データレース**(ラボ 50、51)。どちらも「すべて中断」、スレッド ウィンドウ、並列スタックで、その瞬間のスレッドたちを止めて見ます。

## フリーズ (無限ループとデッドロック)

クラッシュもエラーメッセージもなく**ゲームが固まる**バグです。報告は「止まりました」の一言だけで、ログも固まる直前で途切れています。こういうときは、固まったプログラムを**デバッガーで止めて**、各スレッドが今どこで何をしているかを見ます。

### 安全装置: これらのラボは本当には固まりません

本当に固まるとエディターを強制終了しなければならないので、フリーズのラボ(40、41、44)のループと待機には**時間の予算**が付いています(`Core/BugHuntHang.h`、バグなし)。

```
LogBugHunt: Display: [FAIL] 応答なし（2003 ms / 213 回超過） - エレベーター待ち
```

- 予算(デフォルト **2 秒**)を超えると上の行を出して、そのループや待機から**抜け出します**。その後のコードは、バグが作った状態のまま動き続けます。採点ツールはこのラボを TIMEOUT と判定します。
- 安全装置はバグを隠しません。正常なコードは予算の 1% も使わず、引っかかったらほかのチェックがどうであれ、そのラボは失敗です。
- `BH_HANG_CHECK(Guard)` の行や `BugHunt::WaitWithBudget(...)`(= `Future.Get()`)を消さないでください。消すとエディターが本当に固まります。バグはその行ではなく、ループの条件と待ち方にあります。
- 予算はコンソール変数 **`BugHunt.HangBudgetSeconds`** で変えます。「すべて中断」を練習するには、`BugHunt.HangBudgetSeconds 30` に増やしてからラボを実行してください。30 秒間本当に固まったままになります。

### 固まったプログラムを追跡する方法

1. コンソール `BugHunt.HangBudgetSeconds 30` → `BugHunt.Lab 48`。ゲーム画面が固まります。
2. VS で **デバッグ → すべて中断 (Ctrl+Alt+Break)**。プログラム全体がその瞬間に止まります。(エディターのウィンドウではなく VS のウィンドウで押してください。)
3. **デバッグ → ウィンドウ → スレッド (Ctrl+Alt+H)**。数十個のスレッドの中から **GameThread** をダブルクリックすると、呼び出し履歴がゲームスレッドのものに切り替わります。すべて中断した直後の「現在のスレッド」はゲームスレッドではないことがあります。
4. 呼び出し履歴で最初に出てくる `BugHuntUE.dll!` の行をダブルクリック。ローカル ウィンドウ/ウォッチでループの条件に使われている値を見ます。
5. **F5 → もう一度すべて中断** を何度か繰り返します。毎回同じループの中にいて、条件に使われている値が**変わらないか、際限なく大きくなるなら**無限ループです。
6. 複数のスレッドが絡んでいるなら **デバッグ → ウィンドウ → 並列スタック (Ctrl+Shift+D, S)**。すべてのスレッドのコールスタックが 1 つの図に出ます。2 つのスレッドが互いに相手の結果(ロック、Future、イベント)を待っていればデッドロックです。

| ツール | やり方 | いつ |
|---|---|---|
| **すべて中断** | VS: デバッグ → すべて中断 (Ctrl+Alt+Break)。Rider: Pause | 固まった瞬間を止める |
| **スレッド ウィンドウ** | Ctrl+Alt+H、GameThread をダブルクリック、右クリック → フラグ | どのスレッドがどこで回っているか、待っているか |
| **並列スタック** | Ctrl+Shift+D, S | デッドロック: 誰が誰を待っているか (ラボ 52) |
| 次のステートメントの設定 | ループの後の行で右クリック → 次のステートメントの設定 (Ctrl+Shift+F10) | 固まったループから強制的に抜け出して観察を続ける (危険: 状態がおかしくなることがある) |
| **Unreal Insights** | コマンドライン `-trace=default` → ツール → Unreal Insights → タイミング (Timing) | 何番目のフレームに何 ms かかったか、そのフレームの中で何が時間を食ったか。固まる前後の長いフレームを探す |
| `stat unit`、`stat game` | コンソール | ゲームスレッドの ms が急に跳ね上がるか (ヒッチ) |
| ハング検出 (`FThreadHeartBeat`) | `DefaultEngine.ini` `[Core.System]` `HangDuration=秒` | **パッケージ化したゲーム**でゲームスレッドが N 秒間ハートビートを送らないと、そのスレッドのコールスタックをログに残してクラッシュレポートを作る。エディタービルドとデバッグビルドではオフで(エンジンの `Build.h` の `USE_HANG_DETECTION`)、エンジンのデフォルト値も `HangDuration=0`(オフ)。デバッガーがアタッチされていると動作しない |
| ヒッチ検出 | パッケージ化したゲームでコマンドライン `-hitchdetection=0.1` (`ALLOW_HITCH_DETECTION` でビルドしたとき) | 1 フレームが 0.1 秒を超えると "Hitch detected on gamethread" のログ |

> これらのラボはエディタービルドで動くので、エンジンのハング検出は動作しません。`Core/BugHuntHang.h` が同じことをラボの中で代わりに行います。実際のプロジェクトでは、QA ビルド(Development、パッケージ化)で `HangDuration` を有効にしておくと、「止まりました」という報告にコールスタックが付いてきます。

## データレース

ワーカースレッド(`AsyncTask`、`ParallelFor`、`UE::Tasks`)が同じデータにロックなしで触るバグです。本当のレースは「100 回に 1 回」なので練習で再現するのは難しく、この部のラボにはレースが起きる隙間を広げる**拡大鏡** `BugHunt::RaceWindow()` があります(`Core/BugHuntProbe.h`、バグなし)。この行に着いたスレッドは、ほかのスレッドが同じ行に来るまで少し待つので、保護されていないコードでは**毎回**重なります。消してもバグは残り、「たまに」になるだけです。

### データレースで使うツール

| ツール | やり方 | いつ |
|---|---|---|
| **スレッド ウィンドウ** | デバッグ → ウィンドウ → スレッド (Ctrl+Alt+H)。スレッド名(GameThread、Foreground Worker、Background Worker...) | 今止まっているコードがどのスレッドで動いているか |
| **並列スタック** | Ctrl+Shift+D, S | 同じ関数に同時にいるスレッドがいくつあるか |
| トレースポイント `{FPlatformTLS::GetCurrentThreadId()}` | ブレークポイント → アクション | スレッド番号と値を一緒に出して順序を見る |
| **Unreal Insights** | コマンドライン `-trace=default,task` → タイミング ウィンドウのスレッドのトラック | どの作業がどのスレッドでいつ重なって動いたか |
| `check(IsInGameThread())` | ゲームスレッド専用コードの一番上 | ほかのスレッドから入ってきたらその場で止まる (エンジンのあちこちにある) |
| `FCriticalSection` + `FScopeLock` | `FScopeLock Lock(&CS);` ブロックの終わりで自動的に解放 | 読み取り-計算-書き込みをひとかたまりに |
| `std::atomic` / `TAtomic` | `Count.fetch_add(1)` | 1 つの数値に複数のスレッドが足し込むとき |
| `AsyncTask(ENamedThreads::GameThread, ...)` | ワーカースレッドからゲームスレッドへ結果を送る | UObject を書き換える必要があるとき (ラボ 51, 52) |

## 第11部 ラボ一覧

| # | 名前 | 症状 | 新しく学ぶこと | 難易度 |
|---|---|---|---|---|
| 48 | エレベーター | エレベーターを呼ぶとゲームが固まる | ゲームスレッドで**別のアクターの Tick が変えてくれるはずの値を while で待つ**、すべて中断、スレッド ウィンドウ、デリゲートに置き換える | ★ |
| 49 | 経路探索 | 途切れた橋の向こうへ送るとゲームが固まる | 幅優先探索の**訪問チェックの漏れ**(ローカル変数の寿命)、到達できない入力でテストする | ★★ |
| 50 | ギルド金庫 | 同時に入金するとゴールドが足りなくなる | `ParallelFor` と**読み取り-計算-書き込みのレース(lost update)**、`FCriticalSection`/`FScopeLock`、アトミック操作、結果をスロットごとに集める | ★★ |
| 51 | ハイライトスコア | リプレイ解析が終わるとクラッシュする | **ゲームスレッド専用のオブジェクトにワーカースレッドで触る**、スレッド ウィンドウ、`AsyncTask(GameThread)` | ★ |
| 52 | オートセーブ | オートセーブ中にゲームが固まる | ゲームスレッドで**非同期の結果を同期で待つ** → その作業がゲームスレッドを待つ**デッドロック**、並列スタック、`AsyncTask(GameThread)` で結果を受け取る | ★★★ |

- 5 つとも GPU なしで判定されます(`-nullrhi` で検証済み)。ラボ 51 はバグのある状態では CRASH です。
- ラボ 52 でデッドロックが解けても、「オートセーブがゲームを止めなかった」のチェックが残ります (保存が要求したフレームより後に終わったかをフレームで数えます)。結果を待つためにゲームスレッドを止めること自体がヒッチだからです。

---

# 第12部: パフォーマンス

パフォーマンスのバグは結果が正しいです。「遅いです」「カクつきます」だけがあります。そのため、この部のラボは**時間ではなく回数**(検索の回数、メモリ割り当ての回数、Tick の呼び出し回数、同期ロードの回数)で判定します。時間は PC ごとに違いますが、回数は同じだからです。実際のゲームでは、下のツールでまず時間を見て、その中で回数を数えます。

## 第12部で使うツール

| ツール | やり方 | いつ |
|---|---|---|
| `stat unit` | コンソール | Frame / Game / Draw / GPU のどのスレッドが遅いか |
| `stat game` | コンソール | ゲームスレッドの中で Tick、タイマーなどが使う時間 |
| `stat startfile` / `stat stopfile` | コンソール。`Saved/Profiling/` にファイル | 数秒間の統計を記録して後で見る |
| **Unreal Insights** | コマンドライン `-trace=cpu` (割り当てまで: `-trace=cpu,memalloc`、ロードまで: `-trace=cpu,loadtime`) → ツール → Unreal Insights | 長いフレームを選んで、その中の関数ごとの時間を見る。最初に使うツール |
| `dumpticks` | コンソール | 登録されたティック関数の数と一覧 (ラボ 55) |
| `obj list class=...` | コンソール | 何がメモリに読み込まれているか (ラボ 56) |
| `log LogStreaming Verbose` | コンソール | アセットのロードがいつ起きるかをログで |

## 第12部 ラボ一覧

| # | 名前 | 症状 | 新しく学ぶこと | 難易度 |
|---|---|---|---|---|
| 53 | ミニマップ | アイコンが多いとゲームが重くなる | Tick ごとの **`GetAllActorsOfClass`**、BeginPlay で探して覚えておく | ★ |
| 54 | 照準線 | エイム中にメモリ確保が多い | フレームごとのローカル `TArray` の再割り当て、**`Reserve`**、メンバー + `Reset`、`TInlineAllocator` | ★ |
| 55 | たいまつ | たいまつが多いとゲームが重くなる | やることのない **Tick をオフにする**、タイマー、`TickInterval`、Significance Manager | ★ |
| 56 | ボス登場 | ボスが出る瞬間に画面がカクつく | ゲーム中の**同期ロード**(`LoadSynchronous`/`LoadObject`)、`RequestAsyncLoad` で先に読み込んでおく | ★★ |

- 4 つとも GPU なしで判定されます。判定は回数だけで行います(ログの ms は参考用)。

---

# 採点ツール

全ラボをヘッドレス(GPU なしの `-nullrhi`)で実行し、ラボごとの結果と総合点を出します。エディターを閉じて、プロジェクトのフォルダーで:

```bat
grade.bat
```

まず `build.bat` でビルドしてから、ラボごとに**新しいゲームプロセス**を立ち上げて `-BugHuntLab=N -BugHuntExit` で実行し、そのログを読みます。ラボ 1 つにプロセスの起動を含めて 25~35 秒(マルチプレイのラボは 45 秒前後)、48 個全部で 25 分前後かかります。

> ⚠️ 採点ツールはサウンドのラボ(29~31)をオーディオデバイスで実際に再生します。実行する前にシステムの音量を下げ、ヘッドホンを使っている場合は耳から外しておいてください。

```
  #  部   結果       時間  ラボ  - 内容
  1  1部  CRASH     44.5s  回復アイテム  - Unhandled Exception: EXCEPTION_ACCESS_VIOLATION reading address 0x0000000000000470
  5  2部  FAIL      52.3s  戦闘ログウィンドウ  - 開き直したウィンドウの記録行数 (期待: 1, 実際: 2) ほか 1 件
 21  4部  FAIL      46.5s  ヘイト表  - Bram のヘイト（Bram が与えたダメージ） (期待: 15, 実際: 83) ほか 1 件
 38  9部  FAIL      49.1s  ホストの HP バー  - [リッスンサーバー] 画面の HP バー (期待: 43, 実際: 100)
 40  10部  TIMEOUT   28.1s  エレベーター  - 応答なし（2008 ms / 212 回超過） - エレベーター待ち
 ...
スコア: 0 / 56（SKIP 0 件を除く）
```

| 結果 | 意味 |
|---|---|
| PASS | ラボのチェックがすべて通過 |
| FAIL | `[FAIL]` が 1 つでもある (内容の欄は最初の FAIL) |
| TIMEOUT | 第11部の安全装置が「応答なし」を報告したか、ラボのプロセスが制限時間(デフォルト 180 秒)内に終わらず採点ツールが終了させた |
| CRASH | プロセスがクラッシュした(`Unhandled Exception`、`Assertion failed`、`Fatal error`)か、結果の行なしで終わった |
| SKIP | この環境では判定できない (例: オーディオデバイスがないのにラボ 38)。GPU レンダリングの結果が必要なラボも SKIP するが、今の 56 個の中にはない |

スコアは `PASS の数 / (全体 - SKIP)` です。

- **隠し入力:** 採点ツールは実行するたびにランダムなシードを決めて `-BugHuntSeed=シード` で渡します。これに対応しているラボ(5、11~36、46、48~55)は、README に書かれた公開入力とは**違う値**(ダメージ量、個数、グラフの形、時間)で動きます。期待値をコードに埋め込んだ「修正」はここで引っかかります。同じ入力を PIE で再現するには、表の上に出たシードを使って `BugHunt.Lab 29 シード`。
- **マルチプレイのラボ(20、31、38、39)** は、リッスンサーバーのプロセス(`?listen -BugHuntWaitForClient`)とクライアントのプロセス(`127.0.0.1:ポート`)を立ち上げます。クライアント側の `[FAIL]` もそのラボの FAIL として数えます。
- **結果ファイル:** `grade-report.json` (プロジェクトのフォルダー、ほかの BugHunt の版と同じ形式)。ラボごとのログは `Saved/Grade/LabNN.log`、クライアントは `LabNN_client.log`。

```json
{ "edition": "unreal", "version": 1, "total": 56, "passed": 0,
  "labs": [ { "id": 1, "title": "回復アイテム", "part": 1, "result": "CRASH", "ms": 35800, "detail": "Unhandled Exception: EXCEPTION_ACCESS_VIOLATION ..." } ] }
```

| オプション | すること |
|---|---|
| `grade.bat -labs=29-32` | 一部のラボだけ (例: `-labs=1-4,29`) |
| `grade.bat -seed=1234` | 隠し入力のシードを固定 (同じ結果をもう一度見る) |
| `grade.bat -public` | 隠し入力の代わりに公開入力で |
| `grade.bat -nobuild` | ビルドを飛ばす |
| `grade.bat -timeout=300` | ラボ 1 つの制限時間(秒) |
| `grade.bat -gpu` | `-nullrhi` なしで実行 (GPU が必要なラボも採点) |
| `set UE_ROOT=D:\Epic\UE_5.8` | エンジンの場所を変える (デフォルトは `F:\Program Files\UE_5.8`、`build.bat` も同じ) |

採点ツール自体はコマンドレットです(`Core/BugHuntGradeCommandlet.cpp`)。`grade.bat` なしで直接呼ぶには `UnrealEditor-Cmd.exe BugHuntUE.uproject -run=BugHuntGrade -labs=29-32`。

> 採点ツールは正解を知りません。ラボが自分で出す `[PASS]`/`[FAIL]` を集めるだけです。ラボのチェックコード(`StartLab` の `Expect` の行)や `Core/` を書き換えて通過させるのは反則です。

---

# 付録: Live Coding の落とし穴

ライブコーディング (Live Coding)(Ctrl+Alt+F11)は**実行中のエディターの関数コードを差し替え**ます。すでに作られたオブジェクト、すでに登録されたクラス情報は変えられません。実際に体験してみてください。

1. エディターを起動したまま `Lab01_Pickup.h` で `int32 Health = 10;` を `int32 Health = 40;` に変えて Ctrl+Alt+F11。
   → ヘッダーの変更なので、Live Coding が拒否するか警告します。出力ログの **LogLiveCoding** の行を読んでみてください。
2. 今度は `Lab09_Archer.cpp` で `constexpr int32 KillReward = 100;` を `50` に変えて Ctrl+Alt+F11 → PIE でラボ 9。反映されますか? (関数本体/`.cpp` の定数はたいてい反映されます。)
3. `Lab47_Chest.h` に `UFUNCTION(Server, Reliable)` を追加して Ctrl+Alt+F11。何が起きますか? → ラボ 47 の正解は**エディターを閉じてビルド**しないと適用されません。
4. コンストラクターの中の値(例: ラボ 7 の `PrimaryActorTick.TickGroup`)を変えて Live Coding → ラボ 7。反映されますか? エディターを再起動したら?
   コンストラクターは**クラスのデフォルトオブジェクト (CDO)** を作るときにすでに 1 回実行されていて、新しいオブジェクトはその CDO から値をコピーしてきます。

**ルール:** ヘッダー、`UPROPERTY`/`UFUNCTION`/`UCLASS`、コンストラクター、新しいファイル → **エディターを閉じてビルド**。「直したのに変わりません」なら、まずこれを疑ってください。
(ホットリロード (Hot Reload)、つまりエディターの昔の「コンパイル (Compile)」ボタンは、UE5 で Live Coding に置き換えられました。昔の資料に出てくるホットリロードの問題も原因は同じです。)

---

## コンソールコマンド集

| コマンド | すること |
|---|---|
| `BugHunt.List` / `BugHunt.Lab N` | ラボ一覧 / ラボ実行 |
| `BugHunt.Lab N シード` | ラボ N を採点ツールの隠し入力(シード)で実行。シード 0 は公開入力 |
| `BugHunt.HangBudgetSeconds 秒` | 第11部のフリーズのラボの安全装置の時間予算 (デフォルト 2)。「すべて中断」の練習のときは 30 |
| `ListTimers` | 生きているタイマー |
| `dumpticks` | Tick 関数とグループ |
| `getall クラス プロパティ` | そのクラスの全オブジェクトのプロパティの値 |
| `obj list class=クラス` | 生きているオブジェクトの一覧 |
| `obj gc` | 今すぐ GC を実行 |
| `obj refs name=オブジェクト名` | そのオブジェクトを参照している経路 (なぜ消えないのか) |
| `gc.CollectGarbageEveryFrame 1` | 毎フレーム GC → GC のバグをすぐにあぶり出す (遅くなる) |
| `stat fps` / `stat unit` / `stat game` | フレーム、スレッドごとの ms、Tick の時間 |
| `log カテゴリー Verbose` | そのカテゴリーのログの詳細度を上げる |
| `showdebug` | 画面にデバッグ情報 (カメラ、入力など) |
| `BugHunt.Lang ko` / `BugHunt.Lang ja` | ラボ・採点ツールの出力言語を実行中に切り替える。値が空（既定）ならエディター/OS の言語に従う |

## コマンドラインオプション

| オプション | すること |
|---|---|
| `-game` | エディターの代わりにゲームとしてすぐ実行 |
| `-BugHuntLab=N` | ワールドが始まったらラボ N を自動実行 |
| `-BugHuntExit` | ラボが終わったら終了 (自動検証用) |
| `-BugHuntSeed=S` | 隠し入力(シード S)で実行 (採点ツールが使う) |
| `-stomp2malloc` | 解放されたメモリ/範囲外へのアクセスをその場で捕まえるアロケーター (Unreal 版の ASan。UE 5.8 のデフォルトでは **Windows のエディタービルド専用**: `MallocStomp2.h` の `WITH_MALLOC_STOMP2 = WITH_EDITOR && PLATFORM_WINDOWS`。ゲームビルドではコマンドラインで指定しても何も起きない)。とても遅く、メモリを大量に使う。`-MallocStomp2MinSize=`、`-MallocStomp2MaxSize=` でサイズの範囲を絞れる |
| `-log` | 別のログウィンドウを開く |
| `-BugHuntLang=ko` / `-BugHuntLang=ja` | ラボ・採点ツールの出力言語（`grade.bat -BugHuntLang=ja` も同じ）。決まる順: `BugHunt.Lang` の値（このオプションが入れ、実行中にコンソールで変えるとその値）→ エディター/OS の言語 → 韓国語。ヘッドレス採点でも同じ出力になるよう、LOCTEXT ではなくソース内の韓国語／日本語の表（`Core/BugHuntText.h`）を使う。`grade-report.json` のキーと `result` は言語に関係なく同じで、`title`/`detail` だけが変わる |

## もう一度解きたいとき

このフォルダーは git リポジトリです。直した後、`git restore Source` でバグのある状態に戻せます。

</details>
