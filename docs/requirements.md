# RVC Control SW Requirements

## 1. Scope

이 문서는 RVC(Robotic Vacuum Cleaner) 자동 청소 제어 소프트웨어의 유스케이스, 기능 요구사항, 비기능 요구사항을 정의한다. 요구사항의 기준은 `docs/rvc.pdf`의 RVC Control SW 설명이며, RVC는 household surface를 자동으로 청소하고 물걸레질하는 장치로 본다. 현재 제어 SW 범위에서는 별도 mop actuator를 만들지 않고 `Cleaner`가 청소/물걸레질 출력을 대표하는 추상 actuator 역할을 한다.

다음 추가 조건을 포함한다.

- 전방 센서는 interrupt 방식으로 동작한다.
- [R2-변경] 좌측 센서와 먼지 센서는 periodic 방식으로 동작한다.
- [R2-삭제] ~~우측 센서는 periodic 방식으로 동작한다.~~
- [R2-변경] 우측 장애물 판단은 우측 센서값이 아니라 RVC가 우측으로 90도 회전한 뒤 전방 센서 interrupt로 확인한다.
- [R2-변경] 전방과 좌측이 막히고 우측 탐색 결과도 막힌 경우 RVC는 원래 진행 방향으로 자세를 복구한 뒤 후진한다.
- [R3-추가] 후방 장애물 센서가 추가되며, 후방 센서는 interrupt 방식으로 동작한다.
- [R3-변경] 청소기는 전원이 켜져 청소 중이면 기본적으로 항상 `Normal` 청소 상태를 유지한다.
- [R3-변경] 먼지를 감지한 경우에만 필요한 만큼 제자리 회전하면서 `Boost`로 동작하고, 그 외 모든 주행/회피/탈출 동작에서는 `Normal`로 동작한다.
- [R3-변경] 먼지를 만날 때마다 주행 방향을 전진/후진으로 toggle한다. 전진 중 먼지를 감지하면 RVC가 시계 방향으로 제자리 회전 후 후진하고, 후진 중 먼지를 감지하면 RVC가 반시계 방향으로 제자리 회전 후 전진한다.
- [R3-변경] `Boost` 청소 후에도 먼지는 사라지지 않으며, 모든 시나리오에서 같은 먼지를 다시 만나면 다시 먼지 감지 및 청소 동작을 수행한다.
- [R4-검증] 시작 위치 복귀 시나리오는 RVC가 시작 위치로 복귀할 수 있을 만큼 충분한 tick 동안 진행한다.
- 하드웨어의 상세 제어 설계는 범위 밖이며, 제어 소프트웨어는 추상화된 센서 입력과 동작 명령을 다룬다.
- [추가] RVC 제어 소프트웨어는 상위 객체 `Rvc`가 `RvcController`와 `RvcHardwareAdapter`를 소유하는 구조로 본다.
- [추가] `GridSimulator`는 `SimulatedHardwareAdapter`를 통해 테스트용 하드웨어 환경만 제공한다.
- [삭제] ~~`GridSimulator`가 `RvcController`를 직접 소유하는 구조를 시스템 중심 구조로 보지 않는다.~~

## 2. Actors

| Actor | Description |
| --- | --- |
| User | RVC의 자동 청소를 시작하거나 중지하는 사용자 |
| Front Sensor | 전방 장애물을 interrupt로 알리는 센서 |
| Left Sensor | 좌측 장애물 상태를 주기적으로 제공하는 센서 |
| [R2-삭제] ~~Right Sensor~~ | ~~우측 장애물 상태를 주기적으로 제공하는 센서~~ |
| Rear Sensor | [R3-추가] 후진 주행 중 후방 장애물을 interrupt로 알리는 센서 |
| Dust Sensor | 먼지 감지 상태를 주기적으로 제공하는 센서 |
| Digital Clock | 제어 tick을 제공하는 주기적 시간 소스 |
| Motor | 이동, 정지, 회전, 후진 명령을 수행하는 하드웨어 |
| Cleaner | 일반 청소, 강화 청소, 물걸레질 출력을 대표하는 추상 하드웨어 |
| RvcHardwareAdapter | [추가] 센서 입력과 command 적용을 RVC 상위 객체에 제공하는 추상 adapter |
| SimulatedHardwareAdapter | [추가] 그리드 기반 검증에서 `RvcHardwareAdapter` 계약을 구현하는 테스트용 adapter |

