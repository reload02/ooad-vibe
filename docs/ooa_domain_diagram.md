# RVC OOA Domain Diagram

## 1. Domain Model

이 다이어그램의 `Motor`, `Cleaner`, sensor 항목은 문제 영역의 도메인 개념이며 C++ 구현 클래스를 의미하지 않는다. 현재 구현에서는 `RvcController`가 `Command`를 생성하고 `GridSimulator`가 이를 검증 환경에 적용한다.

```mermaid
classDiagram
    class RVC {
        +currentDirection
        +currentPosition
    }

    class RvcController {
        +state
        +startCleaning()
        +stopCleaning()
        +tick()
        +onFrontObstacleInterrupt()
    }

    class FrontSensor {
        +interruptOnObstacle()
    }

    class PeriodicSensor {
        +read()
    }

    class LeftSensor
    class RightSensor
    class DustSensor

    class DigitalClock {
        +tick()
    }

    class Motor {
        +moveForward()
        +moveBackward()
        +turnLeft()
        +turnRight()
        +stop()
    }

    class Cleaner {
        +setNormalPower()
        +setBoostPower()
        +turnOff()
    }

    class Environment {
        +obstacles
        +dustCells
    }

    class Command {
        +motion
        +cleaningPower
        +reason
    }

    RVC *-- RvcController
    RVC *-- Motor
    RVC *-- Cleaner
    RVC *-- FrontSensor
    RVC *-- LeftSensor
    RVC *-- RightSensor
    RVC *-- DustSensor
    PeriodicSensor <|-- LeftSensor
    PeriodicSensor <|-- RightSensor
    PeriodicSensor <|-- DustSensor
    DigitalClock --> RvcController : tick
    FrontSensor --> RvcController : interrupt
    LeftSensor --> RvcController : periodic data
    RightSensor --> RvcController : periodic data
    DustSensor --> RvcController : periodic data
    RvcController --> Command : creates
    Command --> Motor : motion
    Command --> Cleaner : cleaning power
    Environment --> FrontSensor : sensed by
    Environment --> LeftSensor : sensed by
    Environment --> RightSensor : sensed by
    Environment --> DustSensor : sensed by
```

## 2. Domain Concepts

| Concept | Responsibility |
| --- | --- |
| RVC | household surface를 자동으로 청소하고 물걸레질하는 물리적 로봇 청소기 전체를 의미한다. |
| RvcController | 센서 입력과 interrupt를 기반으로 motor/cleaner 명령을 결정한다. |
| FrontSensor | 전방 장애물을 interrupt로 알린다. |
| LeftSensor | 좌측 장애물 상태를 periodic 방식으로 제공한다. |
| RightSensor | 우측 장애물 상태를 periodic 방식으로 제공한다. |
| DustSensor | 현재 위치의 먼지 감지 상태를 periodic 방식으로 제공한다. |
| DigitalClock | 제어 tick을 발생시킨다. |
| Motor | 전진, 후진, 좌회전, 우회전, 정지 동작을 수행한다. |
| Cleaner | normal power, boost power, off 상태를 수행하며, 현재 모델에서는 청소/물걸레질 출력을 대표하는 추상 actuator이다. |
| Environment | 장애물과 먼지가 있는 청소 공간이다. |
| Command | controller가 actuator에 전달하는 추상 명령이다. |

## 3. Important Domain Rules

- FrontSensor는 polling 대상이 아니라 interrupt source이다.
- LeftSensor, RightSensor, DustSensor는 DigitalClock tick에 맞춰 sampling된다.
- RvcController는 sensor와 actuator의 구체 구현을 알지 않는다.
- PDF DFD Level 0의 `Tick`, `Direction`, `Clean` 흐름은 각각 `DigitalClock.tick()`, `Command.motion`, `Command.cleaningPower`에 대응된다.
- 회피/탈출 이동인 `Backward`, `TurnLeft`, `TurnRight` 동안 cleaner output은 `Off`이며, `Forward` 전진 청소에서만 `Normal` 또는 `Boost`가 적용된다.
- `Escaping` 상태에서는 후방 센서 없이 backward command를 반복하며, 좌/우 중 한쪽이 열릴 때까지 전방 open 여부를 탈출 조건으로 사용하지 않는다.
- Simulator의 Environment는 실제 하드웨어가 아니라 테스트용 외부 세계이다.
