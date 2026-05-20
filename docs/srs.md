# RVC Control SW Software Requirements Specification

## 1. 문서 개요

### 1.1 목적

이 문서는 RVC(Robotic Vacuum Cleaner) 자동 청소 제어 소프트웨어의 Software Requirements Specification(SRS)을 정의한다. 본 SRS는 `docs/rvc.pdf`의 RVC Control SW 요구사항과 현재 프로젝트의 OOAD 산출물 및 C++20 구현을 기준으로 작성되며, 기능 요구사항, 비기능 요구사항, 외부 인터페이스, 상태와 데이터 정의, 검증 기준, 요구사항 추적성을 포함한다.

### 1.2 적용 범위

본 SRS의 적용 대상은 RVC 자동 청소 제어 로직과 이를 검증하기 위한 CLI 그리드 시뮬레이터이다. 제어 소프트웨어는 추상화된 센서 입력을 받아 이동 및 청소 명령을 생성한다. 실제 모터 드라이버, 실제 센서 하드웨어, 전원 관리, 배터리 충전, 사용자 앱 또는 네트워크 기능은 본 문서의 범위에 포함하지 않는다.

### 1.3 문서 상태

| 항목 | 내용 |
| --- | --- |
| 문서명 | RVC Control SW Software Requirements Specification |
| 대상 시스템 | RVC 자동 청소 제어 소프트웨어 및 CLI 시뮬레이터 |
| 문서 형식 | Markdown |
| 인코딩 | UTF-8 |
| 기준 구현 언어 | C++20 |
| 기준 빌드 시스템 | CMake |

## 2. 참조 문서

| 문서 | 설명 |
| --- | --- |
| `docs/rvc.pdf` | 원본 RVC Control SW 요구사항 |
| `docs/requirements.md` | 유스케이스, 기능 요구사항, 비기능 요구사항, 핵심 제어 규칙 |
| `docs/ooa_ssd.md` | OOA System Sequence Diagram 및 System Interface |
| `docs/ooa_domain_diagram.md` | OOA Domain Diagram |
| `docs/ood_sequence_diagrams.md` | OOD Sequence Diagram |
| `docs/ood_class_diagram.md` | OOD Class Diagram 및 SOLID 분석 |
| `docs/traceability.md` | 요구사항, 설계, 테스트 추적성 |
| `include/rvc/*.hpp`, `src/*.cpp` | 현재 C++20 구현 |
| `tests/*.cpp` | GoogleTest 기반 단위 및 시스템 테스트 |

## 3. 용어 정의

| 용어 | 정의 |
| --- | --- |
| RVC | Robotic Vacuum Cleaner, 자동 청소 로봇 |
| Control SW | 센서 입력을 해석해 이동 및 청소 명령을 결정하는 제어 소프트웨어 |
| Controller | `RvcController`로 구현되는 핵심 제어 객체 |
| Simulator | `GridSimulator`로 구현되는 그리드 기반 검증 환경 |
| Tick | periodic 센서 값을 읽고 다음 명령을 결정하는 제어 주기 |
| Interrupt | 전방 장애물처럼 즉시 반응해야 하는 비동기 이벤트 |
| Periodic Sensor | tick마다 주기적으로 샘플링되는 센서 |
| Front Sensor | 전방 장애물 감지 센서이며 interrupt 방식으로 동작 |
| Left Sensor | 좌측 장애물 감지 센서이며 periodic 방식으로 동작 |
| Right Sensor | 우측 장애물 감지 센서이며 periodic 방식으로 동작 |
| Dust Sensor | 현재 위치의 먼지 감지 센서이며 periodic 방식으로 동작 |
| Cleaner | 일반 청소 또는 강화 청소를 수행하는 청소 장치 |
| Boost | 먼지 감지 후 일정 tick 동안 유지되는 강화 청소 세기 |
| Escaping | 전방, 좌측, 우측이 모두 막혔을 때 탈출을 위해 후진하는 상태 |

## 4. 시스템 개요

### 4.1 시스템 목적

