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
#include "Widgets/SNullWidget.h"
#include "UObject/UnrealType.h"
#include "UObject/PropertyPortFlags.h"
#include "UObject/UObjectGlobals.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/CoreStyle.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "ExecBlueprintBreakpointContext.h"
#include "Math/UnrealMathUtility.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "BlueprintEditorLibrary.h"

#define LOCTEXT_NAMESPACE "FBlueprintDebugExtensionModule"

bool UCheckPropertyDebugCondition::CheckCondition(const UObject* ActiveObject, const FFrame& StackFrame, UExecBlueprintBreakpointContext* ExecBlueprintPointContext)
{
	if (!IsValid(ActiveObject))
	{
		return false;
	}

	UEdGraphNode* CurrentNode = GetCurrentNodeFromStackFrame(ActiveObject, StackFrame);

	FString LeftValue;
	if (LeftOperandType == EPropertyOperandType::Property)
	{
		if (LeftPropertyName.IsEmpty() || !GetPropertyValue(ActiveObject, LeftPropertyName, bUseObjectNameForLeftProperty, LeftValue))
		{
			return false;
		}
	}
	else if (LeftOperandType == EPropertyOperandType::InputPin)
	{
		if (CurrentNode == nullptr || LeftInputPinName.IsEmpty() || !GetInputPinValue(CurrentNode, LeftInputPinName, LeftValue))
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	FString RightValue;
	if (RightOperandType == EPropertyOperandType::Property)
	{
		if (RightPropertyName.IsEmpty() || !GetPropertyValue(ActiveObject, RightPropertyName, bUseObjectNameForRightProperty, RightValue))
		{
			return false;
		}
	}
	else if (RightOperandType == EPropertyOperandType::InputPin)
	{
		if (CurrentNode == nullptr || RightInputPinName.IsEmpty() || !GetInputPinValue(CurrentNode, RightInputPinName, RightValue))
		{
			return false;
		}
	}
	else
	{
		RightValue = ConstantValue;
	}

	return CompareValues(LeftValue, RightValue);
}

bool UCheckPropertyDebugCondition::CheckValidCondition(UBlueprint* Blueprint) const
{
	if (LeftOperandType == EPropertyOperandType::Property && LeftPropertyName.IsEmpty())
	{
		return false;
	}

	if (LeftOperandType == EPropertyOperandType::InputPin && LeftInputPinName.IsEmpty())
	{
		return false;
	}

	if (RightOperandType == EPropertyOperandType::Property && RightPropertyName.IsEmpty())
	{
		return false;
	}

	if (RightOperandType == EPropertyOperandType::InputPin && RightInputPinName.IsEmpty())
	{
		return false;
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

	const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Object);
	Property->ExportText_Direct(OutValue, ValuePtr, "-1", const_cast<UObject*>(Object), PPF_None);

	return true;
}

UEdGraphNode* UCheckPropertyDebugCondition::GetCurrentNodeFromStackFrame(const UObject* ActiveObject, const FFrame& StackFrame) const
{
	if (!IsValid(ActiveObject))
	{
		return nullptr;
	}

	UFunction* Function = StackFrame.Node;
	if (Function == nullptr)
	{
		return nullptr;
	}

	int32 CodeOffset = StackFrame.Code - StackFrame.Node->Script.GetData();
	UBlueprintGeneratedClass* BGClass = Cast<UBlueprintGeneratedClass>(ActiveObject->GetClass());
	if (BGClass == nullptr)
	{
		return nullptr;
	}

	return BGClass->DebugData.FindSourceNodeFromCodeLocation(Function, CodeOffset, true);
}

bool UCheckPropertyDebugCondition::GetInputPinValue(const UEdGraphNode* Node, const FString& PinName, FString& OutValue) const
{
	if (Node == nullptr || PinName.IsEmpty())
	{
		return false;
	}

	for (UEdGraphPin* Pin : Node->Pins)
	{
		if (Pin != nullptr && Pin->Direction == EGPD_Input && Pin->PinName.ToString() == PinName)
		{
			if (Pin->LinkedTo.Num() > 0)
			{
				return false;
			}

			if (!Pin->DefaultValue.IsEmpty())
			{
				OutValue = Pin->DefaultValue;
				return true;
			}

			if (Pin->DefaultObject != nullptr)
			{
				OutValue = Pin->DefaultObject->GetName();
				return true;
			}

			if (!Pin->DefaultTextValue.IsEmpty())
			{
				OutValue = Pin->DefaultTextValue.ToString();
				return true;
			}

			if (Pin->PinType.PinCategory == TEXT("bool"))
			{
				OutValue = TEXT("False");
				return true;
			}

			OutValue = TEXT("");
			return true;
		}
	}

	return false;
}

bool UCheckPropertyDebugCondition::CompareValues(const FString& LeftValue, const FString& RightValue) const
{
	double LeftNum = 0.0;
	double RightNum = 0.0;
	bool bLeftIsNumeric = LexTryParseString(LeftNum, *LeftValue);
	bool bRightIsNumeric = LexTryParseString(RightNum, *RightValue);

	if (bLeftIsNumeric && bRightIsNumeric)
	{
		switch (Operator)
		{
		case EPropertyComparisonOperator::Equal:
			return FMath::IsNearlyEqual(LeftNum, RightNum, KINDA_SMALL_NUMBER);

		case EPropertyComparisonOperator::NotEqual:
			return !FMath::IsNearlyEqual(LeftNum, RightNum, KINDA_SMALL_NUMBER);

		case EPropertyComparisonOperator::Less:
			return LeftNum < RightNum;

		case EPropertyComparisonOperator::LessOrEqual:
			return LeftNum <= RightNum;

		case EPropertyComparisonOperator::Greater:
			return LeftNum > RightNum;

		case EPropertyComparisonOperator::GreaterOrEqual:
			return LeftNum >= RightNum;

		default:
			return false;
		}
	}

	switch (Operator)
	{
	case EPropertyComparisonOperator::Equal:
		return LeftValue == RightValue;

	case EPropertyComparisonOperator::NotEqual:
		return LeftValue != RightValue;

	case EPropertyComparisonOperator::Less:
		return LeftValue < RightValue;

	case EPropertyComparisonOperator::LessOrEqual:
		return LeftValue <= RightValue;

	case EPropertyComparisonOperator::Greater:
		return LeftValue > RightValue;

	case EPropertyComparisonOperator::GreaterOrEqual:
		return LeftValue >= RightValue;

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
			if (Property->HasAnyPropertyFlags(CPF_Transient | CPF_Deprecated))
			{
				continue;
			}

			OutPropertyNames.Add(Property->GetName());
		}
	}
}

