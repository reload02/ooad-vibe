# RVC 테스트 결과

## 1. 테스트 실행 요약

| 항목 | 값 |
| --- | --- |
| 실행일 | 2026-05-21 |
| 실행 시각 | 04:41:04 +09:00 |
| 빌드 갱신 명령 | `cmake --build build --config Debug` |
| 테스트 실행 명령 | `ctest --test-dir build -C Debug --output-on-failure` |
| 빌드 설정 | Debug |
| 테스트 프레임워크 | CTest, GoogleTest |
| 전체 결과 | 57 passed / 0 failed |
| 총 실행 시간 | 0.94 sec |

## 2. 결과 요약

| 구분 | 범위 | 결과 |
| --- | --- | --- |
| 컨트롤러 유닛 테스트 | 17건 | 17 passed / 0 failed |
| 타입 유닛 테스트 | 4건 | 4 passed / 0 failed |
| 시뮬레이터 시스템 테스트 | 9건 | 9 passed / 0 failed |
| 시나리오 파일 테스트 | 5건 | 5 passed / 0 failed |
| 시나리오 회귀 테스트 | 20건 | 20 passed / 0 failed |
| CLI 테스트 | 2건 | 2 passed / 0 failed |

## 3. 컨트롤러 유닛 테스트 결과

컨트롤러 유닛 테스트는 결정적인 센서 입력과 인터럽트 입력을 사용해 `RvcController`의 상태 전이, 회피 판단, 탈출 판단, 먼지 boost 제어 규칙을 검증한다.

| 번호 | 테스트 케이스 | 검증 대상 | 결과 |
| --- | --- | --- | --- |
| 1 | `RvcControllerTest.ControllerMovesForwardWhenPathIsClear` | 전방 경로가 열려 있을 때 청소를 시작하고 전진하는지 검증 | Passed |
| 2 | `RvcControllerTest.IdleControllerReturnsStopAndIgnoresFrontInterrupt` | idle 상태에서 전방 인터럽트와 먼지 감지를 무시하고 정지/꺼짐 상태를 유지하는지 검증 | Passed |
| 3 | `RvcControllerTest.StopCleaningReturnsStopAndOff` | 청소 중지 요청 시 모터 정지와 클리너 끄기 명령을 반환하는지 검증 | Passed |
| 4 | `RvcControllerTest.FrontInterruptTriggersImmediateAvoidance` | 전방 장애물 인터럽트가 즉시 회피 동작으로 이어지고 클리너가 꺼지는지 검증 | Passed |
| 5 | `RvcControllerTest.FrontInterruptIsConsumedAfterOneTick` | 전방 인터럽트가 1 tick 후 소비되어 정상 전진으로 복귀하는지 검증 | Passed |
| 6 | `RvcControllerTest.TurnsTowardOpenSide` | 열린 측면 방향으로 회전하고 회피 중 클리너가 꺼지는지 검증 | Passed |
| 7 | `RvcControllerTest.AlternatesWhenBothSidesAreOpen` | 좌우가 모두 열려 있을 때 회전 방향을 번갈아 선택하는지 검증 | Passed |
| 8 | `RvcControllerTest.AlternationPersistsAcrossClearTicks` | 정상 전진 tick을 거쳐도 좌우 회전 교대 순서가 유지되는지 검증 | Passed |
| 9 | `RvcControllerTest.AllBlockedEntersEscapingAndKeepsBackingUp` | 전방, 좌측, 우측이 모두 막힌 경우 탈출 상태로 진입하고 후진을 유지하는지 검증 | Passed |
| 10 | `RvcControllerTest.AllBlockedWithDustKeepsCleanerOffButPreservesBoostBudget` | 막힘과 먼지 감지가 동시에 발생해도 클리너를 끄고 boost 예산을 보존하는지 검증 | Passed |
| 11 | `RvcControllerTest.EscapingIgnoresOpenFrontUntilSideOpens` | 탈출 중 전방이 열려도 측면 탈출구가 열릴 때까지 후진하는지 검증 | Passed |
| 12 | `RvcControllerTest.EscapingTurnsTowardRightSideExit` | 탈출 중 우측 탈출구가 열리면 우회전하는지 검증 | Passed |
| 13 | `RvcControllerTest.EscapingUsesAlternationWhenBothSidesOpen` | 탈출 중 양측이 모두 열렸을 때 회전 방향 교대 규칙을 적용하는지 검증 | Passed |
| 14 | `RvcControllerTest.DustBoostLastsConfiguredTicks` | 먼지 감지 후 설정된 tick 동안 boost 청소가 유지되는지 검증 | Passed |
| 15 | `RvcControllerTest.DustDetectionRefreshesBoostBudget` | boost 중 먼지를 다시 감지하면 boost 잔여 tick이 갱신되는지 검증 | Passed |
| 16 | `RvcControllerTest.ZeroTickBoostConfigurationDoesNotBoostOnDust` | boost tick이 0인 설정에서 먼지 감지 시에도 normal 청소를 유지하는지 검증 | Passed |
| 17 | `RvcControllerTest.AvoidanceOutputStaysOffWhileBoostStateIsMaintained` | boost 상태가 남아 있어도 회피 중에는 클리너가 꺼지고 전진 복귀 후 boost가 이어지는지 검증 | Passed |

