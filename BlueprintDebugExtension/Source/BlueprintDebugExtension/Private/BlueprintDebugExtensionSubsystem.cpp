/*
 * Publisher: AO
 * Year of Publication: 2026
 * Copyright AO All Rights Reserved.
 */


#include "BlueprintDebugExtensionSubsystem.h"
#include "Widgets/SWindow.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "Styling/SlateTypes.h"
#include "Kismet2/KismetDebugUtilities.h"
#include "ClassViewerModule.h"
#include "Editor/EditorStyle/Public/EditorStyleSet.h"
#include "Styling/SlateColor.h"
#include "UI/BreakpointConditionWindow.h"
#include "BlueprintEditorLibrary.h"
#include "Kismet2/Breakpoint.h"
#include "Conditions/Instance/RepeatDebugCondition.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"
#include "UObject/UnrealType.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "Engine/Blueprint.h"
#include "Interfaces/IPluginManager.h"
#include "UObject/UObjectIterator.h"


static const TArray<TSharedPtr<FString>>& GetConditionTypeOptions()
{
	static TArray<TSharedPtr<FString>> Options
	{
		MakeShared<FString>(TEXT("And")),
		MakeShared<FString>(TEXT("Or"))
	};
	return Options;
}

static FGuid GetNodeGuidSafe(const UEdGraphNode* Node)
{
	if (Node != nullptr)
	{
		return Node->NodeGuid;
	}
	return FGuid();
}

void UBlueprintDebugExtensionSubsystem::OpenConditionEditorWindow(UBlueprint* Blueprint, const UEdGraphNode* Node)
{
    TSharedRef<SWindow> ConditionWindow = SNew(SWindow)
        .Title(FText::FromString(TEXT("Breakpoint Condition")))
        .ClientSize(FVector2D(600.0f, 400.0f))
        .AutoCenter(EAutoCenter::PreferredWorkArea)
        .SupportsMaximize(false)
        .SupportsMinimize(false)
        .HasCloseButton(true)
        [
            SNew(SBreakpointConditionWindow)
                .Node(Node)
                .Blueprint(Blueprint)
                .OnApplyClicked_Lambda([this, Node](const TArray<FBlueprintDebugExtensionConditionData>& EditedConditions)
                    {
                        AddNewConditions(const_cast<UEdGraphNode*>(Node), EditedConditions);
                    })
                .OnResetClicked_Lambda([this, Node]()
                    {

                    })
        ];

    FSlateApplication::Get().AddWindow(ConditionWindow);

}

void UBlueprintDebugExtensionSubsystem::AddNewConditions(UEdGraphNode* Node, TArray<FBlueprintDebugExtensionConditionData> NewConditions)
{
    if (Node == nullptr)
    {
        return;
    }

    FGuid NodeGuid = Node->NodeGuid;
    if (Conditions.Contains(NodeGuid))
    {
        Conditions.Remove(NodeGuid);
    }
    Conditions.Add(NodeGuid, FArrayBlueprintDebugExtensionConditionData(NewConditions));

    if (NewConditions.Num() > 0)
    {
        UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNode(Node);
        FBlueprintBreakpoint* BlueprintBreakpoint = FKismetDebugUtilities::FindBreakpointForNode(Node, Blueprint, false);

        if (!BlueprintBreakpoint)
        {
            UEdGraphNode* MutableNode = const_cast<UEdGraphNode*>(Node);
            FKismetDebugUtilities::CreateBreakpoint(Blueprint, MutableNode, true);
        }

        if (CheckValidConditions(Node))
        {
            Node->Modify();
            Node->NodeComment = TEXT("Has Custom Breakpoint");
            Node->SetMakeCommentBubbleVisible(true);
            Node->bCommentBubblePinned = true;
        }
        else
        {
            Node->Modify();
            Node->NodeComment = TEXT("Has Error Custom Breakpoint");
            Node->SetMakeCommentBubbleVisible(true);
            Node->bCommentBubblePinned = true;
        }
    }
    else
    {
        Node->Modify();
        Node->NodeComment = TEXT("");
        Node->SetMakeCommentBubbleVisible(false);
        Node->bCommentBubblePinned = false;
    }

}