void UCheckPropertyDebugCondition::GetAvailableInputPins(const UEdGraphNode* Node, TArray<FString>& OutPinNames) const
{
	OutPinNames.Empty();

	if (Node == nullptr)
	{
		return;
	}

	for (UEdGraphPin* Pin : Node->Pins)
	{
		if (Pin != nullptr && Pin->Direction == EGPD_Input && !Pin->PinName.IsNone())
		{
			FString PinNameStr = Pin->PinName.ToString();
			if (!PinNameStr.IsEmpty())
			{
				OutPinNames.Add(PinNameStr);
			}
		}
	}
}

TSharedPtr<SWidget> UCheckPropertyDebugCondition::InitializationWidget(UBlueprint* Blueprint, const UEdGraphNode* Node)
{
	return SNew(SCheckPropertyInitializationWidget)
		.Blueprint(Blueprint)
		.Node(Node)
		.Condition(this);
}

void SCheckPropertyInitializationWidget::Construct(const FArguments& InArgs)
{
	Blueprint = InArgs._Blueprint;
	CurrentNode = const_cast<UEdGraphNode*>(InArgs._Node);
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
	BuildInputPinOptions(CurrentNode);

	OperatorOptions = MakeShared<TArray<TSharedPtr<FString>>>();
	OperatorOptions->Add(MakeShared<FString>(TEXT("==")));
	OperatorOptions->Add(MakeShared<FString>(TEXT("!=")));
	OperatorOptions->Add(MakeShared<FString>(TEXT("<")));
	OperatorOptions->Add(MakeShared<FString>(TEXT("<=")));
	OperatorOptions->Add(MakeShared<FString>(TEXT(">")));
	OperatorOptions->Add(MakeShared<FString>(TEXT(">=")));

	OperandTypeOptions = MakeShared<TArray<TSharedPtr<FString>>>();
	OperandTypeOptions->Add(MakeShared<FString>(TEXT("Property")));
	OperandTypeOptions->Add(MakeShared<FString>(TEXT("InputPin")));
	OperandTypeOptions->Add(MakeShared<FString>(TEXT("Constant")));

	LeftOperandTypeOptions = MakeShared<TArray<TSharedPtr<FString>>>();
	LeftOperandTypeOptions->Add(MakeShared<FString>(TEXT("Property")));
	LeftOperandTypeOptions->Add(MakeShared<FString>(TEXT("InputPin")));

	if (!InputPinOptions.IsValid())
	{
		InputPinOptions = MakeShared<TArray<TSharedPtr<FString>>>();
		InputPinOptions->Add(MakeShared<FString>(FString()));
	}

	ChildSlot

	[
		RebuildContent()
	];
}

