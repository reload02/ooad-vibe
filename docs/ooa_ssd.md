# RVC OOA System Sequence Diagrams

## 1. SSD-01 Start Automatic Cleaning

```mermaid
sequenceDiagram
    actor User
    participant RVC as RVC
    actor Motor
    actor Cleaner

    User->>RVC: startCleaning()
    User->>RVC: tick(periodicSensors)
    RVC-->>Motor: Command(Forward)
    RVC-->>Cleaner: Command(Normal)
```

## 2. SSD-02 Stop Automatic Cleaning

```mermaid
sequenceDiagram
    actor User
    participant RVC as RVC
    actor Motor
    actor Cleaner

    User->>RVC: stopCleaning()
    User->>RVC: tick(periodicSensors)
    RVC-->>Motor: Command(Stop)
    RVC-->>Cleaner: Command(Off)
```

## 3. SSD-03 Front Obstacle Avoidance

```mermaid
sequenceDiagram
    actor FrontSensor as Front Sensor
    actor LeftSensor as Left Sensor
    actor RightSensor as Right Sensor
    participant RVC as RVC
    actor Motor
    actor Cleaner

    FrontSensor->>RVC: onFrontObstacleInterrupt()
    LeftSensor-->>RVC: leftObstacle
    RightSensor-->>RVC: rightObstacle
    RVC->>RVC: tick(periodicSensors)
    RVC-->>Motor: Command(TurnLeft or TurnRight)
    RVC-->>Cleaner: Command(Off)
```

## 4. SSD-04 Dust Boost

```mermaid
sequenceDiagram
    actor DustSensor as Dust Sensor
    participant RVC as RVC
    actor Cleaner

    DustSensor-->>RVC: dustDetected = true
    RVC->>RVC: tick(periodicSensors)
    RVC-->>Cleaner: Command(Boost) when motion is Forward
    loop configured boost ticks
        RVC->>RVC: tick(dustDetected = false)
        RVC-->>Cleaner: Command(Boost or Off by motion)
    end
```

## 5. SSD-05 Escape From Blocked Area

```mermaid
sequenceDiagram
    actor FrontSensor as Front Sensor
    actor LeftSensor as Left Sensor
    actor RightSensor as Right Sensor
    participant RVC as RVC
    actor Motor
    actor Cleaner

    FrontSensor->>RVC: onFrontObstacleInterrupt()
    LeftSensor-->>RVC: leftObstacle = true
    RightSensor-->>RVC: rightObstacle = true
    RVC->>RVC: tick(periodicSensors)
    RVC-->>Motor: Command(Backward)
    RVC-->>Cleaner: Command(Off)
    loop while both sides blocked
        RVC->>RVC: tick(periodicSensors)
        RVC-->>Motor: Command(Backward)
    end
    RVC-->>Motor: Command(TurnLeft or TurnRight)
```

## 6. SSD-06 Simulator Verification

```mermaid
sequenceDiagram
    participant Simulator as GridSimulator
    participant RVC as RVC

    Simulator->>Simulator: calculate front/left/right/dust from grid
    opt front obstacle
        Simulator->>RVC: onFrontObstacleInterrupt()
    end
    Simulator->>RVC: tick(periodicSensors)
    RVC-->>Simulator: Command
    Simulator->>Simulator: apply motion, cleaner power, position, direction, dust
```

## 7. System Operations

| Operation | Input | Output | Responsibility |
| --- | --- | --- | --- |
| `startCleaning()` | none | none | RVC를 running 상태로 전환한다. |
| `stopCleaning()` | none | none | RVC를 idle 상태로 전환하고 cleaner output을 끈다. |
| `onFrontObstacleInterrupt()` | none | none | 전방 장애물 event를 pending 상태로 기록한다. |
| `tick(periodicSensors)` | `PeriodicSensorData` | `Command` | 감지 결합, 이동 판단, 청소 세기 판단, command 조립을 수행한다. |
