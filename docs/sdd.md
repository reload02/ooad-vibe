# RVC Control SW Software Design Description

본 문서는 IEEE Std 1016-2009의 SDD 구조를 기준으로 작성한다.

## 1. Scope

### 1.1 Purpose

이 문서는 RVC(Robotic Vacuum Cleaner) 자동 청소 제어 소프트웨어의 Software Design Description(SDD)이다. `docs/srs.md`에 정의된 요구사항을 현재 C++20 구현 구조와 연결하고, 설계 context, stakeholder concern, design viewpoint, design view, design rationale을 정리한다.

### 1.2 System Scope

설계 대상은 `RvcController` 기반 제어 로직과 `GridSimulator` 기반 CLI 시뮬레이터이다. controller는 sensor 입력을 판단해 motor motion과 cleaner power를 포함한 `Command`를 생성한다. simulator는 검증 환경으로서 scenario map에서 sensor 값을 만들고 command를 적용한다.

본 SDD의 범위에 포함하지 않는 항목은 실제 모터 드라이버, 실제 센서 하드웨어, 배터리, 네트워크, UI, 영구 저장소 설계이다.

### 1.3 Design Goals

| 목표 | 설명 |
| --- | --- |
| 요구사항 충족 | FR-01부터 FR-18까지의 자동 청소, 장애물 회피, 탈출, 먼지 boost, 시뮬레이터 검증 요구사항을 만족한다. |
| 결정적 동작 | 동일한 초기 상태와 동일한 센서 입력 순서에 대해 동일한 명령을 생성한다. |
| 하드웨어 독립성 | controller는 실제 하드웨어나 simulator 구현에 직접 의존하지 않고 값 객체와 command만 사용한다. |
| 테스트 가능성 | controller 단위 테스트와 simulator 시스템 테스트로 핵심 흐름을 검증한다. |
| 단순한 확장성 | sensor 입력과 actuator 명령을 구조체와 enum으로 분리하여 향후 sensor나 command 확장을 쉽게 한다. |

## 2. References

| 문서 | 설명 |
| --- | --- |
| IEEE Std 1016-2009 | Software Design Descriptions 작성 기준 |
| `docs/rvc.pdf` | 원본 RVC Control SW 요구사항 |
| `docs/srs.md` | Software Requirements Specification |
| `docs/requirements.md` | 유스케이스, FR, NFR, 핵심 제어 규칙 |
| `docs/ooa_domain_diagram.md` | OOA domain model |
| `docs/ooa_ssd.md` | System Sequence Diagram과 system operation |
| `docs/ood_class_diagram.md` | OOD class diagram과 SOLID 분석 |
| `docs/ood_sequence_diagrams.md` | OOD sequence diagram |
| `docs/traceability.md` | 요구사항, 설계, 테스트 추적성 |
| `include/rvc/*.hpp`, `src/*.cpp` | 현재 C++20 설계와 구현 |
| `tests/*.cpp` | GoogleTest 기반 단위/시스템 테스트 |

## 3. Terms and Definitions

| 용어 | 정의 |
| --- | --- |
| Design Context | 설계 대상 시스템의 경계, 외부 주체, 운영 환경을 설명하는 정보 |
| Design Stakeholder | 설계 결과에 이해관계가 있는 사용자, 검증자, 구현자 |
| Design Concern | stakeholder가 설계에서 확인해야 하는 관심사 |
| Design Viewpoint | 특정 concern을 다루기 위한 표현 규칙과 분석 관점 |
| Design View | viewpoint를 적용해 작성한 실제 설계 표현 |
| Controller | `RvcController`로 구현되는 핵심 제어 객체 |
| Simulator | `GridSimulator`로 구현되는 검증 환경 |
| Command | controller가 actuator 또는 simulator에 전달하는 추상 명령 |
| SensorSnapshot | pending front interrupt와 periodic sensor 값을 결합한 판단 입력 |
| Escaping | 삼방향이 막힌 상황에서 후진 탈출을 수행하는 controller 상태 |

## 4. Design Context

### 4.1 System Boundary

