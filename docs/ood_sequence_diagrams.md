# RVC OOD Sequence Diagrams

## 1. SD-01 Control Tick Loop

[변경] control tick은 `GridSimulator`가 `RvcController`를 직접 호출하지 않고 `Rvc`를 통해 진행한다.
[삭제] ~~`Simulator->>Controller: tick(periodicSensors)`~~
[추가] `Rvc`가 `RvcHardwareAdapter`에서 sensor/event를 읽고 `RvcController` command를 adapter에 적용한다.
[R3-추가] 후진 주행 중에는 rear obstacle interrupt도 같은 tick 입력 흐름에 포함된다.

```mermaid
sequenceDiagram
    participant Simulator as GridSimulator
    participant Rvc
    participant Adapter as SimulatedHardwareAdapter
    participant Controller as RvcController

    Simulator->>Rvc: tick()
    Rvc->>Adapter: hasFrontObstacleInterrupt()
    alt front obstacle exists
        Rvc->>Controller: onFrontObstacleInterrupt()
    end
    Rvc->>Adapter: hasRearObstacleInterrupt()
    alt rear obstacle exists while traveling backward
        Rvc->>Controller: onRearObstacleInterrupt()
    end
    Rvc->>Adapter: readPeriodicSensors()
    Adapter-->>Rvc: PeriodicSensorData
    Rvc->>Controller: tick(periodicSensors)
    Controller->>Controller: readPeriodicSensors(periodicSensors)
    Controller->>Controller: decideNextCommand(snapshot)
    Controller-->>Rvc: Command
    Rvc->>Adapter: applyCommand(command)
    Simulator->>Adapter: render or inspect state
    Simulator->>Simulator: makeLogLine(...)
```

## 2. SD-02 Front Interrupt Handling

[변경] 전방 interrupt 감지는 `SimulatedHardwareAdapter`가 제공하고, `Rvc`가 이를 `RvcController` interrupt API로 전달한다.
[삭제] ~~`Simulator->>Controller: onFrontObstacleInterrupt()`~~

```mermaid
sequenceDiagram
    participant Simulator as GridSimulator
    participant Rvc
    participant Adapter as SimulatedHardwareAdapter
    participant Controller as RvcController

    Simulator->>Rvc: tick()
    Rvc->>Adapter: hasFrontObstacleInterrupt()
    alt front obstacle exists
        Rvc->>Controller: onFrontObstacleInterrupt()
    end
    Rvc->>Adapter: readPeriodicSensors()
    Adapter-->>Rvc: PeriodicSensorData
    Rvc->>Controller: tick(periodicSensors)
    Controller->>Controller: include pending front interrupt in SensorSnapshot
    Controller-->>Rvc: avoid command
    Rvc->>Adapter: applyCommand(avoid command)
```

## 3. SD-03 Periodic Sensor Sampling

[변경] periodic sensor sampling 책임은 `GridSimulator`가 아니라 `SimulatedHardwareAdapter`에 둔다.
[삭제] ~~`Simulator-->>Simulator: PeriodicSensorData`~~
[R2-변경] 우측 장애물은 periodic sensor가 아니라 `TurnRight` 후 front interrupt로 탐색한다.

```mermaid
sequenceDiagram
    participant Rvc
    participant Adapter as SimulatedHardwareAdapter

    Rvc->>Adapter: readPeriodicSensors()
    Adapter->>Adapter: isObstacle(adjacent(left))
    Adapter->>Adapter: hasDust(robotPosition)
    Adapter-->>Rvc: PeriodicSensorData
```

## 4. SD-04 Obstacle Avoidance

[변경] 장애물 회피 판단은 여전히 `RvcController` 내부 책임이며, controller는 adapter나 simulator 구체 타입을 알지 않는다. [R3-변경] R3 적용 시 회피/탈출 motion도 청소 중 기본 cleaner output을 `Normal`로 유지한다. [R2-기존] ~~회피/탈출 motion에서는 cleaner output을 `Off`로 둔다.~~

```mermaid
sequenceDiagram
    participant Controller as RvcController

    Controller->>Controller: decideNextCommand(snapshot)
    alt front open
        Controller-->>Controller: Command(Forward, activeCleanerPower)
    else front blocked and left open
        Controller-->>Controller: Command(TurnLeft, Normal)
    else front blocked and left blocked
        Controller->>Controller: state = RightProbing
        Controller-->>Controller: Command(TurnRight, Normal)
    else right probe front open
        Controller-->>Controller: Command(Forward, activeCleanerPower)
    else right probe front blocked
        Controller->>Controller: state = EscapeAligning
        Controller-->>Controller: Command(TurnLeft, Normal)
    else original heading restored
        Controller->>Controller: state = Escaping
        Controller-->>Controller: Command(Backward, Normal)
    end
```

## 5. SD-05 Escape Until Possible

[변경] 탈출 반복 흐름도 `Rvc`가 adapter sensor 값을 읽어 controller에 전달하고, 반환 command를 adapter에 적용한다.
[삭제] ~~`Simulator->>Controller: tick(allBlockedSnapshot)`~~

