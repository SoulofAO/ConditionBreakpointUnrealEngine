/*
 * Publisher: AO
 * Year of Publication: 2026
 * Copyright AO All Rights Reserved.
 */


#include "ExecBlueprintBreakpointContext.h"


UCustomExtenderContext* UExecBlueprintBreakpointContext::GetNewGlobalCustomExtender(TSubclassOf<UCustomExtenderContext> Class)
{
    for (UCustomExtenderContext* CustomExtender : GlobalCustomExtenders)
    {
        if (CustomExtender->GetClass() == Class)
        {
            return CustomExtender;
        }
    }

    UCustomExtenderContext* CustomExtenderContext = NewObject<UCustomExtenderContext>(this, Class);
    GlobalCustomExtenders.Add(CustomExtenderContext);
    return CustomExtenderContext;
}

UCustomExtenderContext* UExecBlueprintBreakpointContext::GetNewGlobalCustomExtenderByObject(UObject* Object, TSubclassOf<UCustomExtenderContext> Class)
{
    if (!CustomExtendersForObject.Contains(Object))
    {
        CustomExtendersForObject.Add(FCustomExtenderData(Object));
    }
    FCustomExtenderData* CustomExtenderData = CustomExtendersForObject.FindByKey(Object);

    for (UCustomExtenderContext* CustomExtender : CustomExtenderData->CustomExtenders)
    {
        if (CustomExtender->GetClass() == Class)
        {
            return CustomExtender;
        }
    }
	UCustomExtenderContext* CustomExtenderContext = NewObject<UCustomExtenderContext>(this, Class);
	CustomExtenderData->CustomExtenders.Add(CustomExtenderContext);
    return CustomExtenderContext;
}