RVC Control SW는 자동 청소 중 센서 이벤트와 주기적 센서 값을 기반으로 충돌을 회피하고, 먼지를 감지하면 청소 세기를 높이며, 막힌 공간에서는 탈출 가능한 상태가 될 때까지 후진 명령을 유지해야 한다. 제어 로직은 실제 하드웨어와 분리되어 테스트 가능한 형태로 제공되어야 한다.

### 4.2 주요 기능

- 사용자의 자동 청소 시작 및 중지 요청을 처리한다.
- 전방 장애물 interrupt를 수신하면 즉시 회피 판단으로 전환한다.
- 좌측, 우측, 먼지 센서 값을 tick마다 주기적으로 샘플링한다.
- 전방 장애물 상황에서 열린 좌측 또는 우측 방향으로 회전한다.
- 좌우가 모두 열린 경우 좌우 회전 방향을 번갈아 선택한다.
- 전방, 좌측, 우측이 모두 막힌 경우 `Escaping` 상태로 전환하고 후진 명령을 지속한다.
- 먼지 감지 시 설정된 tick 동안 청소 세기를 `Boost`로 유지한다.
- CLI 시뮬레이터를 통해 지도, 센서, 명령, 위치, 방향, 청소 결과를 검증한다.

### 4.3 사용자 및 외부 주체

| 주체 | 역할 |
| --- | --- |
| User | 자동 청소 시작 또는 중지를 요청한다. |
| Tester | CLI 시뮬레이터와 테스트 코드를 통해 제어 동작을 검증한다. |
| Front Sensor | 전방 장애물을 interrupt 이벤트로 알린다. |
| Left Sensor | 좌측 장애물 상태를 주기적으로 제공한다. |
| Right Sensor | 우측 장애물 상태를 주기적으로 제공한다. |
| Dust Sensor | 먼지 감지 상태를 주기적으로 제공한다. |
| Digital Clock | tick 기반 제어 주기를 제공한다. |
| Motor | `Forward`, `Backward`, `TurnLeft`, `TurnRight`, `Stop` 명령을 수행한다. |
| Cleaner | `Off`, `Normal`, `Boost` 청소 세기를 수행한다. |

## 5. 운영 환경

### 5.1 소프트웨어 환경

| 항목 | 요구사항 |
| --- | --- |
| 언어 | C++20 |
| 빌드 | CMake 3.24 이상 |
| 테스트 | CTest 및 GoogleTest |
| 실행 파일 | `rvc_simulator` |
| 문서 인코딩 | UTF-8 |

### 5.2 시뮬레이터 환경

시뮬레이터는 2차원 문자 그리드에서 RVC의 위치, 방향, 장애물, 먼지를 표현한다. 시뮬레이터는 실제 하드웨어를 대체하는 검증 환경이며, 컨트롤러와 동일한 제어 인터페이스를 사용해야 한다.

## 6. 제약사항과 가정

### 6.1 제약사항

- 전방 센서는 interrupt 방식으로 처리해야 한다.
- 좌측 센서, 우측 센서, 먼지 센서는 periodic 방식으로 처리해야 한다.
- 핵심 제어 로직은 하드웨어와 직접 결합되지 않아야 한다.
- 컨트롤러의 판단은 동일 입력에 대해 결정적이어야 한다.
- 프로젝트는 CMake와 C++20로 빌드되어야 한다.
- 문서와 소스 파일은 UTF-8 인코딩을 사용해야 한다.

### 6.2 가정

- 사용자의 자동 청소 시작 요청은 `startCleaning()` 호출로 추상화한다.
- 사용자의 자동 청소 중지 요청은 `stopCleaning()` 호출로 추상화한다.
- 전방 장애물 interrupt는 `onFrontObstacleInterrupt()` 호출로 추상화한다.
- tick마다 좌측, 우측, 먼지 센서 값이 `PeriodicSensorData`로 제공된다.
- 실제 하드웨어의 물리적 지연, 모터 가속도, 센서 노이즈, 배터리 상태는 본 SRS의 범위 밖이다.
- 시뮬레이터에서 지도 밖 영역은 장애물로 간주한다.

## 7. 외부 인터페이스 요구사항

