#include "WrPuppets.h"

#define LOCTEXT_NAMESPACE "FWrPuppetsModule"

void FWrPuppetsModule::StartupModule()
{
	//UE_LOG(LogTemp, Warning, TEXT("WrPuppets"));
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FWrPuppetsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}


#include "IKRigDataTypes.h"
#include "IKRigSkeleton.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(WrPuppets)

void UIKRig_TwistSolver::GetChildren(const int32 BoneIndex, const FIKRigSkeleton& InSkeleton, TArray<int32>& OutChildrenIndices)
{
	TArray<int32> CurrentChildrenIndices;
	InSkeleton.GetChildIndices(BoneIndex, CurrentChildrenIndices);
	for (int32 ChildIndex : CurrentChildrenIndices)
	{
		OutChildrenIndices.Add(ChildIndex);
		GetChildren(ChildIndex, InSkeleton, OutChildrenIndices);
	}
}

void UIKRig_TwistSolver::Initialize(const FIKRigSkeleton& IKRigSkeleton)
{
	const int32 EndIndex = IKRigSkeleton.GetBoneIndexFromName(EndName);
	if (EndIndex == INDEX_NONE)
	{
		return;
	}

	const int32 RootIndex = IKRigSkeleton.GetBoneIndexFromName(RootName);
	if (RootIndex == INDEX_NONE)
	{
		return;
	}

	ChainIndices.Empty();
	ChainIndices.Add(EndIndex);
	int32 ParentBoneIndex = IKRigSkeleton.GetParentIndex(EndIndex);
	while (ParentBoneIndex != INDEX_NONE && ParentBoneIndex >= RootIndex)
	{
		ChainIndices.Add(ParentBoneIndex);
		ParentBoneIndex = IKRigSkeleton.GetParentIndex(ParentBoneIndex);
	};

	if (ChainIndices.Num() < 3)
	{
		ChainIndices.Empty();
		return;
	}

	Algo::Reverse(ChainIndices);

	TArray<int32> Children;
	for (int32 Cursor = 0; Cursor < ChainIndices.Num()-1; ++Cursor)
	{
		IKRigSkeleton.GetChildIndices(ChainIndices[Cursor], Children);
		const int32 NextChildIndex = ChainIndices[Cursor +1];
		for (const int32 ChildIndex: Children)
		{
			if (ChildIndex != NextChildIndex)
			{
				ChildrenIndices.Add(ChildIndex);
				GetChildren(ChildIndex, IKRigSkeleton, ChildrenIndices);
			}
		}
	}
}