## 3. Use Cases

### UC-01 Start Automatic Cleaning

| Item | Description |
| --- | --- |
| Primary Actor | User |
| Goal | RVC가 자동 청소를 시작한다. |
| Precondition | RVC 제어 소프트웨어가 초기화되어 있다. |
| Main Flow | User가 청소 시작을 요청한다. System은 청소 상태로 전환하고, cleaner를 켠 상태로 전진 청소를 시작한다. |
| Postcondition | RVC는 자동 청소 상태가 된다. |

### UC-02 Stop Automatic Cleaning

| Item | Description |
| --- | --- |
| Primary Actor | User |
| Goal | RVC가 자동 청소를 중지한다. |
| Precondition | RVC가 자동 청소 중이다. |
| Main Flow | User가 청소 중지를 요청한다. System은 motor를 정지하고 cleaner를 끈다. |
| Postcondition | RVC는 idle 상태가 된다. |

### UC-03 Avoid Front Obstacle

| Item | Description |
| --- | --- |
| Primary Actor | Front Sensor |
| Goal | 전방 장애물을 감지했을 때 충돌 없이 회피한다. |
| Precondition | RVC가 자동 청소 중이다. |
| Main Flow | [R2-변경] Front Sensor가 interrupt를 발생시킨다. System은 즉시 전진 청소를 멈추고 cleaner를 끈 뒤 좌측 periodic sensor 값을 확인한다. 좌측이 열려 있으면 `TurnLeft`로 회피한다. 좌측이 막혀 있으면 `TurnRight`로 우측 탐색을 시작하고, 회전 후 전방 센서 interrupt로 기존 우측 방향의 장애물 여부를 확인한다. |
| Alternative Flow | [R2-변경] 우측 탐색 후 전방 interrupt가 없으면 기존 우측 방향이 열린 것으로 보고 전진 청소를 재개한다. 우측 탐색 후 전방 interrupt가 있으면 `TurnLeft`로 원래 진행 방향을 복구한 뒤 `Escaping` 후진 시퀀스에 진입한다. [R2-삭제] ~~좌/우가 모두 열려 있으면 기본 회전 정책에 따라 좌우를 번갈아 선택한다.~~ |
| Postcondition | RVC는 장애물을 회피하고 청소를 계속한다. |

### UC-04 Escape From Blocked Area

| Item | Description |
| --- | --- |
| Primary Actor | [R2-변경] Front Sensor, Left Sensor |
| Goal | [R2-변경] 전방과 좌측이 막히고 우측 탐색으로도 막힘이 확인된 상황에서 탈출한다. |
| Precondition | [R2-변경] RVC가 자동 청소 중이며 전방 interrupt가 발생했고 좌측 periodic sensor가 막힘을 보고한다. |
| Main Flow | [R2-변경] System은 좌측이 막힌 상태에서 `TurnRight`로 우측을 탐색한다. 탐색 후 전방 interrupt가 다시 발생하면 `TurnLeft`로 원래 진행 방향을 복구하고 cleaner off 상태로 `Backward`를 수행한다. 이후 매 후진 후 좌측이 계속 막혀 있으면 다시 우측 탐색을 수행하고, 좌측이 열리면 `TurnLeft`로 탈출한다. |
| Postcondition | RVC는 탈출 가능 위치로 이동한 뒤 자동 청소를 계속한다. |

### UC-05 Avoid Rear Obstacle During Backward Travel

