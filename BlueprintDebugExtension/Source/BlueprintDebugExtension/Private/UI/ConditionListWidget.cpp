/*
 * Publisher: AO
 * Year of Publication: 2025
 * Copyright AO All Rights Reserved.
 */



#include "UI/ConditionListWidget.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "ClassViewerModule.h"
#include "Editor/EditorStyle/Public/EditorStyleSet.h"
#include "UObject/UObjectGlobals.h"
#include "Framework/Application/SlateApplication.h"
#if WITH_EDITOR
#include "Editor.h"
#endif

void SConditionListWidget::Construct(const FArguments& InArgs)
{
	Node = InArgs._Node;
	Blueprint = InArgs._Blueprint;

	EditingConditions.Empty();

#if WITH_EDITOR
	if (GEditor != nullptr)
	{
		UBlueprintDebugExtensionSubsystem* Subsystem = GEditor->GetEditorSubsystem<UBlueprintDebugExtensionSubsystem>();

		if (Subsystem != nullptr)
		{
			if (Subsystem->Conditions.Contains(Node))
			{
				TArray<FBlueprintDebugExtensionConditionData> Found = Subsystem->Conditions.Find(Node)->Conditions;
				if (!Found.IsEmpty())
				{
					for (const FBlueprintDebugExtensionConditionData& Source : Found)
					{
						if (IsValid(Source.Condition))
						{
							FBlueprintDebugExtensionConditionData Copy;
							Copy.ConditionType = Source.ConditionType;
							Copy.Condition = Source.Condition ? DuplicateObject<UBlueprintDebugExtensionCondition>(Source.Condition, Subsystem) : nullptr;
							EditingConditions.Add(MoveTemp(Copy));
						}
					}
				}
			}
		}
	}
#endif

	ChildSlot
		[
			SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(4)
				[
					SNew(SBorder)
						.BorderBackgroundColor(FSlateColor(FLinearColor(0.06f, 0.06f, 0.06f, 1.0f)))
						.Padding(6)
						[
							SNew(SScrollBox)
								+ SScrollBox::Slot()
								[
									SAssignNew(ConditionsContainer, SVerticalBox)
								]
						]
				]

			+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(6)
				[
					SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SButton)
								.Text(FText::FromString(TEXT("Add Condition")))
								.OnClicked_Lambda([this]() -> FReply
									{
										FClassViewerInitializationOptions InitOptions;
										InitOptions.Mode = EClassViewerMode::ClassPicker;
										InitOptions.DisplayMode = EClassViewerDisplayMode::TreeView;
										InitOptions.bShowObjectRootClass = false;
										InitOptions.bExpandAllNodes = true;
										InitOptions.bShowDefaultClasses = false;

										TSharedRef<FConditionClassFilter> ConditionFilter = MakeShareable(new FConditionClassFilter());
										InitOptions.ClassFilters = { ConditionFilter };

										TSharedRef<SWindow> PickerWindow = SNew(SWindow)
											.Title(FText::FromString(TEXT("Pick Condition Class")))
											.ClientSize(FVector2D(640, 480))
											.SupportsMinimize(false)
											.SupportsMaximize(false);

										FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
										TSharedRef<SWidget> ClassViewer = ClassViewerModule.CreateClassViewer(InitOptions, FOnClassPicked::CreateLambda([this, PickerWindow](UClass* ChosenClass)
											{
												if (ChosenClass != nullptr && ChosenClass->IsChildOf(UBlueprintDebugExtensionCondition::StaticClass()))
												{
													AddNewConditionOfClass(ChosenClass);
													RebuildConditionsList();
												}
												FSlateApplication::Get().RequestDestroyWindow(PickerWindow);
											}));

										PickerWindow->SetContent(SNew(SBox).WidthOverride(640).HeightOverride(480)[ClassViewer]);
										FSlateApplication::Get().AddWindow(PickerWindow);
										return FReply::Handled();
									})
						]
				]
		];

	RebuildConditionsList();
}

TArray<FBlueprintDebugExtensionConditionData> SConditionListWidget::GetEditedConditions() const
{
	return EditingConditions;
}

