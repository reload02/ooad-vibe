# RVC Traceability

| Requirement | Use Case | Design Element | Verification |
| --- | --- | --- | --- |
| FR-01 | UC-01 | `Rvc`, `RvcController`, `NavigationPolicy` | `RvcFacadeTest`, controller tests |
| FR-02 | UC-02 | `Rvc`, `CommandComposer` | controller tests |
| FR-03 | UC-01 | `NavigationPolicy`, `Command` | controller and scenario tests |
| FR-04 | UC-03, UC-04 | `SensorFusion` | `SensorFusionCombinesFrontInterruptAndPeriodicInputsWithoutRightSensor` |
| [변경] FR-05 to FR-06 | UC-03, UC-05 | `SensorFusion`, `PeriodicSensorData` | subsystem and controller tests |
| [삭제] FR-07-old to FR-08-old | UC-03 | removed right periodic sensor and alternation policy | replaced by probe tests |
| [변경] FR-09 to FR-10 | UC-04 | `NavigationPolicy`, `ControllerState::Escaping` | controller and scenario tests |
| [신규] FR-11 to FR-13 | UC-04 | `NavigationPolicy::EscapeProbePhase` | right probe controller, subsystem, scenario tests |
| FR-14 to FR-16 | UC-05, UC-03, UC-04 | `CleaningPolicy`, `CommandComposer` | cleaning policy and controller tests |
| [변경] FR-17 to FR-18 | UC-06 | `GridSimulator`, CLI | system, scenario, CLI tests |