RVC Control SW는 sensor 입력을 읽고 actuator 명령을 결정하는 controller 중심 구조이다. 전방 장애물은 interrupt로 controller에 기록되고, 좌측/우측/먼지 sensor는 제어 tick마다 periodic 데이터로 전달된다. controller는 두 입력 흐름을 `SensorSnapshot`으로 결합한 뒤 `Command`를 생성한다.

```mermaid
flowchart LR
    User[User] -->|start/stop| Controller[RvcController]
    FrontSensor[Front Sensor] -->|interrupt| Controller
    LeftSensor[Left Sensor] --> PeriodicData[PeriodicSensorData]
    RightSensor[Right Sensor] --> PeriodicData
    DustSensor[Dust Sensor] --> PeriodicData
    Clock[Digital Clock] -->|tick| PeriodicData
    PeriodicData --> Controller
    Controller --> Snapshot[SensorSnapshot]
    Snapshot --> Controller
    Controller --> Command[Command]
    Command --> Motor[Motor Motion]
    Command --> Cleaner[Cleaner Power]
    GridSimulator[GridSimulator] -.test environment.-> FrontSensor
    GridSimulator -.test environment.-> PeriodicData
    GridSimulator -.applies.-> Command
```

### 4.2 External Actors and Environment

| 외부 요소 | 설계상 관계 |
| --- | --- |
| User | `startCleaning()`과 `stopCleaning()` 요청으로 controller 상태를 전환한다. |
| Front Sensor | `onFrontObstacleInterrupt()`로 전방 장애물 event를 전달한다. |
| Left Sensor | `PeriodicSensorData::leftObstacle`로 좌측 장애물 상태를 전달한다. |
| Right Sensor | `PeriodicSensorData::rightObstacle`로 우측 장애물 상태를 전달한다. |
| Dust Sensor | `PeriodicSensorData::dustDetected`로 먼지 감지 상태를 전달한다. |
| Digital Clock | `tick()` 호출 주기를 제공한다. |
| Motor | `Command::motion` 값을 수행한다. |
| Cleaner | `Command::cleaningPower` 값을 수행한다. |
| GridSimulator | 실제 하드웨어 대신 sensor/event를 만들고 command를 격자 상태에 적용한다. |

### 4.3 Design Constraints

- C++20과 CMake 기반으로 빌드한다.
- Controller는 concrete hardware 및 `GridSimulator` 세부 구현에 의존하지 않는다.
- Front sensor는 interrupt, left/right/dust sensor는 periodic 입력으로 분리한다.
- 출력은 motor motion과 cleaner power를 포함하는 `Command`로 통일한다.
- 테스트는 deterministic input sequence를 기반으로 반복 가능해야 한다.
- 모든 Markdown 문서는 UTF-8로 작성한다.

## 5. Design Stakeholders and Concerns

| Stakeholder | Concern | 설계 대응 |
| --- | --- | --- |
| User | 청소 시작, 중지, 장애물 회피, 먼지 청소가 요구사항대로 동작해야 한다. | controller 상태와 command 규칙을 명시적으로 정의한다. |
| Tester | 요구사항별 동작을 단위 및 시스템 테스트로 재현할 수 있어야 한다. | sensor 입력과 command 출력을 값 객체로 제공하고 simulator 로그를 남긴다. |
| Implementer | 제어 로직, simulator, 공용 타입의 책임 경계가 명확해야 한다. | `RvcController`, `GridSimulator`, core type을 모듈별로 분리한다. |
| Maintainer | sensor나 command 확장 시 영향 범위가 제한되어야 한다. | 입력 구조체와 enum 기반 command를 사용한다. |
| Reviewer | SRS 요구사항과 설계 요소, 테스트가 추적 가능해야 한다. | FR별 설계 요소와 테스트 매핑을 유지한다. |

## 6. Design Viewpoints

| Viewpoint | Concern | 주요 표현 |
| --- | --- | --- |
| Structural Viewpoint | 모듈과 클래스 책임 분리 | 모듈 표, class diagram, 책임 표 |
| Interface Viewpoint | public API와 입출력 계약 | operation table, command table |
| Data Viewpoint | 핵심 데이터 구조와 enum | data type table |
| Interaction Viewpoint | 객체 간 실행 흐름 | sequence diagram |
| State Viewpoint | controller 상태와 전이 | state diagram, 상태 표 |
| Algorithm Viewpoint | 회피, 탈출, boost 판단 규칙 | 절차 목록, 조건 표 |
| Error Handling Viewpoint | simulator 입력 오류와 controller 상태 처리 | 오류 처리 표 |
| Verification Viewpoint | 요구사항과 테스트 연결 | traceability table |