| Item | Description |
| --- | --- |
| Primary Actor | Rear Sensor |
| Goal | 후진 주행 중 후방 장애물을 감지했을 때 충돌 없이 회피한다. |
| Precondition | RVC가 자동 청소 중이며 현재 주행 방향이 후진이다. |
| Main Flow | [R3-추가] Rear Sensor가 interrupt를 발생시킨다. System은 즉시 후진을 멈추고 주행 방향 기준 장애물 회피 정책을 적용한다. 좌측이 열려 있으면 RVC는 `TurnRight` 후 후진을 재개한다. 좌측이 막혀 있으면 `TurnLeft`로 기존 우측 방향을 후방 센서로 탐색한다. |
| Alternative Flow | [R3-추가] 우측 탐색 후 후방 interrupt가 없으면 기존 우측 방향이 열린 것으로 보고 후진을 재개한다. 우측 탐색 후 후방 interrupt가 있으면 `TurnRight`로 원래 진행 방향을 복구한 뒤 전진 주행 모드로 전환한다. 전방도 막힌 경우에는 전방 interrupt 기반 회피 정책을 다시 적용한다. |
| Postcondition | RVC는 후방 장애물을 회피하고 현재 청소 정책을 유지한다. |

### UC-06 Boost Cleaning On Dust

| Item | Description |
| --- | --- |
| Primary Actor | Dust Sensor |
| Goal | 먼지를 감지하면 RVC가 필요한 제자리 회전 동안 `Boost`로 청소하고 주행 방향을 전환한다. |
| Precondition | RVC가 자동 청소 중이다. |
| Main Flow | [R3-변경] Dust Sensor가 periodic sampling에서 먼지를 감지한다. 전진 중이면 RVC는 `Boost` 상태로 시계 방향 제자리 회전 후 후진 주행 모드로 전환한다. 후진 중이면 RVC는 `Boost` 상태로 반시계 방향 제자리 회전 후 전진 주행 모드로 전환한다. 제자리 회전이 끝나면 cleaner는 `Normal`로 복귀한다. |
| Postcondition | RVC는 먼지 위치에서 강화 청소를 수행하되 먼지 셀은 제거하지 않고, 이후 모든 시나리오에서 해당 먼지를 다시 만나면 같은 정책을 다시 수행한다. |

## 4. Verification Support

[변경] 시뮬레이터는 RVC의 사용자 또는 센서 중심 유스케이스가 아니라 `Rvc`와 `RvcHardwareAdapter` 기반 제어 소프트웨어 구현을 검증하기 위한 지원 기능으로 분리한다.

| ID | Item | Description |
| --- | --- | --- |
| VS-01 | CLI simulator scenario execution | [변경] 기본 맵 또는 시나리오 맵으로 `Rvc`와 `SimulatedHardwareAdapter` 동작을 실행하고 tick별 sensor/event, command, robot position, direction, cleaning power 로그를 제공한다. [삭제] ~~기본 맵 또는 시나리오 맵으로 controller 동작을 실행한다.~~ |

## 5. Functional Requirements

