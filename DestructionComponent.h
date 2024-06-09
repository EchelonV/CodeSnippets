// Copyright 2024, Talos Interactive, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DestructionData.h"
#include "DestructionActor.h"
#include "GameplayTagContainer.h"
#include "Components/GameStateComponent.h"
#include "DestructionComponent.generated.h"

// This component is the core of a system that allows UE developers to implement
// destruction in their games using the Instanced Static Mesh system
// It it large data driven and centrally controls the logic reponsible for the destruction of individual ISM instances
// In its form here, the destruction is represented by a scalar value passed to each instance to then be read in its Material for example.

class ADestructionLevelScript;
class FGameplayDebuggerCategory;

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class GUNZILLATEST_API UDestructionComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UDestructionComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	~UDestructionComponent();

	//~UObject interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~End of UObject interface

	//~UActorComponent interface
	virtual void BeginPlay() override;
	//~End of UActorComponent interface

	/** Apply damage to a list of hit results */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Destruction Component")
	void ApplyDamageToHitResult(FHitResult HitResult, const float Damage);

private:
	
	/** get the destruction data set for a given destruction tag */
	UFUNCTION(BlueprintPure, Category = "Destruction Component")
	FDestructionDataSet GetDestructionDataSet(FGameplayTag DestructionTag);
	FDestructionDataSet* GetDestructionDataSetPtr(FGameplayTag DestructionTag);

	/** Apply damage to a instance */
	void ApplyDamageToInstance(FGameplayTag InstanceTag, int32 InstanceIndex, float Damage);

	/** Get the transform of a given instance index */
	UFUNCTION(BlueprintPure, Category = "Destruction Component")
	void GetInstanceTransform(FGameplayTag InstanceTag, int32 InstanceIndex, FTransform& InstanceTransform);

	// World data
	TMap<int32, float> GetDestructibleHealth() { return DestructiblesHealth; };

	float GetDestructibleHealthForIndex(FGameplayTag InstanceTag, int32 InstanceIndex) const;

	void GetDestructionDataAssets();

	void InitializeDestructibleInstances();

	TObjectPtr<ADestructionActor> SpawnNewDestructionActor(FGameplayTag InstanceTag, FDestructionDataSet* CurrentDataSet);

	void AddNewDestructionInstance(FGameplayTag InstanceTag, FDestructionDataSet* CurrentDataSet, TObjectPtr<ADestructionActor> DestructibleActor, FTransform CurrentTransform);

	UFUNCTION(NetMulticast, Reliable)
	void UpdateInstance(FGameplayTag InstanceTag, int32 InstanceIndex, float NewHealth);

	UFUNCTION(NetMulticast, Reliable)
	void DestroyInstance(FGameplayTag InstanceTag, int32 InstanceIndex, int32 GlobalInstanceIndex);

	int32 GetGlobalInstanceIndex(FGameplayTag InstanceTag, int32 InstanceIndex) const;

	// List of destruction data assets
	TArray<FAssetData> DestructionDataAssetList;

	// The list of interaction data
	UPROPERTY()
	TMap<FGameplayTag, FDestructionDataSet> DestructionDataSets;

	/** The number of all instances */
	int32 NumGlobalInstances;

	/** List of all destructibles' instanced static mesh instances */
	TMap<FGameplayTag, TObjectPtr<ADestructionActor>> DestructibleInstanceActors;

	/** List of all destructibles' indeces that is reflected in the TMaps below */
	TMap<FGameplayTag, TMap<int32, int32>> DestructiblesIndices;

	/** List of all destructibles' current total health */
	TMap<int32, float> DestructiblesHealth;

	/**
	*	List of all destructibles' transforms.
	*	Keep us from having to access the ISM comp to get them
	*/
	TMap<int32, FTransform> DestructibleInstanceTransforms;

	/** A reference to the levelscript actor, needed to read the initial destructible pieces setup data */
	TObjectPtr<ADestructionLevelScript> LevelScript;

#if WITH_GAMEPLAY_DEBUGGER_MENU
public:

	virtual void DescribeSelfToGameplayDebugger(FGameplayDebuggerCategory* DebuggerCategory);

private:

#endif //WITH_GAMEPLAY_DEBUGGER_MENU
};