## 7. Design Views

### 7.1 Structural View

#### 7.1.1 Module Structure

| 모듈 | 주요 파일 | 책임 |
| --- | --- | --- |
| Core type | `include/rvc/Types.hpp`, `src/Types.cpp` | 방향, 동작, 청소 세기, 상태, 위치, sensor data, command type을 정의한다. |
| Controller | `include/rvc/RvcController.hpp`, `src/RvcController.cpp` | 자동 청소 상태와 sensor snapshot을 기반으로 다음 motion/cleaner command를 결정한다. |
| Simulator | `include/rvc/GridSimulator.hpp`, `src/GridSimulator.cpp` | 격자 환경에서 sensor 입력을 생성하고 command를 적용하며 로그와 실행 결과를 만든다. |
| CLI | `src/main.cpp` | simulator 실행 옵션을 파싱하고 초기/최종 지도, tick 로그, summary를 출력한다. |
| Tests | `tests/controller_tests.cpp`, `tests/system_tests.cpp` | controller 규칙과 simulator 통합 흐름을 검증한다. |

#### 7.1.2 OOD Class Diagram

```mermaid
classDiagram
    class RvcController {
        -ControllerConfig config
        -ControllerState state
        -bool running
        -bool frontInterruptPending
        -bool preferLeftTurn
        -int boostTicksRemaining
        +startCleaning()
        +stopCleaning()
        +onFrontObstacleInterrupt()
        +tick(PeriodicSensorData) Command
        +readPeriodicSensors(PeriodicSensorData) SensorSnapshot
        +decideNextCommand(SensorSnapshot) Command
    }

    class ControllerConfig {
        +int dustBoostTicks
    }

    class PeriodicSensorData {
        +bool leftObstacle
        +bool rightObstacle
        +bool dustDetected
    }

    class SensorSnapshot {
        +bool frontObstacle
        +bool leftObstacle
        +bool rightObstacle
        +bool dustDetected
    }

    class Command {
        +Motion motion
        +CleaningPower cleaningPower
        +string reason
    }

    class GridSimulator {
        -vector~string~ grid
        -Position robot
        -Direction direction
        -RvcController controller
        +fromLines(lines) GridSimulator
        +loadScenario(path) Scenario
        +defaultMap() vector~string~
        +run(maxTicks, includeFrames) SimulationResult
        +step(tick, includeFrame) bool
        +render() string
        +remainingDust() int
    }

    class Scenario {
        +vector~string~ mapLines
        +int ticks
    }

    class SimulationResult {
        +int ticksRun
        +int dustCleaned
        +Position finalPosition
        +Direction finalDirection
        +vector~string~ logs
    }

    class Position {
        +int row
        +int col
    }

    class Direction
    class Motion
    class CleaningPower
    class ControllerState

    RvcController --> ControllerConfig
    RvcController --> PeriodicSensorData
    RvcController --> SensorSnapshot
    RvcController --> Command
    GridSimulator *-- RvcController
    GridSimulator --> Scenario
    GridSimulator --> SimulationResult
    GridSimulator --> Position
```

#### 7.1.3 Class Responsibilities

| Class | Responsibility |
| --- | --- |
| `RvcController` | 문서의 system interface를 구현하고 핵심 제어 규칙에 따라 command를 결정한다. |
| `ControllerConfig` | boost duration 같은 제어 정책 값을 제공한다. |
| `PeriodicSensorData` | 좌측, 우측, 먼지 periodic sensor 값을 전달한다. |
| `SensorSnapshot` | pending front interrupt와 periodic sensor 값을 결합한 판단 입력이다. |
| `Command` | motor motion과 cleaner power를 함께 표현하는 추상 actuator 명령이다. |
| `GridSimulator` | 격자 환경에서 sensor/event를 만들고 controller command를 적용한다. |
| `Scenario` | 시나리오 파일에서 읽은 지도와 기본 tick 수를 담는다. |
| `SimulationResult` | 시스템 테스트와 CLI 출력에 필요한 실행 결과를 담는다. |

