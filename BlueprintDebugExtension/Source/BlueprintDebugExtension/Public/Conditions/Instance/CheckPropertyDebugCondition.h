/*
 * Publisher: AO
 * Year of Publication: 2025
 * Copyright AO All Rights Reserved.
 */



#pragma once

#include "CoreMinimal.h"
#include "Conditions/Abstract/BlueprintDebugExtensionCondition.h"
#include "Widgets/SCompoundWidget.h"
#include "CheckPropertyDebugCondition.generated.h"

class UObject;
struct FFrame;
class UExecBlueprintBreakpointContext;
class UBlueprint;
class SWidget;
class UEdGraphNode;
class UEdGraphPin;

UENUM()
enum class EPropertyComparisonOperator : uint8
{
	Equal,
	NotEqual,
	Less,
	LessOrEqual,
	Greater,
	GreaterOrEqual
};

UENUM()
enum class EPropertyOperandType : uint8
{
	Property,
	InputPin,
	Constant
};

UCLASS(NotBlueprintable)
class BLUEPRINTDEBUGEXTENSION_API UCheckPropertyDebugCondition : public UBlueprintDebugExtensionCondition
{
	GENERATED_BODY()

public:
	virtual bool CheckCondition(const UObject* ActiveObject, const FFrame& StackFrame, UExecBlueprintBreakpointContext* ExecBlueprintPointContext) override;

	virtual bool CheckValidCondition(UBlueprint* Blueprint) const override;

	virtual TSharedPtr<SWidget> InitializationWidget(UBlueprint* Blueprint, const UEdGraphNode* Node) override;

	UPROPERTY(SaveGame)
	EPropertyOperandType LeftOperandType = EPropertyOperandType::Property;

	UPROPERTY(SaveGame)
	FString LeftPropertyName;

	UPROPERTY(SaveGame)
	FString LeftInputPinName;

	UPROPERTY(SaveGame)
	bool bUseObjectNameForLeftProperty = false;

	UPROPERTY(SaveGame)
	EPropertyComparisonOperator Operator = EPropertyComparisonOperator::Equal;

	UPROPERTY(SaveGame)
	EPropertyOperandType RightOperandType = EPropertyOperandType::Constant;

	UPROPERTY(SaveGame)
	FString RightPropertyName;

	UPROPERTY(SaveGame)
	FString RightInputPinName;

	UPROPERTY(SaveGame)
	bool bUseObjectNameForRightProperty = false;

	UPROPERTY(SaveGame)
	FString ConstantValue;

	friend class SCheckPropertyInitializationWidget;

protected:
	bool GetPropertyValue(const UObject* Object, const FString& PropertyName, bool bPreferObjectName, FString& OutValue) const;

	bool GetInputPinValue(const UEdGraphNode* Node, const FString& PinName, FString& OutValue) const;

	UEdGraphNode* GetCurrentNodeFromStackFrame(const UObject* ActiveObject, const FFrame& StackFrame) const;

	bool CompareValues(const FString& LeftValue, const FString& RightValue) const;

	void GetAvailableProperties(const UObject* Object, TArray<FString>& OutPropertyNames) const;

	void GetAvailableInputPins(const UEdGraphNode* Node, TArray<FString>& OutPinNames) const;
};

class SCheckPropertyInitializationWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCheckPropertyInitializationWidget) {}
		SLATE_ARGUMENT(UBlueprint*, Blueprint)
		SLATE_ARGUMENT(const UEdGraphNode*, Node)
		SLATE_ARGUMENT(UCheckPropertyDebugCondition*, Condition)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	void BuildPropertyOptions(UBlueprint* Blueprint);
	void BuildInputPinOptions(const UEdGraphNode* Node);
	TSharedRef<SWidget> RebuildContent();
	bool IsObjectProperty(const FString& PropertyName) const;
	bool IsLeftOperandBool() const;

private:
	UBlueprint* Blueprint = nullptr;
	UEdGraphNode* CurrentNode = nullptr;
	UCheckPropertyDebugCondition* Condition = nullptr;
	TSharedPtr<TArray<TSharedPtr<FString>>> PropertyOptions;
	TSharedPtr<TArray<TSharedPtr<FString>>> InputPinOptions;
	TSharedPtr<TArray<TSharedPtr<FString>>> OperatorOptions;
	TSharedPtr<TArray<TSharedPtr<FString>>> OperandTypeOptions;
	TSharedPtr<TArray<TSharedPtr<FString>>> LeftOperandTypeOptions;
	TSet<FString> ObjectPropertyNames;
};
