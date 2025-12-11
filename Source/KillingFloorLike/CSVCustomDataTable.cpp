// Fill out your copyright notice in the Description page of Project Settings.


#include "CSVCustomDataTable.h"

void UCSVCustomDataTable::SaveStructData(FStructuredArchiveSlot Slot)
{
	UScriptStruct* SaveUsingStruct = RowStruct;
	if (IsValid(SaveUsingStruct) == false)
	{
		if (!HasAnyFlags(RF_ClassDefaultObject) && GetOutermost() != GetTransientPackage())
		{
			UE_LOG(LogDataTable, Error, TEXT("Missing RowStruct while saving DataTable '%s', NeedLoad: '%s'!"),
				   *GetPathName(), HasAnyFlags(RF_NeedLoad) ? TEXT("true") : TEXT("false"));
		}
		SaveUsingStruct = FTableRowBase::StaticStruct();
	}

	int32 NumRows = RowMap.Num();
	FStructuredArchiveArray Array = Slot.EnterArray(NumRows);

	// Now iterate over rows in the map
	for (auto RowIt = RowMap.CreateIterator(); RowIt; ++RowIt)
	{
		// Save out name
		FName RowName = RowIt.Key();
		FStructuredArchiveRecord Row = Array.EnterElement().EnterRecord();
		Row << SA_VALUE(TEXT("Name"), RowName);

		// Save out data
		uint8* RowData = RowIt.Value();

		SaveUsingStruct->SerializeItem(Row.EnterField(TEXT("Value")), RowData, nullptr);
	}
}
