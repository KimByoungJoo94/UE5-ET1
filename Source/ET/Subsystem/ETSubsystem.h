#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"

// 서브시스템 조회 공용 함수 (GameInstance, World 서브시스템을 같은 형태로 조회)
namespace ETSubsystem
{
	// 월드 컨텍스트로 서브시스템 조회 (월드나 서브시스템이 없을 수 있는 곳에서 사용, 없으면 nullptr)
	template <typename T>
	T* GetPtr(const UObject* InWorldContextObject)
	{
		const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(InWorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
		if (World == nullptr)
		{
			return nullptr;
		}

		if constexpr (TIsDerivedFrom<T, UGameInstanceSubsystem>::IsDerived)
		{
			return UGameInstance::GetSubsystem<T>(World->GetGameInstance());
		}
		else
		{
			static_assert(TIsDerivedFrom<T, UWorldSubsystem>::IsDerived, "T must derive from UGameInstanceSubsystem or UWorldSubsystem.");
			return World->GetSubsystem<T>();
		}
	}

	// 기본 조회 (서브시스템이 반드시 있는 곳에서 사용, 없으면 check)
	// 썸네일, BP 에디터 등 프리뷰 월드나 서브시스템이 아직 생성되지 않은 시점(액터 생성 단계 등)에는 GetPtr 로 nullptr 체크 후 사용
	template <typename T>
	T& Get(const UObject* InWorldContextObject)
	{
		T* Subsystem = GetPtr<T>(InWorldContextObject);
		check(Subsystem);
		return *Subsystem;
	}
}
