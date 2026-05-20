# RVC Control SW Requirements

## 1. Scope

이 문서는 RVC(Robotic Vacuum Cleaner) 자동 청소 제어 소프트웨어의 유스케이스, 기능 요구사항, 비기능 요구사항을 정의한다. 요구사항의 기준은 `docs/rvc.pdf`의 RVC Control SW 설명이며, RVC는 household surface를 자동으로 청소하고 물걸레질하는 장치로 본다. 현재 제어 SW 범위에서는 별도 mop actuator를 만들지 않고 `Cleaner`가 청소/물걸레질 출력을 대표하는 추상 actuator 역할을 한다.

다음 추가 조건을 포함한다.

- 전방 센서는 interrupt 방식으로 동작한다.
- 좌측 센서, 우측 센서, 먼지 센서는 periodic 방식으로 동작한다.
- 전방, 좌측, 우측이 모두 막힌 경우 RVC는 좌/우 중 한쪽이 열릴 때까지 계속 후진한다.
- 하드웨어의 상세 제어 설계는 범위 밖이며, 제어 소프트웨어는 추상화된 센서 입력과 동작 명령을 다룬다.

## 2. Actors

| Actor | Description |
| --- | --- |
| User | RVC의 자동 청소를 시작하거나 중지하는 사용자 |
| Front Sensor | 전방 장애물을 interrupt로 알리는 센서 |
| Left Sensor | 좌측 장애물 상태를 주기적으로 제공하는 센서 |
| Right Sensor | 우측 장애물 상태를 주기적으로 제공하는 센서 |
| Dust Sensor | 먼지 감지 상태를 주기적으로 제공하는 센서 |
| Digital Clock | 제어 tick을 제공하는 주기적 시간 소스 |
| Motor | 이동, 정지, 회전, 후진 명령을 수행하는 하드웨어 |
| Cleaner | 일반 청소, 강화 청소, 물걸레질 출력을 대표하는 추상 하드웨어 |

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
| Main Flow | Front Sensor가 interrupt를 발생시킨다. System은 즉시 전진 청소를 멈추고 cleaner를 끈 뒤 좌/우 periodic sensor 값을 확인한다. 열린 방향이 있으면 cleaner off 상태로 해당 방향으로 회전한 뒤, 전진 청소를 재개할 때 cleaner를 다시 켠다. |
| Alternative Flow | 좌/우가 모두 열려 있으면 기본 회전 정책에 따라 좌우를 번갈아 선택한다. |
| Postcondition | RVC는 장애물을 회피하고 청소를 계속한다. |

### UC-04 Escape From Blocked Area

| Item | Description |
| --- | --- |
| Primary Actor | Front Sensor, Left Sensor, Right Sensor |
| Goal | 전방, 좌측, 우측이 모두 막힌 상황에서 탈출한다. |
| Precondition | RVC가 자동 청소 중이며 삼방향이 모두 막혀 있다. |
| Main Flow | System은 cleaner를 끄고 `Escaping` 상태로 전환한다. 좌측 또는 우측이 열릴 때까지 cleaner off 상태로 계속 후진한다. 탈출 가능해지면 cleaner off 상태로 열린 방향으로 회전하고, 전진 청소를 재개할 때 cleaner를 다시 켠다. 후진 직후 전방이 열리더라도 좌/우가 모두 막혀 있으면 탈출 가능 상태로 보지 않는다. |
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

시뮬레이터는 RVC의 사용자 또는 센서 중심 유스케이스가 아니라 제어 소프트웨어 구현을 검증하기 위한 지원 기능으로 분리한다.

| ID | Item | Description |
| --- | --- | --- |
| VS-01 | CLI simulator scenario execution | 기본 맵 또는 시나리오 맵으로 controller 동작을 실행하고 tick별 sensor/event, command, robot position, direction, cleaning power 로그를 제공한다. |

## 5. Functional Requirements