### 7.1 사용자 제어 인터페이스

| 인터페이스 | 설명 | 기대 결과 |
| --- | --- | --- |
| 청소 시작 | 사용자가 자동 청소를 시작한다. | 컨트롤러가 `Cleaning` 상태가 되고 cleaner가 켜진 전진 청소 명령을 생성한다. |
| 청소 중지 | 사용자가 자동 청소를 중지한다. | 컨트롤러가 `Idle` 상태가 되고 motor는 정지, cleaner는 off가 된다. |

### 7.2 센서 입력 인터페이스

| 입력 | 방식 | 데이터 | 설명 |
| --- | --- | --- | --- |
| 전방 장애물 | Interrupt | `frontObstacle` | 전방 장애물이 감지되면 즉시 컨트롤러에 이벤트를 전달한다. |
| 좌측 장애물 | Periodic | `leftObstacle` | tick마다 좌측 방향의 장애물 여부를 제공한다. |
| 우측 장애물 | Periodic | `rightObstacle` | tick마다 우측 방향의 장애물 여부를 제공한다. |
| 먼지 감지 | Periodic | `dustDetected` | tick마다 현재 위치의 먼지 감지 여부를 제공한다. |

### 7.3 액추에이터 출력 인터페이스

컨트롤러는 매 tick마다 `Command`를 생성한다.

| 출력 | 값 | 설명 |
| --- | --- | --- |
| `motion` | `None`, `Stop`, `Forward`, `Backward`, `TurnLeft`, `TurnRight` | 모터 이동 또는 회전 명령 |
| `cleaningPower` | `Off`, `Normal`, `Boost` | 청소 장치 세기 |
| `reason` | 문자열 | 명령 결정 사유를 설명하는 디버그 및 로그용 메시지 |

### 7.4 소프트웨어 API 인터페이스

| API | 설명 |
| --- | --- |
| `RvcController::startCleaning()` | 자동 청소를 시작한다. |
| `RvcController::stopCleaning()` | 자동 청소를 중지하고 boost 잔여 tick을 초기화한다. |
| `RvcController::onFrontObstacleInterrupt()` | 실행 중 전방 장애물 interrupt를 기록한다. |
| `RvcController::tick(const PeriodicSensorData&)` | periodic 센서 값을 받아 다음 명령을 반환한다. |
| `RvcController::readPeriodicSensors(const PeriodicSensorData&)` | 전방 interrupt 상태와 periodic 센서 값을 하나의 snapshot으로 결합한다. |
| `RvcController::decideNextCommand(const SensorSnapshot&)` | 센서 snapshot을 기반으로 다음 명령을 결정한다. |
| `GridSimulator::run(int, bool)` | 지정된 tick 수만큼 시뮬레이션을 실행한다. |
| `GridSimulator::loadScenario(const std::filesystem::path&)` | 시나리오 파일을 읽어 tick 수와 지도 정보를 구성한다. |

### 7.5 CLI 시뮬레이터 인터페이스

실행 형식은 다음과 같다.

```powershell
rvc_simulator [--ticks N] [--scenario FILE] [--quiet-map]
```

| 옵션 | 설명 |
| --- | --- |
| `--help`, `-h` | 사용법을 출력하고 종료한다. |
| `--ticks N` | 실행할 제어 tick 수를 지정한다. |
| `--scenario FILE` | 지정한 시나리오 파일을 로드한다. |
| `--quiet-map` | 초기/최종 지도와 tick별 frame 출력을 생략하고 로그 중심으로 출력한다. |

### 7.6 시나리오 파일 형식

시나리오 파일은 선택적 tick 설정과 지도 섹션으로 구성된다.

```text
ticks=10
map:
##########
#..*.....#
#.>..*#..#
##########
```

| 문자 | 의미 |
| --- | --- |
| `#` | 장애물 |
| `.` | 빈 칸 |
| `*` | 먼지 |
| `R` | 북쪽을 바라보는 로봇 |
| `^` | 북쪽을 바라보는 로봇 |
| `>` | 동쪽을 바라보는 로봇 |
| `v` | 남쪽을 바라보는 로봇 |
| `<` | 서쪽을 바라보는 로봇 |