### 7.2 Interface View

#### 7.2.1 Controller Interface

| Operation | Input | Output | 책임 | 관련 요구사항 |
| --- | --- | --- | --- | --- |
| `RvcController::startCleaning()` | 없음 | 없음 | 자동 청소 실행 상태로 전환하고 전방 interrupt pending 값을 초기화한다. | FR-01, FR-03 |
| `RvcController::stopCleaning()` | 없음 | 없음 | idle 상태로 전환하고 motor stop/cleaner off 상태가 되도록 내부 실행 상태와 boost timer를 초기화한다. | FR-02 |
| `RvcController::onFrontObstacleInterrupt()` | 없음 | 없음 | 실행 중일 때 전방 장애물 interrupt를 pending 상태로 기록한다. | FR-04, FR-05 |
| `RvcController::tick(const PeriodicSensorData&)` | periodic sensor 값 | `Command` | 제어 tick마다 sensor 값을 반영하고 다음 command를 반환한다. | FR-06 to FR-15 |
| `RvcController::readPeriodicSensors(const PeriodicSensorData&)` | periodic sensor 값 | `SensorSnapshot` | pending front interrupt와 periodic 값을 결합한다. | FR-04, FR-06 |
| `RvcController::decideNextCommand(const SensorSnapshot&)` | sensor snapshot | `Command` | 상태, 장애물, 먼지 정보를 기반으로 motion과 cleaner power를 결정한다. | FR-07 to FR-15 |

#### 7.2.2 Simulator Interface

| Operation | Input | Output | 책임 | 관련 요구사항 |
| --- | --- | --- | --- | --- |
| `GridSimulator::loadScenario(const std::filesystem::path&)` | scenario path | `Scenario` | scenario file에서 tick 설정과 map을 읽는다. | FR-16 |
| `GridSimulator::run(int, bool)` | max tick, frame 포함 여부 | `SimulationResult` | 지정 tick 수만큼 simulation을 실행하고 결과를 반환한다. | FR-16, FR-17, FR-18 |
| `GridSimulator::step(int, bool)` | tick 번호, frame 포함 여부 | `bool` | 한 tick의 sensor 생성, controller 호출, command 적용, log 생성을 수행한다. | FR-17, FR-18 |
| `GridSimulator::render()` | 없음 | `std::string` | 현재 grid와 robot 방향을 문자열 map으로 렌더링한다. | FR-16 |

#### 7.2.3 CLI Interface

```powershell
rvc_simulator [--ticks N] [--scenario FILE] [--quiet-map]
```

| 옵션 | 설명 |
| --- | --- |
| `--help`, `-h` | 사용법을 출력하고 종료한다. |
| `--ticks N` | simulation tick 수를 지정한다. |
| `--scenario FILE` | scenario file을 읽어 초기 map과 기본 tick 수를 설정한다. |
| `--quiet-map` | 초기/최종 map과 tick별 frame 출력을 생략하고 log와 summary 중심으로 출력한다. |

알 수 없거나 값이 부족한 인자는 오류 메시지와 usage를 출력하고 종료 코드 2를 반환한다. scenario 처리나 실행 중 예외가 발생하면 오류 메시지를 출력하고 종료 코드 1을 반환한다.

### 7.3 Data View

#### 7.3.1 Core Data Types

| 타입 | 필드/값 | 설명 |
| --- | --- | --- |
| `ControllerConfig` | `dustBoostTicks` | 먼지 감지 후 boost를 유지할 tick 수이다. 기본값은 3이다. |
| `PeriodicSensorData` | `leftObstacle`, `rightObstacle`, `dustDetected` | tick마다 sampling되는 좌측/우측/먼지 sensor 값이다. |
| `SensorSnapshot` | `frontObstacle`, `leftObstacle`, `rightObstacle`, `dustDetected` | pending front interrupt와 periodic sensor 값을 결합한 판단 입력이다. |
| `Command` | `motion`, `cleaningPower`, `reason` | controller가 actuator 또는 simulator에 전달하는 추상 명령이다. |
| `SimulationResult` | `ticksRun`, `dustCleaned`, `finalPosition`, `finalDirection`, `logs` | simulator 실행 결과와 검증 로그이다. |
| `Position` | `row`, `col` | simulator 격자 내 robot 좌표이다. |

