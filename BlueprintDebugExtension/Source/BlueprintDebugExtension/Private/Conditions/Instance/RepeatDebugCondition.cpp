/*
 * Publisher: AO
 * Year of Publication: 2025
 * Copyright AO All Rights Reserved.
 */



#include "Conditions/Instance/RepeatDebugCondition.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Input/SComboBox.h"

#define LOCTEXT_NAMESPACE "FBlueprintDebugExtensionModule"

bool URepeatDebugCondition::CheckCondition(const UObject* ActiveObject, const FFrame& StackFrame, UExecBlueprintBreakpointContext* ExecBlueprintPointContext)
{
	if (!IsValid(ExecBlueprintPointContext))
	{
		return false;
	}

	int32 CurrentCount = ExecBlueprintPointContext->BreakpointExecuteCount;

	switch (ComparisonType)
	{
	case ERepeatComparisonType::Less:
		return CurrentCount < Threshold;
	case ERepeatComparisonType::Equal:
		return CurrentCount == Threshold;
	case ERepeatComparisonType::Greater:
		return CurrentCount > Threshold;
	default:
		return false;
	}
}

bool URepeatDebugCondition::CheckValidCondition(UBlueprint* Blueprint) const
{
	return true; // Always valid as long as we have a threshold
}

TSharedPtr<SWidget> URepeatDebugCondition::InitializationWidget(UBlueprint* Blueprint, const UEdGraphNode* Node)
{
	TSharedPtr<TArray<TSharedPtr<FString>>> ComparisonOptions = MakeShared<TArray<TSharedPtr<FString>>>();
	ComparisonOptions->Add(MakeShared<FString>(TEXT("Less")));
	ComparisonOptions->Add(MakeShared<FString>(TEXT("Equal")));
	ComparisonOptions->Add(MakeShared<FString>(TEXT("Greater")));

	TSharedPtr<FString> InitialComparison;
	switch (ComparisonType)
	{
	case ERepeatComparisonType::Less:
		InitialComparison = (*ComparisonOptions)[0];
		break;
	case ERepeatComparisonType::Equal:
		InitialComparison = (*ComparisonOptions)[1];
		break;
	case ERepeatComparisonType::Greater:
		InitialComparison = (*ComparisonOptions)[2];
		break;
	default:
		InitialComparison = (*ComparisonOptions)[1];
		break;
	}

	return SNew(SRepeatConditionWidget)
		.InitialComparisonType(ComparisonType)
		.InitialThreshold(Threshold)
		.OnConditionChanged(FOnRepeatConditionChanged::CreateLambda([this](ERepeatComparisonType NewType, int32 NewThreshold)
			{
				this->ComparisonType = NewType;
				this->Threshold = NewThreshold;
			}));
}

void SRepeatConditionWidget::Construct(const FArguments& InArgs)
{
	CurrentComparisonType = InArgs._InitialComparisonType;
	CurrentThreshold = InArgs._InitialThreshold;
	OnConditionChanged = InArgs._OnConditionChanged;

	TSharedPtr<TArray<TSharedPtr<FString>>> ComparisonOptions = MakeShared<TArray<TSharedPtr<FString>>>();
	ComparisonOptions->Add(MakeShared<FString>(TEXT("Less")));
	ComparisonOptions->Add(MakeShared<FString>(TEXT("Equal")));
	ComparisonOptions->Add(MakeShared<FString>(TEXT("Greater")));

	TSharedPtr<FString> InitialSelection;
	switch (CurrentComparisonType)
	{
	case ERepeatComparisonType::Less:
		InitialSelection = (*ComparisonOptions)[0];
		break;
	case ERepeatComparisonType::Equal:
		InitialSelection = (*ComparisonOptions)[1];
		break;
	case ERepeatComparisonType::Greater:
		InitialSelection = (*ComparisonOptions)[2];
		break;
	default:
		InitialSelection = (*ComparisonOptions)[1];
		break;
	}

	ChildSlot
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(6.0f)
				.AutoHeight()
				[
					SNew(STextBlock)
						.Text(LOCTEXT("RepeatConditionLabel", "Trigger breakpoint when execution count:"))
				]
				+ SVerticalBox::Slot()
				.Padding(6.0f)
				.AutoHeight()
				[
					SAssignNew(ComparisonComboBox, SComboBox<TSharedPtr<FString>>)
						.OptionsSource(ComparisonOptions.Get())
						.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem)
							{
								return SNew(STextBlock).Text(FText::FromString(InItem.IsValid() ? *InItem : FString("None")));
							})
						.OnSelectionChanged_Lambda([this, ComparisonOptions](TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
							{
								if (SelectInfo != ESelectInfo::Direct && NewSelection.IsValid())
								{
									if (*NewSelection == TEXT("Less"))
									{
										OnComparisonTypeChanged(ERepeatComparisonType::Less);
									}
									else if (*NewSelection == TEXT("Equal"))
									{
										OnComparisonTypeChanged(ERepeatComparisonType::Equal);
									}
									else if (*NewSelection == TEXT("Greater"))
									{
										OnComparisonTypeChanged(ERepeatComparisonType::Greater);
									}
								}
							})
						.InitiallySelectedItem(InitialSelection)
						[
							SNew(STextBlock)
								.Text_Lambda([this]()
									{
										switch (CurrentComparisonType)
										{
										case ERepeatComparisonType::Less:
											return FText::FromString(TEXT("Less"));
										case ERepeatComparisonType::Equal:
											return FText::FromString(TEXT("Equal"));
										case ERepeatComparisonType::Greater:
											return FText::FromString(TEXT("Greater"));
										default:
											return FText::FromString(TEXT("Equal"));
										}
									})
						]
				]
				+ SVerticalBox::Slot()
				.Padding(6.0f)
				.AutoHeight()
				[
					SNew(STextBlock)
						.Text(LOCTEXT("ThresholdLabel", "Threshold value:"))
				]
				+ SVerticalBox::Slot()
				.Padding(6.0f)
				.AutoHeight()
				[
					SAssignNew(ThresholdSpinBox, SSpinBox<int32>)
						.MinValue(0)
						.MaxValue(TNumericLimits<int32>::Max())
						.Value(CurrentThreshold)
						.OnValueChanged_Lambda([this](int32 NewValue)
							{
								OnThresholdChanged(NewValue);
							})
						.OnValueCommitted_Lambda([this](int32 NewValue, ETextCommit::Type CommitType)
							{
								OnThresholdChanged(NewValue);
							})
				]
		];
}

void SRepeatConditionWidget::OnComparisonTypeChanged(ERepeatComparisonType NewType)
{
	CurrentComparisonType = NewType;
	if (OnConditionChanged.IsBound())
	{
		OnConditionChanged.Execute(CurrentComparisonType, CurrentThreshold);
	}
}

void SRepeatConditionWidget::OnThresholdChanged(int32 NewValue)
{
	CurrentThreshold = NewValue;
	if (OnConditionChanged.IsBound())
	{
		OnConditionChanged.Execute(CurrentComparisonType, CurrentThreshold);
	}
}

#undef LOCTEXT_NAMESPACE