TSharedRef<SWidget> SCheckPropertyInitializationWidget::RebuildContent()
{
	if (Condition == nullptr)
	{
		return SNullWidget::NullWidget;
	}

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

	FString LeftOperandTypeStr;
	switch (Condition->LeftOperandType)
	{
	case EPropertyOperandType::Property: LeftOperandTypeStr = TEXT("Property"); break;
	case EPropertyOperandType::InputPin: LeftOperandTypeStr = TEXT("InputPin"); break;
	default: 
		LeftOperandTypeStr = TEXT("Property");
		Condition->LeftOperandType = EPropertyOperandType::Property;
		break;
	}
	const TSharedPtr<FString> InitialLeftOperandType = FindInitialSelection(LeftOperandTypeOptions, LeftOperandTypeStr);

	const TSharedPtr<FString> InitialLeftProp = FindInitialSelection(PropertyOptions, Condition->LeftPropertyName);
	const TSharedPtr<FString> InitialLeftInputPin = FindInitialSelection(InputPinOptions, Condition->LeftInputPinName);

	FString OperatorStr;
	switch (Condition->Operator)
	{
	case EPropertyComparisonOperator::Equal: OperatorStr = TEXT("=="); break;
	case EPropertyComparisonOperator::NotEqual: OperatorStr = TEXT("!="); break;
	case EPropertyComparisonOperator::Less: OperatorStr = TEXT("<"); break;
	case EPropertyComparisonOperator::LessOrEqual: OperatorStr = TEXT("<="); break;
	case EPropertyComparisonOperator::Greater: OperatorStr = TEXT(">"); break;
	case EPropertyComparisonOperator::GreaterOrEqual: OperatorStr = TEXT(">="); break;
	}
	const TSharedPtr<FString> InitialOperator = FindInitialSelection(OperatorOptions, OperatorStr);

	FString RightOperandTypeStr;
	switch (Condition->RightOperandType)
	{
	case EPropertyOperandType::Property: RightOperandTypeStr = TEXT("Property"); break;
	case EPropertyOperandType::InputPin: RightOperandTypeStr = TEXT("InputPin"); break;
	case EPropertyOperandType::Constant: RightOperandTypeStr = TEXT("Constant"); break;
	default: RightOperandTypeStr = TEXT("Constant"); break;
	}
	const TSharedPtr<FString> InitialRightOperandType = FindInitialSelection(OperandTypeOptions, RightOperandTypeStr);

	TSharedPtr<FString> InitialRightProp = FindInitialSelection(PropertyOptions, Condition->RightPropertyName);
	TSharedPtr<FString> InitialRightInputPin = FindInitialSelection(InputPinOptions, Condition->RightInputPinName);

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(6.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(0.4f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.3f)
				[
					SNew(SComboBox<TSharedPtr<FString>>)
					.OptionsSource(LeftOperandTypeOptions.Get())
					.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem)
						{
							return SNew(STextBlock).Text(FText::FromString(InItem.IsValid() ? *InItem : FString("Property")));
						})
					.OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
						{
							if (SelectInfo != ESelectInfo::Direct && NewSelection.IsValid() && Condition != nullptr)
							{
								if (*NewSelection == TEXT("Property"))
								{
									Condition->LeftOperandType = EPropertyOperandType::Property;
								}
								else if (*NewSelection == TEXT("InputPin"))
								{
									Condition->LeftOperandType = EPropertyOperandType::InputPin;
								}
								Invalidate(EInvalidateWidget::Layout);
							}
						})
					.InitiallySelectedItem(InitialLeftOperandType)
					[
						SNew(STextBlock)
						.Text_Lambda([this]()
							{
								if (Condition != nullptr)
								{
									switch (Condition->LeftOperandType)
									{
									case EPropertyOperandType::Property: return FText::FromString(TEXT("Property"));
									case EPropertyOperandType::InputPin: return FText::FromString(TEXT("InputPin"));
									default: return FText::FromString(TEXT("Property"));
									}
								}
								return FText::FromString(TEXT("Property"));
							})
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.7f)
				.Padding(4, 0, 0, 0)
				[
					SNew(SBox)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SComboBox<TSharedPtr<FString>>)
							.Visibility_Lambda([this]()
								{
									return (Condition != nullptr && Condition->LeftOperandType == EPropertyOperandType::Property) ? EVisibility::Visible : EVisibility::Collapsed;
								})
							.OptionsSource(PropertyOptions.Get())
							.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem)
								{
									return SNew(STextBlock).Text(FText::FromString(InItem.IsValid() ? *InItem : FString("None")));
								})
							.OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
								{
									if (SelectInfo != ESelectInfo::Direct && NewSelection.IsValid() && Condition != nullptr)
									{
										Condition->LeftPropertyName = *NewSelection;
										if (!IsObjectProperty(*NewSelection))
										{
											Condition->bUseObjectNameForLeftProperty = false;
										}
										Invalidate(EInvalidateWidget::Layout);
									}
								})
							.InitiallySelectedItem(InitialLeftProp)
							[
								SNew(STextBlock)
								.Text_Lambda([this]()
									{
										return FText::FromString(Condition != nullptr && !Condition->LeftPropertyName.IsEmpty() ? Condition->LeftPropertyName : TEXT("(Select Property)"));
									})
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 4.f, 0.f, 0.f)
						[
							SNew(SCheckBox)
							.Visibility_Lambda([this]()
								{
									return (Condition != nullptr && Condition->LeftOperandType == EPropertyOperandType::Property && IsObjectProperty(Condition->LeftPropertyName)) ? EVisibility::Visible : EVisibility::Collapsed;
								})
							.IsChecked_Lambda([this]()
								{
									return (Condition != nullptr && Condition->bUseObjectNameForLeftProperty) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
								})
							.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
								{
									if (Condition != nullptr)
									{
										Condition->bUseObjectNameForLeftProperty = (NewState == ECheckBoxState::Checked);
									}
								})
							[
								SNew(STextBlock)
								.Text(LOCTEXT("CompareLeftByName", "Compare by object name"))
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SComboBox<TSharedPtr<FString>>)
							.Visibility_Lambda([this]()
								{
									return (Condition != nullptr && Condition->LeftOperandType == EPropertyOperandType::InputPin) ? EVisibility::Visible : EVisibility::Collapsed;
								})
							.OptionsSource(InputPinOptions.Get())
							.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem)
								{
									return SNew(STextBlock).Text(FText::FromString(InItem.IsValid() ? *InItem : FString("None")));
								})
							.OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
								{
									if (SelectInfo != ESelectInfo::Direct && NewSelection.IsValid() && Condition != nullptr)
									{
										Condition->LeftInputPinName = *NewSelection;
										Invalidate(EInvalidateWidget::Layout);
									}
								})
							.InitiallySelectedItem(InitialLeftInputPin)
							[
								SNew(STextBlock)
								.Text_Lambda([this]()
									{
										return FText::FromString(Condition != nullptr && !Condition->LeftInputPinName.IsEmpty() ? Condition->LeftInputPinName : TEXT("(Select Input Pin)"));
									})
							]
						]
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
				.OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
					{
						if (SelectInfo != ESelectInfo::Direct && NewSelection.IsValid() && Condition != nullptr)
						{
							if (*NewSelection == TEXT("=="))
								Condition->Operator = EPropertyComparisonOperator::Equal;
							else if (*NewSelection == TEXT("!="))
								Condition->Operator = EPropertyComparisonOperator::NotEqual;
							else if (*NewSelection == TEXT("<"))
								Condition->Operator = EPropertyComparisonOperator::Less;
							else if (*NewSelection == TEXT("<="))
								Condition->Operator = EPropertyComparisonOperator::LessOrEqual;
							else if (*NewSelection == TEXT(">"))
								Condition->Operator = EPropertyComparisonOperator::Greater;
							else if (*NewSelection == TEXT(">="))
								Condition->Operator = EPropertyComparisonOperator::GreaterOrEqual;
						}
					})
				.InitiallySelectedItem(InitialOperator)
				[
					SNew(STextBlock)
					.Text_Lambda([this]()
						{
							if (Condition != nullptr)
							{
								FString OpStr;
								switch (Condition->Operator)
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
					.OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
						{
							if (SelectInfo != ESelectInfo::Direct && NewSelection.IsValid() && Condition != nullptr)
							{
								if (*NewSelection == TEXT("Property"))
									Condition->RightOperandType = EPropertyOperandType::Property;
								else if (*NewSelection == TEXT("InputPin"))
									Condition->RightOperandType = EPropertyOperandType::InputPin;
								else if (*NewSelection == TEXT("Constant"))
									Condition->RightOperandType = EPropertyOperandType::Constant;
							}
						})
					.InitiallySelectedItem(InitialRightOperandType)
					[
						SNew(STextBlock)
						.Text_Lambda([this]()
							{
								if (Condition != nullptr)
								{
									switch (Condition->RightOperandType)
									{
									case EPropertyOperandType::Property: return FText::FromString(TEXT("Property"));
									case EPropertyOperandType::InputPin: return FText::FromString(TEXT("InputPin"));
									case EPropertyOperandType::Constant: return FText::FromString(TEXT("Constant"));
									default: return FText::FromString(TEXT("Constant"));
									}
								}
								return FText::FromString(TEXT("Constant"));
							})
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.7f)
				.Padding(4, 0, 0, 0)
				[
					SNew(SBox)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SComboBox<TSharedPtr<FString>>)
							.Visibility_Lambda([this]()
								{
									return (Condition != nullptr && Condition->RightOperandType == EPropertyOperandType::Property) ? EVisibility::Visible : EVisibility::Collapsed;
								})
							.OptionsSource(PropertyOptions.Get())
							.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem)
								{
									return SNew(STextBlock).Text(FText::FromString(InItem.IsValid() ? *InItem : FString("None")));
								})
							.OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
								{
									if (SelectInfo != ESelectInfo::Direct && NewSelection.IsValid() && Condition != nullptr)
									{
										Condition->RightPropertyName = *NewSelection;
										if (!IsObjectProperty(*NewSelection))
										{
											Condition->bUseObjectNameForRightProperty = false;
										}
									}
								})
							.InitiallySelectedItem(InitialRightProp)
							[
								SNew(STextBlock)
								.Text_Lambda([this]()
									{
										return FText::FromString(Condition != nullptr && !Condition->RightPropertyName.IsEmpty() ? Condition->RightPropertyName : TEXT("(Select Property)"));
									})
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 4.f, 0.f, 0.f)
						[
							SNew(SCheckBox)
							.Visibility_Lambda([this]()
								{
									return (Condition != nullptr
										&& Condition->RightOperandType == EPropertyOperandType::Property
										&& IsObjectProperty(Condition->RightPropertyName))
										? EVisibility::Visible
										: EVisibility::Collapsed;
								})
							.IsChecked_Lambda([this]()
								{
									return (Condition != nullptr && Condition->bUseObjectNameForRightProperty)
										? ECheckBoxState::Checked
										: ECheckBoxState::Unchecked;
								})
							.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
								{
									if (Condition != nullptr)
									{
										Condition->bUseObjectNameForRightProperty = (NewState == ECheckBoxState::Checked);
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
							SNew(SComboBox<TSharedPtr<FString>>)
							.Visibility_Lambda([this]()
								{
									return (Condition != nullptr && Condition->RightOperandType == EPropertyOperandType::InputPin) ? EVisibility::Visible : EVisibility::Collapsed;
								})
							.OptionsSource(InputPinOptions.Get())
							.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem)
								{
									return SNew(STextBlock).Text(FText::FromString(InItem.IsValid() ? *InItem : FString("None")));
								})
							.OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
								{
									if (SelectInfo != ESelectInfo::Direct && NewSelection.IsValid() && Condition != nullptr)
									{
										Condition->RightInputPinName = *NewSelection;
									}
								})
							.InitiallySelectedItem(InitialRightInputPin)
							[
								SNew(STextBlock)
								.Text_Lambda([this]()
									{
										return FText::FromString(Condition != nullptr && !Condition->RightInputPinName.IsEmpty() ? Condition->RightInputPinName : TEXT("(Select Input Pin)"));
									})
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SCheckBox)
							.Visibility_Lambda([this]()
								{
									return (Condition != nullptr 
										&& Condition->RightOperandType == EPropertyOperandType::Constant 
										&& IsLeftOperandBool()) 
										? EVisibility::Visible 
										: EVisibility::Collapsed;
								})
							.IsChecked_Lambda([this]()
								{
									if (Condition != nullptr)
									{
										FString Value = Condition->ConstantValue;
										return (Value == TEXT("True") || Value == TEXT("true") || Value == TEXT("1")) 
											? ECheckBoxState::Checked 
											: ECheckBoxState::Unchecked;
									}
									return ECheckBoxState::Unchecked;
								})
							.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
								{
									if (Condition != nullptr)
									{
										Condition->ConstantValue = (NewState == ECheckBoxState::Checked) ? TEXT("True") : TEXT("False");
									}
								})
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SEditableTextBox)
							.Visibility_Lambda([this]()
								{
									return (Condition != nullptr 
										&& Condition->RightOperandType == EPropertyOperandType::Constant 
										&& !IsLeftOperandBool()) 
										? EVisibility::Visible 
										: EVisibility::Collapsed;
								})
							.Text_Lambda([this]()
								{
									return FText::FromString(Condition != nullptr ? Condition->ConstantValue : FString());
								})
							.OnTextChanged_Lambda([this](const FText& NewText)
								{
									if (Condition != nullptr)
									{
										Condition->ConstantValue = NewText.ToString();
									}
								})
							.HintText(LOCTEXT("ConstantValueHint", "Enter constant value"))
						]
					]
				]
			]
		];
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

	PropertyOptions->Add(MakeShared<FString>(FString()));
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