#### 7.3.2 Enum Values

| Enum | 값 | 설명 |
| --- | --- | --- |
| `Direction` | `North`, `East`, `South`, `West` | simulator에서 robot이 바라보는 방향이다. |
| `Motion` | `None`, `Stop`, `Forward`, `Backward`, `TurnLeft`, `TurnRight` | motor에 전달할 추상 이동 명령이다. |
| `CleaningPower` | `Off`, `Normal`, `Boost` | cleaner 출력 세기이다. |
| `ControllerState` | `Idle`, `Cleaning`, `Avoiding`, `Escaping` | controller의 현재 제어 상태이다. |

#### 7.3.3 Scenario Data

scenario file은 선택적 `ticks=N` 설정과 `map:` 섹션으로 구성된다. map 문자는 다음 의미를 갖는다.

| 문자 | 의미 |
| --- | --- |
| `#` | 장애물 |
| `.` | 빈 칸 |
| `*` | 먼지 |
| `R`, `^` | 북쪽을 바라보는 robot |
| `>` | 동쪽을 바라보는 robot |
| `v` | 남쪽을 바라보는 robot |
| `<` | 서쪽을 바라보는 robot |

map에는 robot marker가 정확히 하나 있어야 한다. simulator는 robot marker를 읽은 뒤 내부 grid에서는 해당 칸을 빈 칸으로 보관하고, render 시 현재 방향 marker를 다시 표시한다.

### 7.4 Interaction View

#### 7.4.1 SD-01 Control Tick Loop

```mermaid
sequenceDiagram
    participant Simulator
    participant Controller as RvcController

    Simulator->>Simulator: samplePeriodicSensors()
    Simulator->>Controller: tick(periodicSensors)
    Controller->>Controller: readPeriodicSensors(periodicSensors)
    Controller->>Controller: decideNextCommand(snapshot)
    Controller-->>Simulator: Command
    Simulator->>Simulator: cleanCurrentCell(command)
    Simulator->>Simulator: applyMotion(command.motion)
    Simulator->>Simulator: makeLogLine(...)
```

#### 7.4.2 SD-02 Front Interrupt Handling

```mermaid
sequenceDiagram
    participant Simulator
    participant Controller as RvcController

    Simulator->>Simulator: isObstacle(forwardPosition())
    alt front obstacle exists
        Simulator->>Controller: onFrontObstacleInterrupt()
    end
    Simulator->>Simulator: samplePeriodicSensors()
    Simulator->>Controller: tick(periodicSensors)
    Controller->>Controller: include pending front interrupt in SensorSnapshot
    Controller-->>Simulator: avoid command
```

#### 7.4.3 SD-03 Periodic Sensor Sampling

```mermaid
sequenceDiagram
    participant Simulator

    Simulator->>Simulator: isObstacle(adjacent(left))
    Simulator->>Simulator: isObstacle(adjacent(right))
    Simulator->>Simulator: hasDust(robotPosition)
    Simulator-->>Simulator: PeriodicSensorData
```

#### 7.4.4 SD-04 Obstacle Avoidance

```mermaid
sequenceDiagram
    participant Controller as RvcController

    Controller->>Controller: decideNextCommand(snapshot)
    alt front open
        Controller-->>Controller: Command(Forward, currentCleanerPower)
    else left open and right blocked
        Controller-->>Controller: Command(TurnLeft, currentCleanerPower)
    else left blocked and right open
        Controller-->>Controller: Command(TurnRight, currentCleanerPower)
    else left open and right open
        Controller->>Controller: choose alternating turn direction
        Controller-->>Controller: Command(TurnLeft or TurnRight, currentCleanerPower)
    else all front, left, and right blocked
        Controller->>Controller: state = Escaping
        Controller-->>Controller: Command(Backward, currentCleanerPower)
    end
```

#### 7.4.5 SD-05 Escape Until Possible