컨트롤러 유닛 테스트 소계: 17 passed / 0 failed.

## 4. 타입 유닛 테스트 결과

타입 유닛 테스트는 방향 전환과 로그용 문자열 변환처럼 다른 모듈이 공유하는 기본 타입 연산의 안정성을 검증한다.

| 번호 | 테스트 케이스 | 검증 대상 | 결과 |
| --- | --- | --- | --- |
| 1 | `RvcTypesTest.TurnLeftCyclesThroughAllDirections` | 좌회전 연산이 네 방향을 올바른 순서로 순환하는지 검증 | Passed |
| 2 | `RvcTypesTest.TurnRightCyclesThroughAllDirections` | 우회전 연산이 네 방향을 올바른 순서로 순환하는지 검증 | Passed |
| 3 | `RvcTypesTest.OppositeMatchesTwoRightTurns` | 반대 방향 계산이 우회전 두 번과 일치하는지 검증 | Passed |
| 4 | `RvcTypesTest.StringConversionsExposeStableLogNames` | 방향, 이동 명령, 청소 세기, 컨트롤러 상태의 로그 문자열이 안정적인지 검증 | Passed |

타입 유닛 테스트 소계: 4 passed / 0 failed.

## 5. 시뮬레이터 시스템 테스트 결과

시뮬레이터 시스템 테스트는 `GridSimulator`, `RvcController`, 맵 렌더링, 로그, 예외 처리가 통합 흐름에서 올바르게 동작하는지 검증한다.

| 번호 | 테스트 케이스 | 검증 대상 | 결과 |
| --- | --- | --- | --- |
| 1 | `RvcSystemTest.SimulatorCleansDustAndLogsCommands` | 시뮬레이터가 먼지를 청소하고 먼지 감지 및 boost 명령 로그를 남기는지 검증 | Passed |
| 2 | `RvcSystemTest.SimulatorUsesBackwardEscape` | 막힌 맵에서 후진 탈출 동작과 클리너 off가 적용되는지 검증 | Passed |
| 3 | `RvcSystemTest.SimulatorKeepsCommandingBackwardWhenBoxedIn` | 로봇이 완전히 갇힌 동안 계속 후진 명령을 내리는지 검증 | Passed |
| 4 | `RvcSystemTest.SimulatorKeepsBackingUpUntilSideExitOpens` | 탈출 중 측면 탈출구가 열릴 때까지 후진하고 열린 측면으로 회전하는지 검증 | Passed |
| 5 | `RvcSystemTest.SimulatorKeepsCleanerOffDuringBoostedEscape` | boost 상태에서도 후진 탈출 및 회피 회전 중에는 클리너가 꺼지는지 검증 | Passed |
| 6 | `RvcSystemTest.SimulatorTurnsAfterFrontInterrupt` | 시뮬레이터가 전방 장애물을 인터럽트로 변환하고 올바르게 회전하는지 검증 | Passed |
| 7 | `RvcSystemTest.SimulatorRunWithZeroTicksLeavesRobotAtInitialPose` | 0 tick 실행 시 로봇 위치, 방향, 먼지, 로그가 초기 상태로 유지되는지 검증 | Passed |
| 8 | `RvcSystemTest.SimulatorRejectsNegativeTicks` | 음수 tick 실행 요청을 예외로 거부하는지 검증 | Passed |
| 9 | `RvcSystemTest.SimulatorCanAppendRenderedFrames` | 맵 렌더링을 포함한 실행 로그가 기대 형식으로 추가되는지 검증 | Passed |