시나리오 지도에는 로봇이 정확히 하나 있어야 한다. 지도 섹션이 없거나 로봇이 없거나 로봇이 둘 이상이면 오류로 처리한다.

### 7.7 로그 출력 형식

시뮬레이터는 tick마다 다음 정보를 포함하는 로그를 출력한다.

| 필드 | 설명 |
| --- | --- |
| `tick` | 현재 제어 tick 번호 |
| `frontInterrupt` | 전방 장애물 interrupt 발생 여부 |
| `leftPeriodic` | 좌측 periodic 센서 상태 |
| `rightPeriodic` | 우측 periodic 센서 상태 |
| `dustPeriodic` | 먼지 periodic 센서 상태 |
| `motion` | 컨트롤러가 생성한 이동 명령 |
| `cleaner` | 컨트롤러가 생성한 청소 세기 |
| `position` | 명령 적용 후 로봇 위치 |
| `direction` | 명령 적용 후 로봇 방향 |
| `cleaned` | 누적 청소 먼지 수 |
| `reason` | 명령 결정 사유 |

실행 종료 시 summary는 실행 tick 수, 청소된 먼지 수, 남은 먼지 수, 최종 위치, 최종 방향을 포함해야 한다.

## 8. 주요 유스케이스

### UC-01 자동 청소 시작

| 항목 | 내용 |
| --- | --- |
| 주요 주체 | User |
| 목표 | RVC가 자동 청소를 시작한다. |
| 사전 조건 | 제어 소프트웨어가 초기화되어 있다. |
| 기본 흐름 | 사용자가 청소 시작을 요청하면 시스템은 `Cleaning` 상태로 전환하고 cleaner를 켠 상태로 전진 청소를 시작한다. |
| 사후 조건 | RVC는 자동 청소 상태가 된다. |

### UC-02 자동 청소 중지

| 항목 | 내용 |
| --- | --- |
| 주요 주체 | User |
| 목표 | RVC가 자동 청소를 중지한다. |
| 사전 조건 | RVC가 자동 청소 중이다. |
| 기본 흐름 | 사용자가 청소 중지를 요청하면 시스템은 motor를 정지하고 cleaner를 끈다. |
| 사후 조건 | RVC는 `Idle` 상태가 된다. |

### UC-03 전방 장애물 회피

| 항목 | 내용 |
| --- | --- |
| 주요 주체 | Front Sensor |
| 목표 | 전방 장애물 감지 시 충돌 없이 회피한다. |
| 사전 조건 | RVC가 자동 청소 중이다. |
| 기본 흐름 | 전방 센서가 interrupt를 발생시키면 시스템은 즉시 전진을 중단하고 좌측/우측 periodic 센서 값을 기준으로 열린 방향을 선택한다. |
| 대안 흐름 | 좌우가 모두 열려 있으면 이전 선택과 반대 방향을 선택하여 좌우를 번갈아 회전한다. |
| 사후 조건 | RVC는 장애물을 회피하고 청소를 계속한다. |

### UC-04 막힌 영역 탈출

| 항목 | 내용 |
| --- | --- |
| 주요 주체 | Front Sensor, Left Sensor, Right Sensor |
| 목표 | 전방, 좌측, 우측이 모두 막힌 상황에서 탈출 가능한 상태까지 이동한다. |
| 사전 조건 | RVC가 자동 청소 중이며 전방, 좌측, 우측이 모두 막혀 있다. |
| 기본 흐름 | 시스템은 `Escaping` 상태로 전환하고 전방, 좌측, 우측 중 하나 이상이 열릴 때까지 후진 명령을 지속한다. |
| 사후 조건 | 탈출 가능해지면 청소 상태로 복귀하거나 열린 방향으로 회전한다. |

### UC-05 먼지 감지 시 강화 청소