void SConditionListWidget::RebuildConditionsList()
{
	if (!ConditionsContainer.IsValid())
	{
		return;
	}

	ConditionsContainer->ClearChildren();

	for (int32 Index = 0; Index < EditingConditions.Num(); ++Index)
	{
		FBlueprintDebugExtensionConditionData& Entry = EditingConditions[Index];

		TSharedPtr<SWidget> ConfigWidget = SNullWidget::NullWidget;
		if (Entry.Condition != nullptr)
		{
			ConfigWidget = Entry.Condition->InitializationWidget(Blueprint);
			if (!ConfigWidget.IsValid())
			{
				ConfigWidget = SNew(STextBlock).Text(FText::FromString(TEXT("No configuration widget")));
			}
		}
		else
		{
			ConfigWidget = SNew(STextBlock).Text(FText::FromString(TEXT("Condition instance is null")));
		}

		ConditionsContainer->AddSlot()
			.AutoHeight()
			.Padding(6)
			[
				SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
					.Padding(8)
					[
						SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.Padding(4, 4)
							.FillHeight(1.0)
							[
								SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.VAlign(VAlign_Center)

									[
										SNew(STextBlock)
											.Visibility_Lambda([this, Index]()-> EVisibility
												{
													return Index == 0 ? EVisibility::Hidden : EVisibility::Visible;
												})
											.Text(FText::FromString(TEXT("Combine:")))
									]

								+ SHorizontalBox::Slot()
									.AutoWidth()
									.Padding(8, 0)
									[
										SNew(SButton)
											.Visibility_Lambda([this, Index]()-> EVisibility
												{
													return Index == 0 ? EVisibility::Hidden : EVisibility::Visible;
												})
											.Text_Lambda([this, Index]() -> FText
												{
													if (EditingConditions.IsValidIndex(Index) && EditingConditions[Index].ConditionType == EBlueprintDebugExtensionConditionType::And)
													{
														return FText::FromString(TEXT("And"));
													}
													return FText::FromString(TEXT("Or"));
												})
											.OnClicked_Lambda([this, Index]() -> FReply
												{
													ToggleConditionTypeAt(Index);
													RebuildConditionsList();
													return FReply::Handled();
												})
									]
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.Padding(4, 0)
									[
										SNew(SButton)
											.Text(FText::FromString(TEXT("Change Class")))
											.OnClicked_Lambda([this, Index]() -> FReply
												{
													FClassViewerInitializationOptions InitOptions;
													InitOptions.Mode = EClassViewerMode::ClassPicker;
													InitOptions.DisplayMode = EClassViewerDisplayMode::TreeView;
													InitOptions.bShowObjectRootClass = false;
													InitOptions.bExpandAllNodes = true;
													InitOptions.bShowDefaultClasses = false;

													TSharedRef<FConditionClassFilter> ConditionFilter = MakeShareable(new FConditionClassFilter());
													InitOptions.ClassFilters = { ConditionFilter };


													TSharedRef<SWindow> PickerWindow = SNew(SWindow)
														.Title(FText::FromString(TEXT("Pick Condition Class")))
														.ClientSize(FVector2D(640, 480))
														.SupportsMinimize(false)
														.SupportsMaximize(false);

													FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
													TSharedRef<SWidget> ClassViewer = ClassViewerModule.CreateClassViewer(InitOptions, FOnClassPicked::CreateLambda([this, PickerWindow, Index](UClass* ChosenClass)
														{
															if (ChosenClass != nullptr && ChosenClass->IsChildOf(UBlueprintDebugExtensionCondition::StaticClass()))
															{
																if (EditingConditions.IsValidIndex(Index))
																{
																	UBlueprintDebugExtensionSubsystem* BlueprintDebugExtensionSubsystem = GEditor->GetEditorSubsystem<UBlueprintDebugExtensionSubsystem>();
																	EditingConditions[Index].Condition = NewObject<UBlueprintDebugExtensionCondition>(BlueprintDebugExtensionSubsystem, ChosenClass);
																}
															}
															FSlateApplication::Get().RequestDestroyWindow(PickerWindow);
															RebuildConditionsList();
														}));

													PickerWindow->SetContent(ClassViewer);
													FSlateApplication::Get().AddWindow(PickerWindow);

													return FReply::Handled();
												})

									]

								+ SHorizontalBox::Slot()
									.AutoWidth()
									.Padding(4, 0)
									[
										SNew(SButton)
											.Text(FText::FromString(TEXT("Remove")))
											.OnClicked_Lambda([this, Index]() -> FReply
												{
													RemoveConditionAt(Index);
													RebuildConditionsList();
													return FReply::Handled();
												})
									]
							]

						+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(6)
							[
								SNew(SBorder)
									.Padding(6)
									.BorderBackgroundColor(FSlateColor(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f)))
									[
										ConfigWidget.ToSharedRef()
									]
							]
					]
			];
	}
}

void SConditionListWidget::AddNewConditionOfClass(UClass* ChosenClass)
{
	if (ChosenClass == nullptr || !ChosenClass->IsChildOf(UBlueprintDebugExtensionCondition::StaticClass()))
	{
		return;
	}
	UBlueprintDebugExtensionSubsystem* BlueprintDebugExtensionSubsystem = GEditor->GetEditorSubsystem<UBlueprintDebugExtensionSubsystem>();

	FBlueprintDebugExtensionConditionData NewEntry;

	NewEntry.ConditionType = EBlueprintDebugExtensionConditionType::And;
	NewEntry.Condition = NewObject<UBlueprintDebugExtensionCondition>(GetTransientPackage(), ChosenClass);
	EditingConditions.Add(MoveTemp(NewEntry));
}

void SConditionListWidget::RemoveConditionAt(int32 Index)
{
	if (EditingConditions.IsValidIndex(Index))
	{
		EditingConditions.RemoveAt(Index);
	}
}

void SConditionListWidget::ToggleConditionTypeAt(int32 Index)
{
	if (EditingConditions.IsValidIndex(Index))
	{
		FBlueprintDebugExtensionConditionData& Entry = EditingConditions[Index];
		if (Entry.ConditionType == EBlueprintDebugExtensionConditionType::And)
		{
			Entry.ConditionType = EBlueprintDebugExtensionConditionType::Or;
		}
		else
		{
			Entry.ConditionType = EBlueprintDebugExtensionConditionType::And;
		}
	}
}