void SCheckPropertyInitializationWidget::BuildInputPinOptions(const UEdGraphNode* Node)
{
	InputPinOptions = MakeShared<TArray<TSharedPtr<FString>>>();
	InputPinOptions->Add(MakeShared<FString>(FString()));

	if (Node != nullptr)
	{
		TArray<FString> PinNames;
		if (Condition != nullptr)
		{
			Condition->GetAvailableInputPins(Node, PinNames);
		}

		TArray<FString> SortedPinNames = PinNames;
		SortedPinNames.Sort();

		for (const FString& PinName : SortedPinNames)
		{
			InputPinOptions->Add(MakeShared<FString>(PinName));
		}
	}
	else if (Blueprint != nullptr)
	{
		TSet<FString> AllPinNames;

		auto ProcessGraph = [&AllPinNames](UEdGraph* Graph)
		{
			if (Graph != nullptr)
			{
				for (UEdGraphNode* GraphNode : Graph->Nodes)
				{
					if (GraphNode != nullptr)
					{
						for (UEdGraphPin* Pin : GraphNode->Pins)
						{
							if (Pin != nullptr && Pin->Direction == EGPD_Input && !Pin->PinName.IsNone())
							{
								FString PinNameStr = Pin->PinName.ToString();
								if (!PinNameStr.IsEmpty())
								{
									AllPinNames.Add(PinNameStr);
								}
							}
						}
					}
				}
			}
		};

		for (UEdGraph* Graph : Blueprint->UbergraphPages)
		{
			ProcessGraph(Graph);
		}

		for (UEdGraph* Graph : Blueprint->FunctionGraphs)
		{
			ProcessGraph(Graph);
		}

		TArray<FString> SortedPinNames = AllPinNames.Array();
		SortedPinNames.Sort();

		for (const FString& PinName : SortedPinNames)
		{
			InputPinOptions->Add(MakeShared<FString>(PinName));
		}
	}
}