| 항목 | 내용 |
| --- | --- |
| 주요 주체 | Dust Sensor |
| 목표 | 먼지 감지 구간에서 청소 세기를 높인다. |
| 사전 조건 | RVC가 자동 청소 중이다. |
| 기본 흐름 | 먼지 센서가 먼지를 감지하면 시스템은 cleaner를 `Boost`로 설정하고 설정된 tick 동안 유지한다. |
| 사후 조건 | boost 시간이 만료되고 새 먼지가 감지되지 않으면 cleaner는 `Normal`로 복귀한다. |

### UC-06 시뮬레이터 시나리오 실행

| 항목 | 내용 |
| --- | --- |
| 주요 주체 | Tester |
| 목표 | CLI 시뮬레이터로 제어 소프트웨어의 주요 동작을 검증한다. |
| 사전 조건 | 기본 지도 또는 시나리오 파일이 준비되어 있다. |
| 기본 흐름 | Tester가 시뮬레이터를 실행하면 시스템은 tick마다 센서, 명령, 위치, 방향, 청소 세기를 로그로 출력한다. |
| 사후 조건 | Tester는 로그와 summary를 통해 요구사항 충족 여부를 검토할 수 있다. |

## 9. 기능 요구사항

| ID | 요구사항 | 수용 기준 | 관련 유스케이스 |
| --- | --- | --- | --- |
| FR-01 | System shall start automatic cleaning when requested. | `startCleaning()` 호출 후 컨트롤러는 실행 중 상태가 되고 다음 tick에서 청소 명령을 생성한다. | UC-01 |
| FR-02 | System shall stop motor and cleaner when cleaning is stopped. | `stopCleaning()` 호출 후 tick 결과는 `motion=Stop`, `cleaningPower=Off`가 된다. | UC-02 |
| FR-03 | System shall move forward while cleaning when no obstacle blocks the front direction. | 전방 interrupt가 없으면 `motion=Forward`, `cleaningPower=Normal` 또는 `Boost`를 반환한다. | UC-01 |
| FR-04 | Front obstacle detection shall be handled as an interrupt event. | 전방 장애물은 periodic 입력이 아니라 `onFrontObstacleInterrupt()` 이벤트로 컨트롤러에 전달된다. | UC-03 |
| FR-05 | System shall stop forward motion immediately when a front obstacle interrupt is received. | interrupt가 pending 상태인 tick에서 컨트롤러는 `Forward` 대신 회피, 회전, 또는 후진 명령을 반환한다. | UC-03 |
| FR-06 | Left, right, and dust sensors shall be sampled periodically on control tick. | `tick()`은 `PeriodicSensorData`의 좌측, 우측, 먼지 값을 사용하여 명령을 결정한다. | UC-03, UC-05 |
| FR-07 | If front is blocked and only left is open, System shall turn left. | `frontObstacle=true`, `leftObstacle=false`, `rightObstacle=true`이면 `motion=TurnLeft`를 반환한다. | UC-03 |
| FR-08 | If front is blocked and only right is open, System shall turn right. | `frontObstacle=true`, `leftObstacle=true`, `rightObstacle=false`이면 `motion=TurnRight`를 반환한다. | UC-03 |
| FR-09 | If front is blocked and both left and right are open, System shall choose turn direction by alternating left and right. | 동일 조건이 반복되면 첫 선택은 좌회전, 다음 선택은 우회전처럼 방향을 번갈아 반환한다. | UC-03 |
| FR-10 | If front, left, and right are all blocked, System shall enter `Escaping` state. | 세 방향이 모두 막힌 tick 이후 컨트롤러 상태는 `Escaping`이 된다. | UC-04 |
| FR-11 | In `Escaping` state, System shall keep moving backward until escape is possible. | `Escaping` 상태에서 세 방향이 계속 막혀 있으면 매 tick `motion=Backward`를 반환한다. | UC-04 |
| FR-12 | Escape shall be considered possible when at least one of front, left, and right is open. | `Escaping` 상태에서 세 방향 중 하나 이상이 열리면 탈출 가능 상태로 판단한다. | UC-04 |
| FR-13 | After escape becomes possible, System shall turn toward an open side or move forward if front is open. | 전방이 열리면 `Forward`, 전방은 막히고 한쪽 측면이 열리면 열린 방향 회전 명령을 반환한다. | UC-04 |
| FR-14 | If dust is detected, System shall set cleaner power to boost for a configured number of ticks. | `dustDetected=true`인 tick에서 `cleaningPower=Boost`가 되고 `dustBoostTicks` 동안 유지된다. | UC-05 |
| FR-15 | If boost duration expires and no new dust is detected, System shall return cleaner power to normal. | 먼지가 새로 감지되지 않고 boost 잔여 tick이 끝나면 `cleaningPower=Normal`로 복귀한다. | UC-05 |
| FR-16 | Simulator shall render a grid map with robot, obstacle, dust, and empty cells. | 지도 출력은 로봇 방향 문자, `#`, `.`, `*`를 사용해 현재 상태를 표현한다. | UC-06 |
| FR-17 | Simulator shall log tick, sensor values, command, robot position, direction, and cleaning power. | 각 tick 로그는 센서 상태, motion, cleaner, position, direction, cleaned, reason을 포함한다. | UC-06 |
| FR-18 | Simulator shall use the same controller interface as the production control SW. | 시뮬레이터는 `RvcController`에 interrupt와 periodic 데이터를 전달하고 반환된 `Command`를 적용한다. | UC-06 |

