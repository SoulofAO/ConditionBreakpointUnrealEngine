/*
 * Publisher: AO
 * Year of Publication: 2025
 * Copyright AO All Rights Reserved.
 */



#include "Conditions/Instance/FunctionBindingDebugCondition.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Toolkits/ToolkitManager.h"
#include "BlueprintEditor.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"

#define LOCTEXT_NAMESPACE "FBlueprintDebugExtensionModule"

bool UFunctionBindingDebugCondition::CheckCondition(const UObject* ActiveObject, const FFrame& StackFrame, UExecBlueprintBreakpointContext* ExecBlueprintPointContext)
{
    if (ActiveObject == nullptr)
    {
        return false;
    }

    UFunction* Function = const_cast<UObject*>(ActiveObject)->FindFunction(*FunctionName);

    if (!Function)
    {
        return false;
    }

    int32 ParmsSize = Function->ParmsSize;
    FBoolProperty* BoolReturn = FindBoolOutParameter(Function);

    TArray<uint8> Params;
    Params.AddZeroed(ParmsSize);
    const_cast<UObject*>(ActiveObject)->ProcessEvent(Function, Params.GetData());
    return BoolReturn->GetPropertyValue_InContainer(Params.GetData());

}

bool UFunctionBindingDebugCondition::CheckValidCondition(UBlueprint* Blueprint) const
{
    return !FunctionName.IsEmpty();
}

TSharedPtr<SWidget> UFunctionBindingDebugCondition::InitializationWidget(UBlueprint* Blueprint, const UEdGraphNode* Node)
{
    if (Blueprint == nullptr || Blueprint->GeneratedClass == nullptr)
    {
        return SNullWidget::NullWidget;
    }

    UClass* TargetClass = Blueprint->GeneratedClass;
    TSharedPtr<TArray<TSharedPtr<FString>>> CandidateFunctionNames = MakeShared<TArray<TSharedPtr<FString>>>();
    CandidateFunctionNames->Add(MakeShared<FString>(FString()));

    for (TFieldIterator<UFunction> FuncIt(TargetClass, EFieldIteratorFlags::ExcludeSuper); FuncIt; ++FuncIt)
    {
        UFunction* Function = *FuncIt;

        FProperty* ReturnProperty = nullptr;

        FBoolProperty* BoolReturn = FindBoolOutParameter(Function);

        if (Function->NumParms == 1 && BoolReturn)
        {
            CandidateFunctionNames->Add(MakeShared<FString>(Function->GetName()));
        }
    }
    TSharedPtr<FString> InitialSelection;
    for (TSharedPtr<FString> CandidateFunctionName : *CandidateFunctionNames)
    {
        if (*CandidateFunctionName == FunctionName)
        {
            InitialSelection = CandidateFunctionName;
        }
    }

    return SNew(SFunctionBindingWidget)
        .CandidateFunctionNames(CandidateFunctionNames)
        .InitialSelection(InitialSelection)
        .Blueprint(Blueprint)
        .OnFunctionSelected(FOnFunctionSelected::CreateLambda([this](const FString& NewSelection)
            {
                this->FunctionName = NewSelection;
            }));
}

FBoolProperty* UFunctionBindingDebugCondition::FindBoolOutParameter(UFunction* Function)
{
    for (TFieldIterator<FProperty> PropIt(Function); PropIt; ++PropIt)
    {
        FProperty* Property = *PropIt;

        if (Property->HasAnyPropertyFlags(CPF_OutParm))
        {
            if (FBoolProperty* ResultProperty = CastField<FBoolProperty>(Property))
            {
                return ResultProperty;
            }
        }
    }

    return nullptr;
}


