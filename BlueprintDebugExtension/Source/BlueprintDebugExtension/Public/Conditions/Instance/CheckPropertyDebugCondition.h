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
class SScrollBox;

/**
 * Comparison operators for property checks
 */
UENUM()
enum class EPropertyComparisonOperator : uint8
{
	Equal,          // ==
	NotEqual,       // !=
	Less,           // <
	LessOrEqual,    // <=
	Greater,        // >
	GreaterOrEqual  // >=
};

/**
 * Type of operand in comparison
 */
UENUM()
enum class EPropertyOperandType : uint8
{
	Property,       // Use another property value
	Constant        // Use constant value
};

/**
 * Logical operator for combining multiple comparisons
 */
UENUM()
enum class EPropertyLogicalOperator : uint8
{
	And,            // &&
	Or              // ||
};

/**
 * Single property comparison operation
 */
USTRUCT()
struct FPropertyComparison
{
	GENERATED_BODY()

	// Left operand: property name
	UPROPERTY(SaveGame)
	FString LeftPropertyName;

	// If left operand is FObjectProperty, compare object name instead of export text
	UPROPERTY(SaveGame)
	bool bUseObjectNameForLeftProperty = false;

	// Comparison operator
	UPROPERTY(SaveGame)
	EPropertyComparisonOperator Operator = EPropertyComparisonOperator::Equal;

	// Right operand type
	UPROPERTY(SaveGame)
	EPropertyOperandType RightOperandType = EPropertyOperandType::Constant;

	// Right operand: property name (if RightOperandType == Property)
	UPROPERTY(SaveGame)
	FString RightPropertyName;

	// If right operand (property type) is FObjectProperty, compare object name
	UPROPERTY(SaveGame)
	bool bUseObjectNameForRightProperty = false;

	// Right operand: constant value as string (if RightOperandType == Constant)
	UPROPERTY(SaveGame)
	FString ConstantValue;

	// Logical operator to combine with next comparison (if any)
	UPROPERTY(SaveGame)
	EPropertyLogicalOperator LogicalOperator = EPropertyLogicalOperator::And;
};

/**
 * Condition that checks property values using a chain of comparison operations
 * Supports comparing properties with constants or other properties
 */

UCLASS(NotBlueprintable)
class BLUEPRINTDEBUGEXTENSION_API UCheckPropertyDebugCondition : public UBlueprintDebugExtensionCondition
{
	GENERATED_BODY()

public:
	virtual bool CheckCondition(const UObject* ActiveObject, const FFrame& StackFrame, UExecBlueprintBreakpointContext* ExecBlueprintPointContext) override;

	virtual bool CheckValidCondition(UBlueprint* Blueprint) const override;

	virtual TSharedPtr<SWidget> InitializationWidget(UBlueprint* Blueprint) override;

	// List of comparison operations to evaluate
	UPROPERTY(SaveGame)
	TArray<FPropertyComparison> Comparisons;

	friend class SCheckPropertyInitializationWidget;

protected:
	/**
	 * Get property value from object by name
	 * @param Object Object to get property from
	 * @param PropertyName Name of the property
	 * @param bPreferObjectName If property is FObjectProperty, use object name instead of export text
	 * @param OutValue Output string representation of the value
	 * @return true if property was found and value extracted
	 */
	bool GetPropertyValue(const UObject* Object, const FString& PropertyName, bool bPreferObjectName, FString& OutValue) const;

	/**
	 * Compare two values using the specified operator
	 * @param LeftValue Left operand as string
	 * @param RightValue Right operand as string
	 * @param Operator Comparison operator
	 * @return Result of comparison
	 */
	bool CompareValues(const FString& LeftValue, const FString& RightValue, EPropertyComparisonOperator Operator) const;

	/**
	 * Get all available properties from object for selection
	 * @param Object Object to get properties from
	 * @param OutPropertyNames Output array of property names
	 */
	void GetAvailableProperties(const UObject* Object, TArray<FString>& OutPropertyNames) const;
};

class SCheckPropertyInitializationWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCheckPropertyInitializationWidget) {}
		SLATE_ARGUMENT(UBlueprint*, Blueprint)
		SLATE_ARGUMENT(UCheckPropertyDebugCondition*, Condition)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	void BuildPropertyOptions(UBlueprint* Blueprint);
	void RebuildContent();
	bool IsObjectProperty(const FString& PropertyName) const;

private:
	UBlueprint* Blueprint = nullptr;
	UCheckPropertyDebugCondition* Condition = nullptr;
	TSharedPtr<TArray<TSharedPtr<FString>>> PropertyOptions;
	TSharedPtr<TArray<TSharedPtr<FString>>> OperatorOptions;
	TSharedPtr<TArray<TSharedPtr<FString>>> OperandTypeOptions;
	TSet<FString> ObjectPropertyNames;
	TSharedPtr<SScrollBox> ScrollBox;
};
