// Copyright 2024, Talos Interactive, LLC. All Rights Reserved.

#include "DestructionData.h"
#include "UObject/UObjectIterator.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DestructionData)

UDestructionData::UDestructionData()
{
	
}
// This here should validate the data in these assets. Haven't gotten around to that though
#if WITH_EDITOR
EDataValidationResult UDestructionData::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	return Result;
}
#endif

#if WITH_EDITORONLY_DATA
void UDestructionData::UpdateAssetBundleData()
{
	Super::UpdateAssetBundleData();
}
#endif // WITH_EDITORONLY_DATA