```mermaid
sequenceDiagram
    participant Rvc
    participant Adapter as RvcHardwareAdapter
    participant Controller as RvcController

    Rvc->>Adapter: readPeriodicSensors()
    Adapter-->>Rvc: leftBlockedPeriodicSensors
    Rvc->>Controller: tick(leftBlockedPeriodicSensors)
    Controller-->>Rvc: Command(TurnRight, Normal)
    Rvc->>Adapter: applyCommand(TurnRight)
    Rvc->>Adapter: hasFrontObstacleInterrupt()
    Adapter-->>Rvc: front blocked after right probe
    Controller-->>Rvc: Command(TurnLeft, Normal)
    Rvc->>Adapter: applyCommand(TurnLeft)
    Controller->>Controller: enter Escaping
    Controller-->>Rvc: Command(Backward, Normal)
    Rvc->>Adapter: applyCommand(Backward)
    loop while left is blocked
        Rvc->>Adapter: readPeriodicSensors()
        Adapter-->>Rvc: leftBlockedPeriodicSensors
        Rvc->>Controller: tick(leftBlockedPeriodicSensors)
        Controller-->>Rvc: Command(TurnRight, Normal)
        Rvc->>Adapter: applyCommand(TurnRight)
        Rvc->>Adapter: hasFrontObstacleInterrupt()
        alt right probe open
            Controller-->>Rvc: Command(Forward, activeCleanerPower)
        else right probe blocked
            Controller-->>Rvc: Command(TurnLeft, Normal)
            Rvc->>Adapter: applyCommand(TurnLeft)
            Controller-->>Rvc: Command(Backward, Normal)
            Rvc->>Adapter: applyCommand(Backward)
        end
    end
    Controller-->>Rvc: Command(TurnLeft, Normal)
    Rvc->>Adapter: applyCommand(TurnLeft)
```

## 6. SD-06 Dust Boost

[변경] 먼지 boost 입력도 adapter를 통해 `Rvc`로 들어오며, controller command는 `Rvc`가 adapter에 적용한다.
[R3-변경] R3 기준에서는 fixed boost tick이 아니라 현재 travel direction에 따른 제자리 회전 구간에만 `Boost`를 적용하고, 회전 후 travel direction을 toggle한다.
[삭제] ~~`Simulator->>Controller: tick(dustDetected = true)`~~

```mermaid
sequenceDiagram
    participant Rvc
    participant Adapter as RvcHardwareAdapter
    participant Controller as RvcController

    Rvc->>Adapter: readPeriodicSensors()
    Adapter-->>Rvc: dustDetected = true
    Rvc->>Controller: tick(dustDetected = true)
    alt travel direction is Forward
        Controller->>Controller: set dust rotation clockwise
        Controller-->>Rvc: Command(TurnRight in place, Boost)
        Rvc->>Adapter: applyCommand(command)
        Controller->>Controller: travelDirection = Backward
    else travel direction is Backward
        Controller->>Controller: set dust rotation counterclockwise
        Controller-->>Rvc: Command(TurnLeft in place, Boost)
        Rvc->>Adapter: applyCommand(command)
        Controller->>Controller: travelDirection = Forward
    end
    Adapter->>Adapter: leave dust cell detectable
    Rvc->>Adapter: readPeriodicSensors()
    Adapter-->>Rvc: dustDetected = false
    Rvc->>Controller: tick(dustDetected = false)
    Controller-->>Rvc: Command(next travel motion, Normal)
    Rvc->>Adapter: applyCommand(command)
```

## 7. SD-07 Rear Interrupt Handling

[R3-추가] Rear interrupt는 후진 주행 중 현재 travel direction의 전방 장애물처럼 처리한다. 좌측이 열려 있으면 기존 좌측 공간으로 후진 방향을 돌리고, 좌측이 막혀 있으면 기존 우측 방향을 rear sensor로 탐색한다.

```mermaid
sequenceDiagram
    participant Rvc
    participant Adapter as RvcHardwareAdapter
    participant Controller as RvcController

    Rvc->>Adapter: hasRearObstacleInterrupt()
    Adapter-->>Rvc: rear obstacle exists
    Rvc->>Controller: onRearObstacleInterrupt()
    Rvc->>Adapter: readPeriodicSensors()
    Adapter-->>Rvc: leftObstacle
    Rvc->>Controller: tick(periodicSensors)
    alt left open
        Controller-->>Rvc: Command(TurnRight, Normal)
        Rvc->>Adapter: applyCommand(TurnRight)
        Controller->>Controller: continue backward travel
    else left blocked
        Controller-->>Rvc: Command(TurnLeft, Normal)
        Rvc->>Adapter: applyCommand(TurnLeft)
        Rvc->>Adapter: hasRearObstacleInterrupt()
        alt rear probe open
            Controller-->>Rvc: Command(Backward, Normal)
        else rear probe blocked
            Controller-->>Rvc: Command(TurnRight, Normal)
            Controller->>Controller: travelDirection = Forward
        end
        Rvc->>Adapter: applyCommand(command)
    end
```