bool UBlueprintDebugExtensionSubsystem::CheckValidConditions(UEdGraphNode* Node)
{
    if (Node == nullptr)
    {
        return false;
    }

    FGuid NodeGuid = Node->NodeGuid;
    if (!Conditions.Contains(NodeGuid))
    {
        return false;
    }

    UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNode(Node);

    FBlueprintBreakpoint* BlueprintBreakpoint = FKismetDebugUtilities::FindBreakpointForNode(Node, Blueprint, false);

    for (FBlueprintDebugExtensionConditionData ConditionData : Conditions.Find(NodeGuid)->Conditions)
    {
        if (!ConditionData.Condition->CheckValidCondition(Blueprint))
        {
            return false;
        }
    }

    return true;
}

void UBlueprintDebugExtensionSubsystem::RemoveCondition(UEdGraphNode* RemoveNode)
{
    if (RemoveNode == nullptr)
    {
        return;
    }

    FGuid NodeGuid = RemoveNode->NodeGuid;
    if (Conditions.Contains(NodeGuid))
    {
        Conditions.Remove(NodeGuid);
    }

    RemoveNode->Modify();
    RemoveNode->NodeComment = TEXT("");
    RemoveNode->SetMakeCommentBubbleVisible(false);
    RemoveNode->bCommentBubblePinned = false;
}

void UBlueprintDebugExtensionSubsystem::PlaceTriggeredOnceBreakpoint(UBlueprint* Blueprint, const UEdGraphNode* Node)
{
    FBlueprintBreakpoint* BlueprintBreakpoint = FKismetDebugUtilities::FindBreakpointForNode(Node, Blueprint, false);

    if (!BlueprintBreakpoint)
    {
        UEdGraphNode* MutableNode = const_cast<UEdGraphNode*>(Node);
        FKismetDebugUtilities::CreateBreakpoint(Blueprint, MutableNode, true);
    }

    BlueprintBreakpoint = FKismetDebugUtilities::FindBreakpointForNode(Node, Blueprint, false);

    AddNewConditions(const_cast<UEdGraphNode*>(Node),
        {
            {
            .ConditionType = EBlueprintDebugExtensionConditionType::Or,
            .Condition = NewObject<URepeatDebugCondition>()
            }
        });
}


static FString GetSaveFilePath()
{
    constexpr TCHAR PluginName[] = TEXT("BlueprintDebugExtension"); 
    TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName);

    if (Plugin.IsValid())
    {
        const FString PluginBaseDir = Plugin->GetBaseDir();
        return FPaths::Combine(PluginBaseDir, TEXT("Conditions.json"));
    }

    return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("BlueprintDebugExtension"), TEXT("Conditions.json"));
}


