# RVC Traceability

## 1. Requirement to Design Traceability

| Requirement | Use Case | OOA SSD | OOD SD | Class |
| --- | --- | --- | --- | --- |
| FR-01 | UC-01 | SSD-01 | SD-01 | `RvcController` |
| FR-02 | UC-02 | SSD-01 | SD-01 | `RvcController` |
| FR-03 | UC-01 | SSD-01 | SD-01, SD-04 | `RvcController`, `Command` |
| FR-04 | UC-03 | SSD-02 | SD-02 | `RvcController` |
| FR-05 | UC-03 | SSD-02 | SD-02, SD-04 | `RvcController`, `Command` |
| FR-06 | UC-03, UC-05 | SSD-03 | SD-03 | `PeriodicSensorData`, `SensorSnapshot` |
| FR-07 | UC-03 | SSD-02 | SD-04 | `RvcController` |
| FR-08 | UC-03 | SSD-02 | SD-04 | `RvcController` |
| FR-09 | UC-03 | SSD-02 | SD-04 | `RvcController` |
| FR-10 | UC-04 | SSD-05 | SD-05 | `RvcController` |
| FR-11 | UC-04 | SSD-05 | SD-05 | `RvcController` |
| FR-12 | UC-04 | SSD-05 | SD-05 | `SensorSnapshot` |
| FR-13 | UC-04 | SSD-05 | SD-05 | `RvcController` |
| FR-14 | UC-05 | SSD-04 | SD-06 | `RvcController` |
| FR-15 | UC-05 | SSD-04 | SD-06 | `RvcController` |
| FR-16 | UC-03, UC-04 | SSD-02, SSD-05 | SD-04, SD-05 | `RvcController`, `Command` |
| FR-17 | UC-01, UC-05 | SSD-01, SSD-04 | SD-04, SD-06 | `RvcController`, `Command` |
| FR-18 | UC-03, UC-04, UC-05 | SSD-02, SSD-04, SSD-05 | SD-04, SD-05, SD-06 | `RvcController`, `Command` |
| FR-19 | UC-06 | SSD-03 | SD-01 | `GridSimulator` |
| FR-20 | UC-06 | SSD-03 | SD-01 | `GridSimulator`, `SimulationResult` |
| FR-21 | UC-06 | SSD-01 to SSD-05 | SD-01 | `RvcController`, `GridSimulator` |

## 2. Requirement to Test Traceability

| Requirement | Test Case |
| --- | --- |
| FR-01, FR-03 | `ControllerMovesForwardWhenPathIsClear` |
| FR-04, FR-05 | `FrontInterruptTriggersImmediateAvoidance` |
| FR-07, FR-08 | `TurnsTowardOpenSide` |
| FR-09 | `AlternatesWhenBothSidesAreOpen` |
| FR-10, FR-11 | `AllBlockedEntersEscapingAndKeepsBackingUp` |
| FR-12, FR-13 | `EscapingIgnoresOpenFrontUntilSideOpens`, `SimulatorKeepsBackingUpUntilSideExitOpens` |
| FR-14, FR-15 | `DustBoostLastsConfiguredTicks` |
| FR-16 | 향후 추가: 회피 회전 및 탈출 후진/회전 명령의 `cleaningPower == Off` 검증 |
| FR-17 | `ControllerMovesForwardWhenPathIsClear`, `DustBoostLastsConfiguredTicks` |
| FR-18 | 향후 추가: dust boost 중 회피/탈출은 `Off`, 전진 재개 시 남은 boost 반영 검증 |
| FR-19, FR-20, FR-21 | `SimulatorCleansDustAndLogsCommands`, `SimulatorUsesBackwardEscape`, `SimulatorKeepsBackingUpUntilSideExitOpens` |