```mermaid
sequenceDiagram
    participant Simulator
    participant Controller as RvcController

    Simulator->>Controller: tick(allBlockedSnapshot)
    Controller->>Controller: enter Escaping
    Controller-->>Simulator: Command(Backward)
    loop while front, left, and right are blocked
        Simulator->>Controller: tick(allBlockedSnapshot)
        Controller-->>Simulator: Command(Backward)
    end
    Simulator->>Controller: tick(escapePossibleSnapshot)
    alt front is open
        Controller->>Controller: state = Cleaning
        Controller-->>Simulator: Command(Forward)
    else left is open
        Controller->>Controller: state = Avoiding
        Controller-->>Simulator: Command(TurnLeft)
    else right is open
        Controller->>Controller: state = Avoiding
        Controller-->>Simulator: Command(TurnRight)
    end
```

#### 7.4.6 SD-06 Dust Boost

```mermaid
sequenceDiagram
    participant Simulator
    participant Controller as RvcController

    Simulator->>Controller: tick(dustDetected = true)
    Controller->>Controller: boostTicksRemaining = configured duration
    Controller-->>Simulator: Command(anyMotion, Boost)
    loop while boostTicksRemaining > 0
        Simulator->>Controller: tick(dustDetected = false)
        Controller->>Controller: decrement boostTicksRemaining
        Controller-->>Simulator: Command(anyMotion, Boost)
    end
    Simulator->>Controller: tick(dustDetected = false)
    Controller-->>Simulator: Command(anyMotion, Normal)
```

### 7.5 State View

#### 7.5.1 Controller State Transition

```mermaid
stateDiagram-v2
    [*] --> Idle
    Idle --> Cleaning: startCleaning()
    Cleaning --> Idle: stopCleaning()
    Avoiding --> Idle: stopCleaning()
    Escaping --> Idle: stopCleaning()
    Cleaning --> Cleaning: tick(front open)
    Cleaning --> Avoiding: front interrupt and side open
    Cleaning --> Escaping: front, left, right blocked
    Avoiding --> Cleaning: next tick front open
    Avoiding --> Escaping: all directions blocked
    Escaping --> Escaping: all directions blocked
    Escaping --> Cleaning: front opened
    Escaping --> Avoiding: side opened
```

#### 7.5.2 Controller States

| 상태 | 의미 | 주요 출력 |
| --- | --- | --- |
| `Idle` | 자동 청소가 실행되지 않는 상태 | `Stop`, `Off` |
| `Cleaning` | 기본 자동 청소 상태 | 전방 open 시 `Forward` |
| `Avoiding` | 전방 장애물 interrupt 후 측면 회피를 수행하는 상태 | `TurnLeft` 또는 `TurnRight` |
| `Escaping` | 삼방향이 모두 막혀 후진 탈출을 수행하는 상태 | `Backward` 반복 |

`onFrontObstacleInterrupt()`는 실행 중일 때만 pending interrupt를 기록한다. idle 상태의 interrupt는 다음 tick 판단에 영향을 주지 않는다.

### 7.6 Algorithm View

#### 7.6.1 Tick Processing

1. `running_`이 false이면 `Motion::Stop`, `CleaningPower::Off` command를 반환한다.
2. `readPeriodicSensors()`가 `frontInterruptPending_`와 `PeriodicSensorData`를 결합해 `SensorSnapshot`을 만든다.
3. `decideNextCommand()`가 snapshot과 현재 state를 기반으로 command를 결정한다.
4. `tick()`은 command 결정 후 `frontInterruptPending_`를 false로 되돌려 interrupt를 소비한다.

#### 7.6.2 Obstacle Avoidance

| 조건 | 상태 변화 | Motion |
| --- | --- | --- |
| 전방이 열림 | `Cleaning` | `Forward` |
| 전방이 막히고 좌측만 열림 | `Avoiding` | `TurnLeft` |
| 전방이 막히고 우측만 열림 | `Avoiding` | `TurnRight` |
| 전방이 막히고 좌/우 모두 열림 | `Avoiding` | `TurnLeft`와 `TurnRight`를 번갈아 선택 |
| 전방, 좌측, 우측 모두 막힘 | `Escaping` | `Backward` |

#### 7.6.3 Escape Rule

