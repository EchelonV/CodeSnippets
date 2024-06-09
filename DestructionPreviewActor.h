// Copyright 2024, Talos Interactive, LLC. All Rights Reserved.

#pragma once

#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "NativeGameplayTags.h"
#include "DestructionPreviewActor.generated.h"

/*
* An actor that represents one destroyed piece of a building or asset in the editor during design time
*/
UCLASS(meta=(DisplayName="Destruction Preview Actor"))
class ADestructionPreviewActor : public AStaticMeshActor
{
	GENERATED_UCLASS_BODY()

public:

	// The tag that describes what building part this actor represents
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Destruction")
	FGameplayTag DestructionTag;

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface

	//~UObject interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
	
private:

	FGameplayTag CachedDestructionTag;
};
