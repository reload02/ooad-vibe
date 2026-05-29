# RVC Software Requirements Specification

## 1. Scope

이 문서는 RVC(Robotic Vacuum Cleaner) 자동 청소 기능의 요구사항을 정의한다. 대상은 실제 로봇 전체를 나타내는 `Rvc` facade와 그 내부 제어 subsystem, 그리고 이를 검증하기 위한 `GridSimulator`이다.

현재 버전은 실제 하드웨어 driver를 포함하지 않는다. 대신 sensor 입력은 값 객체와 interrupt API로 추상화하고, motor/cleaner actuator 결과는 `Command`로 표현한다. 격자 좌표와 방향은 실제 로봇 내부 상태가 아니라 simulator 환경 상태로 둔다.

## 2. Actors And External Interfaces

| Actor | Description |
| --- | --- |
| User | 자동 청소 시작/중지를 요청한다. |
| Front Sensor | 전방 장애물을 interrupt 방식으로 알린다. |
| Left Sensor | 좌측 장애물 상태를 control tick마다 제공한다. |
| Right Sensor | 우측 장애물 상태를 control tick마다 제공한다. |
| Dust Sensor | 현재 위치의 먼지 감지 상태를 control tick마다 제공한다. |
| Motor | `Command.motion` 요청을 수행하는 이동 actuator이다. |
| Cleaner | `Command.cleaningPower` 요청을 수행하는 청소 actuator이다. |
| GridSimulator | 실제 환경과 actuator 결과를 대체하는 검증 환경이다. |

## 3. Functional Requirements

| ID | Requirement |
| --- | --- |
| FR-01 | RVC shall start automatic cleaning when requested. |
| FR-02 | RVC shall stop motor and cleaner when cleaning is stopped. |
| FR-03 | RVC shall move forward with normal cleaning power when the front path is clear. |
| FR-04 | Front obstacle detection shall be handled as an interrupt event. |
| FR-05 | RVC shall stop forward cleaning and enter avoidance when a front obstacle interrupt is received. |
| FR-06 | Left, right, and dust sensor values shall be sampled periodically on control tick. |
| FR-07 | If front is blocked and only left is open, RVC shall turn left. |
| FR-08 | If front is blocked and only right is open, RVC shall turn right. |
| FR-09 | If front is blocked and both sides are open, RVC shall alternate left and right turn choices. |
| FR-10 | If front, left, and right are all blocked, RVC shall enter `Escaping` state. |
| FR-11 | In `Escaping` state, RVC shall keep commanding backward motion while both sides are blocked. |
| FR-12 | Escape shall be considered possible only when at least one side is open. |
| FR-13 | After escape becomes possible, RVC shall turn toward an open side. |
| FR-14 | If dust is detected, RVC shall keep boost cleaning power for the configured number of ticks. |
| FR-15 | If boost duration expires and no new dust is detected, RVC shall return to normal cleaning power. |
| FR-16 | RVC shall output cleaner `Off` while turning, moving backward, or stopping. |
| FR-17 | GridSimulator shall own grid map, robot grid position, grid direction, obstacle state, and dust state. |
| FR-18 | GridSimulator shall apply RVC `Command` results to its environment state without moving that state into RVC. |
| FR-19 | CLI simulator shall preserve the existing scenario format, options, and human-readable logs. |

## 4. Non-Functional Requirements

| ID | Requirement |
| --- | --- |
| NFR-01 | RVC internal design shall follow SOLID where applicable. |
| NFR-02 | Sensor fusion, navigation, cleaning policy, and command composition shall have separate responsibilities. |
| NFR-03 | Controller logic shall not depend on concrete simulator or hardware driver classes. |
| NFR-04 | Tests shall be deterministic and runnable without real hardware. |
| NFR-05 | The project shall build with CMake and C++20. |
| NFR-06 | Markdown documents and source files shall use UTF-8 encoding. |

## 5. Core Data And Operations

| Element | Requirement Role |
| --- | --- |
| `Rvc` | 실제 로봇 전체 facade. start, stop, interrupt, tick API를 제공한다. |
| `SensorFusion` | front interrupt와 periodic sensor data를 `SensorSnapshot`으로 결합한다. |
| `NavigationPolicy` | snapshot과 controller state를 바탕으로 `Motion`을 결정한다. |
| `CleaningPolicy` | dust boost tick 예산과 청소 세기 후보를 관리한다. |
| `CommandComposer` | motion과 cleaning power 후보를 최종 `Command`로 변환한다. |
| `GridSimulator` | 환경 상태를 보관하고 `Command`를 적용한다. |

## 6. Traceability

| Requirement | Design Element | Verification |
| --- | --- | --- |
| FR-01 to FR-03 | `Rvc`, `RvcController`, `NavigationPolicy` | controller and facade tests |
| FR-04 to FR-06 | `SensorFusion`, `RvcController::tick` | sensor fusion and controller tests |
| FR-07 to FR-13 | `NavigationPolicy` | navigation, controller, scenario tests |
| FR-14 to FR-16 | `CleaningPolicy`, `CommandComposer` | subsystem and controller tests |
| FR-17 to FR-19 | `GridSimulator`, CLI | system, scenario, CLI tests |
