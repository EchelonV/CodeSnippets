// Copyright 2024, Talos Interactive, LLC. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "NativeGameplayTags.h"
#include "Engine/LevelScriptActor.h"
#include "DestructionLevelScript.generated.h"

UCLASS(notplaceable, meta=(KismetHideOverrides = "ReceiveAnyDamage,ReceivePointDamage,ReceiveRadialDamage,ReceiveActorBeginOverlap,ReceiveActorEndOverlap,ReceiveHit,ReceiveDestroyed,ReceiveActorBeginCursorOver,ReceiveActorEndCursorOver,ReceiveActorOnClicked,ReceiveActorOnReleased,ReceiveActorOnInputTouchBegin,ReceiveActorOnInputTouchEnd,ReceiveActorOnInputTouchEnter,ReceiveActorOnInputTouchLeave"), HideCategories=(Collision,Rendering,Transformation))
class GUNZILLATEST_API ADestructionLevelScript : public ALevelScriptActor
{
	GENERATED_UCLASS_BODY()

public:

#if WITH_EDITOR
	virtual void PreSave(FObjectPreSaveContext ObjectSaveContext) override;
#endif //WITH_EDITOR

	// Get the tags of all destructible instances
	TArray<FGameplayTag> GetDestuctibleTags() { return DestructibleTags; };

	// Get the transform of all destructible instances
	TArray<FTransform> GetDestructibleTransforms() { return DestructibleTransforms; };

private:

	void CollectDestructibleActors();

	/** Array of all destructible instance tags */
	UPROPERTY()
	TArray<FGameplayTag> DestructibleTags;

	/** Array of all destructible transforms */
	UPROPERTY()
	TArray<FTransform> DestructibleTransforms;

};
