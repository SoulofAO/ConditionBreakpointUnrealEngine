/*
 * Publisher: AO
 * Year of Publication: 2025
 * Copyright AO All Rights Reserved.
 */



#include "UI/BreakpointConditionWindow.h"
#include "UI/ConditionListWidget.h"
#include "BlueprintDebugExtensionSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

void SBreakpointConditionWindow::Construct(const FArguments& InArgs)
{
	Node = InArgs._Node;
	Blueprint = InArgs._Blueprint;
	OnApplyClickedDelegate = InArgs._OnApplyClicked;
	OnResetClickedDelegate = InArgs._OnResetClicked;

	ChildSlot
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(6)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.HAlign(HAlign_Right)
						.VAlign(VAlign_Center)
						[
							SNew(SHorizontalBox)

								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.HAlign(HAlign_Right)
								.VAlign(VAlign_Center)
								.Padding(4, 0)
								[
									SNew(SButton)
										.Text(FText::FromString(TEXT("Reset")))
										.OnClicked_Lambda([this]() -> FReply
											{
												OnResetClickedDelegate.ExecuteIfBound();
												return FReply::Handled();
											})
								]

							+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.HAlign(HAlign_Right)
								.VAlign(VAlign_Center)
								.Padding(4, 0)
								[
									SNew(SButton)
										.Text(FText::FromString(TEXT("Apply")))
										.OnClicked_Lambda([this]() -> FReply
											{
												OnApplyClickedDelegate.Execute(ConditionListWidgetPtr->GetEditedConditions());
												return FReply::Handled();
											})
								]
						]
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(6)
				[
					SAssignNew(ConditionListWidgetPtr, SConditionListWidget)
						.Node(Node)
						.Blueprint(Blueprint)
				]
		];
}

void SBreakpointConditionWindow::SetParentWindow(const TSharedRef<SWindow>& InWindow)
{
	WindowPtr = InWindow;
}