## 10. 비기능 요구사항

| ID | 요구사항 | 수용 기준 |
| --- | --- | --- |
| NFR-01 | Controller logic shall be independent from concrete hardware and simulator classes. | `RvcController`는 하드웨어 또는 `GridSimulator` 구체 타입에 직접 의존하지 않는다. |
| NFR-02 | Controller shall be testable through deterministic inputs without real hardware. | 단위 테스트에서 `PeriodicSensorData`와 interrupt 호출만으로 동작을 검증할 수 있다. |
| NFR-03 | The project shall build with CMake and C++20. | `cmake -S . -B build` 및 CMake build 구성을 지원한다. |
| NFR-04 | Unit tests shall be written with GoogleTest. | 컨트롤러 단위 테스트는 GoogleTest 기반으로 작성된다. |
| NFR-05 | System tests shall exercise the controller through the simulator. | 시스템 테스트는 `GridSimulator`를 통해 컨트롤러 명령과 결과를 검증한다. |
| NFR-06 | Sensor and actuator abstractions shall allow future sensor changes or additions. | 센서 입력과 명령 출력은 구조체와 enum으로 추상화되어 있다. |
| NFR-07 | CLI simulator output shall be human-readable for manual review. | 로그는 key-value 형식과 readable reason 문자열을 포함한다. |
| NFR-08 | Core control decisions shall be deterministic for repeatable tests. | 같은 초기 상태와 같은 입력 순서는 같은 명령 순서를 생성한다. |
| NFR-09 | Source files and documents shall be encoded in UTF-8. | 한글 문서가 깨지지 않아야 하며 UTF-8로 저장되어야 한다. |
| NFR-10 | The design shall follow SOLID principles where applicable. | 제어 로직, 시뮬레이션, 타입 정의가 역할별로 분리되어야 한다. |

## 11. 상태 및 데이터 정의

### 11.1 컨트롤러 상태

| 상태 | 설명 | 진입 조건 | 이탈 조건 |
| --- | --- | --- | --- |
| `Idle` | 청소하지 않는 대기 상태 | 초기 상태 또는 청소 중지 요청 | 청소 시작 요청 |
| `Cleaning` | 기본 자동 청소 상태 | 청소 시작, 전방이 열린 상태, 탈출 완료 | 전방 interrupt, 청소 중지 |
| `Avoiding` | 전방 장애물 회피를 위해 회전하는 상태 | 전방 interrupt 후 열린 좌/우 방향 존재 | 다음 tick에서 전방 상태에 따라 청소 또는 탈출 |
| `Escaping` | 세 방향 막힘 상황에서 후진하는 상태 | 전방, 좌측, 우측이 모두 막힘 | 세 방향 중 하나 이상 열림 또는 청소 중지 |

### 11.2 핵심 데이터 타입