`Escaping` 상태에서는 전방, 좌측, 우측이 모두 막힌 동안 계속 `Backward` command를 반환한다. 세 방향 중 하나 이상이 열리면 탈출 가능 상태로 판단한다. 전방이 열렸으면 `Cleaning`으로 복귀하며 `Forward`를 반환하고, 전방은 막혔지만 한쪽 측면이 열렸으면 `Avoiding`으로 전환해 열린 방향으로 회전한다.

#### 7.6.4 Dust Boost Rule

`updateCleaningPower(bool dustDetected)`는 motion 판단과 독립적으로 cleaner power를 결정한다. 먼지가 감지되면 `boostTicksRemaining_`을 `config_.dustBoostTicks`로 재설정한다. 먼지가 감지되지 않고 남은 boost tick이 있으면 1 감소시킨다. 남은 boost tick이 0보다 크면 `Boost`, 아니면 `Normal`을 반환한다.

### 7.7 Error Handling View

| 상황 | 처리 |
| --- | --- |
| 빈 map으로 simulator 생성 | `std::invalid_argument` 발생 |
| robot marker 없음 | `std::invalid_argument` 발생 |
| robot marker가 둘 이상 | `std::invalid_argument` 발생 |
| scenario file open 실패 | `std::runtime_error` 발생 |
| scenario file에 map section 없음 | `std::runtime_error` 발생 |
| `run()`에 음수 tick 입력 | `std::invalid_argument` 발생 |
| grid 범위 밖 좌표 | 장애물로 간주하여 이동하지 않는다. |
| 알 수 없는 CLI option | usage를 출력하고 종료 코드 2를 반환한다. |
| 실행 중 scenario 예외 | 오류 메시지를 출력하고 종료 코드 1을 반환한다. |

### 7.8 Verification View

#### 7.8.1 Test Strategy

| 테스트 수준 | 대상 | 검증 내용 |
| --- | --- | --- |
| Controller unit test | `RvcController` | 전진, 중지, interrupt 회피, 좌/우 회전 선택, 교대 회전, 탈출, dust boost duration |
| Simulator system test | `GridSimulator` | 실제 격자에서 dust 청소, 후진 탈출, boxed-in 반복 후진, front interrupt 후 회전 |
| CLI CTest | `rvc_simulator` | 기본 실행과 scenario 기반 실행 가능 여부 |

#### 7.8.2 Requirement Traceability

| 요구사항 | 핵심 설계 요소 | 검증 |
| --- | --- | --- |
| FR-01 | `RvcController::startCleaning`, `ControllerState::Cleaning` | `ControllerMovesForwardWhenPathIsClear` |
| FR-02 | `RvcController::stopCleaning`, idle command | `StopCleaningReturnsStopAndOff` |
| FR-03 | `decideNextCommand`, `Motion::Forward` | `ControllerMovesForwardWhenPathIsClear` |
| FR-04 | `onFrontObstacleInterrupt`, `frontInterruptPending_` | `FrontInterruptTriggersImmediateAvoidance`, `SimulatorTurnsAfterFrontInterrupt` |
| FR-05 | `tick`, `SensorSnapshot::frontObstacle`, 회피 command | `FrontInterruptTriggersImmediateAvoidance`, `SimulatorTurnsAfterFrontInterrupt` |
| FR-06 | `PeriodicSensorData`, `readPeriodicSensors` | controller unit tests |
| FR-07 | `chooseOpenSideTurn`, `Motion::TurnLeft` | `FrontInterruptTriggersImmediateAvoidance` |
| FR-08 | `chooseOpenSideTurn`, `Motion::TurnRight` | `TurnsTowardOpenSide`, `SimulatorTurnsAfterFrontInterrupt` |
| FR-09 | `preferLeftTurn_`, 좌우 교대 정책 | `AlternatesWhenBothSidesAreOpen` |
| FR-10 | all-blocked 판단, `ControllerState::Escaping` | `AllBlockedEntersEscapingAndKeepsBackingUp`, `SimulatorUsesBackwardEscape` |
| FR-11 | `Escaping` 상태의 `Motion::Backward` 반복 | `AllBlockedEntersEscapingAndKeepsBackingUp`, `SimulatorKeepsCommandingBackwardWhenBoxedIn` |
| FR-12 | 탈출 가능 조건 판단 | `EscapingExitsWhenFrontBecomesOpen` |
| FR-13 | 탈출 후 `Forward` 또는 측면 회전 | `EscapingExitsWhenFrontBecomesOpen` |
| FR-14 | `updateCleaningPower`, `ControllerConfig::dustBoostTicks` | `DustBoostLastsConfiguredTicks`, `SimulatorCleansDustAndLogsCommands` |
| FR-15 | `boostTicksRemaining_` 감소와 `CleaningPower::Normal` 복귀 | `DustBoostLastsConfiguredTicks` |
| FR-16 | `GridSimulator::render`, scenario map symbol | `SimulatorCliDefaultRuns`, `SimulatorCliContinuousBackwardScenarioRuns` |
| FR-17 | `GridSimulator::makeLogLine`, `SimulationResult::logs` | `SimulatorCleansDustAndLogsCommands`, CLI CTest |
| FR-18 | `GridSimulator`가 `RvcController`와 `Command`를 사용 | `SimulatorCleansDustAndLogsCommands`, `SimulatorUsesBackwardEscape` |

