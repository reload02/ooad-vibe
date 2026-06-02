# R2 변경 전/후 비교

이 문서는 R2에서 꼭 발표해야 할 핵심 변경사항을 변경 전/후 관점으로 정리한다.
핵심 메시지는 **우측 센서 기반 판단 제거**와 **`TurnRight` 후 전방 센서를 재사용하는 우측 탐색 정책 도입**이다.

## 1. 핵심 변경 요약

| 구분 | 변경 전 | 변경 후 |
| --- | --- | --- |
| 우측 장애물 판단 | `Right Sensor`가 우측 장애물을 periodic 방식으로 제공 | 별도 우측 센서 없이 `TurnRight` 후 `Front Sensor` interrupt로 기존 우측 방향을 확인 |
| periodic sensor 입력 | 좌측, 우측, 먼지 센서 값을 tick마다 전달 | 좌측, 먼지 센서 값만 tick마다 전달 |
| 회피 정책 | 전방 장애물 발생 후 좌/우 센서값으로 열린 방향 선택 | 좌측이 열려 있으면 `TurnLeft`, 좌측이 막혀 있으면 `TurnRight`로 우측 탐색 |
| 좌우 모두 open 정책 | 좌우가 모두 열려 있으면 좌/우를 번갈아 선택 | 우측 센서가 제거되어 좌우 동시 open 판단 및 교대 정책 삭제 |
| 삼방향 blocked 처리 | 전방, 좌측, 우측이 모두 막히면 바로 `Escaping`에서 후진 | 전방과 좌측이 막히면 우측 탐색을 먼저 수행하고, 우측도 막히면 원래 방향 복구 후 후진 |
| 탈출 조건 | 좌측 또는 우측 센서 중 하나가 open이면 탈출 가능 | 좌측이 open이거나 우측 탐색 결과가 open이면 탈출 가능 |

## 2. 요구사항 변경

R2에서 요구사항의 중심이 "우측 센서값을 읽는다"에서 "전방 센서를 우측 탐색에도 재사용한다"로 바뀌었다.

| 파일 | 위치 | 발표 포인트 |
| --- | --- | --- |
| `docs/requirements.md` | 10-13행 | 좌측/먼지 센서만 periodic으로 남고, 우측 센서 periodic 요구사항은 삭제됨 |
| `docs/requirements.md` | 63-64행 | UC-03에서 좌측 blocked 시 `TurnRight`로 우측 탐색 후 전방 interrupt로 판단 |
| `docs/requirements.md` | 104-111행 | FR-06, FR-08~FR-13이 R2 정책 기준으로 변경되고 FR-09 좌우 교대 정책 삭제 |
| `docs/srs.md` | 513-520행 | SRS 요구사항 표에서 같은 변경을 정식 요구사항으로 반영 |

발표 문장:

> R2에서는 하드웨어 조건 변경으로 우측 센서가 제거되었다. 따라서 우측 장애물은 주기 센서값이 아니라, RVC가 우측으로 90도 회전한 뒤 전방 센서 interrupt를 통해 확인하도록 요구사항이 바뀌었다.

## 3. 설계 변경

우측 센서가 사라지면서 controller 상태와 판단 입력 구조도 바뀌었다.

| 파일 | 위치 | 변경 전 | 변경 후 |
| --- | --- | --- | --- |
| `include/rvc/Types.hpp` | 29-35행 | `Idle`, `Cleaning`, `Avoiding`, `Escaping` 중심 상태 | `RightProbing`, `EscapeAligning` 상태 추가 |
| `include/rvc/Types.hpp` | 38-43행 | 우측 탐색 결과를 표현하는 enum 없음 | `RightProbeState` 추가: `None`, `Checking`, `Open`, `Blocked` |
| `include/rvc/Types.hpp` | 52-61행 | `PeriodicSensorData.rightObstacle`, `SensorSnapshot.rightObstacle` 사용 | `rightObstacle` 제거, `SensorSnapshot.rightProbe` 추가 |
| `docs/sdd.md` | 600-633행 | 측면 센서값으로 회피/탈출 판단 | 우측 탐색, 원래 방향 복구, 후진 후 재탐색 알고리즘 명시 |

중요 설계 의미:

- `PeriodicSensorData`는 이제 좌측 장애물과 먼지만 가진다.
- `SensorSnapshot`은 전방 interrupt, 좌측 periodic 값, 먼지 periodic 값, 우측 탐색 상태를 결합한다.
- `RightProbing`은 `TurnRight` 후 전방 센서로 기존 우측 방향을 확인하는 상태다.
- `EscapeAligning`은 우측 탐색 실패 후 `TurnLeft`로 원래 진행 방향을 복구하는 상태다.

## 4. 구현 변경

실제 제어 로직은 `RvcController`의 상태 머신에서 가장 크게 바뀌었다.

