# RVC Traceability

## 1. Requirement to Design Traceability

| Requirement | Use Case / Verification Item | OOA SSD | OOD SD | Class |
| --- | --- | --- | --- | --- |
| FR-01 | UC-01 | SSD-01 | SD-01 | `RvcController` |
| FR-02 | UC-02 | SSD-06 | SD-01 | `RvcController` |
| FR-03 | UC-01 | SSD-01 | SD-01, SD-04 | `RvcController`, `Command` |
| FR-04 | UC-03 | SSD-02 | SD-02 | `RvcController` |
| FR-05 | UC-03 | SSD-02 | SD-02, SD-04 | `RvcController`, `Command` |
| FR-06 | UC-03, UC-05 | SSD-03 | SD-03 | [R2-변경] `PeriodicSensorData`, `SensorSnapshot` without `rightObstacle` |
| FR-07 | UC-03 | SSD-02 | SD-04 | `RvcController` |
| FR-08 | UC-03 | SSD-02 | SD-04 | [R2-변경] `RightProbing`, `RvcController` |
| FR-09 | UC-03 | SSD-02 | SD-04 | [R2-삭제] ~~좌우 교대 정책~~ |
| FR-10 | UC-04 | SSD-05 | SD-05 | [R2-변경] `EscapeAligning`, `Escaping`, `RvcController` |
| FR-11 | UC-04 | SSD-05 | SD-05 | [R2-변경] `Escaping`, `RightProbing` |
| FR-12 | UC-04 | SSD-05 | SD-05 | [R2-변경] `SensorSnapshot`, `rightProbe` |
| FR-13 | UC-04 | SSD-05 | SD-05 | [R2-변경] `RvcController`, `Command` |
| FR-14 | UC-05 | SSD-04 | SD-06 | `RvcController` |
| FR-15 | UC-05 | SSD-04 | SD-06 | `RvcController` |
| FR-16 | VS-01 | SSD-03 | SD-01 | `GridSimulator` |
| FR-17 | VS-01 | SSD-03 | SD-01 | `GridSimulator`, `SimulationResult` |
| FR-18 | VS-01 | SSD-01 to SSD-05 | SD-01 | [변경] `Rvc`, `RvcHardwareAdapter`, `SimulatedHardwareAdapter`, `GridSimulator` |

## 2. Requirement to Test Traceability

| Requirement | Test Case |
| --- | --- |
| FR-01, FR-03 | `ControllerMovesForwardWhenPathIsClear` |
| FR-02 | `StopCleaningReturnsStopAndOff` |
| FR-04, FR-05 | `FrontInterruptTriggersImmediateAvoidance`, `SimulatorTurnsAfterFrontInterrupt` |
| FR-07, FR-08 | [R2-변경] `FrontInterruptTriggersImmediateAvoidance`, `RightProbeStartsWhenLeftBlocked`, `SimulatorProbesRightAfterFrontInterrupt` |
| FR-09 | [R2-삭제] ~~`AlternatesWhenBothSidesAreOpen`~~ |
| FR-10, FR-11 | [R2-변경] `RightProbeBlockedAlignsBeforeEscaping`, `EscapingBacksUpThenReprobesRight`, `SimulatorRestoresHeadingBeforeBackward` |
| FR-12, FR-13 | [R2-변경] `EscapingTurnsLeftWhenLeftOpens`, `RightProbeOpenResumesForward`, `SimulatorRepeatsRightProbeAfterBackward` |
| FR-14, FR-15 | `DustBoostLastsConfiguredTicks`, `AvoidanceOutputStaysOffWhileBoostStateIsMaintained`, `SimulatorCleansDustAndLogsCommands`, `SimulatorKeepsCleanerOffDuringBoostedEscape` |
| FR-16, FR-17, FR-18 | [변경] `SimulatorCleansDustAndLogsCommands`, `SimulatorUsesBackwardEscape`, `SimulatorKeepsBackingUpUntilSideExitOpens`, `SimulatorCliDefaultRuns`, `SimulatorCliContinuousBackwardScenarioRuns`는 `GridSimulator`와 `SimulatedHardwareAdapter` 기반으로 `Rvc` 흐름을 검증한다. |

## 3. 최신 테스트 실행 결과

| 항목 | 결과 |
| --- | --- |
| 실행일 | 2026-05-20 |
| 빌드 갱신 명령 | `cmake --build build --config Debug` |
| 실행 명령 | `ctest --test-dir build -C Debug --output-on-failure` |
| 유닛 테스트 | [R2-변경] 우측 센서 제거 정책 반영 후 재검증 필요 |
| 시스템 및 CLI 테스트 | [R2-변경] 우측 탐색 시나리오 교체 후 재검증 필요 |
| 전체 | [R2-변경] 기존 결과는 우측 센서 기반 정책의 과거 결과이며 새 정책 기준으로 재실행 필요 |
| 상세 문서 | `docs/test_results.md` |
