// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ETInteractionInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UETInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ET_API IETInteractionInterface
{
	GENERATED_BODY()
	
public:	
	virtual void OnInteraction(AActor* InInteractor) = 0;
};
