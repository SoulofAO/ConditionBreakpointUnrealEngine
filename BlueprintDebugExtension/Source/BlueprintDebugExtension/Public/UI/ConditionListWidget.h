/*
 * Publisher: AO
 * Year of Publication: 2025
 * Copyright AO All Rights Reserved.
 */


#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Kismet2/Breakpoint.h"
#include "ClassViewerFilter.h"
#include "BlueprintDebugExtensionSubsystem.h"

class UBlueprintDebugExtensionCondition;

struct FConditionClassFilter : public IClassViewerFilter
{
	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InitOptions, const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> FilterFuncs) override
	{
		return InClass != nullptr
			&& InClass->IsChildOf(UBlueprintDebugExtensionCondition::StaticClass())
			&& !InClass->HasAnyClassFlags(CLASS_Abstract);
	}
	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const class IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< class FClassViewerFilterFuncs > InFilterFuncs) override
	{
		return InUnloadedClassData->IsChildOf(UBlueprintDebugExtensionCondition::StaticClass())
			&& !InUnloadedClassData->HasAnyClassFlags(CLASS_Abstract);
	}

};

class SConditionListWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SConditionListWidget) {}
		SLATE_ARGUMENT(const UEdGraphNode*, Node)
		SLATE_ARGUMENT(UBlueprint*, Blueprint)
	SLATE_END_ARGS()


	void Construct(const FArguments& InArgs);
	TArray<FBlueprintDebugExtensionConditionData> GetEditedConditions() const;

private:
	void RebuildConditionsList();
	void AddNewConditionOfClass(UClass* ChosenClass);
	void RemoveConditionAt(int32 Index);
	void ToggleConditionTypeAt(int32 Index);

private:
	UBlueprint* Blueprint;
	const UEdGraphNode* Node;
	TArray<FBlueprintDebugExtensionConditionData> EditingConditions;
	TSharedPtr<SVerticalBox> ConditionsContainer;
};
