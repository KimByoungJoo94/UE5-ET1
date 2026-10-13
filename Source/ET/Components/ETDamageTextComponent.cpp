#include "Components/ETDamageTextComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/ETDamageTextWidget.h"
#include "Common/ETCommonSettings.h"

void UETDamageTextComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* OwnerActor = GetOwner();
	const TSubclassOf<UETDamageTextWidget> DamageTextWidgetClass = GetDefault<UETCommonSettings>()->UI.DamageTextWidgetClass.LoadSynchronous();
	if (OwnerActor == nullptr || DamageTextWidgetClass == nullptr)
	{
		return;
	}

	for (int32 Index = 0; Index < PoolSize; ++Index)
	{
		// 소유 액터를 따라다니지 않도록 부착하지 않고 월드 위치로만 사용
		UWidgetComponent* WidgetComponent = NewObject<UWidgetComponent>(OwnerActor);
		WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		WidgetComponent->SetDrawAtDesiredSize(true);
		WidgetComponent->SetWidgetClass(DamageTextWidgetClass);
		WidgetComponent->RegisterComponent();
		WidgetComponent->InitWidget();

		WidgetComponentArray.Add(WidgetComponent);
	}
}

void UETDamageTextComponent::ShowDamageText(const FVector& InLocation, const float InDamage)
{
	if (WidgetComponentArray.IsEmpty())
	{
		return;
	}

	UWidgetComponent* WidgetComponent = WidgetComponentArray[NextWidgetComponentIndex];
	NextWidgetComponentIndex = (NextWidgetComponentIndex + 1) % WidgetComponentArray.Num();

	// 피벗으로 화면 기준 위치 보정 (거리와 상관없이 맞은 위치 위쪽, 좌우는 랜덤으로 흩어서 연속 타격 시 겹치지 않게)
	const float PivotX = TextPivot.X + FMath::FRandRange(-RandomPivotRangeX, RandomPivotRangeX);
	WidgetComponent->SetPivot(FVector2D(PivotX, TextPivot.Y));
	WidgetComponent->SetWorldLocation(InLocation);

	if (UETDamageTextWidget* DamageTextWidget = Cast<UETDamageTextWidget>(WidgetComponent->GetUserWidgetObject()))
	{
		DamageTextWidget->ShowDamage(InDamage);
	}
}
