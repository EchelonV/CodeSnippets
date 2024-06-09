// Copyright 2024, Talos Interactive, LLC. All Rights Reserved.

#include "DestructionActor.h"
#include "GameFramework/Gamestate.h"

ADestructionActor::ADestructionActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// create new component, set mesh, and attach
	if(GetWorld() != nullptr)
	{
		ISMComp = CreateDefaultSubobject<UInstancedStaticMeshComponent>("InstancedStaticMeshComp");
		ISMComp->RegisterComponent();
		ISMComp->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
		ISMComp->Mobility = EComponentMobility::Stationary;
		ISMComp->SetGenerateOverlapEvents(true);
		ISMComp->bUseDefaultCollision = true;
		ISMComp->SetNumCustomDataFloats(3);
		
		SetRootComponent(ISMComp);
	}
}
