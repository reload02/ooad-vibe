# RVC OOD Sequence Diagrams

## 1. SD-01 Control Tick Loop

```mermaid
sequenceDiagram
    participant Simulator
    participant Controller as RvcController
    participant Sampler as PeriodicSensorSampler
    participant Motor
    participant Cleaner

    Simulator->>Sampler: samplePeriodicSensors()
    Sampler-->>Simulator: PeriodicSensorData
    Simulator->>Controller: tick(periodicSensors)
    Controller->>Controller: readPeriodicSensors(periodicSensors)
    Controller->>Controller: decideNextCommand(snapshot)
    Controller-->>Simulator: Command
    Simulator->>Motor: apply(command.motion)
    Simulator->>Cleaner: apply(command.cleaningPower)
```

## 2. SD-02 Front Interrupt Handling

```mermaid
sequenceDiagram
    participant Simulator
    participant FrontSensor
    participant Controller as RvcController

    Simulator->>FrontSensor: detectFrontCell()
    alt front obstacle exists
        FrontSensor->>Controller: onFrontObstacleInterrupt()
    end
    Simulator->>Controller: tick(periodicSensors)
    Controller->>Controller: include pending front interrupt in SensorSnapshot
    Controller-->>Simulator: avoid command
```

## 3. SD-03 Periodic Sensor Sampling

```mermaid
sequenceDiagram
    participant Simulator
    participant LeftSensor
    participant RightSensor
    participant DustSensor
    participant Sampler as PeriodicSensorSampler

    Simulator->>Sampler: sample()
    Sampler->>LeftSensor: read()
    LeftSensor-->>Sampler: leftObstacle
    Sampler->>RightSensor: read()
    RightSensor-->>Sampler: rightObstacle
    Sampler->>DustSensor: read()
    DustSensor-->>Sampler: dustDetected
    Sampler-->>Simulator: PeriodicSensorData
```

## 4. SD-04 Obstacle Avoidance

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

## 5. SD-05 Escape Until Possible

```mermaid
sequenceDiagram
    participant Simulator
    participant Controller as RvcController

    Simulator->>Controller: tick(allBlockedSnapshot)
    Controller->>Controller: enter Escaping
    Controller-->>Simulator: Command(Backward)
    loop while left and right are blocked
        Simulator->>Controller: tick(sideBlockedSnapshot)
        Controller-->>Simulator: Command(Backward)
    end
    Simulator->>Controller: tick(sideExitSnapshot)
    alt left is open
        Controller->>Controller: state = Avoiding
        Controller-->>Simulator: Command(TurnLeft)
    else right is open
        Controller->>Controller: state = Avoiding
        Controller-->>Simulator: Command(TurnRight)
    end
```

## 6. SD-06 Dust Boost

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
