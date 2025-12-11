/*
 * Publisher: AO
 * Year of Publication: 2025
 * Copyright AO All Rights Reserved.
 */


#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Kismet2/Breakpoint.h"
#include "Widgets/SWindow.h"

struct FBlueprintDebugExtensionConditionData;
class SConditionListWidget;

DECLARE_DELEGATE_OneParam(FOnApplyClicked, const TArray<FBlueprintDebugExtensionConditionData>&);
DECLARE_DELEGATE(FOnResetClicked);

class SBreakpointConditionWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBreakpointConditionWindow) {}
		SLATE_ARGUMENT(const UEdGraphNode*, Node)
		SLATE_ARGUMENT(UBlueprint*, Blueprint)
		SLATE_EVENT(FOnApplyClicked, OnApplyClicked)
		SLATE_EVENT(FOnResetClicked, OnResetClicked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetParentWindow(const TSharedRef<SWindow>& InWindow);

private:
	UBlueprint* Blueprint;
	const UEdGraphNode* Node;
	TSharedPtr<SConditionListWidget> ConditionListWidgetPtr;
	TSharedPtr<SWindow> WindowPtr;
	FOnApplyClicked OnApplyClickedDelegate;
	FOnResetClicked OnResetClickedDelegate;
};
