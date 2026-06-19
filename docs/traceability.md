# RVC Traceability

## 1. Requirement to Design Traceability

| Requirement | Use Case / Verification Item | OOA SSD | OOD SD | Class / Design Element |
| --- | --- | --- | --- | --- |
| FR-01 | UC-01 | SSD-01 | SD-01 | `RvcController`, `Rvc` |
| FR-02 | UC-02 | SSD-06 | SD-01 | `RvcController`, `Command` |
| FR-03 | UC-01 | SSD-01 | SD-01, SD-04 | `RvcController`, `NavigationPolicy`, `Command` |
| FR-04 | UC-03 | SSD-02 | SD-02 | `RvcController`, `RvcHardwareAdapter` |
| FR-05 | UC-03 | SSD-02 | SD-02, SD-04 | `RvcController`, `Command` |
| FR-06 | UC-03, UC-06 | SSD-03 | SD-03 | [R2-변경] `PeriodicSensorData`, `SensorSnapshot` without `rightObstacle` |
| FR-07 | UC-03 | SSD-02 | SD-04 | `NavigationPolicy`, `Command` |
| FR-08 | UC-03 | SSD-02 | SD-04 | [R2-변경] `RightProbing`, `NavigationPolicy` |
| FR-09 | UC-03 | SSD-02 | SD-04 | [R2-삭제] ~~좌우 교대 정책~~ |
| FR-10 | UC-04 | SSD-05 | SD-05 | [R2-변경] `EscapeAligning`, `Escaping`, `NavigationPolicy` |
| FR-11 | UC-04 | SSD-05 | SD-05 | [R2-변경] `Escaping`, `RightProbing` |
| FR-12 | UC-04 | SSD-05 | SD-05 | [R2-변경] `SensorSnapshot`, `rightProbe` |
| FR-13 | UC-04 | SSD-05 | SD-05 | [R2-변경] `NavigationPolicy`, `Command` |
| FR-14 | UC-06 | SSD-04 | SD-06 | [R2-기존] `CleaningPowerPolicy`, `ControllerConfig::dustBoostTicks` |
| FR-15 | UC-06 | SSD-04 | SD-06 | [R2-기존] `CleaningPowerPolicy` boost aging |
| FR-16 | VS-01 | SSD-03 | SD-01 | `GridSimulator`, `SimulatedHardwareAdapter` |
| FR-17 | VS-01 | SSD-03 | SD-01 | `GridSimulator`, `SimulationResult` |
| FR-18 | VS-01 | SSD-01 to SSD-05 | SD-01 | [변경] `Rvc`, `RvcHardwareAdapter`, `SimulatedHardwareAdapter`, `GridSimulator` |
| FR-19 | UC-05 | SSD-07 | SD-07 | [R3-추가] `RearSensor`, rear obstacle interrupt input |
| FR-20 | UC-05 | SSD-07 | SD-07 | [R3-추가] backward travel obstacle avoidance, rear interrupt handling |
| FR-21 | UC-01, UC-03, UC-04, UC-05 | SSD-01, SSD-02, SSD-05, SSD-07 | SD-04, SD-05, SD-07 | [R3-변경] cleaner default `Normal` policy during cleaning |
| FR-22 | UC-06 | SSD-04 | SD-06 | [R3-변경] forward dust detection, clockwise in-place `Boost`, backward travel mode |
| FR-23 | UC-06 | SSD-04 | SD-06 | [R3-변경] backward dust detection, counterclockwise in-place `Boost`, forward travel mode |
| FR-24 | UC-03, UC-04, UC-05, UC-06 | SSD-02, SSD-04, SSD-05, SSD-07 | SD-04, SD-05, SD-06, SD-07 | [R3-변경] movement-time cleaner output policy |
| FR-25 | UC-06, VS-01 | SSD-04 | SD-06 | [R3-변경] persistent dust fixture and repeated detection |
| FR-26 | VS-01 | SSD-03 | SD-01 | [R4-검증] return-to-start scenario duration |