void SFunctionBindingWidget::GenerateNewFunction()
{
    if (Blueprint == nullptr)
    {
        return;
    }

    const FScopedTransaction Transaction(LOCTEXT("GenerateNewFunctionTransaction", "Generate New Function"));

    Blueprint->Modify();

    int32 NextIndex = 1;
    const FString Prefix = TEXT("Debug_BreakpointExtention_");

    if (Blueprint->FunctionGraphs.Num() > 0)
    {
        for (UEdGraph* Graph : Blueprint->FunctionGraphs)
        {
            if (Graph != nullptr)
            {
                const FString GraphName = Graph->GetName();
                if (GraphName.StartsWith(Prefix))
                {
                    const FString Suffix = GraphName.RightChop(Prefix.Len());
                    const int32 FoundIndex = FCString::Atoi(*Suffix);
                    if (FoundIndex >= NextIndex)
                    {
                        NextIndex = FoundIndex + 1;
                    }
                }
            }
        }
    }

    FString GraphBaseName = FString::Format(TEXT("{0}{1}"), { Prefix, FString::FromInt(NextIndex) });
    FName GraphName = FName(*GraphBaseName);

    UEdGraph* NewGraph = FBlueprintEditorUtils::CreateNewGraph(Blueprint, GraphName, UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass());
    if (NewGraph == nullptr)
    {
        return;
    }
    TSharedPtr<FString> NewName = MakeShared<FString>(GraphBaseName);
    CandidateFunctionNames->Add(NewName);
    ComboBox->SetSelectedItem(NewName);

    CurrentSelection = NewName;
    if (OnFunctionSelected.IsBound())
    {
        OnFunctionSelected.Execute(*CurrentSelection);
    }


    FBlueprintEditorUtils::AddFunctionGraph(Blueprint, NewGraph, true, static_cast<UFunction*>(nullptr));

    FKismetUserDeclaredFunctionMetadata* FunctionMeta = FBlueprintEditorUtils::GetGraphFunctionMetaData(NewGraph);
    if (FunctionMeta != nullptr)
    {
        FunctionMeta->Category = FText::FromString(TEXT("Debug|Generated"));
    }

    UK2Node_FunctionEntry* FunctionEntryNode = nullptr;
    for (UEdGraphNode* Node : NewGraph->Nodes)
    {
        FunctionEntryNode = Cast<UK2Node_FunctionEntry>(Node);
        if (FunctionEntryNode)
        {
            break;
        }
    }

    UK2Node_FunctionResult* ReturnNode = NewObject<UK2Node_FunctionResult>(NewGraph);
    NewGraph->AddNode(ReturnNode);
    ReturnNode->Modify();
    ReturnNode->NodeComment = TEXT("Return True, if Breakpoint should stop");
    ReturnNode->SetMakeCommentBubbleVisible(true);
    ReturnNode->bCommentBubblePinned = true;

    ReturnNode->SetNodePosX(FunctionEntryNode->GetNodePosX() + 300);
    ReturnNode->SetNodePosY(FunctionEntryNode->GetNodePosY());
	ReturnNode->AllocateDefaultPins();

    FEdGraphPinType ReturnPinType;
    ReturnPinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
    ReturnNode->CreateUserDefinedPin("Output", ReturnPinType, EGPD_Input);

    FunctionEntryNode->Modify();
    FunctionEntryNode->GetAllPins()[0]->MakeLinkTo(ReturnNode->GetAllPins()[0]);

    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

    if (GEditor != nullptr)
    {
        UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
        if (AssetEditorSubsystem != nullptr)
        {
            AssetEditorSubsystem->OpenEditorForAsset(Blueprint);

            TSharedPtr<IToolkit> Toolkit = FToolkitManager::Get().FindEditorForAsset(Blueprint);
            if (Toolkit.IsValid())
            {
                TSharedPtr<FBlueprintEditor> BlueprintEditor = StaticCastSharedPtr<FBlueprintEditor>(Toolkit);
                if (BlueprintEditor.IsValid())
                {
                    BlueprintEditor->OpenGraphAndBringToFront(NewGraph, true);
                }
            }
        }
    }
}

void SFunctionBindingWidget::Construct(const FArguments& InArgs)
{
    CandidateFunctionNames = InArgs._CandidateFunctionNames;
    CurrentSelection = InArgs._InitialSelection;
    OnFunctionSelected = InArgs._OnFunctionSelected;
    Blueprint = InArgs._Blueprint;

    ChildSlot
        [
            SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .Padding(6.0)
                .AutoHeight()
                [
                    SNew(STextBlock)
                        .Text(LOCTEXT("FunctionSelectLabel", "Select Blueprint bool() function:"))
                ]
                + SVerticalBox::Slot()
                .Padding(6.0)
                .AutoHeight()
                [
                    SAssignNew(ComboBox, SComboBox<TSharedPtr<FString>>)
                        .OptionsSource(CandidateFunctionNames.Get())
                        .OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem)
                            {
                                return SNew(STextBlock).Text(FText::FromString(InItem.IsValid() ? *InItem : FString("None")));
                            })
                        .OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
                            {
                                if (SelectInfo != ESelectInfo::Direct)
                                {
                                    if (NewSelection.IsValid())
                                    {
                                        CurrentSelection = NewSelection;
                                        if (OnFunctionSelected.IsBound())
                                        {
                                            OnFunctionSelected.Execute(*NewSelection);
                                        }
                                    }
                                }
                            })
                        .InitiallySelectedItem(CurrentSelection)
                        [
                            SNew(STextBlock)
                                .Text_Lambda([this]()
                                    {
                                        return FText::FromString(CurrentSelection.IsValid() ? *CurrentSelection : FString(TEXT("None")));
                                    })
                        ]
                ]
                + SVerticalBox::Slot()
                    .Padding(6.0)
                    .AutoHeight()
                    [
                        SNew(SButton)
                            .Text(FText::FromString(TEXT("GenerateNewFunction")))
                            .OnClicked_Lambda([this]() -> FReply
                                {
                                    GenerateNewFunction();
                                    return FReply::Handled();
                                })
                    ]
        ];
}

#undef LOCTEXT_NAMESPACE