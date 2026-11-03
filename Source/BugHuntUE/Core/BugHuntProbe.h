#pragma once

#include "CoreMinimal.h"

// 실습용 계측 도구 모음. 이 파일에는 버그가 없다. 랩이 시작할 때마다 ResetProbes()로 초기화된다.
namespace BugHunt
{
BUGHUNTUE_API void ResetProbes();

// ---------------------------------------------------------------------------
// 고정 프레임레이트 (프레임레이트 의존, 물리 랩)
//
// -benchmark -fps=N 과 같은 방식(FApp::SetUseFixedTimeStep)으로 매 프레임 DeltaSeconds 를 정확히 1/Fps 로 만든다.
// 실제 시간과 상관없이 게임 시간이 1/Fps 씩 흐르므로, 느린 PC 나 헤드리스 실행에서도 결과가 같다.
// 0 을 주면 해제. 랩이 끝나면 자동으로 해제된다.
BUGHUNTUE_API void SetFixedFrameRate(float Fps);

// ---------------------------------------------------------------------------
// 데이터 레이스 확대경 (동시성 랩)
//
// 진짜 데이터 레이스는 "가끔" 일어나서 실습으로 재현하기 어렵다. 이 줄은 레이스가 일어날 수 있는 틈을 넓힌다:
// 이 줄에 도착한 스레드는 다른 스레드가 같은 줄에 도착할 때까지(최대 TimeoutMs) 기다렸다가 둘이 함께 지나간다.
// 동기화가 없는 코드에서는 두 스레드가 반드시 겹치고, 잠금으로 보호된 코드에서는 다른 스레드가 잠금에 막혀
// 이 줄에 오지 못하므로 시간 제한 뒤 혼자 지나간다(결과는 맞고 조금 느릴 뿐).
// 이 줄을 지운다고 버그가 고쳐지지 않는다. 겹칠 확률이 낮아질 뿐이다.
BUGHUNTUE_API void RaceWindow(int32 TimeoutMs = 200);

// ---------------------------------------------------------------------------
// 배열 재할당 세기 (성능 랩)
//
// TArray<T, BugHunt::FCountingAllocator> 는 보통 TArray 와 똑같이 동작하고, 메모리를 새로 잡거나 늘릴 때마다
// ArrayReallocCount() 를 1 올린다.
struct FCountingMalloc
{
	static BUGHUNTUE_API void* Realloc(void* Original, SIZE_T Count, uint32 Alignment = DEFAULT_ALIGNMENT);
	static BUGHUNTUE_API void Free(void* Original);
};
using FCountingAllocator = TSizedExternalAllocator<32, FCountingMalloc>;

BUGHUNTUE_API int32 ArrayReallocCount();
}
