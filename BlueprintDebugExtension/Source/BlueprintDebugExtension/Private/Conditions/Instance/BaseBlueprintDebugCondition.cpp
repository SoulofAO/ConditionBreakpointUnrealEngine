/*
 * Publisher: AO
 * Year of Publication: 2025
 * Copyright AO All Rights Reserved.
 */


#include "Conditions/Instance/BaseBlueprintDebugCondition.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Widgets/Layout/SBox.h"

bool UBaseBlueprintDebugCondition::CheckCondition(const UObject* ActiveObject, const FFrame& StackFrame, UExecBlueprintBreakpointContext* ExecBlueprintPointContext)
{
    UObject* ActiveObjectNonConst = const_cast<UObject*>(ActiveObject);
    bool bResult = false;
    bResult = CheckConditionBlueprint(ActiveObjectNonConst, ExecBlueprintPointContext);
    return bResult;
}

bool UBaseBlueprintDebugCondition::CheckValidCondition(UBlueprint* Blueprint) const
{
    return CheckValidConditionBlueprint(Blueprint);
}

TSharedPtr<SWidget> UBaseBlueprintDebugCondition::InitializationWidget(UBlueprint* Blueprint, const UEdGraphNode* Node)
{
    UWidget* WidgetFromBlueprint = InitializationWidgetBlueprint(Blueprint, Node);

    if (WidgetFromBlueprint != nullptr)
    {
        return WidgetFromBlueprint->TakeWidget();
    }
    return CreateBlueprintDetailPanel(Blueprint);
}


bool UBaseBlueprintDebugCondition::CheckConditionBlueprint_Implementation(UObject* ActiveObject, UExecBlueprintBreakpointContext* ExecBlueprintPointContext)
{
    return false;
}

bool UBaseBlueprintDebugCondition::CheckValidConditionBlueprint_Implementation(UBlueprint* Blueprint) const
{
    return true;
}

UWidget* UBaseBlueprintDebugCondition::InitializationWidgetBlueprint_Implementation(UBlueprint* Blueprint, const UEdGraphNode* Node)
{
    return nullptr;
}

TSharedRef<SWidget> UBaseBlueprintDebugCondition::CreateBlueprintDetailPanel(UBlueprint* Blueprint)
{
    FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.bHideSelectionTip = true;
    DetailsViewArgs.bAllowSearch = true;

	//Blueprint->OnCompiled().AddUObject(this, &UBaseBlueprintDebugCondition::OnCompile);

    TSharedRef<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
    DetailsView->SetObject(this);
    return DetailsView;
}

void UBaseBlueprintDebugCondition::OnCompile(UBlueprint* Blueprint)
{
    //DetailsView->SetObject(this);
}
