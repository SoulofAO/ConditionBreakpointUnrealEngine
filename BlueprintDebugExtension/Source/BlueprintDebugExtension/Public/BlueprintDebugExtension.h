/*
 * Publisher: AO
 * Year of Publication: 2026
 * Copyright AO All Rights Reserved.
 */



#pragma once

#include "Modules/ModuleManager.h"
#include "Containers/Ticker.h"
#include "Framework/Commands/Commands.h"
#include "GraphEditorModule.h"


class FBlueprintDebugExtensionModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static void OnScriptException(const UObject* ActiveObject, const FFrame& StackFrame, const FBlueprintExceptionInfo& Info);

	void RegisterCommands();

	void OnAssetEditorOpened(UObject* EditedAsset);

	TSharedRef<FExtender> ExtendSelectNodeSetttings(const TSharedRef<FUICommandList> CmdList,
		const UEdGraph* Graph, const UEdGraphNode* Node, const UEdGraphPin* Pin, bool bReadOnly);

	void AddMenu(FMenuBuilder& MenuBuilder);
	void OnOpenBreakpointConditionSettings(const UEdGraph* Graph, const UEdGraphNode* Node, const UEdGraphPin* Pin);
	void OnPlaceTriggeredOnceBreakpoint(const UEdGraph* Graph, const UEdGraphNode* Node, const UEdGraphPin* Pin);

private:
	FTSTicker::FDelegateHandle InitializationTickerHandle;
};

