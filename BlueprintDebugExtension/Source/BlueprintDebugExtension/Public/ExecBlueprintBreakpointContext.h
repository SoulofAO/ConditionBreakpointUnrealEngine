/*
 * Publisher: AO
 * Year of Publication: 2025
 * Copyright AO All Rights Reserved.
 */


#pragma once

#include "CoreMinimal.h"
#include "ExecBlueprintBreakpointContext.generated.h"


UCLASS(Blueprintable)
class UCustomExtenderContext : public UObject
{
	GENERATED_BODY()

public:
};

USTRUCT(BlueprintType)
struct FCustomExtenderData
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<UObject> ObjectWeakPtr;

	UPROPERTY()
	TArray<UCustomExtenderContext*> CustomExtenders;

	FCustomExtenderData() = default;

	FCustomExtenderData(UObject* InObject, const TArray<UCustomExtenderContext*>& InCustomExtenders)
	{
		CustomExtenders = InCustomExtenders;
		ObjectWeakPtr = InObject;
	}

	FCustomExtenderData(UObject* InObject)
	{
		ObjectWeakPtr = InObject;
	}

	bool operator==(const UObject* Other) const
	{
		return ObjectWeakPtr == Other;
	}
};


UCLASS(Blueprintable)
class UExecBlueprintBreakpointContext : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "Class"), Category = "CustomExtender")
	UCustomExtenderContext* GetNewGlobalCustomExtender(TSubclassOf<UCustomExtenderContext> Class);

	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "Class"), Category = "CustomExtender")
	UCustomExtenderContext* GetNewGlobalCustomExtenderByObject(UObject* Object, TSubclassOf<UCustomExtenderContext> Class);

	UPROPERTY()
	TArray<UCustomExtenderContext*> GlobalCustomExtenders;

	UPROPERTY()
	TArray<FCustomExtenderData> CustomExtendersForObject;

	UPROPERTY()
	int BreakpointExecuteCount = 0;
};