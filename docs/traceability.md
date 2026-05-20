# RVC Traceability

## 1. Requirement to Design Traceability

| Requirement | Use Case / Verification Item | OOA SSD | OOD SD | Class |
| --- | --- | --- | --- | --- |
| FR-01 | UC-01 | SSD-01 | SD-01 | `RvcController` |
| FR-02 | UC-02 | SSD-06 | SD-01 | `RvcController` |
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
| FR-16 | VS-01 | SSD-03 | SD-01 | `GridSimulator` |
| FR-17 | VS-01 | SSD-03 | SD-01 | `GridSimulator`, `SimulationResult` |
| FR-18 | VS-01 | SSD-01 to SSD-05 | SD-01 | `RvcController`, `GridSimulator` |

## 2. Requirement to Test Traceability

| Requirement | Test Case |
| --- | --- |
| FR-01, FR-03 | `ControllerMovesForwardWhenPathIsClear` |
| FR-02 | `StopCleaningReturnsStopAndOff` |
| FR-04, FR-05 | `FrontInterruptTriggersImmediateAvoidance`, `SimulatorTurnsAfterFrontInterrupt` |
| FR-07, FR-08 | `FrontInterruptTriggersImmediateAvoidance`, `TurnsTowardOpenSide`, `SimulatorTurnsAfterFrontInterrupt` |
| FR-09 | `AlternatesWhenBothSidesAreOpen` |
| FR-10, FR-11 | `AllBlockedEntersEscapingAndKeepsBackingUp` |
| FR-12, FR-13 | `EscapingExitsWhenFrontBecomesOpen` |
| FR-14, FR-15 | `DustBoostLastsConfiguredTicks` |
| FR-16, FR-17, FR-18 | `SimulatorCleansDustAndLogsCommands`, `SimulatorUsesBackwardEscape`, `SimulatorCliDefaultRuns`, `SimulatorCliContinuousBackwardScenarioRuns` |

## 3. 최신 테스트 실행 결과

| 항목 | 결과 |
| --- | --- |
| 실행일 | 2026-05-20 |
| 실행 명령 | `ctest --test-dir build -C Debug --output-on-failure` |
| 유닛 테스트 | 8 passed / 0 failed |
| 시스템 및 CLI 테스트 | 6 passed / 0 failed |
| 전체 | 14 passed / 0 failed |
| 상세 문서 | `docs/test_results.md` |
