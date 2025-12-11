/*
 * Publisher: AO
 * Year of Publication: 2025
 * Copyright AO All Rights Reserved.
 */


#pragma once

#include "CoreMinimal.h"
#include "Conditions/Abstract/BlueprintDebugExtensionCondition.h"
#include "BaseBlueprintDebugCondition.generated.h"

/**
 * 
 */

class UExecBlueprintBreakpointContext;
class IDetailsView;

UCLASS(Abstract, Blueprintable)
class BLUEPRINTDEBUGEXTENSION_API UBaseBlueprintDebugCondition : public UBlueprintDebugExtensionCondition
{
    GENERATED_BODY()

public:
    virtual bool CheckCondition(const UObject* ActiveObject, const FFrame& StackFrame, UExecBlueprintBreakpointContext* ExecBlueprintPointContext) override;

    virtual bool CheckValidCondition(UBlueprint* Blueprint) const override;

    virtual TSharedPtr<SWidget> InitializationWidget(UBlueprint* Blueprint) override;

protected:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "BlueprintDebug|Condition")
    bool CheckConditionBlueprint(UObject* ActiveObject, UExecBlueprintBreakpointContext* ExecBlueprintPointContext);
	virtual bool CheckConditionBlueprint_Implementation(UObject* ActiveObject, UExecBlueprintBreakpointContext* ExecBlueprintPointContext);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "BlueprintDebug|Condition")
    bool CheckValidConditionBlueprint(UBlueprint* Blueprint) const;
	virtual bool CheckValidConditionBlueprint_Implementation(UBlueprint* Blueprint) const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "BlueprintDebug|Condition")
    UWidget* InitializationWidgetBlueprint(UBlueprint* Blueprint);
	virtual UWidget* InitializationWidgetBlueprint_Implementation(UBlueprint* Blueprint);

private:
    TSharedRef<SWidget> CreateBlueprintDetailPanel(UBlueprint* Blueprint);

    void OnCompile(class UBlueprint* Blueprint);
};