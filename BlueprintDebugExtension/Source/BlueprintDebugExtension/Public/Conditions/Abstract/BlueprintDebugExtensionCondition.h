/*
 * Publisher: AO
 * Year of Publication: 2026
 * Copyright AO All Rights Reserved.
 */



#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Engine/Blueprint.h"
#include "BlueprintDebugExtensionCondition.generated.h"


class UExecBlueprintBreakpointContext;
class UEdGraphNode;

UCLASS(Abstract)
class BLUEPRINTDEBUGEXTENSION_API UBlueprintDebugExtensionCondition : public UObject
{
	GENERATED_BODY()
public:

	virtual bool CheckCondition(const UObject* ActiveObject, const FFrame& StackFrame, UExecBlueprintBreakpointContext* ExecBlueprintPointContext);

	virtual bool CheckValidCondition(UBlueprint* Blueprint) const;

	virtual class TSharedPtr<SWidget> InitializationWidget(UBlueprint* Blueprint, const UEdGraphNode* Node);
};