시뮬레이터 시스템 테스트 소계: 9 passed / 0 failed.

## 6. 시나리오 파일 테스트 결과

시나리오 파일 테스트는 `.rvc` 파일 로딩, 필수 맵 검증, 로봇 마커 오류, 음수 tick, 현재 알려진 심볼 검증 간극을 문서화한다.

| 번호 | 테스트 케이스 | 검증 대상 | 결과 |
| --- | --- | --- | --- |
| 1 | `RvcScenarioFileTest.LoadsScenarioFileWithTicksAndMap` | 시나리오 파일에서 tick과 map 섹션을 올바르게 로드하는지 검증 | Passed |
| 2 | `RvcScenarioFileTest.RejectsMissingOrEmptyMapFilesDuringLoad` | map 섹션이 없거나 비어 있는 시나리오를 로딩 단계에서 거부하는지 검증 | Passed |
| 3 | `RvcScenarioFileTest.RejectsRobotMarkerErrorsAfterLoadingMap` | 로봇 마커가 없거나 여러 개인 맵을 시뮬레이터 생성 단계에서 거부하는지 검증 | Passed |
| 4 | `RvcScenarioFileTest.RejectsNegativeTickScenarioWhenRun` | 음수 tick 시나리오가 실행 단계에서 예외 처리되는지 검증 | Passed |
| 5 | `RvcScenarioFileTest.UnknownSymbolScenarioDocumentsCurrentValidationGap` | 알 수 없는 심볼에 대한 현재 검증 간극과 실제 동작을 회귀 기준으로 문서화 | Passed |

시나리오 파일 테스트 소계: 5 passed / 0 failed.

## 7. 시나리오 회귀 테스트 결과

시나리오 회귀 테스트는 `scenarios` 디렉터리의 대표 입력 파일을 실행해 최종 위치, 최종 방향, 청소된 먼지 수, 잔여 먼지 수, 이동 명령 수, 클리너 명령 수, 필수 로그 조각을 함께 검증한다.

