/*
 * Publisher: AO
 * Year of Publication: 2025
 * Copyright AO All Rights Reserved.
 */


#include "Conditions/Instance/CheckPropertyDebugCondition.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SNullWidget.h"
#include "UObject/UnrealType.h"
#include "UObject/PropertyPortFlags.h"
#include "UObject/UObjectGlobals.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/CoreStyle.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "ExecBlueprintBreakpointContext.h"

#define LOCTEXT_NAMESPACE "FBlueprintDebugExtensionModule"

bool UCheckPropertyDebugCondition::CheckCondition(const UObject* ActiveObject, const FFrame& StackFrame, UExecBlueprintBreakpointContext* ExecBlueprintPointContext)
{
	if (!IsValid(ActiveObject) || Comparisons.Num() == 0)
	{
		return false;
	}

	bool bResult = false;
	bool bFirstComparison = true;

	for (int32 i = 0; i < Comparisons.Num(); ++i)
	{
		const FPropertyComparison& Comparison = Comparisons[i];

		// Get left operand value
		FString LeftValue;
		if (!GetPropertyValue(ActiveObject, Comparison.LeftPropertyName, Comparison.bUseObjectNameForLeftProperty, LeftValue))
		{
			// If property not found, skip this comparison
			continue;
		}

		// Get right operand value
		FString RightValue;
		if (Comparison.RightOperandType == EPropertyOperandType::Property)
		{
			if (!GetPropertyValue(ActiveObject, Comparison.RightPropertyName, Comparison.bUseObjectNameForRightProperty, RightValue))
			{
				// If property not found, skip this comparison
				continue;
			}
		}
		else
		{
			RightValue = Comparison.ConstantValue;
		}

		// Perform comparison
		bool bComparisonResult = CompareValues(LeftValue, RightValue, Comparison.Operator);

		// Combine with previous result
		if (bFirstComparison)
		{
			bResult = bComparisonResult;
			bFirstComparison = false;
		}
		else
		{
			// Use logical operator from previous comparison
			EPropertyLogicalOperator LogicalOp = (i > 0) ? Comparisons[i - 1].LogicalOperator : EPropertyLogicalOperator::And;
			if (LogicalOp == EPropertyLogicalOperator::And)
			{
				bResult = bResult && bComparisonResult;
			}
			else
			{
				bResult = bResult || bComparisonResult;
			}
		}
	}

	return bResult;
}

bool UCheckPropertyDebugCondition::CheckValidCondition(UBlueprint* Blueprint) const
{
	if (Comparisons.Num() == 0)
	{
		return false;
	}

	// Check that all comparisons have valid property names
	for (const FPropertyComparison& Comparison : Comparisons)
	{
		if (Comparison.LeftPropertyName.IsEmpty())
		{
			return false;
		}

		if (Comparison.RightOperandType == EPropertyOperandType::Property && Comparison.RightPropertyName.IsEmpty())
		{
			return false;
		}
	}

	return true;
}

bool UCheckPropertyDebugCondition::GetPropertyValue(const UObject* Object, const FString& PropertyName, bool bPreferObjectName, FString& OutValue) const
{
	if (!IsValid(Object) || PropertyName.IsEmpty())
	{
		return false;
	}

	UClass* ObjectClass = Object->GetClass();
	FProperty* Property = FindFProperty<FProperty>(ObjectClass, FName(*PropertyName));

	if (Property == nullptr)
	{
		return false;
	}

	// If requested and property is object, use object name for comparison
	if (bPreferObjectName)
	{
		if (const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
		{
			const void* ValuePtr = ObjectProperty->ContainerPtrToValuePtr<void>(Object);
			UObject* const* ObjectPtr = reinterpret_cast<UObject* const*>(ValuePtr);
			UObject* ResolvedObject = ObjectPtr ? *ObjectPtr : nullptr;
			OutValue = IsValid(ResolvedObject) ? ResolvedObject->GetName() : TEXT("None");
			return true;
		}
	}

	// Export property value to string
	const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Object);
	Property->ExportText_Direct(OutValue, ValuePtr, "-1", const_cast<UObject*>(Object), PPF_None);

	return true;
}