| 파일 | 위치 | 발표 포인트 |
| --- | --- | --- |
| `src/RvcController.cpp` | 41-47행 | `readPeriodicSensors()`가 전방 interrupt, 좌측/먼지 periodic 값, 우측 탐색 상태를 `SensorSnapshot`으로 결합 |
| `src/RvcController.cpp` | 57-67행 | `RightProbing` 상태에서 전방 interrupt가 있으면 blocked, 없으면 open으로 판단 |
| `src/RvcController.cpp` | 69-72행 | 우측 탐색 blocked 시 `TurnLeft`로 원래 진행 방향을 복구한 뒤 `Backward` |
| `src/RvcController.cpp` | 75-84행 | `Escaping` 중 좌측이 계속 막히면 다시 `TurnRight`로 우측 탐색 반복 |
| `src/RvcController.cpp` | 93-101행 | 전방 interrupt 발생 시 좌측 open이면 `TurnLeft`, 좌측 blocked면 `TurnRight` |

변경 전 흐름:

```text
Front blocked
-> Left/Right periodic sensor 확인
-> 열린 측면 방향으로 회전
-> 좌우 모두 blocked이면 Escaping에서 계속 Backward
```

변경 후 흐름:

```text
Front blocked
-> Left periodic sensor 확인
-> Left open이면 TurnLeft
-> Left blocked이면 TurnRight로 우측 탐색
-> 우측 탐색 후 Front clear이면 Forward
-> 우측 탐색 후 Front blocked이면 TurnLeft로 원래 방향 복구
-> Backward
-> Left blocked이면 다시 TurnRight로 우측 탐색 반복
```

## 5. 하드웨어/시뮬레이터 구조 변경

R2 변경은 센서 정책뿐 아니라 RVC와 시뮬레이터 연결 구조에도 반영되었다.

| 파일 | 위치 | 발표 포인트 |
| --- | --- | --- |
| `include/rvc/RvcHardwareAdapter.hpp` | 7-13행 | 전방 interrupt, periodic sensor 읽기, command 적용을 추상화 |
| `include/rvc/Rvc.hpp` | 10-27행 | `Rvc`가 `RvcController`와 `RvcHardwareAdapter`를 소유 |
| `src/Rvc.cpp` | 23-32행 | tick마다 interrupt 확인, periodic sensor 읽기, controller 호출, command 적용 순서 수행 |
| `src/SimulatedHardwareAdapter.cpp` | 68-76행 | 시뮬레이터 periodic sensor도 좌측 장애물과 먼지만 제공 |

발표 문장:

> R2에서는 controller가 실제 하드웨어나 시뮬레이터를 직접 알지 않도록 `RvcHardwareAdapter`를 두었고, `Rvc`가 센서 입력 수집과 command 적용 순서를 조율한다. 이 구조 덕분에 우측 센서 제거 같은 하드웨어 변경을 adapter와 상태 모델로 흡수할 수 있다.

## 6. 테스트 변경

기존 테스트는 우측 센서값과 좌우 교대 정책을 검증했다. R2 이후에는 우측 탐색 성공/실패와 후진 후 재탐색을 검증한다.

| 파일 | 위치 | 변경 내용 |
| --- | --- | --- |
| `tests/controller_tests.cpp` | 110-123행 | 좌측 blocked 시 `RightProbing` 진입과 `TurnRight` 확인 |
| `tests/controller_tests.cpp` | 126-148행 | 우측 탐색 open 시 `Forward`로 청소 재개 확인 |
| `tests/controller_tests.cpp` | 151-176행 | 우측 탐색 blocked 시 원래 방향 복구 후 `Backward` 확인 |
| `tests/controller_tests.cpp` | 179-206행 | `Escaping` 중 후진 후 우측 재탐색 확인 |
| `tests/system_tests.cpp` | 90-108행 | 시뮬레이터에서 `TurnRight -> TurnLeft -> Backward` 흐름 검증 |
| `tests/system_tests.cpp` | 131-150행 | 후진 중 우측 탐색 반복 후 open 시 전진 재개 검증 |
| `docs/traceability.md` | 12-19행 | FR-06, FR-08~FR-13 변경 추적 |
| `docs/traceability.md` | 33-36행 | 변경된 요구사항과 테스트 연결 |

## 7. 발표용 결론

R2에서 가장 중요한 변화는 다음 한 문장으로 정리할 수 있다.

> 우측 센서를 제거하면서, 우측 장애물 판단을 periodic sensor 입력이 아니라 `TurnRight` 후 전방 센서 interrupt로 확인하는 탐색 절차로 바꾸었다.

이 변화 때문에 요구사항, 타입 구조, controller 상태 머신, 시뮬레이터 adapter, 테스트 케이스가 모두 함께 변경되었다.

발표 순서는 다음이 적절하다.

1. 하드웨어 변경: 우측 센서 제거
2. 요구사항 변경: FR-06, FR-08~FR-13, FR-09 삭제
3. 설계 변경: `RightProbing`, `EscapeAligning`, `RightProbeState`
4. 구현 변경: `RvcController` 상태 전이
5. 검증 변경: 우측 탐색 성공/실패와 후진 후 재탐색 테스트
