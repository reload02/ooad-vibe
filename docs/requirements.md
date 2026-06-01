# RVC Control SW Requirements

## 1. Scope

이 문서는 RVC(Robotic Vacuum Cleaner) 자동 청소 제어 소프트웨어의 유스케이스, 기능 요구사항, 비기능 요구사항을 정의한다. 요구사항의 기준은 `docs/rvc.pdf`의 RVC Control SW 설명이며, RVC는 household surface를 자동으로 청소하고 물걸레질하는 장치로 본다. 현재 제어 SW 범위에서는 별도 mop actuator를 만들지 않고 `Cleaner`가 청소/물걸레질 출력을 대표하는 추상 actuator 역할을 한다.

다음 추가 조건을 포함한다.

- 전방 센서는 interrupt 방식으로 동작한다.
- [R2-변경] 좌측 센서와 먼지 센서는 periodic 방식으로 동작한다.
- [R2-삭제] ~~우측 센서는 periodic 방식으로 동작한다.~~
- [R2-변경] 우측 장애물 판단은 우측 센서값이 아니라 RVC가 우측으로 90도 회전한 뒤 전방 센서 interrupt로 확인한다.
- [R2-변경] 전방과 좌측이 막히고 우측 탐색 결과도 막힌 경우 RVC는 원래 진행 방향으로 자세를 복구한 뒤 후진한다.
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

### UC-05 Boost Cleaning On Dust

| Item | Description |
| --- | --- |
| Primary Actor | Dust Sensor |
| Goal | 먼지를 감지하면 일정 시간 청소 세기를 높인다. |
| Precondition | RVC가 자동 청소 중이다. |
| Main Flow | Dust Sensor가 periodic sampling에서 먼지를 감지한다. System은 cleaner를 boost power로 설정한다. 설정된 tick 동안 boost를 유지한 뒤 normal power로 복귀한다. |
| Postcondition | 먼지 구간에서 강화 청소가 수행된다. |

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
| FR-06 | [R2-변경] Left and dust sensors shall be sampled periodically on control tick. [R2-삭제] ~~Right sensor shall be sampled periodically.~~ | UC-03, UC-05 |
| FR-07 | If front is blocked and only left is open, System shall turn left. | UC-03 |
| FR-08 | [R2-변경] If front is blocked and left is blocked, System shall turn right to probe the former right direction with the front sensor. | UC-03 |
| FR-09 | [R2-삭제] ~~If front is blocked and both left and right are open, System shall choose turn direction by alternating left and right.~~ | UC-03 |
| FR-10 | [R2-변경] If front is blocked, left is blocked, and right probing also reports blocked, System shall restore the original heading and enter `Escaping`. | UC-04 |
| FR-11 | [R2-변경] In `Escaping` state, System shall execute one `Backward` command, then probe right again when left remains blocked. | UC-04 |
| FR-12 | [R2-변경] Escape shall be considered possible when left is open or right probing confirms that the former right direction is open. | UC-04 |
| FR-13 | [R2-변경] After escape becomes possible, System shall turn toward the confirmed open direction or continue forward after successful right probing. | UC-04 |
| FR-14 | If dust is detected, System shall set cleaner power to boost for a configured number of ticks. | UC-05 |
| FR-15 | If boost duration expires and no new dust is detected, System shall return cleaner power to normal. | UC-05 |
| FR-16 | Simulator shall render a grid map with robot, obstacle, dust, and empty cells. | VS-01 |
| FR-17 | Simulator shall log tick, sensor values, command, robot position, direction, and cleaning power. | VS-01 |
| FR-18 | [변경] Simulator shall exercise the same `RvcHardwareAdapter` contract as the production control SW. | VS-01 |

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
10. 먼지가 감지되면 일정 tick 동안 청소 세기 상태를 높이고, 시간이 끝나면 기본 세기 상태로 복귀한다.
11. dust boost 상태가 회피/탈출 중 유지되더라도 실제 cleaner 출력은 `Backward`, `TurnLeft`, `TurnRight` 동안 `Off`가 우선한다.

## 7. Future or Extended Requirements

다음 항목은 `docs/rvc.pdf`의 future requirement로 식별되지만, 현재 버전의 기능 요구사항에는 포함하지 않는다.

- 센서가 추가되거나 기존 센서가 변경될 수 있다.
- RVC가 한 지점을 일정 시간 순환 청소할 수 있다.
- RVC가 모바일 앱과 통신할 수 있다.
- RVC가 더 효율적인 청소를 위해 machine learning과 inference를 사용할 수 있다.
