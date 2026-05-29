# RVC Software Requirements Specification

## 1. Scope

현재 버전은 실제 하드웨어 driver를 포함하지 않는다. sensor 입력은 값 객체와 interrupt API로 추상화하고, motor/cleaner actuator 결과는 `Command`로 표현한다. 격자 좌표와 방향은 simulator 환경 상태로 둔다.

## 2. Actors And External Interfaces

| Actor | Description |
| --- | --- |
| User | 자동 청소 시작/중지를 요청한다. |
| Front Sensor | 전방 장애물을 interrupt 방식으로 알린다. clear 값은 제공하지 않으며, interrupt가 없으면 clear로 간주한다. |
| Left Sensor | 좌측 장애물 상태를 control tick마다 제공한다. |
| [삭제] Right Sensor | 우측 장애물 상태를 control tick마다 제공하던 주기 센서. 현재 요구에서는 제거되었다. |
| Dust Sensor | 현재 위치의 먼지 감지 상태를 control tick마다 제공한다. |
| Motor | `Command.motion` 요청을 수행한다. |
| Cleaner | `Command.cleaningPower` 요청을 수행한다. |
| GridSimulator | 실제 환경과 actuator 결과를 대체하는 검증 환경이다. |

## 3. Functional Requirements

| ID | Requirement |
| --- | --- |
| FR-01 | RVC shall start automatic cleaning when requested. |
| FR-02 | RVC shall stop motor and cleaner when cleaning is stopped. |
| FR-03 | RVC shall move forward with normal cleaning power when the front path is clear. |
| FR-04 | Front obstacle detection shall be handled as an interrupt event. |
| FR-05 | RVC shall stop forward cleaning and enter avoidance or escape handling when a front obstacle interrupt is received. |
| [변경] FR-06 | Left and dust sensor values shall be sampled periodically on control tick. |
| [삭제] FR-06-old | Left, right, and dust sensor values shall be sampled periodically on control tick. |
| [변경] FR-07 | If front interrupt is received and left side is open, RVC shall turn left. |
| [삭제] FR-08-old | If front is blocked and only right is open, RVC shall turn right using right periodic sensor input. |
| [삭제] FR-09-old | If front is blocked and both sides are open, RVC shall alternate left and right turn choices. |
| [변경] FR-10 | If front interrupt is received and left side is blocked, RVC shall enter `Escaping` state. |
| [변경] FR-11 | In `Escaping`, RVC shall back up before probing the right side. |
| [신규] FR-12 | RVC shall probe the right side by issuing `TurnRight` and evaluating the next tick's front interrupt. |
| [신규] FR-13 | If the right probe tick has no front interrupt, RVC shall keep the probed heading and move `Forward`. |
| [신규] FR-14 | If the right probe tick has a front interrupt, RVC shall issue `TurnLeft` to restore the original heading and continue escaping. |
| [신규] FR-15 | Each motion command consumes one tick, including probe turns and restore turns. |
| FR-16 | If dust is detected, RVC shall keep boost cleaning power for the configured number of ticks. |
| FR-17 | If boost duration expires and no new dust is detected, RVC shall return to normal cleaning power. |
| FR-18 | RVC shall output cleaner `Off` while turning, moving backward, or stopping. |
| FR-19 | GridSimulator shall own grid map, robot grid position, grid direction, obstacle state, and dust state. |
| [변경] FR-20 | CLI simulator shall preserve the scenario format and human-readable logs, except that `rightPeriodic` is removed from log lines. |

## 4. Core Data And Operations

| Element | Requirement Role |
| --- | --- |
| `PeriodicSensorData` | [변경] 주기 입력은 `leftObstacle`, `dustDetected`만 포함한다. |
| `SensorSnapshot` | [변경] 판단 snapshot은 `frontObstacle`, `leftObstacle`, `dustDetected`만 포함한다. |
| `SensorFusion` | front interrupt와 periodic sensor data를 `SensorSnapshot`으로 결합한다. |
| `NavigationPolicy` | [변경] 좌측 회피와 후진 우측 probe 상태머신을 관리한다. |
| `GridSimulator` | [변경] 우측 주기 센서 샘플링 없이 현재 전방 interrupt, 좌측 주기 센서, 먼지 센서를 제공한다. |

## 5. Traceability

| Requirement | Design Element | Verification |
| --- | --- | --- |
| FR-01 to FR-05 | `Rvc`, `RvcController`, `SensorFusion` | controller and facade tests |
| [변경] FR-06 to FR-15 | `NavigationPolicy` | navigation, controller, scenario tests |
| FR-16 to FR-18 | `CleaningPolicy`, `CommandComposer` | subsystem and controller tests |
| [변경] FR-19 to FR-20 | `GridSimulator`, CLI | system, scenario, CLI tests |