| ID | Requirement | Related Context |
| --- | --- | --- |
| FR-01 | System shall start automatic cleaning when requested. | UC-01 |
| FR-02 | System shall stop motor and cleaner when cleaning is stopped. | UC-02 |
| FR-03 | System shall move forward while cleaning when no obstacle blocks the front direction. | UC-01 |
| FR-04 | Front obstacle detection shall be handled as an interrupt event. | UC-03 |
| FR-05 | System shall stop forward motion immediately when a front obstacle interrupt is received. | UC-03 |
| FR-06 | Left, right, and dust sensors shall be sampled periodically on control tick. | UC-03, UC-05 |
| FR-07 | If front is blocked and only left is open, System shall turn left. | UC-03 |
| FR-08 | If front is blocked and only right is open, System shall turn right. | UC-03 |
| FR-09 | If front is blocked and both left and right are open, System shall choose turn direction by alternating left and right. | UC-03 |
| FR-10 | If front, left, and right are all blocked, System shall enter `Escaping` state. | UC-04 |
| FR-11 | In `Escaping` state, System shall keep moving backward while both left and right are blocked. | UC-04 |
| FR-12 | Escape shall be considered possible only when at least one of left or right is open. | UC-04 |
| FR-13 | After escape becomes possible, System shall turn toward an open side. | UC-04 |
| FR-14 | If dust is detected, System shall set cleaner power to boost for a configured number of ticks. | UC-05 |
| FR-15 | If boost duration expires and no new dust is detected, System shall return cleaner power to normal. | UC-05 |
| FR-16 | Simulator shall render a grid map with robot, obstacle, dust, and empty cells. | VS-01 |
| FR-17 | Simulator shall log tick, sensor values, command, robot position, direction, and cleaning power. | VS-01 |
| FR-18 | Simulator shall use the same controller interface as the production control SW. | VS-01 |

## 6. Non-Functional Requirements

| ID | Requirement |
| --- | --- |
| NFR-01 | Controller logic shall be independent from concrete hardware and simulator classes. |
| NFR-02 | Controller shall be testable through deterministic inputs without real hardware. |
| NFR-03 | The project shall build with CMake and C++20. |
| NFR-04 | Unit tests shall be written with GoogleTest. |
| NFR-05 | System tests shall exercise the controller through the simulator. |
| NFR-06 | Sensor and actuator abstractions shall allow future sensor changes or additions. |
| NFR-07 | CLI simulator output shall be human-readable for manual review. |
| NFR-08 | Core control decisions shall be deterministic for repeatable tests. |
| NFR-09 | Source files and documents shall be encoded in UTF-8. |
| NFR-10 | The design shall follow SOLID principles where applicable. |

## 7. Core Control Rules

1. 청소가 시작되면 RVC는 기본적으로 cleaner를 켠 상태로 전진한다.
2. 전방 센서는 interrupt 방식이며, 전방 장애물이 감지되면 즉시 전진을 멈추고 회피 판단으로 진입한다.
3. 좌측 센서, 우측 센서, 먼지 센서는 periodic 방식이며 제어 tick마다 최신 값을 갱신한다.
4. 전방 장애물이 있고 좌/우 중 한쪽만 열려 있으면 cleaner를 끄고 열린 방향으로 회전한 뒤 전진 청소를 재개할 때 cleaner를 다시 켠다.
5. 전방 장애물이 있고 좌/우가 모두 열려 있으면 기본 회전 정책에 따라 방향을 선택한다. 기본값은 좌우 번갈아 선택이다.
6. 전방, 좌측, 우측이 모두 막혀 있으면 cleaner를 끄고 `Escaping` 상태로 진입한다.
7. `Escaping` 상태에서는 좌/우 중 한쪽이 열릴 때까지 cleaner off 상태로 계속 후진한다.
8. 탈출 가능 조건은 좌측 또는 우측 중 하나 이상이 열린 상태로 정의한다. 전방은 후진 직후 자연스럽게 열릴 수 있으므로 `Escaping` 상태의 탈출 판단에 사용하지 않는다.
9. 탈출 가능해지면 cleaner off 상태로 열린 방향으로 회전하고 전진 청소를 재개할 때 cleaner를 다시 켠다.
10. 먼지가 감지되면 일정 tick 동안 청소 세기 상태를 높이고, 시간이 끝나면 기본 세기 상태로 복귀한다.
11. dust boost 상태가 회피/탈출 중 유지되더라도 실제 cleaner 출력은 `Backward`, `TurnLeft`, `TurnRight` 동안 `Off`가 우선한다.

## 7. Future or Extended Requirements

다음 항목은 `docs/rvc.pdf`의 future requirement로 식별되지만, 현재 버전의 기능 요구사항에는 포함하지 않는다.

- 센서가 추가되거나 기존 센서가 변경될 수 있다.
- RVC가 한 지점을 일정 시간 순환 청소할 수 있다.
- RVC가 모바일 앱과 통신할 수 있다.
- RVC가 더 효율적인 청소를 위해 machine learning과 inference를 사용할 수 있다.
