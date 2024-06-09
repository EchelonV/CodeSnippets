// Copyright 2024, Talos Interactive, LLC. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "Engine/StaticMesh.h"
#include "NativeGameplayTags.h"
#include "DestructionData.generated.h"

/** Parameter struct to initialize objectives. */
USTRUCT(BlueprintType)
struct GUNZILLATEST_API FDestructionDataSet
{
	GENERATED_BODY()

	// The geometry used for this destructible piece when fully intact
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMesh> Mesh;

	// The amount of hit points a certain destruction piece can take before being fully destroyed
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Health = 1000;

	// The color to use per each health state
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UCurveLinearColor* HealthStateColorCurve;
};

UCLASS(BlueprintType, Meta = (DisplayName = "Destruction Data", ShortTooltip = "Data asset containing all relevant data for initializing the destruction assets."))
class GUNZILLATEST_API UDestructionData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

	UDestructionData();
	
	//~UObject interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~End of UObject interface

	//~UPrimaryDataAsset interface
#if WITH_EDITORONLY_DATA
	virtual void UpdateAssetBundleData() override;
#endif
	//~End of UPrimaryDataAsset interface

	// The destruction sets that coming with this building piece
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, FDestructionDataSet> DestructionDataSets;
};
