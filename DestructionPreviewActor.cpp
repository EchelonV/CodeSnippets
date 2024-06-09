// Copyright 2024, Talos Interactive, LLC. All Rights Reserved.

#include "DestructionPreviewActor.h"
#include "DestructionData.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Engine/AssetManager.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"

ADestructionPreviewActor::ADestructionPreviewActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorEnableCollision(false);
	bNetLoadOnClient = false;
	bIsEditorOnlyActor = false;

#if WITH_EDITORONLY_DATA
	bIsEditorPreviewActor = true;
	//HiddenEditorViews;
#endif //WITH_EDITORONLY_DATA

	PrimaryActorTick.bCanEverTick = false;
}

#if WITH_EDITOR
void ADestructionPreviewActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if(!CachedDestructionTag.MatchesTagExact(DestructionTag))
	{
		CachedDestructionTag = DestructionTag;
		
		TArray<FAssetData> DestructionDataAssetList;
		UAssetManager& AssetManager = UAssetManager::Get();
		AssetManager.GetAssetRegistry().GetAssetsByClass(UDestructionData::StaticClass()->GetClassPathName(), DestructionDataAssetList, true);

		// Fill up the interaction map
		for (FAssetData AssetData : DestructionDataAssetList)
		{
			if (AssetData.IsValid())
			{
				UDestructionData* DestructionDataSet = Cast<UDestructionData>(AssetData.GetAsset());
				if (DestructionDataSet != nullptr)
				{
					TArray<FGameplayTag> Keys;
					DestructionDataSet->DestructionDataSets.GenerateKeyArray(Keys);

					for (FGameplayTag Key : Keys)
					{
						if (FDestructionDataSet* DataSet = DestructionDataSet->DestructionDataSets.Find(Key))
						{
							if (Key.MatchesTagExact(CachedDestructionTag) && DataSet->Mesh != nullptr)
							{
								GetStaticMeshComponent()->SetStaticMesh(DataSet->Mesh);
							}
						}
					}
				}
			}
		}
	}
	
	Super::Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif // WITH_EDITOR

void ADestructionPreviewActor::BeginPlay()
{
#if WITH_EDITOR
	GetStaticMeshComponent()->bHiddenInGame = true;
	SetActorHiddenInGame(true);
#endif // WITH_EDITOR
	Super::BeginPlay();
}

void ADestructionPreviewActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if WITH_EDITOR
	GetStaticMeshComponent()->bHiddenInGame = false;
	SetActorHiddenInGame(false);
#endif // WITH_EDITOR
	Super::EndPlay(EndPlayReason);
}

