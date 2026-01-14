/*
 * Publisher: AO
 * Year of Publication: 2026
 * Copyright AO All Rights Reserved.
 */



#pragma once

#include "CoreMinimal.h"
#include "Conditions/Abstract/BlueprintDebugExtensionCondition.h"
#include "Kismet2/Breakpoint.h"
#include "Containers/Ticker.h"
#include "ExecBlueprintBreakpointContext.h"

#include "BlueprintDebugExtensionSubsystem.generated.h"
/**
 * 
 */

UENUM()
enum class EBlueprintDebugExtensionConditionType : uint8
{
	And,
	Or
};

USTRUCT()
struct FBlueprintDebugExtensionConditionData
{
	GENERATED_BODY()

	UPROPERTY()
	EBlueprintDebugExtensionConditionType ConditionType;

	UPROPERTY()
	UBlueprintDebugExtensionCondition* Condition;
};


USTRUCT()
struct FArrayBlueprintDebugExtensionConditionData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FBlueprintDebugExtensionConditionData> Conditions;

	UPROPERTY()
	UExecBlueprintBreakpointContext* Context;

	FArrayBlueprintDebugExtensionConditionData() = default;

	FArrayBlueprintDebugExtensionConditionData(TArray<FBlueprintDebugExtensionConditionData> NewConditions)
		:Conditions(NewConditions) {};
};

UCLASS(NotBlueprintable)
class BLUEPRINTDEBUGEXTENSION_API UBlueprintDebugExtensionSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	bool CheckCondition(const UObject* ActiveObject, const FFrame& StackFrame);

	void OpenConditionEditorWindow(UBlueprint* Blueprint, const UEdGraphNode* Node);

	void AddNewConditions(UEdGraphNode* Node, TArray<FBlueprintDebugExtensionConditionData> NewConditions);

	bool CheckValidConditions(UEdGraphNode* Node);

	void RemoveCondition(UEdGraphNode* RemoveNode);

	void PlaceTriggeredOnceBreakpoint(UBlueprint* Blueprint, const UEdGraphNode* Node);

	void LoadConditions();

	void SaveConditions();

	bool Tick(float DeltaTime);

	void OnBeginPIE(bool bIsSimulating);

	FTSTicker::FDelegateHandle TickDelegateHandle;

	UPROPERTY()
	TMap<FGuid, FArrayBlueprintDebugExtensionConditionData> Conditions;
};
