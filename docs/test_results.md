# RVC 테스트 결과

## 1. 테스트 실행 요약

| 항목 | 값 |
| --- | --- |
| 실행일 | 2026-06-19 |
| 테스트 실행 명령 | `"C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe" --test-dir build-ninja --output-on-failure` |
| 빌드 디렉터리 | `build-ninja` |
| 테스트 프레임워크 | CTest, GoogleTest |
| 전체 결과 | 64 passed / 0 failed |
| 총 실행 시간 | 3.03 sec |
| 검증 범위 | 현재 구현의 R2 우측 탐색, `Rvc`/adapter orchestration, fixed tick dust boost, 회피/탈출 중 cleaner off 정책 |
| R3/R4 상태 | 요구사항과 설계 문서에는 반영되었지만 후방 interrupt, dust rotation, cleaner `Normal` 기본 정책, persistent dust, 시작 위치 복귀 검증 테스트는 추가 필요 |

## 2. 결과 요약

| 구분 | 범위 | 결과 |
| --- | --- | --- |
| 컨트롤러 유닛 테스트 | 15건 | 15 passed / 0 failed |
| 정책 유닛 테스트 | 6건 | 6 passed / 0 failed |
| RVC orchestration 테스트 | 2건 | 2 passed / 0 failed |
| 시뮬레이터 시스템 테스트 | 9건 | 9 passed / 0 failed |
| 시나리오 파일 테스트 | 5건 | 5 passed / 0 failed |
| 타입 유닛 테스트 | 4건 | 4 passed / 0 failed |
| 시나리오 회귀 테스트 | 20건 | 20 passed / 0 failed |
| CLI 테스트 | 2건 | 2 passed / 0 failed |
| 외부 인터페이스 smoke 테스트 | 1건 | 1 passed / 0 failed |

## 3. 주요 통과 항목

| 영역 | 대표 테스트 | 검증 내용 |
| --- | --- | --- |
| 시작/중지 | `ControllerMovesForwardWhenPathIsClear`, `StopCleaningReturnsStopAndOff` | 자동 청소 시작 후 전진, 중지 후 `Stop`/`Off` command |
| 전방 interrupt | `FrontInterruptTriggersImmediateAvoidanceWhenLeftIsOpen`, `SimulatorTurnsAfterFrontInterrupt` | front interrupt pending 처리와 좌측 open 회피 |
| R2 우측 탐색 | `RightProbeStartsWhenLeftBlocked`, `RightProbeOpenResumesForward`, `RightProbeBlockedAlignsBeforeEscaping` | 우측 센서 없이 `TurnRight` 후 front interrupt로 기존 우측 방향 판단 |
| 탈출 | `EscapingBacksUpThenReprobesRight`, `SimulatorRepeatsRightProbeAfterBackward`, `SimulatorRepeatsProbeCycleWhenBoxedIn` | 우측 탐색 실패 후 원래 방향 복구, 후진, 우측 재탐색 반복 |
| R2 dust boost | `DustBoostLastsConfiguredTicks`, `DustDetectionRefreshesBoostBudget`, `CleaningPowerPolicyTest.DustRefreshesAndAgesBoostBudget` | fixed tick boost 유지와 먼지 재감지 시 boost budget 갱신 |
| RVC/adapter 연결 | `RvcTest.TickReadsAdapterInputsAndAppliesControllerCommand`, `SimulatorRvcInterfaceSmoke` | `Rvc`가 adapter 입력을 읽고 controller command를 adapter에 적용하는 흐름 |
| 시나리오 회귀 | `ScenarioFiles/RvcScenarioRegressionTest.*` | `scenarios/`의 20개 정상 시나리오가 기대 위치, 방향, 로그 카운트와 일치 |

## 4. R3/R4 추가 검증 필요 항목

| 요구사항 | 현재 상태 | 필요한 테스트 |
| --- | --- | --- |
| FR-19, FR-20 | 후방 interrupt API와 후진 중 후방 회피는 아직 테스트 없음 | rear interrupt pending, 후진 즉시 중단, 좌측 open/blocked 분기 |
| FR-21, FR-24 | 현재 구현은 R2의 movement-time cleaner off 정책을 검증함 | 회피/탈출/후진 중 cleaner `Normal` 유지 검증 |
| FR-22, FR-23 | 현재 구현은 fixed tick boost를 검증함 | 전진/후진 중 먼지 감지 시 제자리 `Boost` 회전과 travel direction toggle 검증 |
| FR-25 | 현재 구현은 dust cell 제거 및 `dustCleaned` 카운트를 사용함 | `Boost` 후 dust cell persistent, 재방문 재감지 검증 |
| FR-26 | `return_to_start_dust_refresh.rvc`는 존재하지만 회귀 테스트에 아직 연결되지 않음 | 충분 tick 실행 후 시작 위치 복귀 여부 검증 |

## 5. 결론

2026-06-19 기준 기존 구현의 R2 회귀 테스트는 모두 통과했다. 다만 `docs/requirements.md`의 R3/R4 변경은 구현과 자동 테스트에 아직 반영되지 않았으므로, 다음 구현 단계에서는 위 추가 검증 항목을 테스트 우선순위로 둔다.
