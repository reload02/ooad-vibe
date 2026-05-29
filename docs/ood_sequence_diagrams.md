# RVC OOD Sequence Diagrams

## 1. SD-01 Control Tick

```mermaid
sequenceDiagram
    participant Simulator as GridSimulator
    participant RVC as Rvc
    participant Controller as RvcController
    participant Fusion as SensorFusion
    participant Navigation as NavigationPolicy
    participant Cleaning as CleaningPolicy
    participant Composer as CommandComposer

    Simulator->>RVC: tick(periodicSensors)
    RVC->>Controller: tick(periodicSensors)
    Controller->>Fusion: fuse(periodicSensors)
    Fusion-->>Controller: SensorSnapshot
    Controller->>Cleaning: update(snapshot.dustDetected)
    Cleaning-->>Controller: CleaningPower
    Controller->>Navigation: decide(snapshot)
    Navigation-->>Controller: NavigationDecision
    Controller->>Composer: compose(motion, power, reason)
    Composer-->>Controller: Command
    Controller-->>RVC: Command
    RVC-->>Simulator: Command
```

## 2. SD-02 Front Interrupt

```mermaid
sequenceDiagram
    participant Simulator as GridSimulator
    participant RVC as Rvc
    participant Controller as RvcController
    participant Fusion as SensorFusion

    Simulator->>Simulator: isObstacle(forwardPosition)
    opt front blocked
        Simulator->>RVC: onFrontObstacleInterrupt()
        RVC->>Controller: onFrontObstacleInterrupt()
        Controller->>Fusion: recordFrontObstacleInterrupt()
    end
    Simulator->>RVC: tick(periodicSensors)
    RVC->>Controller: tick(periodicSensors)
    Controller->>Fusion: fuse(periodicSensors)
    Fusion-->>Controller: SensorSnapshot(frontObstacle)
    Controller->>Fusion: clearFrontObstacleInterrupt()
```

## 3. SD-03 Escape Right Probe

```mermaid
sequenceDiagram
    participant Simulator as GridSimulator
    participant RVC as Rvc
    participant Navigation as NavigationPolicy

    Simulator->>RVC: onFrontObstacleInterrupt()
    Simulator->>RVC: tick(leftObstacle=true)
    RVC->>Navigation: decide(frontObstacle=true, leftObstacle=true)
    Navigation-->>RVC: Backward
    Simulator->>RVC: tick(leftObstacle=true)
    RVC->>Navigation: decide(...)
    Navigation-->>RVC: TurnRight
    alt right direction clear
        Simulator->>RVC: tick(no front interrupt)
        Navigation-->>RVC: Forward
    else right direction blocked
        Simulator->>RVC: onFrontObstacleInterrupt()
        Simulator->>RVC: tick(...)
        Navigation-->>RVC: TurnLeft
    end
```

## 4. Change Notes

| Tag | Item |
| --- | --- |
| [삭제] | Control tick에서 right periodic sample을 전달하지 않는다. |
| [변경] | `SensorSnapshot`에는 `frontObstacle`, `leftObstacle`, `dustDetected`만 포함된다. |
| [신규] | 우측 probe 결과는 다음 tick의 front interrupt 존재 여부로 판단한다. |
| [신규] | probe용 `TurnRight`와 복구용 `TurnLeft` 모두 독립 command이며 tick을 소비한다. |
