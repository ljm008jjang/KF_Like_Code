// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CSVCustomDataTable.generated.h"

/**
 * 
 */
UCLASS(MinimalAPI, BlueprintType, AutoExpandCategories = "UCSVCustomDataTable,ImportOptions", Meta = (LoadBehavior = "LazyOnDemand"))
class UCSVCustomDataTable : public UDataTable
{
	GENERATED_BODY()

protected:
	void SaveStructData(FStructuredArchiveSlot Slot);
	
};
