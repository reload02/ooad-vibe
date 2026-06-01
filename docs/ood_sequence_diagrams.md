# RVC OOD Sequence Diagrams

## 1. SD-01 Control Tick Loop

[변경] control tick은 `GridSimulator`가 `RvcController`를 직접 호출하지 않고 `Rvc`를 통해 진행한다.
[삭제] ~~`Simulator->>Controller: tick(periodicSensors)`~~
[추가] `Rvc`가 `RvcHardwareAdapter`에서 sensor/event를 읽고 `RvcController` command를 adapter에 적용한다.

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

```mermaid
sequenceDiagram
    participant Rvc
    participant Adapter as SimulatedHardwareAdapter

    Rvc->>Adapter: readPeriodicSensors()
    Adapter->>Adapter: isObstacle(adjacent(left))
    Adapter->>Adapter: isObstacle(adjacent(right))
    Adapter->>Adapter: hasDust(robotPosition)
    Adapter-->>Rvc: PeriodicSensorData
```

## 4. SD-04 Obstacle Avoidance

[변경] 장애물 회피 판단은 여전히 `RvcController` 내부 책임이며, controller는 adapter나 simulator 구체 타입을 알지 않는다.

```mermaid
sequenceDiagram
    participant Controller as RvcController

    Controller->>Controller: decideNextCommand(snapshot)
    alt front open
        Controller-->>Controller: Command(Forward, activeCleanerPower)
    else left open and right blocked
        Controller-->>Controller: Command(TurnLeft, Off)
    else left blocked and right open
        Controller-->>Controller: Command(TurnRight, Off)
    else left open and right open
        Controller->>Controller: choose alternating turn direction
        Controller-->>Controller: Command(TurnLeft or TurnRight, Off)
    else all front, left, and right blocked
        Controller->>Controller: state = Escaping
        Controller-->>Controller: Command(Backward, Off)
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
    Adapter-->>Rvc: allBlockedPeriodicSensors
    Rvc->>Controller: tick(allBlockedPeriodicSensors)
    Controller->>Controller: enter Escaping
    Controller-->>Rvc: Command(Backward, Off)
    Rvc->>Adapter: applyCommand(Backward)
    loop while left and right are blocked
        Rvc->>Adapter: readPeriodicSensors()
        Adapter-->>Rvc: sideBlockedPeriodicSensors
        Rvc->>Controller: tick(sideBlockedPeriodicSensors)
        Controller-->>Rvc: Command(Backward, Off)
        Rvc->>Adapter: applyCommand(Backward)
    end
    Rvc->>Adapter: readPeriodicSensors()
    Adapter-->>Rvc: sideExitPeriodicSensors
    Rvc->>Controller: tick(sideExitPeriodicSensors)
    alt left is open
        Controller->>Controller: state = Avoiding
        Controller-->>Rvc: Command(TurnLeft, Off)
    else right is open
        Controller->>Controller: state = Avoiding
        Controller-->>Rvc: Command(TurnRight, Off)
    end
    Rvc->>Adapter: applyCommand(turn command)
```

## 6. SD-06 Dust Boost

[변경] 먼지 boost 입력도 adapter를 통해 `Rvc`로 들어오며, controller command는 `Rvc`가 adapter에 적용한다.
[삭제] ~~`Simulator->>Controller: tick(dustDetected = true)`~~

```mermaid
sequenceDiagram
    participant Rvc
    participant Adapter as RvcHardwareAdapter
    participant Controller as RvcController

    Rvc->>Adapter: readPeriodicSensors()
    Adapter-->>Rvc: dustDetected = true
    Rvc->>Controller: tick(dustDetected = true)
    Controller->>Controller: boostTicksRemaining = configured duration
    alt next motion is Forward
        Controller-->>Rvc: Command(Forward, Boost)
    else next motion is Backward or Turn
        Controller-->>Rvc: Command(avoidanceMotion, Off)
    end
    Rvc->>Adapter: applyCommand(command)
    loop while boostTicksRemaining > 0
        Rvc->>Adapter: readPeriodicSensors()
        Adapter-->>Rvc: dustDetected = false
        Rvc->>Controller: tick(dustDetected = false)
        Controller->>Controller: decrement boostTicksRemaining
        alt next motion is Forward
            Controller-->>Rvc: Command(Forward, Boost)
        else next motion is Backward or Turn
            Controller-->>Rvc: Command(avoidanceMotion, Off)
        end
        Rvc->>Adapter: applyCommand(command)
    end
    Rvc->>Adapter: readPeriodicSensors()
    Adapter-->>Rvc: dustDetected = false
    Rvc->>Controller: tick(dustDetected = false)
    Controller-->>Rvc: Command(Forward, Normal)
    Rvc->>Adapter: applyCommand(command)
```
