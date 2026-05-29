# RVC Requirements

## 1. Scope

이 문서는 RVC 자동 청소 기능의 유스케이스, 기능 요구사항, 비기능 요구사항을 정리한다. RVC는 실제 로봇 전체를 나타내는 facade이며, 내부에는 감지 입력 결합, 이동 판단, 청소 세기 판단, command 조립 책임이 분리되어 있다.

실제 하드웨어 driver는 현재 범위에 포함하지 않는다. `GridSimulator`는 검증 환경으로서 격자 map, 로봇의 격자 위치와 방향, 먼지 상태를 소유한다. RVC는 위치와 방향을 직접 갖지 않고, sensor 입력을 받아 actuator 요청인 `Command`를 반환한다.

## 2. Use Cases

| ID | Name | Main Flow |
| --- | --- | --- |
| UC-01 | Start Automatic Cleaning | 사용자가 청소 시작을 요청하면 RVC는 running 상태가 되고 clear path에서 전진 청소 command를 낸다. |
| UC-02 | Stop Automatic Cleaning | 사용자가 청소 중지를 요청하면 RVC는 idle 상태가 되고 `Stop/Off` command를 낸다. |
| UC-03 | Avoid Front Obstacle | front sensor interrupt가 들어오면 RVC는 전진 청소를 멈추고 열린 좌/우 방향으로 회전한다. |
| UC-04 | Escape From Blocked Area | 전방, 좌측, 우측이 모두 막히면 RVC는 `Escaping` 상태로 들어가 측면이 열릴 때까지 후진한다. |
| UC-05 | Boost Cleaning On Dust | dust sensor가 먼지를 감지하면 설정된 tick 동안 boost power 후보를 유지한다. |
| UC-06 | Simulate Environment | simulator는 sensor 값을 생성하고 RVC command를 격자 위치, 방향, 먼지 상태에 적용한다. |

## 3. Functional Requirements

| ID | Requirement | Related Context |
| --- | --- | --- |
| FR-01 | RVC shall start automatic cleaning when requested. | UC-01 |
| FR-02 | RVC shall stop motion and cleaning when cleaning is stopped. | UC-02 |
| FR-03 | RVC shall command forward motion while the front path is clear. | UC-01 |
| FR-04 | Front obstacle detection shall be handled as an interrupt event. | UC-03 |
| FR-05 | Left, right, and dust sensors shall be sampled periodically on control tick. | UC-03, UC-05 |
| FR-06 | If front is blocked and only left is open, RVC shall command left turn. | UC-03 |
| FR-07 | If front is blocked and only right is open, RVC shall command right turn. | UC-03 |
| FR-08 | If front is blocked and both sides are open, RVC shall alternate left and right turns. | UC-03 |
| FR-09 | If front, left, and right are all blocked, RVC shall enter `Escaping`. | UC-04 |
| FR-10 | In `Escaping`, RVC shall keep commanding backward motion while both sides are blocked. | UC-04 |
| FR-11 | RVC shall leave `Escaping` only when at least one side is open. | UC-04 |
| FR-12 | If dust is detected, RVC shall keep boost candidate power for configured ticks. | UC-05 |
| FR-13 | RVC shall return to normal candidate power when boost expires. | UC-05 |
| FR-14 | Cleaner output shall be `Off` for turn, backward, stop, and idle motions. | UC-03, UC-04 |
| FR-15 | GridSimulator shall own grid position, direction, obstacle state, and dust state. | UC-06 |
| FR-16 | GridSimulator shall preserve existing CLI scenario execution and log output. | UC-06 |

## 4. Non-Functional Requirements

| ID | Requirement |
| --- | --- |
| NFR-01 | RVC internals shall follow SOLID where applicable. |
| NFR-02 | Sensor fusion, navigation, cleaning policy, and command composition shall be separately testable. |
| NFR-03 | RVC control logic shall be independent from concrete simulator and hardware drivers. |
| NFR-04 | Tests shall run deterministically without real hardware. |
| NFR-05 | The project shall build with CMake and C++20. |
| NFR-06 | Documents and source files shall be UTF-8. |