| ID | Requirement | Related Context |
| --- | --- | --- |
| FR-01 | System shall start automatic cleaning when requested. | UC-01 |
| FR-02 | System shall stop motor and cleaner when cleaning is stopped. | UC-02 |
| FR-03 | System shall move forward while cleaning when no obstacle blocks the front direction. | UC-01 |
| FR-04 | Front obstacle detection shall be handled as an interrupt event. | UC-03 |
| FR-05 | System shall stop forward motion immediately when a front obstacle interrupt is received. | UC-03 |
| FR-06 | [R2-변경] Left and dust sensors shall be sampled periodically on control tick. [R2-삭제] ~~Right sensor shall be sampled periodically.~~ | UC-03, UC-06 |
| FR-07 | If front is blocked and only left is open, System shall turn left. | UC-03 |
| FR-08 | [R2-변경] If front is blocked and left is blocked, System shall turn right to probe the former right direction with the front sensor. | UC-03 |
| FR-09 | [R2-삭제] ~~If front is blocked and both left and right are open, System shall choose turn direction by alternating left and right.~~ | UC-03 |
| FR-10 | [R2-변경] If front is blocked, left is blocked, and right probing also reports blocked, System shall restore the original heading and enter `Escaping`. | UC-04 |
| FR-11 | [R2-변경] In `Escaping` state, System shall execute one `Backward` command, then probe right again when left remains blocked. | UC-04 |
| FR-12 | [R2-변경] Escape shall be considered possible when left is open or right probing confirms that the former right direction is open. | UC-04 |
| FR-13 | [R2-변경] After escape becomes possible, System shall turn toward the confirmed open direction or continue forward after successful right probing. | UC-04 |
| FR-14 | [R2-기존] If dust is detected, System shall set cleaner power to boost for a configured number of ticks. | UC-06 |
| FR-15 | [R2-기존] If boost duration expires and no new dust is detected, System shall return cleaner power to normal. | UC-06 |
| FR-16 | Simulator shall render a grid map with robot, obstacle, dust, and empty cells. | VS-01 |
| FR-17 | Simulator shall log tick, sensor values, command, robot position, direction, and cleaning power. | VS-01 |
| FR-18 | [변경] Simulator shall exercise the same `RvcHardwareAdapter` contract as the production control SW. | VS-01 |
| FR-19 | [R3-추가] Rear obstacle detection shall be handled as an interrupt event. | UC-05 |
| FR-20 | [R3-추가] When a rear obstacle interrupt is received during backward travel, System shall stop backward motion immediately and apply travel-direction obstacle avoidance. | UC-05 |
| FR-21 | [R3-변경] While powered on and cleaning, System shall keep cleaner power at `Normal` by default in every travel and avoidance state. | UC-01, UC-03, UC-04, UC-05 |
| FR-22 | [R3-변경] If dust is detected while moving forward, RVC shall rotate in place clockwise with `Boost` only for the required rotation, then continue in backward travel mode. | UC-06 |
| FR-23 | [R3-변경] If dust is detected while moving backward, RVC shall rotate in place counterclockwise with `Boost` only for the required rotation, then continue in forward travel mode. | UC-06 |
| FR-24 | [R3-변경] Outside dust-triggered in-place rotation, System shall use `Normal` cleaner power instead of fixed-duration boost or movement-time cleaner off. | UC-03, UC-04, UC-05, UC-06 |
| FR-25 | [R3-변경] Dust shall remain detectable after `Boost` cleaning in all scenarios, so revisited dust can trigger cleaning again. | UC-06, VS-01 |
| FR-26 | [R4-검증] The return-to-start scenario shall run for enough ticks to allow RVC to return to its starting position. | VS-01 |

## 6. Non-Functional Requirements

| ID | Requirement |
| --- | --- |
| NFR-01 | Controller logic shall be independent from concrete hardware and simulator classes. |
| NFR-02 | Controller shall be testable through deterministic inputs without real hardware. |
| NFR-03 | The project shall build with CMake and C++20. |
| NFR-04 | Unit tests shall be written with GoogleTest. |
| NFR-05 | [변경] System tests shall exercise the `Rvc` system through `GridSimulator` and `SimulatedHardwareAdapter`. |
| NFR-06 | Sensor and actuator abstractions shall allow future sensor changes or additions. |
| NFR-07 | CLI simulator output shall be human-readable for manual review. |
| NFR-08 | Core control decisions shall be deterministic for repeatable tests. |
| NFR-09 | Source files and documents shall be encoded in UTF-8. |
| NFR-10 | The design shall follow SOLID principles where applicable. |

## 7. Core Control Rules

