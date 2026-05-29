# RVC Traceability

| Requirement | Use Case | Design Element | Verification |
| --- | --- | --- | --- |
| FR-01 | UC-01 | `Rvc`, `RvcController`, `NavigationPolicy` | `RvcFacadeTest`, controller tests |
| FR-02 | UC-02 | `Rvc`, `CommandComposer` | controller tests |
| FR-03 | UC-01 | `NavigationPolicy`, `Command` | controller and scenario tests |
| FR-04 | UC-03 | `SensorFusion` | `SensorFusionCombinesFrontInterruptAndPeriodicInputs` |
| FR-05 | UC-03, UC-05 | `SensorFusion`, `PeriodicSensorData` | subsystem and controller tests |
| FR-06 to FR-08 | UC-03 | `NavigationPolicy` | navigation and controller tests |
| FR-09 to FR-11 | UC-04 | `NavigationPolicy`, `ControllerState::Escaping` | controller and scenario tests |
| FR-12 to FR-13 | UC-05 | `CleaningPolicy` | cleaning policy and controller tests |
| FR-14 | UC-03, UC-04 | `CommandComposer` | command composer and system tests |
| FR-15 to FR-16 | UC-06 | `GridSimulator`, CLI | system, scenario, CLI tests |