| 번호 | 시나리오 | 검증 대상 | 결과 |
| --- | --- | --- | --- |
| 1 | `backward_escape.rvc` | 막힌 구역에서 후진 후 좌측 탈출구로 회전하는 기본 탈출 흐름 | Passed |
| 2 | `backward_escape2.rvc` | 긴 탈출 경로에서 후진과 우회전 탈출이 기대 횟수로 발생하는지 검증 | Passed |
| 3 | `boundary_without_outer_wall.rvc` | 외곽 벽이 없는 경계 맵에서 전방 인터럽트와 회전이 안정적으로 동작하는지 검증 | Passed |
| 4 | `clear_corridor_forward.rvc` | 장애물이 없는 복도에서 계속 전진 청소하는지 검증 | Passed |
| 5 | `continuous_backward.rvc` | 계속 막힌 상태에서 매 tick 후진 명령과 클리너 off를 유지하는지 검증 | Passed |
| 6 | `dense_dust_maze_extreme.rvc` | 먼지가 많은 복잡한 미로에서 boost 청소와 회피가 기대 흐름으로 발생하는지 검증 | Passed |
| 7 | `dust_and_interrupt.rvc` | 먼지 감지와 전방 인터럽트가 함께 있는 시나리오에서 boost와 회피 로그가 남는지 검증 | Passed |
| 8 | `dust_before_dead_end_escape.rvc` | 먼지 청소 직후 막힌 길에서 탈출하며 클리너를 끄는지 검증 | Passed |
| 9 | `dust_trail_boost_refresh.rvc` | 먼지 궤적을 따라 이동하며 boost 예산을 갱신하는지 검증 | Passed |
| 10 | `front_both_sides_open.rvc` | 전방 장애물과 양측 개방 상태에서 좌우 회전을 모두 수행하는지 검증 | Passed |
| 11 | `front_clears_but_sides_still_blocked.rvc` | 전방이 다시 열려도 측면이 막힌 탈출 흐름을 안정적으로 처리하는지 검증 | Passed |
| 12 | `front_left_only_open.rvc` | 좌측만 열린 경우 좌회전 회피를 수행하는지 검증 | Passed |
| 13 | `front_right_only_open.rvc` | 우측만 열린 경우 우회전 회피를 수행하는지 검증 | Passed |
| 14 | `large_open_room_dust_sweep.rvc` | 큰 열린 방에서 먼지 청소와 일반 전진이 기대 횟수로 발생하는지 검증 | Passed |
| 15 | `long_escape_left_exit_extreme.rvc` | 긴 후진 탈출 끝에 좌측 출구로 회전하는 극단 사례 검증 | Passed |
| 16 | `long_escape_right_exit_extreme.rvc` | 긴 후진 탈출 끝에 우측 출구로 회전하는 극단 사례 검증 | Passed |
| 17 | `narrow_tunnel_sides_blocked_front_clear.rvc` | 좁은 터널에서 측면이 막혀도 전방이 열리면 계속 전진하는지 검증 | Passed |
| 18 | `ragged_map_edge_extreme.rvc` | 들쭉날쭉한 맵 경계에서 이동과 전방 인터럽트가 안정적으로 처리되는지 검증 | Passed |
| 19 | `repeated_front_interrupts_alternation.rvc` | 반복되는 전방 인터럽트에서 회전 방향 교대 규칙이 유지되는지 검증 | Passed |
| 20 | `sealed_box_extreme.rvc` | 완전 봉쇄 맵에서 위치와 방향을 유지하며 후진 명령을 반복하는지 검증 | Passed |

시나리오 회귀 테스트 소계: 20 passed / 0 failed.

## 8. CLI 테스트 결과

CLI 테스트는 빌드된 `rvc_simulator` 실행 파일이 기본 옵션과 시나리오 입력 옵션으로 정상 종료되는지 검증한다.

| 번호 | 테스트 케이스 | 검증 대상 | 결과 |
| --- | --- | --- | --- |
| 1 | `SimulatorCliDefaultRuns` | 기본 CLI 시뮬레이터 실행이 성공적으로 완료되는지 검증 | Passed |
| 2 | `SimulatorCliContinuousBackwardScenarioRuns` | CLI가 continuous backward 시나리오를 성공적으로 실행하는지 검증 | Passed |

CLI 테스트 소계: 2 passed / 0 failed.

## 9. 결론

최신 Debug 빌드 기준으로 전체 57개 테스트가 모두 통과했다. 현재 구현은 시작/중지, idle 상태 처리, 장애물 인터럽트 소비, 좌우 회피, 막힌 구역 탈출, 먼지 boost 청소, boost 갱신, 회피/탈출 중 클리너 off, 타입 변환, 시뮬레이터 로그와 렌더링, 시나리오 파일 로딩 및 오류 처리, 대표 시나리오 회귀 동작, CLI 실행에 대해 검증된 동작을 만족한다.