1. 청소가 시작되면 RVC는 기본적으로 cleaner를 켠 상태로 전진한다.
2. 전방 센서는 interrupt 방식이며, 전방 장애물이 감지되면 즉시 전진을 멈추고 회피 판단으로 진입한다.
3. [R2-변경] 좌측 센서와 먼지 센서는 periodic 방식이며 제어 tick마다 최신 값을 갱신한다. [R2-삭제] ~~우측 센서는 periodic 방식으로 갱신한다.~~
4. [R2-변경] 전방 장애물이 있고 좌측이 열려 있으면 cleaner를 끄고 `TurnLeft`로 회피한 뒤 전진 청소를 재개할 때 cleaner를 다시 켠다.
5. [R2-변경] 전방 장애물이 있고 좌측이 막혀 있으면 cleaner를 끄고 `TurnRight`로 우측 탐색을 수행한다. 탐색 후 전방 interrupt가 없으면 기존 우측 방향이 열린 것으로 판단한다.
6. [R2-삭제] ~~전방 장애물이 있고 좌/우가 모두 열려 있으면 기본 회전 정책에 따라 방향을 선택한다. 기본값은 좌우 번갈아 선택이다.~~
7. [R2-변경] 우측 탐색 후 전방 interrupt가 다시 발생하면 `TurnLeft`로 원래 진행 방향을 복구한 뒤 `Escaping` 상태로 진입한다.
8. [R2-변경] `Escaping` 상태에서는 `Backward`를 한 번 수행하고, 좌측이 계속 막혀 있으면 다시 우측 탐색을 수행한다.
9. [R2-변경] 탈출 가능 조건은 좌측이 열렸거나 우측 탐색 결과 기존 우측 방향이 열린 상태로 정의한다.
10. [R2-기존] 먼지가 감지되면 일정 tick 동안 청소 세기 상태를 높이고, 시간이 끝나면 기본 세기 상태로 복귀한다.
11. [R2-기존] dust boost 상태가 회피/탈출 중 유지되더라도 실제 cleaner 출력은 `Backward`, `TurnLeft`, `TurnRight` 동안 `Off`가 우선한다.
12. [R3-변경] R3 요구사항을 적용하는 경우 10~11번의 fixed-duration boost와 movement-time cleaner off 정책은 대체된다.
13. [R3-변경] 청소 중 기본 cleaner 출력은 모든 주행/회피/탈출 상태에서 `Normal`이며, `Boost`는 먼지 감지로 필요한 RVC 제자리 회전 구간에만 사용한다.
14. [R3-변경] 먼지 감지 시 주행 방향은 전진/후진으로 toggle된다. 전진 중에는 RVC가 시계 방향 제자리 회전 후 후진으로, 후진 중에는 RVC가 반시계 방향 제자리 회전 후 전진으로 전환한다.
15. [R3-변경] `Boost` 동작은 먼지 셀을 제거하지 않는다. 이미 지나간 먼지라도 모든 시나리오에서 다시 만나면 다시 먼지 감지/청소 판단 대상이 된다.
16. [R3-추가] 후방 센서는 interrupt 방식이며, 후진 중 후방 장애물이 감지되면 즉시 후진을 멈추고 후진 주행 기준 장애물 회피 정책으로 진입한다.
17. [R3-추가] 후진 중 후방 장애물을 만나고 좌측이 열려 있으면 RVC는 `TurnRight` 후 후진을 재개한다. 이는 후진 주행 방향을 기존 좌측 열린 공간으로 바꾸기 위한 동작이다.
18. [R3-추가] 후진 중 후방 장애물을 만나고 좌측이 막혀 있으면 RVC는 `TurnLeft`로 기존 우측 방향을 후방 센서로 탐색한다. 탐색 결과가 열림이면 후진을 재개하고, 막힘이면 `TurnRight`로 원래 진행 방향을 복구한 뒤 전진 주행 모드로 전환한다.
19. [R4-검증] 시작 위치 복귀 테스트는 RVC가 시작 위치로 복귀할 수 있을 만큼 충분한 tick 동안 진행한다.

## 8. R3/R4 Additional Requirements and Policy Conflicts

### 8.1 Added Requirement Summary

| ID | Type | Requirement Summary | Requirement Session Placement |
| --- | --- | --- | --- |
| R3-01 | Sensor | 후방 장애물 interrupt 센서를 추가한다. | Actor, sensor input, backward safety requirement |
| R3-02 | Cleaner policy | 전원이 켜진 청소 중에는 모든 주행/회피/탈출 상태에서 기본 청소 상태를 `Normal`로 유지한다. | Cleaner output policy |
| R3-03 | Dust boost policy | 먼지 감지 시에만 필요한 제자리 회전 동안 `Boost`를 사용한다. | Dust handling policy |
| R3-04 | Direction toggle | 먼지를 만날 때마다 전진/후진 주행 방향을 toggle한다. | Movement state policy |
| R3-05 | Rotation direction | RVC는 전진 중 먼지 감지 시 시계 방향, 후진 중 먼지 감지 시 반시계 방향으로 제자리 회전한다. | Dust handling policy |
| R3-06 | Backward avoidance | 후진 중 후방 obstacle interrupt가 발생하면 후진 주행 기준 회피 정책을 수행한다. | Backward travel obstacle policy |
| R3-07 | Persistent dust | `Boost` 청소 후에도 먼지는 사라지지 않으며, 모든 시나리오에서 다시 청소 대상이 된다. | Simulator/test data policy |
| R4-01 | Scenario duration | 시작 위치로 복귀할 수 있을 만큼 충분한 tick 동안 진행한다. | Verification support |

