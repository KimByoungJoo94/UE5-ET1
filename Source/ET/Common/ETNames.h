#pragma once

#include "CoreMinimal.h"

// 코드에서 직접 참조하는 이름 (데이터 테이블 RowName, 블랙보드 키 등)
namespace ETNames
{
	namespace RowName
	{		
		inline const FName RadialAttackCharge(TEXT("Charge"));

		inline const FName CameraShakeHit(TEXT("Hit"));
		inline const FName CameraShakeParry(TEXT("Parry"));
		inline const FName CameraShakePerfectParry(TEXT("PerfectParry"));
	}

	namespace BlackboardKey
	{
		inline const FName TargetActor(TEXT("TargetActor"));
		inline const FName HomeLocation(TEXT("HomeLocation"));
		// ET Select Action 이 고른 액션 몽타주 인덱스와 사용 거리
		inline const FName ActionIndex(TEXT("ActionIndex"));
		inline const FName ActionRange(TEXT("ActionRange"));
	}

	namespace AnimCurve
	{
		// 몽타주에서 상하체 분리 대신 전신 재생할 구간
		inline const FName FullBody(TEXT("FullBody"));
	}

	namespace MaterialParameter
	{
		inline const FName Opacity(TEXT("Opacity"));

		// FreeMaterialPack M_Disolve 의 파라미터 이름 (철자 그대로)
		inline const FName Dissolve(TEXT("Disolve"));
	}

	namespace RootMotionSource
	{
		inline const FName Dodge(TEXT("ETDodge"));
	}

	namespace SaveSlot
	{
		inline const FString Default(TEXT("ETSaveSlot"));
		inline constexpr int32 UserIndex = 0;
	}

	namespace Path
	{
		// 이 폴더의 DataTable 은 별도 등록 없이 RowStruct 별로 자동 등록
		inline const FName DataTable(TEXT("/Game/ET/DataTable"));
	}
}