bool UCheckPropertyDebugCondition::CompareValues(const FString& LeftValue, const FString& RightValue, EPropertyComparisonOperator Operator) const
{
	switch (Operator)
	{
	case EPropertyComparisonOperator::Equal:
		return LeftValue == RightValue;

	case EPropertyComparisonOperator::NotEqual:
		return LeftValue != RightValue;

	case EPropertyComparisonOperator::Less:
	{
		// Try numeric comparison
		double LeftNum, RightNum;
		if (LexTryParseString(LeftNum, *LeftValue) && LexTryParseString(RightNum, *RightValue))
		{
			return LeftNum < RightNum;
		}
		// Fallback to string comparison
		return LeftValue < RightValue;
	}

	case EPropertyComparisonOperator::LessOrEqual:
	{
		// Try numeric comparison
		double LeftNum, RightNum;
		if (LexTryParseString(LeftNum, *LeftValue) && LexTryParseString(RightNum, *RightValue))
		{
			return LeftNum <= RightNum;
		}
		// Fallback to string comparison
		return LeftValue <= RightValue;
	}

	case EPropertyComparisonOperator::Greater:
	{
		// Try numeric comparison
		double LeftNum, RightNum;
		if (LexTryParseString(LeftNum, *LeftValue) && LexTryParseString(RightNum, *RightValue))
		{
			return LeftNum > RightNum;
		}
		// Fallback to string comparison
		return LeftValue > RightValue;
	}

	case EPropertyComparisonOperator::GreaterOrEqual:
	{
		// Try numeric comparison
		double LeftNum, RightNum;
		if (LexTryParseString(LeftNum, *LeftValue) && LexTryParseString(RightNum, *RightValue))
		{
			return LeftNum >= RightNum;
		}
		// Fallback to string comparison
		return LeftValue >= RightValue;
	}

	default:
		return false;
	}
}

void UCheckPropertyDebugCondition::GetAvailableProperties(const UObject* Object, TArray<FString>& OutPropertyNames) const
{
	OutPropertyNames.Empty();

	if (!IsValid(Object))
	{
		return;
	}

	UClass* ObjectClass = Object->GetClass();
	for (TFieldIterator<FProperty> PropIt(ObjectClass, EFieldIteratorFlags::ExcludeSuper); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;
		if (Property != nullptr)
		{
			// Skip transient and deprecated properties
			if (Property->HasAnyPropertyFlags(CPF_Transient | CPF_Deprecated))
			{
				continue;
			}

			// Skip functions (functions are not properties)
			// Properties are already filtered by TFieldIterator<FProperty>

			OutPropertyNames.Add(Property->GetName());
		}
	}
}

TSharedPtr<SWidget> UCheckPropertyDebugCondition::InitializationWidget(UBlueprint* Blueprint)
{
	return SNew(SCheckPropertyInitializationWidget)
		.Blueprint(Blueprint)
		.Condition(this);
}

void SCheckPropertyInitializationWidget::Construct(const FArguments& InArgs)
{
	Blueprint = InArgs._Blueprint;
	Condition = InArgs._Condition;

	if (Condition == nullptr || Blueprint == nullptr || Blueprint->GeneratedClass == nullptr)
	{
		ChildSlot
		[
			SNullWidget::NullWidget
		];
		return;
	}

	BuildPropertyOptions(Blueprint);

	OperatorOptions = MakeShared<TArray<TSharedPtr<FString>>>();
	OperatorOptions->Add(MakeShared<FString>(TEXT("==")));
	OperatorOptions->Add(MakeShared<FString>(TEXT("!=")));
	OperatorOptions->Add(MakeShared<FString>(TEXT("<")));
	OperatorOptions->Add(MakeShared<FString>(TEXT("<=")));
	OperatorOptions->Add(MakeShared<FString>(TEXT(">")));
	OperatorOptions->Add(MakeShared<FString>(TEXT(">=")));

	OperandTypeOptions = MakeShared<TArray<TSharedPtr<FString>>>();
	OperandTypeOptions->Add(MakeShared<FString>(TEXT("Property")));
	OperandTypeOptions->Add(MakeShared<FString>(TEXT("Constant")));

	// Ensure we have at least one comparison
	if (Condition->Comparisons.Num() == 0)
	{
		Condition->Comparisons.Add(FPropertyComparison());
	}

	SAssignNew(ScrollBox, SScrollBox);

	ChildSlot
	[
		ScrollBox.ToSharedRef()
	];

	RebuildContent();
}

