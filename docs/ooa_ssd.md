# RVC OOA System Sequence Diagrams

## 1. Overview

이 문서는 RVC Control SW의 OOA 단계 산출물로, 외부 actor와 system 사이의 이벤트 흐름을 System Sequence Diagram으로 정리한다. 전방 센서는 interrupt actor이고, 좌측/우측/먼지 센서는 periodic actor로 모델링한다.

## 2. SSD-01 Start Automatic Cleaning

```mermaid
sequenceDiagram
    actor User
    actor Clock as Digital Clock
    participant System as RVC Control SW
    actor Motor
    actor Cleaner

    User->>System: startCleaning()
    System->>System: state = Cleaning, running = true
    System-->>User: cleaningStarted
    Clock->>System: tick(periodicSensors)
    System-->>Motor: Command(Forward)
    System-->>Cleaner: Command(Normal)
```

## 3. SSD-06 Stop Automatic Cleaning

```mermaid
sequenceDiagram
    actor User
    actor Clock as Digital Clock
    participant System as RVC Control SW
    actor Motor
    actor Cleaner

    User->>System: stopCleaning()
    System->>System: state = Idle, running = false, boostTicksRemaining = 0
    System-->>User: cleaningStopped
    Clock->>System: tick(periodicSensors)
    System-->>Motor: Command(Stop)
    System-->>Cleaner: Command(Off)
```

## 4. SSD-02 Front Obstacle Interrupt

```mermaid
sequenceDiagram
    actor FrontSensor as Front Sensor
    actor Clock as Digital Clock
    participant System as RVC Control SW
    actor LeftSensor as Left Sensor
    actor RightSensor as Right Sensor
    actor Motor
    actor Cleaner

    FrontSensor->>System: onFrontObstacleInterrupt()
    System->>System: mark front interrupt pending
    Clock->>System: tick(periodicSensors)
    LeftSensor-->>System: leftObstacle
    RightSensor-->>System: rightObstacle
    alt one side is open
        System-->>Motor: Command(TurnLeft or TurnRight)
    else both sides are blocked
        System-->>Motor: Command(Backward)
    end
```

## 5. SSD-03 Periodic Sensor Sampling

```mermaid
sequenceDiagram
    actor Clock as Digital Clock
    participant System as RVC Control SW
    actor LeftSensor as Left Sensor
    actor RightSensor as Right Sensor
    actor DustSensor as Dust Sensor

    Clock->>System: tick()
    LeftSensor-->>System: leftObstacle
    RightSensor-->>System: rightObstacle
    DustSensor-->>System: dustDetected
    System->>System: updatePeriodicSensorSnapshot()
```

## 6. SSD-04 Boost Cleaning On Dust

```mermaid
sequenceDiagram
    actor Clock as Digital Clock
    actor DustSensor as Dust Sensor
    participant System as RVC Control SW
    actor Cleaner

    Clock->>System: tick()
    DustSensor-->>System: dustDetected = true
    System->>System: startBoostTimer()
    System-->>Cleaner: Command(Boost)
    loop until boost timer expires
        Clock->>System: tick()
        System-->>Cleaner: Command(Boost)
    end
    System-->>Cleaner: Command(Normal)
```

## 7. SSD-05 Escape From Blocked Area

```mermaid
sequenceDiagram
    actor FrontSensor as Front Sensor
    actor Clock as Digital Clock
    participant System as RVC Control SW
    actor LeftSensor as Left Sensor
    actor RightSensor as Right Sensor
    actor Motor
    actor Cleaner

    FrontSensor->>System: onFrontObstacleInterrupt()
    Clock->>System: tick()
    LeftSensor-->>System: leftObstacle = true
    RightSensor-->>System: rightObstacle = true
    System->>Cleaner: turnOff()
    System->>System: enterEscaping()
    loop while front, left, and right are blocked
        System-->>Motor: Command(Backward)
        Clock->>System: tick()
        opt front is blocked
            FrontSensor->>System: onFrontObstacleInterrupt()
        end
        LeftSensor-->>System: leftObstacle
        RightSensor-->>System: rightObstacle
    end
    alt front is open
        System-->>Motor: Command(Forward)
    else side is open
        System-->>Motor: Command(TurnLeft or TurnRight)
    end
```

## 8. System Interface

| Operation | Input | Output | Responsibility |
| --- | --- | --- | --- |
| `startCleaning()` | none | none | 자동 청소를 시작하고 controller state를 cleaning으로 전환한다. |
| `stopCleaning()` | none | none | 이동과 청소를 중지하고 controller state를 idle로 전환한다. |
| `onFrontObstacleInterrupt()` | none | none | 전방 장애물 interrupt를 기록하여 다음 제어 판단에서 즉시 회피하게 한다. |
| `tick(periodicSensors)` | `PeriodicSensorData` | `Command` | 주기 센서 값을 반영하고 다음 motor/cleaner 명령을 결정한다. |
| `readPeriodicSensors(periodicSensors)` | `PeriodicSensorData` | `SensorSnapshot` | 좌측, 우측, 먼지 periodic 값과 pending front interrupt를 하나의 snapshot으로 결합한다. |
| `decideNextCommand(snapshot)` | `SensorSnapshot` | `Command` | 핵심 제어 규칙에 따라 다음 동작 명령을 계산한다. |

## 9. System Operations

| Operation | Related FR | Notes |
| --- | --- | --- |
| `startCleaning()` | FR-01, FR-03 | 실행 상태를 시작하며 실제 전진/청소 명령은 다음 `tick()`에서 생성된다. |
| `stopCleaning()` | FR-02 | 실행 상태와 boost timer를 초기화하며 다음 `tick()`에서 `Stop`/`Off` command가 생성된다. |
| `onFrontObstacleInterrupt()` | FR-04, FR-05 | interrupt는 다음 `tick()`보다 먼저 들어올 수 있다. |
| `tick(periodicSensors)` | FR-06 | Digital Clock의 제어 주기마다 호출된다. |
| `decideNextCommand(snapshot)` | FR-07 to FR-18 | 회피, 탈출, boost, cleaner off 우선순위 규칙을 포함한다. |
