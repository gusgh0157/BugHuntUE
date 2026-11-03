#pragma once

#include "CoreMinimal.h"
#include "Async/Future.h"

// 10부(동시성과 멈춤)의 멈춤 랩용 안전장치. 이 파일에는 버그가 없다.
//
// 실제 게임에서 무한루프나 데드락은 에디터를 통째로 굳힌다. 실습 중에 그러면 곤란하므로
// 랩 코드의 루프와 대기에는 "시간 예산"이 붙어 있다. 예산을 넘기면
//     [FAIL] 응답 없음 (2000 ms / 812345회 초과) - 엘리베이터 기다리기
// 를 찍고 그 루프(대기)에서 빠져나온다. 그 랩은 다른 검사가 어떻게 나오든 FAIL 이다.
//
// 안전장치는 버그를 고쳐 주지 않는다:
//   - 정상 코드는 예산의 1%도 쓰지 않는다. 예산에 걸렸다면 그 코드는 진짜 게임에서 굳는다.
//   - 걸리면 루프를 "그냥 빠져나올" 뿐이다. 그 뒤 코드는 버그가 만든 상태 그대로 계속 돈다.
//   - 안전장치 줄을 지우면 에디터가 진짜로 굳는다. 버그는 그 줄이 아니라 루프 조건에 있다.
//
// 예산은 콘솔 변수 BugHunt.HangBudgetSeconds (기본 2초). "모두 중단" 실습을 하려면 30 정도로 늘린다.
namespace BugHunt
{
BUGHUNTUE_API double HangBudgetSeconds();

// [FAIL] 응답 없음 (...) - What 을 찍는다.
BUGHUNTUE_API void ReportHang(const FString& What, const FString& Detail);

// 루프 한 바퀴마다 Tripped()를 부른다. 보통은 BH_HANG_CHECK 매크로로 쓴다.
class BUGHUNTUE_API FHangGuard
{
public:
	// MaxIterations: 0이면 시간 예산만 본다.
	explicit FHangGuard(const FString& InWhat, int64 InMaxIterations = 0);

	// 예산(시간 또는 반복 횟수)을 넘었으면 처음 한 번 [FAIL] 을 찍고 true.
	bool Tripped();
	bool HasTripped() const { return bTripped; }

private:
	FString What;
	double StartSeconds;
	double BudgetSeconds;
	int64 Iterations = 0;
	int64 MaxIterations;
	bool bTripped = false;
};

// Future.Get() 대신 쓴다. 예산 안에 결과가 오면 true, 아니면 [FAIL] 응답 없음 (데드락 의심) 을 찍고 false.
template <typename ResultType>
bool WaitWithBudget(const TFuture<ResultType>& Future, const FString& What)
{
	const double Start = FPlatformTime::Seconds();
	if (Future.WaitFor(FTimespan::FromSeconds(HangBudgetSeconds())))
	{
		return true;
	}
	ReportHang(What, FString::Printf(TEXT("%.0f ms 기다림, 데드락 의심"), (FPlatformTime::Seconds() - Start) * 1000.0));
	return false;
}
}

// 예산을 넘었으면 지금 루프를 빠져나온다.
#define BH_HANG_CHECK(Guard) if ((Guard).Tripped()) break