| 타입 | 필드 또는 값 | 설명 |
| --- | --- | --- |
| `PeriodicSensorData` | `leftObstacle`, `rightObstacle`, `dustDetected` | tick마다 샘플링되는 센서 입력 |
| `SensorSnapshot` | `frontObstacle`, `leftObstacle`, `rightObstacle`, `dustDetected` | interrupt와 periodic 센서를 결합한 판단용 snapshot |
| `Command` | `motion`, `cleaningPower`, `reason` | 컨트롤러가 생성하는 액추에이터 명령 |
| `Motion` | `None`, `Stop`, `Forward`, `Backward`, `TurnLeft`, `TurnRight` | 이동 또는 회전 명령 종류 |
| `CleaningPower` | `Off`, `Normal`, `Boost` | cleaner 출력 세기 |
| `ControllerState` | `Idle`, `Cleaning`, `Avoiding`, `Escaping` | 컨트롤러 내부 상태 |
| `Position` | `row`, `col` | 시뮬레이터 내 로봇 좌표 |
| `Direction` | `North`, `East`, `South`, `West` | 시뮬레이터 내 로봇 방향 |

### 11.3 제어 규칙

1. 청소가 시작되면 RVC는 cleaner를 켠 상태로 전진 청소를 수행한다.
2. 전방 장애물이 감지되면 interrupt 이벤트로 처리하고 해당 tick에서 전진 명령을 내리지 않는다.
3. 좌측, 우측, 먼지 센서는 tick마다 periodic 데이터로 반영한다.
4. 전방이 막히고 좌측만 열려 있으면 좌회전한다.
5. 전방이 막히고 우측만 열려 있으면 우회전한다.
6. 전방이 막히고 좌측과 우측이 모두 열려 있으면 좌우를 번갈아 선택한다.
7. 전방, 좌측, 우측이 모두 막히면 `Escaping` 상태로 전환한다.
8. `Escaping` 상태에서는 탈출 가능 조건이 될 때까지 후진 명령을 지속한다.
9. 탈출 가능 조건은 전방, 좌측, 우측 중 하나 이상이 열린 상태이다.
10. 탈출 가능해진 뒤 전방이 열려 있으면 전진하고, 전방이 막힌 경우 열린 측면 방향으로 회전한다.
11. 먼지가 감지되면 `dustBoostTicks`만큼 `Boost`를 유지한다.
12. boost 유지 시간이 끝나고 새 먼지가 감지되지 않으면 `Normal`로 복귀한다.

## 12. 검증 기준

### 12.1 빌드 및 테스트 명령

프로젝트 검증은 다음 명령을 기준으로 수행할 수 있다.

