# RVC OOD Class Diagram

## 1. Class Diagram

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

## 2. Class Responsibilities

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

## 3. SOLID Analysis

| Principle | Application |
| --- | --- |
| SRP | `RvcController`는 제어 결정만 담당하고, `GridSimulator`는 환경과 이동 적용만 담당한다. |
| OCP | sensor 입력은 `PeriodicSensorData`와 interrupt API로 추상화되어 새 sensor 추가 시 controller 확장이 가능하다. |
| LSP | simulator와 실제 하드웨어 어댑터는 같은 `Command` 의미를 따르므로 대체 가능하다. |
| ISP | controller의 public interface는 시작, 중지, interrupt, tick, 판단에 필요한 작은 operation으로 분리된다. |
| DIP | controller는 concrete simulator나 hardware에 의존하지 않고 값 객체와 추상 command에만 의존한다. |

## 4. Design Decisions

- 전방 장애물은 `onFrontObstacleInterrupt()`로만 controller에 전달한다.
- 좌/우/먼지 값은 `tick(PeriodicSensorData)` 호출마다 controller에 전달한다.
- `readPeriodicSensors()`는 pending front interrupt와 periodic 값을 결합하여 `SensorSnapshot`을 만든다.
- `decideNextCommand()`는 단일 판단 지점으로 두어 테스트를 쉽게 한다.
- `Escaping` 상태에서 삼방향이 계속 막혀 있으면 반드시 `Backward` command를 반복한다.