void SCheckPropertyInitializationWidget::BuildPropertyOptions(UBlueprint* InBlueprint)
{
	PropertyOptions = MakeShared<TArray<TSharedPtr<FString>>>();
	ObjectPropertyNames.Reset();

	if (InBlueprint == nullptr || InBlueprint->GeneratedClass == nullptr)
	{
		return;
	}

	UObject* DefaultObject = InBlueprint->GeneratedClass->GetDefaultObject();
	TArray<FString> AvailableProperties;
	if (DefaultObject != nullptr && Condition != nullptr)
	{
		Condition->GetAvailableProperties(DefaultObject, AvailableProperties);
	}

	PropertyOptions->Add(MakeShared<FString>(FString())); // Empty option
	for (const FString& PropName : AvailableProperties)
	{
		PropertyOptions->Add(MakeShared<FString>(PropName));

		FProperty* Property = FindFProperty<FProperty>(DefaultObject->GetClass(), *PropName);
		if (CastField<FObjectProperty>(Property))
		{
			ObjectPropertyNames.Add(PropName);
		}
	}
}

bool SCheckPropertyInitializationWidget::IsObjectProperty(const FString& PropertyName) const
{
	return ObjectPropertyNames.Contains(PropertyName);
}

void SCheckPropertyInitializationWidget::RebuildContent()
{
	if (!ScrollBox.IsValid() || Condition == nullptr)
	{
		return;
	}

	ScrollBox->ClearChildren();

	for (int32 i = 0; i < Condition->Comparisons.Num(); ++i)
	{
		const int32 ComparisonIndex = i;
		FPropertyComparison& Comparison = Condition->Comparisons[i];

		auto FindInitialSelection = [](const TSharedPtr<TArray<TSharedPtr<FString>>>& Options, const FString& Value) -> TSharedPtr<FString>
		{
			if (!Options.IsValid() || Options->Num() == 0)
			{
				return nullptr;
			}
			for (const TSharedPtr<FString>& Option : *Options)
			{
				if (Option.IsValid() && *Option == Value)
				{
					return Option;
				}
			}
			return (*Options)[0];
		};

		const TSharedPtr<FString> InitialLeftProp = FindInitialSelection(PropertyOptions, Comparison.LeftPropertyName);

		FString OperatorStr;
		switch (Comparison.Operator)
		{
		case EPropertyComparisonOperator::Equal: OperatorStr = TEXT("=="); break;
		case EPropertyComparisonOperator::NotEqual: OperatorStr = TEXT("!="); break;
		case EPropertyComparisonOperator::Less: OperatorStr = TEXT("<"); break;
		case EPropertyComparisonOperator::LessOrEqual: OperatorStr = TEXT("<="); break;
		case EPropertyComparisonOperator::Greater: OperatorStr = TEXT(">"); break;
		case EPropertyComparisonOperator::GreaterOrEqual: OperatorStr = TEXT(">="); break;
		}
		const TSharedPtr<FString> InitialOperator = FindInitialSelection(OperatorOptions, OperatorStr);

		const FString OperandTypeStr = (Comparison.RightOperandType == EPropertyOperandType::Property) ? TEXT("Property") : TEXT("Constant");
		const TSharedPtr<FString> InitialOperandType = FindInitialSelection(OperandTypeOptions, OperandTypeStr);

		TSharedPtr<FString> InitialRightProp = FindInitialSelection(PropertyOptions, Comparison.RightPropertyName);

		ScrollBox->AddSlot()
			.Padding(6.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::Format(LOCTEXT("ComparisonLabel", "Comparison {0}:"), ComparisonIndex + 1))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 4, 0, 0)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(0.4f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SComboBox<TSharedPtr<FString>>)
							.OptionsSource(PropertyOptions.Get())
							.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem)
								{
									return SNew(STextBlock).Text(FText::FromString(InItem.IsValid() ? *InItem : FString("None")));
								})
							.OnSelectionChanged_Lambda([this, ComparisonIndex](TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
								{
									if (SelectInfo != ESelectInfo::Direct && NewSelection.IsValid() && Condition->Comparisons.IsValidIndex(ComparisonIndex))
									{
										Condition->Comparisons[ComparisonIndex].LeftPropertyName = *NewSelection;
										if (!IsObjectProperty(*NewSelection))
										{
											Condition->Comparisons[ComparisonIndex].bUseObjectNameForLeftProperty = false;
										}
										RebuildContent();
									}
								})
							.InitiallySelectedItem(InitialLeftProp)
							[
								SNew(STextBlock)
								.Text_Lambda([this, ComparisonIndex]()
									{
										if (Condition->Comparisons.IsValidIndex(ComparisonIndex))
										{
											return FText::FromString(Condition->Comparisons[ComparisonIndex].LeftPropertyName.IsEmpty() ? TEXT("(Select Property)") : Condition->Comparisons[ComparisonIndex].LeftPropertyName);
										}
										return FText::FromString(TEXT("(Select Property)"));
									})
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 4.f, 0.f, 0.f)
						[
							SNew(SCheckBox)
							.Visibility_Lambda([this, ComparisonIndex]()
								{
									return (Condition->Comparisons.IsValidIndex(ComparisonIndex) && IsObjectProperty(Condition->Comparisons[ComparisonIndex].LeftPropertyName)) ? EVisibility::Visible : EVisibility::Collapsed;
								})
							.IsChecked_Lambda([this, ComparisonIndex]()
								{
									return (Condition->Comparisons.IsValidIndex(ComparisonIndex) && Condition->Comparisons[ComparisonIndex].bUseObjectNameForLeftProperty) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
								})
							.OnCheckStateChanged_Lambda([this, ComparisonIndex](ECheckBoxState NewState)
								{
									if (Condition->Comparisons.IsValidIndex(ComparisonIndex))
									{
										Condition->Comparisons[ComparisonIndex].bUseObjectNameForLeftProperty = (NewState == ECheckBoxState::Checked);
									}
								})
							[
								SNew(STextBlock)
								.Text(LOCTEXT("CompareLeftByName", "Compare by object name"))
							]
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(0.2f)
					.Padding(4, 0, 4, 0)
					[
						SNew(SComboBox<TSharedPtr<FString>>)
						.OptionsSource(OperatorOptions.Get())
						.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem)
							{
								return SNew(STextBlock).Text(FText::FromString(InItem.IsValid() ? *InItem : FString("==")));
							})
						.OnSelectionChanged_Lambda([this, ComparisonIndex](TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
							{
								if (SelectInfo != ESelectInfo::Direct && NewSelection.IsValid() && Condition->Comparisons.IsValidIndex(ComparisonIndex))
								{
									if (*NewSelection == TEXT("=="))
										Condition->Comparisons[ComparisonIndex].Operator = EPropertyComparisonOperator::Equal;
									else if (*NewSelection == TEXT("!="))
										Condition->Comparisons[ComparisonIndex].Operator = EPropertyComparisonOperator::NotEqual;
									else if (*NewSelection == TEXT("<"))
										Condition->Comparisons[ComparisonIndex].Operator = EPropertyComparisonOperator::Less;
									else if (*NewSelection == TEXT("<="))
										Condition->Comparisons[ComparisonIndex].Operator = EPropertyComparisonOperator::LessOrEqual;
									else if (*NewSelection == TEXT(">"))
										Condition->Comparisons[ComparisonIndex].Operator = EPropertyComparisonOperator::Greater;
									else if (*NewSelection == TEXT(">="))
										Condition->Comparisons[ComparisonIndex].Operator = EPropertyComparisonOperator::GreaterOrEqual;
								}
							})
						.InitiallySelectedItem(InitialOperator)
						[
							SNew(STextBlock)
							.Text_Lambda([this, ComparisonIndex]()
								{
									if (Condition->Comparisons.IsValidIndex(ComparisonIndex))
									{
										FString OpStr;
										switch (Condition->Comparisons[ComparisonIndex].Operator)
										{
										case EPropertyComparisonOperator::Equal: OpStr = TEXT("=="); break;
										case EPropertyComparisonOperator::NotEqual: OpStr = TEXT("!="); break;
										case EPropertyComparisonOperator::Less: OpStr = TEXT("<"); break;
										case EPropertyComparisonOperator::LessOrEqual: OpStr = TEXT("<="); break;
										case EPropertyComparisonOperator::Greater: OpStr = TEXT(">"); break;
										case EPropertyComparisonOperator::GreaterOrEqual: OpStr = TEXT(">="); break;
										}
										return FText::FromString(OpStr);
									}
									return FText::FromString(TEXT("=="));
								})
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(0.4f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(0.3f)
						[
							SNew(SComboBox<TSharedPtr<FString>>)
							.OptionsSource(OperandTypeOptions.Get())
							.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem)
								{
									return SNew(STextBlock).Text(FText::FromString(InItem.IsValid() ? *InItem : FString("Constant")));
								})
							.OnSelectionChanged_Lambda([this, ComparisonIndex](TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
								{
									if (SelectInfo != ESelectInfo::Direct && NewSelection.IsValid() && Condition->Comparisons.IsValidIndex(ComparisonIndex))
									{
										Condition->Comparisons[ComparisonIndex].RightOperandType = (*NewSelection == TEXT("Property")) ? EPropertyOperandType::Property : EPropertyOperandType::Constant;
										RebuildContent();
									}
								})
							.InitiallySelectedItem(InitialOperandType)
							[
								SNew(STextBlock)
								.Text_Lambda([this, ComparisonIndex]()
									{
										if (Condition->Comparisons.IsValidIndex(ComparisonIndex))
										{
											return FText::FromString((Condition->Comparisons[ComparisonIndex].RightOperandType == EPropertyOperandType::Property) ? TEXT("Property") : TEXT("Constant"));
										}
										return FText::FromString(TEXT("Constant"));
									})
							]
						]
						+ SHorizontalBox::Slot()
						.FillWidth(0.7f)
						.Padding(4, 0, 0, 0)
						[
							// Show property selector or constant input based on operand type
							SNew(SBox)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SComboBox<TSharedPtr<FString>>)
									.Visibility_Lambda([this, ComparisonIndex]()
										{
											return (Condition->Comparisons.IsValidIndex(ComparisonIndex) && Condition->Comparisons[ComparisonIndex].RightOperandType == EPropertyOperandType::Property) ? EVisibility::Visible : EVisibility::Collapsed;
										})
									.OptionsSource(PropertyOptions.Get())
									.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem)
										{
											return SNew(STextBlock).Text(FText::FromString(InItem.IsValid() ? *InItem : FString("None")));
										})
									.OnSelectionChanged_Lambda([this, ComparisonIndex](TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
										{
											if (SelectInfo != ESelectInfo::Direct && NewSelection.IsValid() && Condition->Comparisons.IsValidIndex(ComparisonIndex))
											{
												Condition->Comparisons[ComparisonIndex].RightPropertyName = *NewSelection;
												if (!IsObjectProperty(*NewSelection))
												{
													Condition->Comparisons[ComparisonIndex].bUseObjectNameForRightProperty = false;
												}
												RebuildContent();
											}
										})
									.InitiallySelectedItem(InitialRightProp)
									[
										SNew(STextBlock)
										.Text_Lambda([this, ComparisonIndex]()
											{
												if (Condition->Comparisons.IsValidIndex(ComparisonIndex))
												{
													return FText::FromString(Condition->Comparisons[ComparisonIndex].RightPropertyName.IsEmpty() ? TEXT("(Select Property)") : Condition->Comparisons[ComparisonIndex].RightPropertyName);
												}
												return FText::FromString(TEXT("(Select Property)"));
											})
									]
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.f, 4.f, 0.f, 0.f)
								[
									SNew(SCheckBox)
									.Visibility_Lambda([this, ComparisonIndex]()
										{
											return (Condition->Comparisons.IsValidIndex(ComparisonIndex)
												&& Condition->Comparisons[ComparisonIndex].RightOperandType == EPropertyOperandType::Property
												&& IsObjectProperty(Condition->Comparisons[ComparisonIndex].RightPropertyName))
												? EVisibility::Visible
												: EVisibility::Collapsed;
										})
									.IsChecked_Lambda([this, ComparisonIndex]()
										{
											return (Condition->Comparisons.IsValidIndex(ComparisonIndex)
												&& Condition->Comparisons[ComparisonIndex].bUseObjectNameForRightProperty)
												? ECheckBoxState::Checked
												: ECheckBoxState::Unchecked;
										})
									.OnCheckStateChanged_Lambda([this, ComparisonIndex](ECheckBoxState NewState)
										{
											if (Condition->Comparisons.IsValidIndex(ComparisonIndex))
											{
												Condition->Comparisons[ComparisonIndex].bUseObjectNameForRightProperty = (NewState == ECheckBoxState::Checked);
											}
										})
									[
										SNew(STextBlock)
										.Text(LOCTEXT("CompareRightByName", "Compare by object name"))
									]
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SEditableTextBox)
									.Visibility_Lambda([this, ComparisonIndex]()
										{
											return (Condition->Comparisons.IsValidIndex(ComparisonIndex) && Condition->Comparisons[ComparisonIndex].RightOperandType == EPropertyOperandType::Constant) ? EVisibility::Visible : EVisibility::Collapsed;
										})
									.Text_Lambda([this, ComparisonIndex]()
										{
											if (Condition->Comparisons.IsValidIndex(ComparisonIndex))
											{
												return FText::FromString(Condition->Comparisons[ComparisonIndex].ConstantValue);
											}
											return FText::GetEmpty();
										})
									.OnTextChanged_Lambda([this, ComparisonIndex](const FText& NewText)
										{
											if (Condition->Comparisons.IsValidIndex(ComparisonIndex))
											{
												Condition->Comparisons[ComparisonIndex].ConstantValue = NewText.ToString();
											}
										})
									.HintText(LOCTEXT("ConstantValueHint", "Enter constant value"))
								]
							]
						]
					]
				]
			];
	}
}

#undef LOCTEXT_NAMESPACE