void UBlueprintDebugExtensionSubsystem::SaveConditions()
{
    TSharedRef<FJsonObject> RootJson = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> EntriesArray;

    for (const TPair<FGuid, FArrayBlueprintDebugExtensionConditionData >& Pair : Conditions)
    {
        FGuid NodeGuid = Pair.Key;
        const TArray<FBlueprintDebugExtensionConditionData>& ConditionArray = Pair.Value.Conditions;

        UEdGraphNode* FoundNode = nullptr;
        UBlueprint* FoundBlueprint = nullptr;

        for (TObjectIterator<UBlueprint> BlueprintIt; BlueprintIt; ++BlueprintIt)
        {
            UBlueprint* Blueprint = *BlueprintIt;
            if (Blueprint == nullptr)
            {
                continue;
            }

            TArray<UEdGraph*> AllGraphs;
            Blueprint->GetAllGraphs(AllGraphs);

            for (UEdGraph* Graph : AllGraphs)
            {
                if (Graph == nullptr)
                {
                    continue;
                }

                for (UEdGraphNode* Node : Graph->Nodes)
                {
                    if (Node != nullptr && Node->NodeGuid == NodeGuid)
                    {
                        FoundNode = Node;
                        FoundBlueprint = Blueprint;
                        break;
                    }
                }

                if (FoundNode != nullptr)
                {
                    break;
                }
            }

            if (FoundNode != nullptr)
            {
                break;
            }
        }

        if (FoundNode == nullptr || FoundBlueprint == nullptr)
        {
            continue;
        }

        UEdGraph* Graph = FoundNode->GetGraph();
        if (Graph == nullptr)
        {
            continue;
        }

        // ����� Blueprint-�������� �� ������� �������� �����
        UObject* Outer = Graph;
        UBlueprint* Blueprint = nullptr;
        while (Outer != nullptr)
        {
            Blueprint = Cast<UBlueprint>(Outer);
            if (Blueprint != nullptr)
            {
                break;
            }
            Outer = Outer->GetOuter();
        }

        if (Blueprint == nullptr)
        {
            continue;
        }

        TSharedRef<FJsonObject> EntryJson = MakeShared<FJsonObject>();
        EntryJson->SetStringField(TEXT("BlueprintPath"), FoundBlueprint->GetPathName());
        EntryJson->SetStringField(TEXT("NodeGuid"), NodeGuid.ToString());

        TArray<TSharedPtr<FJsonValue>> ConditionsJsonArray;

        for (const FBlueprintDebugExtensionConditionData& ConditionData : ConditionArray)
        {
            TSharedRef<FJsonObject> ConditionJson = MakeShared<FJsonObject>();
            ConditionJson->SetNumberField(TEXT("ConditionType"), static_cast<int32>(ConditionData.ConditionType));

            UBlueprintDebugExtensionCondition* ConditionObj = ConditionData.Condition;
            if (ConditionObj == nullptr)
            {
                ConditionJson->SetBoolField(TEXT("HasConditionObject"), false);
                ConditionsJsonArray.Add(MakeShared<FJsonValueObject>(ConditionJson));
                continue;
            }

            ConditionJson->SetBoolField(TEXT("HasConditionObject"), true);
            ConditionJson->SetStringField(TEXT("ConditionClass"), ConditionObj->GetClass()->GetPathName());

            // ����������� ������ �������� � ������ CPF_SaveGame
            TSharedRef<FJsonObject> PropsJson = MakeShared<FJsonObject>();
            for (TFieldIterator<FProperty> PropIt(ConditionObj->GetClass()); PropIt; ++PropIt)
            {
                FProperty* Property = *PropIt;

                // ���������� transient � properties ��� SaveGame
                if (Property->HasAllPropertyFlags(CPF_Transient))
                {
                    continue;
                }

                if (!Property->HasAnyPropertyFlags(CPF_SaveGame))
                {
                    continue;
                }

                FString ExportedText;
                Property->ExportText_InContainer(0, ExportedText, ConditionObj, ConditionObj, ConditionObj, PPF_None);

                PropsJson->SetStringField(Property->GetName(), ExportedText);
            }

            ConditionJson->SetObjectField(TEXT("Properties"), PropsJson);
            ConditionsJsonArray.Add(MakeShared<FJsonValueObject>(ConditionJson));
        }

        EntryJson->SetArrayField(TEXT("Conditions"), ConditionsJsonArray);
        EntriesArray.Add(MakeShared<FJsonValueObject>(EntryJson));
    }

    RootJson->SetArrayField(TEXT("Entries"), EntriesArray);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    if (FJsonSerializer::Serialize(RootJson, Writer))
    {
        const FString SavePath = GetSaveFilePath();
        const FString SaveDir = FPaths::GetPath(SavePath);

        IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
        if (!PlatformFile.DirectoryExists(*SaveDir))
        {
            PlatformFile.CreateDirectoryTree(*SaveDir);
        }

        if (!FFileHelper::SaveStringToFile(OutputString, *SavePath))
        {
            UE_LOG(LogTemp, Warning, TEXT("BlueprintDebugExtension: Failed to write conditions to %s"), *SavePath);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BlueprintDebugExtension: Failed to serialize conditions JSON."));
    }
}

void UBlueprintDebugExtensionSubsystem::LoadConditions()
{
    const FString SavePath = GetSaveFilePath();
    if (!FPaths::FileExists(SavePath))
    {
        return;
    }

    FString InputString;
    if (!FFileHelper::LoadFileToString(InputString, *SavePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("BlueprintDebugExtension: Failed to read conditions file %s"), *SavePath);
        return;
    }

    TSharedPtr<FJsonObject> RootJson;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InputString);
    if (!FJsonSerializer::Deserialize(Reader, RootJson) || RootJson == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("BlueprintDebugExtension: Failed to parse conditions JSON."));
        return;
    }

    Conditions.Empty();

    const TArray<TSharedPtr<FJsonValue>>* EntriesArray = nullptr;
    if (!RootJson->TryGetArrayField(TEXT("Entries"), EntriesArray))
    {
        return;
    }

    for (const TSharedPtr<FJsonValue>& EntryVal : *EntriesArray)
    {
        if (!EntryVal.IsValid())
        {
            continue;
        }

        const TSharedPtr<FJsonObject>* EntryObjPtr = nullptr;
        if (!EntryVal->TryGetObject(EntryObjPtr) || EntryObjPtr == nullptr)
        {
            continue;
        }

        const TSharedPtr<FJsonObject>& EntryObj = *EntryObjPtr;

        FString BlueprintPath;
        if (!EntryObj->TryGetStringField(TEXT("BlueprintPath"), BlueprintPath))
        {
            continue;
        }

        FString NodeGuidString;
        if (!EntryObj->TryGetStringField(TEXT("NodeGuid"), NodeGuidString))
        {
            continue;
        }

        FGuid NodeGuid;
        if (!FGuid::Parse(NodeGuidString, NodeGuid))
        {
            continue;
        }

        UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *BlueprintPath);
        if (Blueprint == nullptr)
        {
            UE_LOG(LogTemp, Warning, TEXT("BlueprintDebugExtension: Could not load blueprint %s"), *BlueprintPath);
            continue;
        }

        // ����� ���� �� NodeGuid �� ���� ������
        UEdGraphNode* FoundNode = nullptr;
        TArray<UEdGraph*> AllGraphs;
        Blueprint->GetAllGraphs(AllGraphs);

        for (UEdGraph* Graph : AllGraphs)
        {
            if (Graph == nullptr)
            {
                continue;
            }

            for (UEdGraphNode* Node : Graph->Nodes)
            {
                if (Node == nullptr)
                {
                    continue;
                }

                if (Node->NodeGuid == NodeGuid)
                {
                    FoundNode = Node;
                    break;
                }
            }

            if (FoundNode != nullptr)
            {
                break;
            }
        }

        if (FoundNode == nullptr)
        {
            // ���� �� ������ � ��������, ��� �����/������������
            continue;
        }

        TArray<FBlueprintDebugExtensionConditionData> LoadedConditionArray;

        const TArray<TSharedPtr<FJsonValue>>* ConditionsJsonArray = nullptr;
        if (!EntryObj->TryGetArrayField(TEXT("Conditions"), ConditionsJsonArray))
        {
            continue;
        }

        for (const TSharedPtr<FJsonValue>& CondVal : *ConditionsJsonArray)
        {
            if (!CondVal.IsValid())
            {
                continue;
            }

            const TSharedPtr<FJsonObject>* CondObjPtr = nullptr;
            if (!CondVal->TryGetObject(CondObjPtr) || CondObjPtr == nullptr)
            {
                continue;
            }

            const TSharedPtr<FJsonObject>& CondObj = *CondObjPtr;

            FBlueprintDebugExtensionConditionData DataItem;

            int32 ConditionTypeInt = 0;
            CondObj->TryGetNumberField(TEXT("ConditionType"), ConditionTypeInt);
            DataItem.ConditionType = static_cast<EBlueprintDebugExtensionConditionType>(ConditionTypeInt);


            bool bHasConditionObject = false;
            CondObj->TryGetBoolField(TEXT("HasConditionObject"), bHasConditionObject);

            if (!bHasConditionObject)
            {
                DataItem.Condition = nullptr;
                LoadedConditionArray.Add(DataItem);
                continue;
            }

            FString ConditionClassPath;
            if (!CondObj->TryGetStringField(TEXT("ConditionClass"), ConditionClassPath))
            {
                DataItem.Condition = nullptr;
                LoadedConditionArray.Add(DataItem);
                continue;
            }

            UClass* ConditionClass = LoadObject<UClass>(nullptr, *ConditionClassPath);
            if (ConditionClass == nullptr)
            {
                ConditionClass = FindFirstObjectSafe<UClass>(*ConditionClassPath);
                if (ConditionClass == nullptr)
                {
                    UE_LOG(LogTemp, Warning, TEXT("BlueprintDebugExtension: Could not load condition class %s"), *ConditionClassPath);
                    DataItem.Condition = nullptr;
                    LoadedConditionArray.Add(DataItem);
                    continue;
                }
            }

            // ������ ��������� ������� (RF_Transient, Outer = this subsystem)
            UBlueprintDebugExtensionCondition* NewCondition = NewObject<UBlueprintDebugExtensionCondition>(this, ConditionClass);
            if (NewCondition == nullptr)
            {
                DataItem.Condition = nullptr;
                LoadedConditionArray.Add(DataItem);
                continue;
            }

            // ��������������� ������ �������� � CPF_SaveGame
            const TSharedPtr<FJsonObject>* PropsJsonPtr = nullptr;
            if (CondObj->TryGetObjectField(TEXT("Properties"), PropsJsonPtr) && PropsJsonPtr != nullptr)
            {
                const TSharedPtr<FJsonObject>& PropsJson = *PropsJsonPtr;

                for (TFieldIterator<FProperty> PropIt(NewCondition->GetClass()); PropIt; ++PropIt)
                {
                    FProperty* Property = *PropIt;

                    if (Property->HasAllPropertyFlags(CPF_Transient))
                    {
                        continue;
                    }

                    if (!Property->HasAnyPropertyFlags(CPF_SaveGame))
                    {
                        continue;
                    }

                    FString PropName = Property->GetName();
                    FString SavedTextValue;
                    if (!PropsJson->TryGetStringField(PropName, SavedTextValue))
                    {
                        continue;
                    }

                    void* ValuePtr = Property->ContainerPtrToValuePtr<void>(NewCondition);
                    Property->ImportText_Direct(*SavedTextValue, ValuePtr, NewCondition, PPF_None);
                }
            }

            DataItem.Condition = NewCondition;
            LoadedConditionArray.Add(DataItem);
        }

        Conditions.Add(NodeGuid, LoadedConditionArray);
    }
}



void UBlueprintDebugExtensionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UBlueprintDebugExtensionSubsystem::Tick) ,0.3f);
    FEditorDelegates::BeginPIE.AddUObject(this, &UBlueprintDebugExtensionSubsystem::OnBeginPIE);
    LoadConditions();
}

void UBlueprintDebugExtensionSubsystem::OnBeginPIE(bool bIsSimulating)
{
    for (TPair<FGuid, FArrayBlueprintDebugExtensionConditionData>& Pair : Conditions)
    {
        Pair.Value.Context = NewObject<UExecBlueprintBreakpointContext>(this);
    }
}

void UBlueprintDebugExtensionSubsystem::Deinitialize()
{
    SaveConditions();
}

bool UBlueprintDebugExtensionSubsystem::Tick(float DeltaTime)
{
    TArray<FGuid> ConditionsToRemove;

    for (const TPair<FGuid, FArrayBlueprintDebugExtensionConditionData>& Pair : Conditions)
    {
        FGuid NodeGuid = Pair.Key;

        UEdGraphNode* FoundNode = nullptr;
        UBlueprint* OwnerBlueprint = nullptr;

        for (TObjectIterator<UBlueprint> BlueprintIt; BlueprintIt; ++BlueprintIt)
        {
            UBlueprint* Blueprint = *BlueprintIt;
            if (Blueprint == nullptr)
            {
                continue;
            }

            TArray<UEdGraph*> AllGraphs;
            Blueprint->GetAllGraphs(AllGraphs);

            for (UEdGraph* Graph : AllGraphs)
            {
                if (Graph == nullptr)
                {
                    continue;
                }

                for (UEdGraphNode* Node : Graph->Nodes)
                {
                    if (Node != nullptr && Node->NodeGuid == NodeGuid)
                    {
                        FoundNode = Node;
                        OwnerBlueprint = Blueprint;
                        break;
                    }
                }

                if (FoundNode != nullptr)
                {
                    break;
                }
            }

            if (FoundNode != nullptr)
            {
                break;
            }
        }

        if (FoundNode == nullptr)
        {
            ConditionsToRemove.Add(NodeGuid);
            continue;
        }

        FBlueprintBreakpoint* BlueprintBreakpoint = FKismetDebugUtilities::FindBreakpointForNode(FoundNode, OwnerBlueprint, false);
        if (!BlueprintBreakpoint)
        {
            ConditionsToRemove.Add(NodeGuid);
            continue;
        }
    }

    for (const FGuid& NodeGuidToRemove : ConditionsToRemove)
    {
        Conditions.Remove(NodeGuidToRemove);
    }
    return true;
}