## 8. Design Rationale

### 8.1 Major Design Decisions

| 결정 | 근거 |
| --- | --- |
| 전방 장애물은 `onFrontObstacleInterrupt()`로만 controller에 전달한다. | 요구사항에서 front sensor가 interrupt 방식으로 동작해야 하기 때문이다. |
| 좌/우/먼지 값은 `tick(PeriodicSensorData)` 호출마다 controller에 전달한다. | periodic sensor sampling 요구사항을 controller API에 직접 반영하기 위해서이다. |
| `readPeriodicSensors()`는 pending front interrupt와 periodic 값을 결합하여 `SensorSnapshot`을 만든다. | 서로 다른 입력 timing을 단일 판단 입력으로 정리하기 위해서이다. |
| `decideNextCommand()`는 단일 판단 지점으로 둔다. | 회피, 탈출, boost 판단을 단위 테스트하기 쉽게 만들기 위해서이다. |
| `Escaping` 상태에서 삼방향이 계속 막혀 있으면 반드시 `Backward` command를 반복한다. | FR-11의 탈출 가능 시점까지 후진 유지 요구사항을 직접 만족하기 위해서이다. |
| `GridSimulator`는 controller command를 실제 하드웨어 대신 격자 상태에 적용한다. | 실제 하드웨어 없이도 요구사항을 반복 검증하기 위해서이다. |

### 8.2 SOLID Analysis

| Principle | Application |
| --- | --- |
| SRP | `RvcController`는 제어 결정만 담당하고, `GridSimulator`는 환경과 이동 적용만 담당한다. |
| OCP | sensor 입력은 `PeriodicSensorData`와 interrupt API로 추상화되어 새 sensor 추가 시 controller 확장이 가능하다. |
| LSP | simulator와 실제 하드웨어 어댑터는 같은 `Command` 의미를 따르므로 대체 가능하다. |
| ISP | controller의 public interface는 시작, 중지, interrupt, tick, 판단에 필요한 작은 operation으로 분리된다. |
| DIP | controller는 concrete simulator나 hardware에 의존하지 않고 값 객체와 추상 command에만 의존한다. |

### 8.3 Constraint Satisfaction

| 제약 | 충족 방식 |
| --- | --- |
| C++20 및 CMake 기반 빌드 | `CMakeLists.txt`에서 `CMAKE_CXX_STANDARD 20`과 `rvc_core`, `rvc_simulator`, `rvc_tests` target을 정의한다. |
| UTF-8 문서 | 모든 Markdown 문서는 UTF-8로 작성한다. |
| 하드웨어 독립성 | controller가 concrete hardware에 의존하지 않고 sensor data와 command value만 다룬다. |
| 반복 가능한 테스트 | controller 입력을 값 객체로 제공하고 random 또는 wall-clock 의존성을 두지 않는다. |
| 요구사항 추적성 | FR별 설계 요소와 테스트 이름을 SRS, SDD, traceability 문서에 유지한다. |

### 8.4 Change Impact

본 SDD 재구성은 문서 구조 변경 작업이며 C++ 공개 API, 구현 코드, 테스트 코드, 빌드 설정을 변경하지 않는다. 따라서 런타임 동작 변경, 호환성 변경, 마이그레이션 작업은 발생하지 않는다.
