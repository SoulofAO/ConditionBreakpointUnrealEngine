/*
 * Publisher: AO
 * Year of Publication: 2025
 * Copyright AO All Rights Reserved.
 */



#pragma once

#include "CoreMinimal.h"
#include "Conditions/Abstract/BlueprintDebugExtensionCondition.h"
#include "ExecBlueprintBreakpointContext.h"
#include "Widgets/Input/SSpinBox.h"
#include "RepeatDebugCondition.generated.h"

UENUM()
enum class ERepeatComparisonType : uint8
{
	Less,
	Equal,
	Greater
};

DECLARE_DELEGATE_TwoParams(FOnRepeatConditionChanged, ERepeatComparisonType, int32);

/**
 * Condition that checks if BreakpointExecuteCount meets the comparison criteria
 */
UCLASS(NotBlueprintable)
class BLUEPRINTDEBUGEXTENSION_API URepeatDebugCondition : public UBlueprintDebugExtensionCondition
{
	GENERATED_BODY()
	
public:
	virtual bool CheckCondition(const UObject* ActiveObject, const FFrame& StackFrame, UExecBlueprintBreakpointContext* ExecBlueprintPointContext) override;

	virtual bool CheckValidCondition(UBlueprint* Blueprint) const override;

	virtual TSharedPtr<SWidget> InitializationWidget(UBlueprint* Blueprint) override;

	UPROPERTY(SaveGame)
	ERepeatComparisonType ComparisonType = ERepeatComparisonType::Equal;

	UPROPERTY(SaveGame)
	int32 Threshold = 1;
};

class SRepeatConditionWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRepeatConditionWidget) {}
		SLATE_ARGUMENT(ERepeatComparisonType, InitialComparisonType)
		SLATE_ARGUMENT(int32, InitialThreshold)
		SLATE_EVENT(FOnRepeatConditionChanged, OnConditionChanged)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	void OnComparisonTypeChanged(ERepeatComparisonType NewType);
	void OnThresholdChanged(int32 NewValue);

	ERepeatComparisonType CurrentComparisonType;
	int32 CurrentThreshold;
	FOnRepeatConditionChanged OnConditionChanged;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> ComparisonComboBox;
	TSharedPtr<SSpinBox<int32>> ThresholdSpinBox;
};