bool SCheckPropertyInitializationWidget::IsObjectProperty(const FString& PropertyName) const
{
	return ObjectPropertyNames.Contains(PropertyName);
}

bool SCheckPropertyInitializationWidget::IsLeftOperandBool() const
{
	if (Condition == nullptr)
	{
		return false;
	}

	if (Condition->LeftOperandType == EPropertyOperandType::Property)
	{
		if (Condition->LeftPropertyName.IsEmpty() || Blueprint == nullptr || Blueprint->GeneratedClass == nullptr)
		{
			return false;
		}

		UObject* DefaultObject = Blueprint->GeneratedClass->GetDefaultObject();
		if (DefaultObject == nullptr)
		{
			return false;
		}

		FProperty* Property = FindFProperty<FProperty>(DefaultObject->GetClass(), *Condition->LeftPropertyName);
		if (Property == nullptr)
		{
			return false;
		}

		FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property);
		return BoolProperty != nullptr;
	}
	else if (Condition->LeftOperandType == EPropertyOperandType::InputPin)
	{
		if (Condition->LeftInputPinName.IsEmpty() || CurrentNode == nullptr)
		{
			return false;
		}

		for (UEdGraphPin* Pin : CurrentNode->Pins)
		{
			if (Pin != nullptr 
				&& Pin->Direction == EGPD_Input 
				&& Pin->PinName.ToString() == Condition->LeftInputPinName)
			{
				return Pin->PinType.PinCategory == TEXT("bool");
			}
		}

		if (Blueprint != nullptr)
		{
			auto FindPinInGraph = [this](UEdGraph* Graph) -> bool
			{
				if (Graph == nullptr)
				{
					return false;
				}

				for (UEdGraphNode* GraphNode : Graph->Nodes)
				{
					if (GraphNode != nullptr)
					{
						for (UEdGraphPin* Pin : GraphNode->Pins)
						{
							if (Pin != nullptr 
								&& Pin->Direction == EGPD_Input 
								&& Pin->PinName.ToString() == Condition->LeftInputPinName)
							{
								return Pin->PinType.PinCategory == TEXT("bool");
							}
						}
					}
				}
				return false;
			};

			for (UEdGraph* Graph : Blueprint->UbergraphPages)
			{
				if (FindPinInGraph(Graph))
				{
					return true;
				}
			}

			for (UEdGraph* Graph : Blueprint->FunctionGraphs)
			{
				if (FindPinInGraph(Graph))
				{
					return true;
				}
			}
		}

		return false;
	}

	return false;
}


#undef LOCTEXT_NAMESPACE

