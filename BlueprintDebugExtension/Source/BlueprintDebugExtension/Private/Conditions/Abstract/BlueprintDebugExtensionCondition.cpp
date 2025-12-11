/*
 * Publisher: AO
 * Year of Publication: 2025
 * Copyright AO All Rights Reserved.
 */

#include "Conditions/Abstract/BlueprintDebugExtensionCondition.h"
#include "ExecBlueprintBreakpointContext.h"

bool UBlueprintDebugExtensionCondition::CheckCondition(const UObject* ActiveObject, const FFrame& StackFrame, UExecBlueprintBreakpointContext* ExecBlueprintPointContext)
{
	return false;
}

bool UBlueprintDebugExtensionCondition::CheckValidCondition(UBlueprint* Blueprint) const
{
	return false;
}

TSharedPtr<SWidget> UBlueprintDebugExtensionCondition::InitializationWidget(UBlueprint* Blueprint)
{
	return TSharedPtr<SWidget>();
}