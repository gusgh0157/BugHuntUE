using UnrealBuildTool;

public class BugHuntUE : ModuleRules
{
	public BugHuntUE(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"NetCore",
			"UMG",
			"Slate",
			"SlateCore"
		});

		PublicIncludePaths.AddRange(new string[] {
			"BugHuntUE"
		});

		// 실습용: 디버거에서 변수가 "최적화되어 사용할 수 없음"으로 보이지 않게 이 모듈만 최적화를 끈다.
		// (Development Editor로 빌드해도 랩 코드는 한 줄씩 따라갈 수 있다.)
		OptimizeCode = CodeOptimization.Never;
	}
}
