# RVC 테스트 결과

## 1. 테스트 실행 요약

| 항목 | 값 |
| --- | --- |
| 실행일 | 2026-05-20 |
| 실행 명령 | `ctest --test-dir build -C Debug --output-on-failure` |
| 빌드 설정 | Debug |
| 테스트 프레임워크 | CTest, GoogleTest |
| 전체 결과 | 14 passed / 0 failed |
| 총 실행 시간 | 0.22 sec |

## 2. 유닛 테스트 결과

유닛 테스트는 결정적인 센서 입력을 사용해 `RvcController`의 제어 판단 규칙을 검증한다.

| 번호 | 테스트 케이스 | 검증 대상 | 결과 |
| --- | --- | --- | --- |
| 1 | `RvcControllerTest.ControllerMovesForwardWhenPathIsClear` | 전방 경로가 열려 있을 때 청소를 시작하고 전진하는지 검증 | Passed |
| 2 | `RvcControllerTest.StopCleaningReturnsStopAndOff` | 청소 중지 요청 시 모터 정지와 클리너 끄기 명령을 반환하는지 검증 | Passed |
| 3 | `RvcControllerTest.FrontInterruptTriggersImmediateAvoidance` | 전방 장애물 인터럽트가 즉시 회피 동작으로 이어지는지 검증 | Passed |
| 4 | `RvcControllerTest.TurnsTowardOpenSide` | 열린 측면 방향으로 회전하는지 검증 | Passed |
| 5 | `RvcControllerTest.AlternatesWhenBothSidesAreOpen` | 좌우가 모두 열려 있을 때 회전 방향을 번갈아 선택하는지 검증 | Passed |
| 6 | `RvcControllerTest.AllBlockedEntersEscapingAndKeepsBackingUp` | 전방, 좌측, 우측이 모두 막힌 경우 탈출 상태로 진입하고 후진하는지 검증 | Passed |
| 7 | `RvcControllerTest.EscapingExitsWhenFrontBecomesOpen` | 탈출 중 전방이 열리면 청소 상태로 복귀하는지 검증 | Passed |
| 8 | `RvcControllerTest.DustBoostLastsConfiguredTicks` | 먼지 감지 후 설정된 tick 동안 boost 청소가 유지되는지 검증 | Passed |

유닛 테스트 소계: 8 passed / 0 failed.

## 3. 시스템 테스트 결과

시스템 테스트는 `GridSimulator`, `RvcController`, 시나리오 맵, 로그, CLI 실행이 통합 흐름에서 올바르게 동작하는지 검증한다.

| 번호 | 테스트 케이스 | 검증 대상 | 결과 |
| --- | --- | --- | --- |
| 1 | `RvcSystemTest.SimulatorCleansDustAndLogsCommands` | 시뮬레이터가 먼지를 청소하고 먼지 감지 및 boost 명령 로그를 남기는지 검증 | Passed |
| 2 | `RvcSystemTest.SimulatorUsesBackwardEscape` | 막힌 맵에서 시뮬레이터가 후진 탈출 동작을 적용하는지 검증 | Passed |
| 3 | `RvcSystemTest.SimulatorKeepsCommandingBackwardWhenBoxedIn` | 로봇이 완전히 갇힌 동안 계속 후진 명령을 내리는지 검증 | Passed |
| 4 | `RvcSystemTest.SimulatorTurnsAfterFrontInterrupt` | 시뮬레이터가 전방 장애물을 인터럽트로 변환하고 올바르게 회전하는지 검증 | Passed |
| 5 | `SimulatorCliDefaultRuns` | 기본 CLI 시뮬레이터 실행이 성공적으로 완료되는지 검증 | Passed |
| 6 | `SimulatorCliContinuousBackwardScenarioRuns` | CLI가 continuous backward 시나리오를 성공적으로 실행하는지 검증 | Passed |

시스템 테스트 소계: 6 passed / 0 failed.

## 4. 결론

모든 유닛 테스트, 시스템 테스트, CLI 테스트가 통과했다. 현재 구현은 시작/중지, 장애물 회피, 막힌 구역 탈출, 먼지 boost 청소, 시뮬레이터 로그, 시나리오 기반 실행에 대해 검증된 동작을 만족한다.
