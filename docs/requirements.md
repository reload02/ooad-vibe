# RVC 요구사항

## 1. 범위

이 문서는 RVC 자동 청소 기능의 use case, 기능 요구사항, 비기능 요구사항을 정리한다. 실제 하드웨어 driver는 범위 밖이며, `GridSimulator`는 검증 환경으로서 격자 map, 로봇 위치/방향, 먼지 상태를 소유한다.

## 2. Use Cases

| ID | Name | Main Flow |
| --- | --- | --- |
| UC-01 | Start Automatic Cleaning | 사용자가 청소 시작을 요청하면 RVC는 running 상태가 되고 clear path에서 전진 청소 command를 낸다. |
| UC-02 | Stop Automatic Cleaning | 사용자가 청소 중지를 요청하면 RVC는 idle 상태가 되고 `Stop/Off` command를 낸다. |
| UC-03 | Avoid Front Obstacle | [변경] front sensor interrupt가 들어오면 좌측이 열려 있는 경우 좌회전하고, 좌측이 막힌 경우 후진 탈출 절차로 들어간다. |
| UC-04 | Escape From Blocked Area | [변경] 전방 interrupt와 좌측 막힘이 함께 발생하면 RVC는 `Escaping` 상태로 들어가 후진, 우측 probe 회전, 전방 interrupt 평가를 tick 단위로 수행한다. |
| UC-05 | Boost Cleaning On Dust | dust sensor가 먼지를 감지하면 설정된 tick 동안 boost power 후보를 유지한다. |
| UC-06 | Simulate Environment | simulator는 sensor 값을 생성하고 RVC command를 격자 위치, 방향, 먼지 상태에 적용한다. |

## 3. Functional Requirements

| ID | Requirement | Related Context |
| --- | --- | --- |
| FR-01 | RVC shall start automatic cleaning when requested. | UC-01 |
| FR-02 | RVC shall stop motion and cleaning when cleaning is stopped. | UC-02 |
| FR-03 | RVC shall command forward motion while the front path is clear. | UC-01 |
| FR-04 | Front obstacle detection shall be handled as an interrupt event. | UC-03 |
| [변경] FR-05 | Left and dust sensors shall be sampled periodically on control tick. | UC-03, UC-05 |
| [삭제] FR-05-old | Left, right, and dust sensors shall be sampled periodically on control tick. | 우측 주기 센서 제거 |
| [변경] FR-06 | If front interrupt is received and left side is open, RVC shall command left turn. | UC-03 |
| [삭제] FR-07-old | If front is blocked and only right is open, RVC shall command right turn from periodic right sensor input. | 우측 주기 센서 제거 |
| [삭제] FR-08-old | If front is blocked and both sides are open, RVC shall alternate left and right turns. | 좌우 모두 열림 교대 정책 제거 |
| [변경] FR-09 | If front interrupt is received and left side is blocked, RVC shall enter `Escaping`. | UC-04 |
| [변경] FR-10 | In `Escaping`, RVC shall issue `Backward`, then `TurnRight`, then evaluate the next front interrupt tick as the right probe result. | UC-04 |
| [신규] FR-11 | If the right probe tick has no front interrupt, RVC shall treat the right exit as open and command `Forward`. | UC-04 |
| [신규] FR-12 | If the right probe tick has a front interrupt, RVC shall command `TurnLeft` to restore heading and repeat the escape cycle. | UC-04 |
| [신규] FR-13 | Each `Backward`, `TurnRight`, `TurnLeft`, and `Forward` command consumes exactly one control tick. | UC-04 |
| FR-14 | If dust is detected, RVC shall keep boost candidate power for configured ticks. | UC-05 |
| FR-15 | RVC shall return to normal candidate power when boost expires. | UC-05 |
| FR-16 | Cleaner output shall be `Off` for turn, backward, stop, and idle motions. | UC-03, UC-04 |
| FR-17 | GridSimulator shall own grid position, direction, obstacle state, and dust state. | UC-06 |
| [변경] FR-18 | GridSimulator logs shall include front interrupt, left periodic, dust periodic, motion, cleaner, position, direction, and reason; `rightPeriodic` is no longer logged. | UC-06 |
| [삭제] FR-18-old | GridSimulator logs included `rightPeriodic`. | 우측 주기 센서 제거 |

## 4. Non-Functional Requirements

| ID | Requirement |
| --- | --- |
| NFR-01 | RVC internals shall follow SOLID where applicable. |
| NFR-02 | Sensor fusion, navigation, cleaning policy, and command composition shall be separately testable. |
| NFR-03 | RVC control logic shall be independent from concrete simulator and hardware drivers. |
| NFR-04 | Tests shall run deterministically without real hardware. |
| NFR-05 | The project shall build with CMake and C++20. |
| NFR-06 | Documents and source files shall be UTF-8. |
