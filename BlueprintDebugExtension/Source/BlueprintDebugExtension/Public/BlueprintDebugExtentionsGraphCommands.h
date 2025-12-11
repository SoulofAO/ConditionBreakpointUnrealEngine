/*
 * Publisher: AO
 * Year of Publication: 2025
 * Copyright AO All Rights Reserved.
 */



#pragma once

#include "CoreMinimal.h"


UE_DECLARE_TCOMMANDS(class FBlueprintDebugExtensionGraphCommands, BLUEPRINTDEBUGEXTENSION_API)

class FBlueprintDebugExtensionGraphCommands : public TCommands<FBlueprintDebugExtensionGraphCommands>
{
public:
	FBlueprintDebugExtensionGraphCommands()
		: TCommands(TEXT("BlueprintDebugExtensionCommands"), NSLOCTEXT("BlueprintDebugExtension", "BlueprintDebugExtension", "BlueprintDebugExtension"),
			NAME_None, FAppStyle::GetAppStyleSetName()) {
	}
	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> OpenBreakpointConditionSettingsCommand;

	TSharedPtr<FUICommandInfo> PlaceTriggeredOnceBreakpointCommand;
};