bool UBlueprintDebugExtensionSubsystem::CheckCondition(const UObject* ActiveObject, const FFrame& StackFrame)
{
    uint8 Code = StackFrame.PeekCode();

    UFunction* Function = StackFrame.Node;
    int32 CodeOffset = StackFrame.Code - StackFrame.Node->Script.GetData();
    UBlueprintGeneratedClass* BGClass = Cast<UBlueprintGeneratedClass>(StackFrame.Object->GetClass());
    bool bDoesClassHaveBlueprint = true;
    UBlueprint* EditorBlueprint = UBlueprintEditorLibrary::GetBlueprintForClass(ActiveObject->GetClass(), bDoesClassHaveBlueprint);

    UEdGraphNode* CurrentNode = BGClass->DebugData.FindSourceNodeFromCodeLocation(Function, CodeOffset, true);

    if (CurrentNode == nullptr)
    {
        return true;
    }

    FGuid NodeGuid = CurrentNode->NodeGuid;
    if (!Conditions.Contains(NodeGuid))
    {
        return true;
    }

    FArrayBlueprintDebugExtensionConditionData* ConditionData = Conditions.Find(NodeGuid);
    if (ConditionData == nullptr)
    {
        return true;
    }

    ConditionData->Context->BreakpointExecuteCount += 1;
    TArray<FBlueprintDebugExtensionConditionData> ConditionsForBreakpoint = ConditionData->Conditions;

    if (ConditionsForBreakpoint.Num() <= 0)
    {
        return true;
    }

    if(!CheckValidConditions(CurrentNode))
    {
        return false;
    }

    bool bResult = false;
    int Count = 0;

    for (FBlueprintDebugExtensionConditionData Condition : ConditionsForBreakpoint)
    {
        if (Count == 0)
        {
            bResult = Condition.Condition->CheckCondition(ActiveObject, StackFrame, ConditionData->Context);
        }
        else
        {
            if (Condition.ConditionType == EBlueprintDebugExtensionConditionType::Or)
            {
                bResult = bResult || Condition.Condition->CheckCondition(ActiveObject, StackFrame, ConditionData->Context);
            }
            else if (Condition.ConditionType == EBlueprintDebugExtensionConditionType::And)
            {
                bResult = bResult && Condition.Condition->CheckCondition(ActiveObject, StackFrame, ConditionData->Context);
            }
        }
        Count++;
    }
	return bResult;
}