### 8.2 Conflict Against Existing Policy

| Existing Policy | R3/R4 Requirement | Conflict | Resolution |
| --- | --- | --- | --- |
| FR-14/FR-15: 먼지 감지 시 설정된 tick 동안 `Boost`를 유지하고 만료 후 `Normal`로 복귀한다. | R3-03: 먼지 감지 시 필요한 제자리 회전 구간에만 `Boost`를 사용한다. | `Boost` 지속 기준이 fixed tick에서 required rotation duration으로 바뀐다. | R3 적용 시 FR-14/FR-15는 R3-03, FR-21~FR-23으로 대체한다. |
| Core Rule 11: `Backward`, `TurnLeft`, `TurnRight` 동안 cleaner `Off`가 우선한다. | R3-02/R3-03: 청소 중 기본은 `Normal`, 먼지 제자리 회전 중만 `Boost`다. | 회전/후진 중 cleaner를 끄는 기존 정책과 항상 청소 상태 유지 정책이 충돌한다. | R3 적용 시 이동/회피/후진 중 cleaner는 `Normal`을 유지하고, 먼지 회전 중만 `Boost`로 올린다. |
| 기존 탈출 정책: `Backward`는 막힌 영역 탈출 시 사용하는 보조 동작이다. | R3-04/R3-06: 후진은 일반 주행 모드가 되며, 후진 중 후방 장애물도 회피 대상이다. | 후진이 예외 동작에서 일반 주행 모드 중 하나로 확장된다. | controller 상태에 travel direction을 별도 보관하고, 전/후진 모두 정상 주행으로 처리한다. 후진 중 rear interrupt가 발생하면 UC-05 정책을 수행한다. |
| 기존 센서 정책: 전방 interrupt, 좌측/dust periodic, 우측 센서 삭제 및 우측은 전방 센서로 탐색한다. | R3-01: 후방 장애물 interrupt 센서가 추가된다. | 기존 센서 입력 구조에는 후방 센서가 없다. | `Rear Sensor` interrupt 입력을 추가하고, 후진 주행 중 travel-direction obstacle event로 처리한다. |
| 기존 시뮬레이터 청소 의미: 먼지 셀은 청소 결과로 제거될 수 있다. | R3-07: `Boost`로 청소해도 먼지는 사라지지 않는다. | 동일 먼지를 재방문했을 때 재감지 여부가 달라진다. | 모든 시나리오에서 먼지를 persistent fixture로 취급해 제거하지 않고, 재방문 시 다시 감지되도록 한다. |
| 기존 검증 기준: 경로 회피/탈출과 먼지 청소 로그 확인이 중심이다. | R4-01: 시작 위치 복귀를 확인할 만큼 충분한 tick 동안 진행한다. | 테스트 실행 tick 수가 짧으면 복귀 가능성을 판단할 수 없다. | 시나리오 tick 수를 시작 위치 복귀가 가능한 수준으로 설정하고, 최종 위치 또는 로그로 복귀 여부를 검토한다. |

## 9. Future or Extended Requirements

다음 항목은 `docs/rvc.pdf`의 future requirement로 식별되지만, 현재 버전의 기능 요구사항에는 포함하지 않는다.

- 센서가 추가되거나 기존 센서가 변경될 수 있다.
- RVC가 한 지점을 일정 시간 순환 청소할 수 있다.
- RVC가 모바일 앱과 통신할 수 있다.
- RVC가 더 효율적인 청소를 위해 machine learning과 inference를 사용할 수 있다.