void UIKRig_TwistSolver::Solve(FIKRigSkeleton& IKRigSkeleton, const FIKRigGoalContainer& Goals)
{
	if (ChainIndices.Num() < 3)
	{
		return;
	}

	const int32 WristIndex = ChainIndices.Last();
	IKRigSkeleton.UpdateLocalTransformFromGlobal(WristIndex);
	TArray<FTransform>& InOutLocalTransforms = IKRigSkeleton.CurrentPoseLocal;
	const FQuat WristRotation = InOutLocalTransforms[WristIndex].GetRotation();
	const FVector WristTwistAxis = WristRotation.RotateVector(FVector(0., 1., 0.));
	const float TwistAngle = WristRotation.GetTwistAngle(WristTwistAxis);

	TArray<FTransform>& InOutGlobalTransforms = IKRigSkeleton.CurrentPoseGlobal;	
	const int32 ArmIndex = ChainIndices[0];
	const FVector ArmLocation = InOutGlobalTransforms[ArmIndex].GetLocation();
	const FVector WristLocation = InOutGlobalTransforms[WristIndex].GetLocation();
	const FVector ElbowAxis = (WristLocation - ArmLocation).GetSafeNormal();

	float ElbowAngle = -TwistAngle * 0.5;
	if (ElbowAngle > UE_HALF_PI * 0.5) 
	{
		ElbowAngle = UE_HALF_PI * 0.5;
	} 
	else if (ElbowAngle < -UE_HALF_PI * 0.5)
	{
		ElbowAngle = -UE_HALF_PI * 0.5;
	}

	const FQuat ElbowRotation = FQuat(ElbowAxis, ElbowAngle);

	for (int32 Index = 0; Index < ChainIndices.Num() - 1; Index++)
	{
		int32 BoneIndex = ChainIndices[Index];
		FTransform& BoneTransform = InOutGlobalTransforms[BoneIndex];

		// rotation
		const FQuat BoneRotation = BoneTransform.GetRotation();
		const FQuat TargetRotation = ElbowRotation * BoneRotation;
		BoneTransform.SetRotation(TargetRotation);

		// translation
		const FVector BoneTranslation = BoneTransform.GetLocation();
		const FVector TargetTranslation = ArmLocation + ElbowRotation.RotateVector(BoneTranslation - ArmLocation);
		BoneTransform.SetTranslation(TargetTranslation);
	}

	// propagate to children
	for (const int32 ChildIndex: ChildrenIndices)
	{
		IKRigSkeleton.UpdateGlobalTransformFromLocal(ChildIndex);
	}

	const int32 ForeArmIndex = ChainIndices[1];

	IKRigSkeleton.UpdateLocalTransformFromGlobal(ForeArmIndex);

	FTransform& ForearmTransform = InOutLocalTransforms[ForeArmIndex];

	const FQuat ForearmRotation = ForearmTransform.GetRotation();
	const FVector ForearmTwistAxis = ForearmRotation.RotateVector(FVector(0., 1., 0.));

	const FQuat DeltaRotation = FQuat(ForearmTwistAxis, TwistAngle * 0.5);
	ForearmTransform.SetRotation(DeltaRotation * ForearmRotation);
	IKRigSkeleton.UpdateGlobalTransformFromLocal(ForeArmIndex);

	// propagate to children
	for (const int32 ChildIndex : ChildrenIndices)
	{
		IKRigSkeleton.UpdateGlobalTransformFromLocal(ChildIndex);
	}


}

#if WITH_EDITOR

FText UIKRig_TwistSolver::GetNiceName() const
{
	return FText(LOCTEXT("SolverName", "Twist Solver"));
}

bool UIKRig_TwistSolver::GetWarningMessage(FText& OutWarningMessage) const
{
	if (RootName == NAME_None)
	{
		OutWarningMessage = LOCTEXT("MissingArmRoot", "Missing arm/root bone.");
		return true;
	}

	if (EndName == NAME_None)
	{
		OutWarningMessage = LOCTEXT("MissingWristEnd", "Missing wrist/end bone.");
		return true;
	}

	if (ChainIndices.Num() < 3)
	{
		OutWarningMessage = LOCTEXT("Requires3BonesChain", "Requires at least 3 bones between arm/root and wrist/end bones.");
		return true;
	}

	return false;
}


bool UIKRig_TwistSolver::IsBoneAffectedBySolver(const FName& BoneName, const FIKRigSkeleton& IKRigSkeleton) const
{
	const bool bAffected = IKRigSkeleton.IsBoneInDirectLineage(BoneName, RootName);
	if (!bAffected)
	{
		return false;
	}

	const int32 EndIndex = ChainIndices.IsEmpty() ? INDEX_NONE : ChainIndices.Last();
	if (EndIndex == INDEX_NONE)
	{
		return false;
	}

	const int32 ChildIndex = IKRigSkeleton.GetBoneIndexFromName(BoneName);
	return ChildIndex <= EndIndex;
}

void UIKRig_TwistSolver::SetRootBone(const FName& RootBoneName)
{
	RootName = RootBoneName;
}

void UIKRig_TwistSolver::SetEndBone(const FName& EndBoneName)
{
	EndName = EndBoneName;
}

#endif

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FWrPuppetsModule, WrPuppets)