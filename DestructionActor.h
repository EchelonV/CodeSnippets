// Copyright 2024, Talos Interactive, LLC. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "NativeGameplayTags.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "DestructionActor.generated.h"

/*
* An actor that stores all instances of destructible piece instances in the world
*/
UCLASS(meta=(DisplayName="Destruction Actor"))
class ADestructionActor : public AActor
{
	GENERATED_UCLASS_BODY()

	// The instanced static mesh comp of this actor
	TObjectPtr<UInstancedStaticMeshComponent> ISMComp;

public:

	// The tag that describes what type of destructible actor this is
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag DestructibleInstanceTag;

	TObjectPtr<UInstancedStaticMeshComponent> GetISMComp() { return ISMComp; };
	
};
