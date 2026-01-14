/*
 * Publisher: AO
 * Year of Publication: 2026
 * Copyright AO All Rights Reserved.
 */




#include "BlueprintDebugExtentionsGraphCommands.h"

#define LOCTEXT_NAMESPACE "FBlueprintDebugExtensionModule"

UE_DEFINE_TCOMMANDS(FBlueprintDebugExtensionGraphCommands)

// MyGraphCommands.cpp
void  FBlueprintDebugExtensionGraphCommands::RegisterCommands()
{
    UI_COMMAND(OpenBreakpointConditionSettingsCommand, "OpenBreakpointConditionSettings", "OpenBreakpointConditionSettings", EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(PlaceTriggeredOnceBreakpointCommand, "PlaceTriggeredOnceBreakpointCommand", "PlaceTriggeredOnceBreakpointCommand", EUserInterfaceActionType::Button, FInputChord());
}
#undef LOCTEXT_NAMESPACE