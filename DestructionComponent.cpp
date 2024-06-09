// Copyright 2024, Talos Interactive, LLC. All Rights Reserved.

#include "DestructionComponent.h"
#include "DestructionData.h"
#include "DestructionActor.h"
#include "DestructionLevelScript.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "Engine/AssetManager.h"
#include "GameplayTags.h"
#include "Curves/CurveLinearColor.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

//#pragma optimize("", off)
//#pragma inline_depth(0)

#include UE_INLINE_GENERATED_CPP_BY_NAME(DestructionComponent)

DECLARE_LOG_CATEGORY_EXTERN(LogDestruction, Log, All);
DEFINE_LOG_CATEGORY(LogDestruction);

#if WITH_GAMEPLAY_DEBUGGER_MENU
#include "GameplayDebuggerTypes.h"
#include "GameplayDebuggerCategory.h"
#endif // WITH_GAMEPLAY_DEBUGGER_MENU

UDestructionComponent::UDestructionComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	NumGlobalInstances = INDEX_NONE;
}

UDestructionComponent::~UDestructionComponent()
{
}


#if WITH_EDITOR
EDataValidationResult UDestructionComponent::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	return Result;
}
#endif

void UDestructionComponent::BeginPlay()
{
	Super::BeginPlay();

	GetDestructionDataAssets();
	InitializeDestructibleInstances();
}

void UDestructionComponent::GetDestructionDataAssets()
{
	UAssetManager& AssetManager = UAssetManager::Get();
	AssetManager.GetAssetRegistry().GetAssetsByClass(UDestructionData::StaticClass()->GetClassPathName(), DestructionDataAssetList, true);

	// Fill up the interaction map
	for (FAssetData AssetData : DestructionDataAssetList)
	{
		if (AssetData.IsValid())
		{
			UDestructionData* DestructionData = Cast<UDestructionData>(AssetData.GetAsset());
			if (DestructionData != nullptr)
			{
				DestructionDataSets.Append(DestructionData->DestructionDataSets);
			}
		}
	}
}

void UDestructionComponent::InitializeDestructibleInstances()
{
	if (GetWorld() != nullptr)
	{
		LevelScript = Cast<ADestructionLevelScript>(GetWorld()->GetLevelScriptActor());

		if (DestructionDataSets.Num() > 0 && LevelScript != nullptr)
		{
			TArray<FGameplayTag> DestructibleTags = LevelScript->GetDestuctibleTags();
			TArray<FTransform> DestructibleTransforms = LevelScript->GetDestructibleTransforms();

			//for (FGameplayTag Tag : DestructibleTags)
			for (int32 i = 0; i < DestructibleTags.Num(); i++)
			{
				FGameplayTag InstanceTag = DestructibleTags[i];
				const FTransform CurrentTransform = DestructibleTransforms[i];
				FDestructionDataSet* CurrentDataSet = GetDestructionDataSetPtr(InstanceTag);

				if (CurrentDataSet != nullptr)
				{
					TObjectPtr<ADestructionActor> DestructibleActor = DestructibleInstanceActors.Find(InstanceTag) != nullptr ? *DestructibleInstanceActors.Find(InstanceTag) : nullptr;

					// Spawn an destructible instance actor for this dataset if there doesn't exist one already
					if (DestructibleActor.Get() == nullptr)
					{
						// Set the spawn parameters, such as the spawn location and rotation
						DestructibleActor = SpawnNewDestructionActor(InstanceTag, CurrentDataSet);
					}

					if (DestructibleActor.Get() != nullptr)
					{
						// Add a new instance to the actor
						AddNewDestructionInstance(InstanceTag, CurrentDataSet, DestructibleActor, CurrentTransform);
					}
				}
			}
		}
	}
}

