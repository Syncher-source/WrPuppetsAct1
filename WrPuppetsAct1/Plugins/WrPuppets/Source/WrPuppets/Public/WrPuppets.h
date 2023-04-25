#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "IKRigDefinition.h"
#include "IKRigSolver.h"
#include "WrPuppets.generated.h"

class FWrPuppetsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};


UCLASS(EditInlineNew)
class UIKRig_TwistSolver : public UIKRigSolver
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere, DisplayName = "Arm bone (Root bone)", Category = "Twist Solver Settings")
		FName RootName = NAME_None;

	UPROPERTY(VisibleAnywhere, DisplayName = "Wrist Bone (End bone)", Category = "Twist Solver Settings")
		FName EndName = NAME_None;

	/** UIKRigSolver interface */
	virtual void Initialize(const FIKRigSkeleton& IKRigSkeleton) override;
	virtual void Solve(FIKRigSkeleton& IKRigSkeleton, const FIKRigGoalContainer& Goals) override;

#if WITH_EDITOR
	virtual FText GetNiceName() const override;
	virtual bool GetWarningMessage(FText& OutWarningMessage) const override;
	virtual bool IsBoneAffectedBySolver(const FName& BoneName, const FIKRigSkeleton& IKRigSkeleton) const override;
	// root bone can be set on this solver
	virtual bool RequiresRootBone() const override { return true; };
	virtual void SetRootBone(const FName& RootBoneName) override;
	// end bone can be set on this solver
	virtual void SetEndBone(const FName& EndBoneName) override;
	virtual bool RequiresEndBone() const override { return true; };
	/** END UIKRigSolver interface */
#endif

private:

	static void GetChildren(const int32 BoneIndex, const FIKRigSkeleton& InSkeleton, TArray<int32>& OutChildrenIndices);

	TArray<int32>							ChainIndices;
	TArray<int32>							ChildrenIndices;
};
