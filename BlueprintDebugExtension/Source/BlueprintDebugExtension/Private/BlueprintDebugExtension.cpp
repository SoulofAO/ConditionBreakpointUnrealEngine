/*
 * Publisher: AO
 * Year of Publication: 2026
 * Copyright AO All Rights Reserved.
 */

#include "BlueprintDebugExtension.h"
#include "Blueprint/BlueprintExceptionInfo.h"
#include "Kismet2/DebuggerCommands.h"
#include "GraphEditorModule.h"
#include "BlueprintDebugExtentionsGraphCommands.h"
#include "Kismet2/KismetDebugUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "BlueprintDebugExtensionSubsystem.h"
#include "Misc/DefinePrivateMemberPtr.h"

UE_DEFINE_PRIVATE_MEMBER_PTR(EBlueprintExceptionType::Type, GBlueprintExceptionType, FBlueprintExceptionInfo, EventType);

void FBlueprintDebugExtensionModule::StartupModule()
{
    InitializationTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateLambda([](float /*DeltaSeconds*/) -> bool
            {
                FBlueprintCoreDelegates::OnScriptException.AddStatic(&FBlueprintDebugExtensionModule::OnScriptException);
                return false; 
            }),
        0.1f 
    );
    FBlueprintDebugExtensionGraphCommands::Register();

    FGraphEditorModule& GraphEditor = FModuleManager::LoadModuleChecked<FGraphEditorModule>("GraphEditor");
    GraphEditor.GetAllGraphEditorContextMenuExtender().Add(FGraphEditorModule::FGraphEditorMenuExtender_SelectedNode::CreateRaw(this, &FBlueprintDebugExtensionModule::ExtendSelectNodeSetttings));
}

void FBlueprintDebugExtensionModule::OnAssetEditorOpened(UObject* EditedAsset)
{
      
}

void FBlueprintDebugExtensionModule::ShutdownModule()
{
    FBlueprintDebugExtensionGraphCommands::Unregister();
}

void FBlueprintDebugExtensionModule::OnScriptException(const UObject* ActiveObject, const FFrame& StackFrame, const FBlueprintExceptionInfo& Info)
{
	FBlueprintExceptionInfo& BlueprintExceptionInfo = const_cast<FBlueprintExceptionInfo&>(Info);
	if (BlueprintExceptionInfo.GetType() == EBlueprintExceptionType::Breakpoint)
	{
        UBlueprintDebugExtensionSubsystem* BlueprintDebugExtensionSubsystem = GEditor->GetEditorSubsystem<UBlueprintDebugExtensionSubsystem>();
        if (!BlueprintDebugExtensionSubsystem->CheckCondition(ActiveObject, StackFrame))
        {
            BlueprintExceptionInfo.*GBlueprintExceptionType = EBlueprintExceptionType::Tracepoint;
        }
	}
}


TSharedRef<FExtender> FBlueprintDebugExtensionModule::ExtendSelectNodeSetttings(const TSharedRef<FUICommandList> CmdList,
    const UEdGraph* Graph, const UEdGraphNode* Node, const UEdGraphPin* Pin, bool bReadOnly)
{
    CmdList->MapAction(
        FBlueprintDebugExtensionGraphCommands::Get().OpenBreakpointConditionSettingsCommand,
        FExecuteAction::CreateRaw(this, &FBlueprintDebugExtensionModule::OnOpenBreakpointConditionSettings, Graph, Node, Pin),
        FCanExecuteAction::CreateLambda([bReadOnly]() { return !bReadOnly; })
    );

    CmdList->MapAction(
        FBlueprintDebugExtensionGraphCommands::Get().PlaceTriggeredOnceBreakpointCommand,
        FExecuteAction::CreateRaw(this, &FBlueprintDebugExtensionModule::OnPlaceTriggeredOnceBreakpoint, Graph, Node, Pin),
        FCanExecuteAction::CreateLambda([bReadOnly]() { return !bReadOnly; })
    );

    const FName ExtensionHook(TEXT("EdGraphSchemaBreakpoints"));

    TSharedRef<FExtender> Extender = MakeShared<FExtender>();
    Extender->AddMenuExtension(
        ExtensionHook,
        EExtensionHook::After,
        CmdList,
        FMenuExtensionDelegate::CreateRaw(this, &FBlueprintDebugExtensionModule::AddMenu)
    );
    return Extender;
}

void FBlueprintDebugExtensionModule::AddMenu(FMenuBuilder& MenuBuilder)
{
    MenuBuilder.BeginSection(TEXT("ConditionalBreakPointsTools"), FText::FromString("Conditional BreakPoints"));
    MenuBuilder.AddMenuEntry(FBlueprintDebugExtensionGraphCommands::Get().OpenBreakpointConditionSettingsCommand);
    MenuBuilder.AddMenuEntry(FBlueprintDebugExtensionGraphCommands::Get().PlaceTriggeredOnceBreakpointCommand);
    MenuBuilder.EndSection();

}

void FBlueprintDebugExtensionModule::OnOpenBreakpointConditionSettings(const UEdGraph* Graph, const UEdGraphNode* Node, const UEdGraphPin* Pin)
{
    UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNode(Node);
    if (Blueprint == nullptr)
    {
        return;
    }

    UBlueprintDebugExtensionSubsystem* BlueprintDebugExtensionSubsystem = GEditor->GetEditorSubsystem<UBlueprintDebugExtensionSubsystem>();

    if (BlueprintDebugExtensionSubsystem)
    {
		BlueprintDebugExtensionSubsystem->OpenConditionEditorWindow(Blueprint, Node);
    }
}

void FBlueprintDebugExtensionModule::OnPlaceTriggeredOnceBreakpoint(const UEdGraph* Graph, const UEdGraphNode* Node, const UEdGraphPin* Pin)
{
    if (!Node)
    {
        return;
    }

#if WITH_EDITOR
    UBlueprint* OwnerBlueprint = FBlueprintEditorUtils::FindBlueprintForNode(Node);
    if (!OwnerBlueprint)
    {
        OwnerBlueprint = FBlueprintEditorUtils::FindBlueprintForGraph(Graph);
        if (!OwnerBlueprint)
        {
            return;
        }
    }

    UBlueprintDebugExtensionSubsystem* BlueprintDebugExtensionSubsystem = GEditor->GetEditorSubsystem<UBlueprintDebugExtensionSubsystem>();
    BlueprintDebugExtensionSubsystem->PlaceTriggeredOnceBreakpoint(OwnerBlueprint, Node);
#endif

}

	
IMPLEMENT_MODULE(FBlueprintDebugExtensionModule, BlueprintDebugExtension)