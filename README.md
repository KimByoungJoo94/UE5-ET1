# UE5-ET1

## Required Assets

`Content/ET` 는 직접 제작한 에셋이라 저장소에 포함되어 있습니다.
아래 외부 에셋은 용량 문제로 `.gitignore` 에서 제외되어 있으므로, 프로젝트를 열기 전에 Fab 에서 직접 받아 프로젝트에 추가해야 합니다.

| 에셋 | 출처 | 설치 경로 |
| --- | --- | --- |
| Paragon: Kwang | Fab | `Content/ParagonKwang` |
| Paragon: Sun Wukong | Fab | `Content/ParagonSunWukong` |
| Paragon: Minions | Fab | `Content/ParagonMinions` |
| Paragon: Rampage | Fab | `Content/ParagonRampage` |
| Sci-fi Container Free 02 | Fab | `Content/Fab/Sci-fi_Container_Free_02` |
| The Motherland Calls | Fab | `Content/Fab/The_Motherland_Calls` |
| Torii Gate - 3D Model | Fab | `Content/Fab/Torii_Gate_-_3D_Model` |
| Monitor V - Tactical Emergency Device | Fab | `Content/Fab/Monitor_V_-_Tactical_Emergency_Device` |
| FREE Ritual Stone | Fab | `Content/Fab/FREE_Ritual_Stone` |
| Free Material Pack | Fab | `Content/FreeMaterialPack` |
| Soul: Cave | Fab | `Content/SoulCave` |
| Asian Village | Fab | `Content/Asian_Village` |

> Fab 에서 "프로젝트에 추가(Add to Project)" 로 받으면 위 경로에 자동으로 설치됩니다.

## External Asset Modifications

받은 외부 에셋은 가능하면 수정하지 않고 사용합니다.
다만 꼭 필요한 부분만 최소한으로 편집했으며, 해당 에셋은 저장소에 포함되지 않으므로 새로 받은 뒤 아래 내용을 직접 적용해야 합니다.
이후 외부 에셋을 수정하게 되면 아래에 항목을 추가해 기록합니다.

### Paragon: Minions - 애님 슬롯 `UpperBody_InMotion` 추가

| 항목 | 내용 |
| --- | --- |
| 수정 내용 | 스켈레톤에 애님 슬롯 `UpperBody_InMotion` 추가 (DefaultGroup) |
| 이유 | 이동 중 상체 공격 몽타주 재생 |

대상 스켈레톤

```
Content/ParagonMinions/Characters/Buff/Buff_Red/Meshes/Buff_Red_Skeleton
Content/ParagonMinions/Characters/Minions/White_Camp_Minion/Meshes/Minion_Skeleton
Content/ParagonMinions/Characters/Minions/Down_Minions/Meshes/Minion_Lane_Core_Skeleton
Content/ParagonMinions/Characters/Minions/Down_Minions/Meshes/Minion_Lane_Siege_Skeleton
Content/ParagonMinions/Characters/Minions/Down_Minions/Meshes/SK_Minion_Lane_Super
```

> `Buff_Black_Skeleton` 은 원본에 `UpperBody_InMotion` 슬롯이 이미 있어 수정하지 않았습니다.
> 슬롯은 ABP 컴파일 시 스켈레톤에 자동 등록되기도 하지만, 몽타주에서 슬롯을 선택하려면 미리 추가해 두는 것이 안전합니다.

### FREE Ritual Stone - 폴더 이름 변경, 피벗 위치 수정, 콜리전 추가

| 항목 | 내용 |
| --- | --- |
| 수정 내용 | 설치 폴더 `FREE_Ritual_Stone🪨` 를 `FREE_Ritual_Stone` 으로 이름 변경 (에디터에서 변경 후 리디렉터 정리) |
| 이유 | 경로에 이모지가 포함되어 있어 경로 처리 문제 방지 |
| 수정 내용 | 모델링 모드 Edit Pivot 으로 피벗을 메시 중앙으로 이동 |
| 이유 | 원본 피벗이 메시에서 멀리 떨어져 있어 배치, 위치 조정이 어려움 |
| 수정 내용 | 스태틱 메시 에디터에서 박스 단순화 콜리전 추가 |
| 이유 | 원본 콜리전이 메시 형태와 맞지 않음 |

대상 스태틱 메시

```
Content/Fab/FREE_Ritual_Stone/free_ritual_stone/StaticMeshes/free_ritual_stone
```

## Controls

| 키 | 동작 |
| --- | --- |
| `W` `A` `S` `D` | 이동 |
| 마우스 이동 | 카메라 회전 |
| `Space` | 점프 |
| 마우스 왼쪽 | 공격 (콤보) |
| 마우스 오른쪽 | 강공격 (누르고 있으면 충전) |
| `Left Ctrl` | 회피 |
| `E` | 패링 (누르고 있으면 유지) |
| `R` | 상호작용 |
| `Q` | 아이템 사용 |
| `1` | 스킬 A : 시간 정지 |
| `2` | 스킬 B : 연속 베기 |
| `3` | 스킬 C : 범위 공격 |
| `4` | 스킬 D |
| `Esc` | 메뉴 열기 / 닫기 |
