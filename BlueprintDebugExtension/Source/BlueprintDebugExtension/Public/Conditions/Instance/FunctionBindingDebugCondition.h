/*
 * Publisher: AO
 * Year of Publication: 2025
 * Copyright AO All Rights Reserved.
 */



#pragma once

#include "CoreMinimal.h"
#include "Conditions/Abstract/BlueprintDebugExtensionCondition.h"
#include "FunctionBindingDebugCondition.generated.h"

/**
 * 
 */
UCLASS(NotBlueprintable)
class BLUEPRINTDEBUGEXTENSION_API UFunctionBindingDebugCondition : public UBlueprintDebugExtensionCondition
{
	GENERATED_BODY()
	
public:
    virtual bool CheckCondition(const UObject* ActiveObject, const FFrame& StackFrame, UExecBlueprintBreakpointContext* ExecBlueprintPointContext) override;

    virtual bool CheckValidCondition(UBlueprint* Blueprint) const override;

    virtual TSharedPtr<SWidget> InitializationWidget(UBlueprint* Blueprint) override;

    FBoolProperty* FindBoolOutParameter(UFunction* Function);

    UPROPERTY(SaveGame)
    FString FunctionName;
};


DECLARE_DELEGATE_OneParam(FOnFunctionSelected, const FString&);

class SFunctionBindingWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SFunctionBindingWidget) {}
        SLATE_ARGUMENT(UBlueprint*, Blueprint)
        SLATE_ARGUMENT(TSharedPtr<TArray<TSharedPtr<FString>>>, CandidateFunctionNames)
        SLATE_ARGUMENT(TSharedPtr<FString>, InitialSelection)
        SLATE_EVENT(FOnFunctionSelected, OnFunctionSelected)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void GenerateNewFunction();

private:
    UBlueprint* Blueprint;
    TSharedPtr<SComboBox<TSharedPtr<FString>>> ComboBox;
    TSharedPtr<TArray<TSharedPtr<FString>>> CandidateFunctionNames;
    TSharedPtr<FString> CurrentSelection;
    FOnFunctionSelected OnFunctionSelected;
};
