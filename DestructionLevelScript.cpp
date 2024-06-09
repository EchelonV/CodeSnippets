// Copyright 2024, Talos Interactive, LLC. All Rights Reserved.

#include "DestructionLevelScript.h"
#include "DestructionPreviewActor.h"
#include "EngineUtils.h"

#if WITH_EDITOR
#include "UObject/ObjectSaveContext.h"
#endif //WITH_EDITOR

//#pragma optimize("", off)
//#pragma inline_depth(0)

#include UE_INLINE_GENERATED_CPP_BY_NAME(DestructionLevelScript)

ADestructionLevelScript::ADestructionLevelScript( const FObjectInitializer& ObjectInitializer ) : Super(ObjectInitializer)
{
}

#if WITH_EDITOR
void ADestructionLevelScript::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	CollectDestructibleActors();

	Super::PreSave(ObjectSaveContext);
}
#endif //WITH_EDITOR

void ADestructionLevelScript::CollectDestructibleActors()
{
	DestructibleTags.Empty();
	DestructibleTransforms.Empty();

	if(UWorld* World = GetWorld())
	{
		for (TActorIterator<ADestructionPreviewActor> It(World); It; ++It)
		{
			DestructibleTags.Add(It->DestructionTag);
			DestructibleTransforms.Add(It->GetActorTransform());
		}
	}
}
