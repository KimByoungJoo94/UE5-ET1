#include "UI/ETUserWidget.h"
#include "Character/ETPlayer.h"

AETCharacter* UETUserWidget::GetOwningETCharacter() const
{	
	if (APlayerController* PlayerController = GetOwningPlayer())
	{		
		if (AETCharacter* Character = Cast<AETCharacter>(PlayerController->GetPawn()))
		{
			return Character;
		}
	}

	return nullptr;
}

AETPlayer* UETUserWidget::GetOwningETPlayer() const
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		if (AETPlayer* Player = Cast<AETPlayer>(PlayerController->GetPawn()))
		{
			return Player;
		}
	}

	return nullptr;
}

UETGameStatComponent* UETUserWidget::GetOwningGameStatComponent() const
{
	if (AETCharacter* OwningCharacter = GetOwningETCharacter())
	{
		if (UETGameStatComponent* GameStatComponent = OwningCharacter->GetGameStatComponent())
		{
			return GameStatComponent;
		}
	}

	return nullptr;
}