```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

### 12.2 단위 테스트 기준

| 테스트 | 검증 요구사항 |
| --- | --- |
| `ControllerMovesForwardWhenPathIsClear` | FR-01, FR-03 |
| `FrontInterruptTriggersImmediateAvoidance` | FR-04, FR-05 |
| `TurnsTowardOpenSide` | FR-07, FR-08 |
| `AlternatesWhenBothSidesAreOpen` | FR-09 |
| `AllBlockedEntersEscapingAndKeepsBackingUp` | FR-10, FR-11 |
| `EscapingExitsWhenFrontBecomesOpen` | FR-12, FR-13 |
| `DustBoostLastsConfiguredTicks` | FR-14, FR-15 |

### 12.3 시스템 테스트 기준

| 테스트 | 검증 요구사항 |
| --- | --- |
| `SimulatorCleansDustAndLogsCommands` | FR-14, FR-15, FR-17 |
| `SimulatorUsesBackwardEscape` | FR-10, FR-11, FR-18 |
| `SimulatorKeepsCommandingBackwardWhenBoxedIn` | FR-11 |
| `SimulatorTurnsAfterFrontInterrupt` | FR-04, FR-05, FR-08 |
| `SimulatorCliDefaultRuns` | FR-16, FR-17 |
| `SimulatorCliContinuousBackwardScenarioRuns` | FR-10, FR-11, FR-16, FR-17 |

### 12.4 수동 검토 기준

- 시뮬레이터 로그에서 전방 장애물 interrupt와 회피 명령이 같은 tick 흐름에서 확인되어야 한다.
- 먼지가 있는 칸에서 `dustPeriodic=detected`와 `cleaner=Boost`가 확인되어야 한다.
- 세 방향이 막힌 지도에서는 `motion=Backward`가 반복되어야 한다.
- `--quiet-map` 옵션을 사용하면 지도 frame 출력 없이 로그와 summary 중심으로 확인할 수 있어야 한다.
- 문서의 요구사항 ID와 테스트 추적성이 `docs/traceability.md`와 모순되지 않아야 한다.

## 13. 요구사항 추적성

### 13.1 요구사항과 설계 요소

| 요구사항 | 유스케이스 | 주요 설계/구현 요소 |
| --- | --- | --- |
| FR-01 | UC-01 | `RvcController::startCleaning`, `ControllerState::Cleaning` |
| FR-02 | UC-02 | `RvcController::stopCleaning`, `Motion::Stop`, `CleaningPower::Off` |
| FR-03 | UC-01 | `RvcController::tick`, `Motion::Forward` |
| FR-04 | UC-03 | `RvcController::onFrontObstacleInterrupt` |
| FR-05 | UC-03 | `RvcController::decideNextCommand`, `Motion::TurnLeft`, `Motion::TurnRight`, `Motion::Backward` |
| FR-06 | UC-03, UC-05 | `PeriodicSensorData`, `SensorSnapshot` |
| FR-07 | UC-03 | `RvcController::chooseOpenSideTurn`, `Motion::TurnLeft` |
| FR-08 | UC-03 | `RvcController::chooseOpenSideTurn`, `Motion::TurnRight` |
| FR-09 | UC-03 | `preferLeftTurn_`, `RvcController::chooseOpenSideTurn` |
| FR-10 | UC-04 | `ControllerState::Escaping` |
| FR-11 | UC-04 | `Motion::Backward`, `ControllerState::Escaping` |
| FR-12 | UC-04 | `SensorSnapshot`, all-blocked 판단 |
| FR-13 | UC-04 | `RvcController::decideNextCommand`, 탈출 후 명령 결정 |
| FR-14 | UC-05 | `RvcController::updateCleaningPower`, `ControllerConfig::dustBoostTicks` |
| FR-15 | UC-05 | `boostTicksRemaining_`, `CleaningPower::Normal` |
| FR-16 | UC-06 | `GridSimulator::render`, scenario map symbols |
| FR-17 | UC-06 | `GridSimulator::makeLogLine`, `SimulationResult::logs` |
| FR-18 | UC-06 | `GridSimulator`, `RvcController`, `Command` |

### 13.2 요구사항과 테스트

| 요구사항 | 테스트 |
| --- | --- |
| FR-01, FR-03 | `ControllerMovesForwardWhenPathIsClear` |
| FR-04, FR-05 | `FrontInterruptTriggersImmediateAvoidance`, `SimulatorTurnsAfterFrontInterrupt` |
| FR-07, FR-08 | `TurnsTowardOpenSide`, `SimulatorTurnsAfterFrontInterrupt` |
| FR-09 | `AlternatesWhenBothSidesAreOpen` |
| FR-10, FR-11 | `AllBlockedEntersEscapingAndKeepsBackingUp`, `SimulatorUsesBackwardEscape`, `SimulatorKeepsCommandingBackwardWhenBoxedIn` |
| FR-12, FR-13 | `EscapingExitsWhenFrontBecomesOpen` |
| FR-14, FR-15 | `DustBoostLastsConfiguredTicks`, `SimulatorCleansDustAndLogsCommands` |
| FR-16, FR-17, FR-18 | `SimulatorCleansDustAndLogsCommands`, `SimulatorCliDefaultRuns`, `SimulatorCliContinuousBackwardScenarioRuns` |

## 14. 변경 영향

본 SRS 작성은 문서 추가 작업이며 C++ 공개 API, 구현 코드, 테스트 코드, 빌드 설정을 변경하지 않는다. 따라서 런타임 동작 변경, 호환성 변경, 마이그레이션 작업은 발생하지 않는다.