## 2. Requirement to Test Traceability

| Requirement | Test Case |
| --- | --- |
| FR-01, FR-03 | `ControllerMovesForwardWhenPathIsClear`, `RvcTest.TickReadsAdapterInputsAndAppliesControllerCommand` |
| FR-02 | `StopCleaningReturnsStopAndOff`, `RvcTest.StopCleaningStillAppliesIdleCommandThroughAdapter` |
| FR-04, FR-05 | `FrontInterruptTriggersImmediateAvoidanceWhenLeftIsOpen`, `SimulatorTurnsAfterFrontInterrupt` |
| FR-07, FR-08 | [R2-변경] `FrontInterruptTriggersImmediateAvoidanceWhenLeftIsOpen`, `RightProbeStartsWhenLeftBlocked`, `SimulatorProbesRightAfterFrontInterrupt` |
| FR-09 | [R2-삭제] ~~`AlternatesWhenBothSidesAreOpen`~~ |
| FR-10, FR-11 | [R2-변경] `RightProbeBlockedAlignsBeforeEscaping`, `EscapingBacksUpThenReprobesRight`, `SimulatorRestoresHeadingBeforeBackward`, `SimulatorRepeatsProbeCycleWhenBoxedIn` |
| FR-12, FR-13 | [R2-변경] `EscapingTurnsLeftWhenLeftOpens`, `RightProbeOpenResumesForward`, `SimulatorRepeatsRightProbeAfterBackward` |
| FR-14, FR-15 | [R2-기존] `DustBoostLastsConfiguredTicks`, `DustDetectionRefreshesBoostBudget`, `CleaningPowerPolicyTest.DustRefreshesAndAgesBoostBudget`, `SimulatorCleansDustAndLogsCommands`, `SimulatorKeepsCleanerOffDuringBoostedEscape` |
| FR-16, FR-17, FR-18 | [변경] `SimulatorCleansDustAndLogsCommands`, `SimulatorRestoresHeadingBeforeBackward`, `SimulatorCliDefaultRuns`, `SimulatorCliContinuousBackwardScenarioRuns`, `SimulatorRvcInterfaceSmoke`는 `GridSimulator`와 `SimulatedHardwareAdapter` 기반으로 `Rvc` 흐름을 검증한다. |
| FR-19, FR-20 | [R3-추가] 후방 interrupt API와 후진 중 후방 장애물 회피 테스트 추가 필요 |
| FR-21, FR-24 | [R3-변경] 주행/회피/탈출 중 cleaner `Normal` 유지 테스트 추가 필요. 기존 `*CleanerOff*` 테스트는 R2 정책 회귀 기준으로만 남는다. |
| FR-22, FR-23 | [R3-변경] 먼지 감지 시 주행 방향별 제자리 회전 및 전진/후진 toggle 테스트 추가 필요 |
| FR-25 | [R3-변경] 먼지 셀 persistent 및 재방문 재감지 테스트 추가 필요 |
| FR-26 | [R4-검증] `return_to_start_dust_refresh.rvc` 또는 동등 시나리오의 충분 tick 실행 및 시작 위치 복귀 검증 추가 필요 |

## 3. 최신 테스트 실행 결과

| 항목 | 결과 |
| --- | --- |
| 실행일 | 2026-06-19 |
| 실행 명령 | `"C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe" --test-dir build-ninja --output-on-failure` |
| 전체 결과 | 64 passed / 0 failed |
| 검증 범위 | 현재 구현의 R2 우측 탐색, `Rvc`/adapter orchestration, fixed tick dust boost, 회피/탈출 중 cleaner off 정책 |
| R3/R4 상태 | `docs/requirements.md`에는 반영되었지만 구현 및 테스트는 아직 추가 필요 |
| 상세 문서 | `docs/test_results.md` |
