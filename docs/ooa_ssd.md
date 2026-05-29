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

## 2. SSD-02 Front Obstacle Avoidance

```mermaid
sequenceDiagram
    actor FrontSensor as Front Sensor
    actor LeftSensor as Left Sensor
    participant RVC as RVC
    actor Motor
    actor Cleaner

    FrontSensor->>RVC: onFrontObstacleInterrupt()
    LeftSensor-->>RVC: leftObstacle
    RVC->>RVC: tick(periodicSensors)
    alt left side open
        RVC-->>Motor: Command(TurnLeft)
    else left side blocked
        RVC-->>Motor: Command(Backward)
    end
    RVC-->>Cleaner: Command(Off)
```

## 3. SSD-03 Escape Right Probe

```mermaid
sequenceDiagram
    actor FrontSensor as Front Sensor
    actor LeftSensor as Left Sensor
    participant RVC as RVC
    actor Motor

    FrontSensor->>RVC: onFrontObstacleInterrupt()
    LeftSensor-->>RVC: leftObstacle = true
    RVC->>RVC: tick(periodicSensors)
    RVC-->>Motor: Command(Backward)
    RVC->>RVC: tick(periodicSensors)
    RVC-->>Motor: Command(TurnRight)
    alt next tick has no front interrupt
        RVC->>RVC: tick(periodicSensors)
        RVC-->>Motor: Command(Forward)
    else next tick has front interrupt
        FrontSensor->>RVC: onFrontObstacleInterrupt()
        RVC->>RVC: tick(periodicSensors)
        RVC-->>Motor: Command(TurnLeft)
    end
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

## 5. 변경 이력

| Tag | Item |
| --- | --- |
| [삭제] | `RightSensor` actor와 `rightObstacle` periodic 입력을 제거했다. |
| [변경] | 전방 장애물 회피는 좌측이 열리면 좌회전, 좌측이 막히면 후진 탈출로 분기한다. |
| [신규] | 우측 탈출 확인은 우회전 후 다음 tick의 front interrupt 유무로 판단한다. |
| [신규] | `Backward`, `TurnRight`, `TurnLeft`, `Forward`는 모두 별도 tick을 소비한다. |