TObjectPtr<ADestructionActor> UDestructionComponent::SpawnNewDestructionActor(FGameplayTag InstanceTag, FDestructionDataSet* CurrentDataSet)
{
	TObjectPtr<ADestructionActor> DestructibleActor = GetWorld()->SpawnActorDeferred<ADestructionActor>(ADestructionActor::StaticClass(), FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	DestructibleActor->GetISMComp().Get()->SetStaticMesh(CurrentDataSet->Mesh.Get());
	DestructibleActor->FinishSpawning(FTransform::Identity, true);
	DestructibleActor->DestructibleInstanceTag = InstanceTag;
	DestructibleInstanceActors.Add(InstanceTag, DestructibleActor.Get());

	return DestructibleActor;
}

void UDestructionComponent::AddNewDestructionInstance(FGameplayTag InstanceTag, FDestructionDataSet* CurrentDataSet, TObjectPtr<ADestructionActor> DestructibleActor, FTransform CurrentTransform)
{
	// Increment the instance count and reference it
	NumGlobalInstances++;

	// Add a new instance to the actor
	const int32 CurrentInstanceIndex = DestructibleActor->GetISMComp().Get()->AddInstance(CurrentTransform, true);

	TMap<int32, int32>* LocalInstanceMap = DestructiblesIndices.Find(InstanceTag);
	
	if(LocalInstanceMap == nullptr)
	{
		TMap<int32, int32> NewLocalInstanceMap;
		NewLocalInstanceMap.Add(CurrentInstanceIndex, NumGlobalInstances);
		DestructiblesIndices.Add(InstanceTag, NewLocalInstanceMap);
	}
	else
	{
		LocalInstanceMap->Add(CurrentInstanceIndex, NumGlobalInstances);
	}
	
	DestructiblesHealth.Add(NumGlobalInstances, CurrentDataSet->Health);
	DestructibleInstanceTransforms.Add(NumGlobalInstances, CurrentTransform);

	UpdateInstance(InstanceTag, CurrentInstanceIndex, CurrentDataSet->Health);
}

int32 UDestructionComponent::GetGlobalInstanceIndex(FGameplayTag InstanceTag, int32 InstanceIndex) const
{
	TMap<int32, int32> LocalInstanceMap = DestructiblesIndices.Find(InstanceTag) != nullptr ? *DestructiblesIndices.Find(InstanceTag) : TMap<int32, int32>();

	if (LocalInstanceMap.Find(InstanceIndex))
	{
		return *LocalInstanceMap.Find(InstanceIndex);
	}

	return INDEX_NONE;
}

FDestructionDataSet UDestructionComponent::GetDestructionDataSet(FGameplayTag DestructionTag)
{
	TArray<FGameplayTag> Keys;

	DestructionDataSets.GenerateKeyArray(Keys);

	for (FGameplayTag Key : Keys)
	{
		if (Key.MatchesTagExact(DestructionTag))
		{
			if (DestructionDataSets.Find(Key) != nullptr)
			{
				return *DestructionDataSets.Find(Key);
			}
		}
	}

	return FDestructionDataSet();
}

FDestructionDataSet* UDestructionComponent::GetDestructionDataSetPtr(FGameplayTag DestructionTag)
{
	TArray<FGameplayTag> Keys;

	DestructionDataSets.GenerateKeyArray(Keys);

	for (FGameplayTag Key : Keys)
	{
		if (Key.MatchesTagExact(DestructionTag))
		{
			if (DestructionDataSets.Find(Key) != nullptr)
			{
				return DestructionDataSets.Find(Key);
			}
		}
	}

	return nullptr;
}

void UDestructionComponent::ApplyDamageToHitResult(FHitResult HitResult, const float Damage)
{
	if(HitResult.GetActor())
	{
		if(ADestructionActor* DestActor = Cast<ADestructionActor>(HitResult.GetActor()))
		{
			ApplyDamageToInstance(DestActor->DestructibleInstanceTag, HitResult.Item, Damage);
		}
	}
}

void UDestructionComponent::ApplyDamageToInstance(FGameplayTag InstanceTag, int32 InstanceIndex, float Damage)
{
	const int32 GlobalInstanceIndex = GetGlobalInstanceIndex(InstanceTag, InstanceIndex);

	if (GlobalInstanceIndex != INDEX_NONE)
	{
		const float CurrentHealth = DestructiblesHealth.Find(GlobalInstanceIndex) != nullptr ? *DestructiblesHealth.Find(GlobalInstanceIndex) : -1;

		// Decrease health for the hit instance
		if (CurrentHealth >= 0.0f)
		{
			const float NewHealth = CurrentHealth - Damage;

			// Subtract health without destroying the whole instance
			if (NewHealth > 0)
			{
				DestructiblesHealth.Add(GlobalInstanceIndex, NewHealth);

				// Update the instance mesh to represent the new damage state
				// Pass the new health as param along, as the DestructilesHealth list replication happens *after* this update call
				UpdateInstance(InstanceTag, InstanceIndex, NewHealth);
				
			}
			// The instance hit 0 health and needs cleaning up
			else
			{
				/**
					Calling this directly is fine in a simple use case such as this test.
					However, in more complex scenarios, where we employ AOE weapons that destroy more than one piece at a time
					destroying instances direclty will cause the destructible lists to change indexes
					during the cleanup operation and subsequently delete the wrong instances.
					This can be prevented by queueing the destroyed indixes in an (async) queue and running the cleanup at the end of each frame
				*/
				DestroyInstance(InstanceTag, InstanceIndex, GlobalInstanceIndex);
			}
		}
	}
}

void UDestructionComponent::UpdateInstance_Implementation(FGameplayTag InstanceTag, int32 InstanceIndex, float NewHealth)
{
	FDestructionDataSet* CurrentDestructionDataSet = GetDestructionDataSetPtr(InstanceTag);
	TObjectPtr<ADestructionActor> DestructibleActor = *DestructibleInstanceActors.Find(InstanceTag);

	if (DestructibleActor && CurrentDestructionDataSet)
	{
		const float HealthNormalized = NewHealth / CurrentDestructionDataSet->Health;

		// Grab the color value that corresponds to our normalized health value
		const FLinearColor CurrentColor = CurrentDestructionDataSet->HealthStateColorCurve->GetLinearColorValue(HealthNormalized);

		// Let's update the color value based on the health curve value. This can probably be handled with the SetCustomDataValueFloat3 instead
		DestructibleActor->GetISMComp()->SetCustomDataValue(InstanceIndex, 0, CurrentColor.R, true);
		DestructibleActor->GetISMComp()->SetCustomDataValue(InstanceIndex, 1, CurrentColor.G, true);
		DestructibleActor->GetISMComp()->SetCustomDataValue(InstanceIndex, 2, CurrentColor.B, true);
	}
}

void UDestructionComponent::DestroyInstance_Implementation(FGameplayTag InstanceTag, int32 InstanceIndex, int32 GlobalInstanceIndex)
{
	TObjectPtr<ADestructionActor> DestructibleActor = DestructibleInstanceActors.Find(InstanceTag) != nullptr ? *DestructibleInstanceActors.Find(InstanceTag) : nullptr;

	if (DestructibleActor != nullptr)
	{
		DestructibleActor.Get()->GetISMComp()->RemoveInstance(InstanceIndex);
		
		TMap<int32, int32>* LocalInstanceMap = DestructiblesIndices.Find(InstanceTag);
		
		if(LocalInstanceMap != nullptr)
		{
			LocalInstanceMap->Remove(InstanceIndex);

			// Rebuild the indices
			TArray<int32> Indices;
			LocalInstanceMap->GenerateValueArray(Indices);
			LocalInstanceMap->Empty();

			for (int i = 0; i < Indices.Num(); i++)
			{
				LocalInstanceMap->Add(i, Indices[i]);
			}
		}

		DestructiblesHealth.Remove(GlobalInstanceIndex);
		DestructibleInstanceTransforms.Remove(GlobalInstanceIndex);
	}
}

float UDestructionComponent::GetDestructibleHealthForIndex(FGameplayTag InstanceTag, int32 InstanceIndex) const
{
	float ReturnValue = INDEX_NONE;
	const int32 GlobalInstanceIndex = GetGlobalInstanceIndex(InstanceTag, InstanceIndex);
	// Decrease health for the hit instance
	if (GlobalInstanceIndex != INDEX_NONE && DestructiblesHealth.Find(GlobalInstanceIndex) != nullptr)
	{
		const float CurrentHealthData = *DestructiblesHealth.Find(GlobalInstanceIndex);
		ReturnValue = CurrentHealthData;
	}

	return ReturnValue;
};

void UDestructionComponent::GetInstanceTransform(FGameplayTag InstanceTag, int32 InstanceIndex, FTransform& InstanceTransform)
{
	const int32 GlobalInstanceIndex = GetGlobalInstanceIndex(InstanceTag, InstanceIndex);
	InstanceTransform = DestructibleInstanceTransforms.Find(GlobalInstanceIndex) ? *DestructibleInstanceTransforms.Find(GlobalInstanceIndex) : FTransform();
}

//----------------------------------------------------------------------//
// debug
//----------------------------------------------------------------------//
#if WITH_GAMEPLAY_DEBUGGER_MENU
void UDestructionComponent::DescribeSelfToGameplayDebugger(FGameplayDebuggerCategory* DebuggerCategory)
{
	if (DebuggerCategory == nullptr)
	{
		return;
	}

	if (DebuggerCategory)
	{
		UWorld* World = GetWorld();

		if (World)
		{
			
		}
	}
}

#endif //WITH_GAMEPLAY_DEBUGGER_MENU

