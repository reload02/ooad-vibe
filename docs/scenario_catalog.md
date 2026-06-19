# 시나리오 카탈로그

이 문서는 `docs/requirements.md`의 Use Case, FR, NFR을 기준으로 CLI 시뮬레이터 입력 시나리오를 정리한다. 현재 `.rvc` 파일 대부분은 R2 우측 탐색 및 fixed tick dust boost 정책을 검증한다. R3/R4 요구사항은 설계 문서에는 반영되었지만 후방 interrupt, cleaner `Normal` 기본 정책, dust rotation, persistent dust, 시작 위치 복귀 검증 테스트는 추가 또는 재정의가 필요하다.

## 정상 실행 시나리오

| 파일 | 관련 항목 | 의도 |
| --- | --- | --- |
| `clear_corridor_forward.rvc` | UC-01, FR-01, FR-03, FR-17 | 장애물이 없을 때 cleaner normal 상태로 계속 전진 청소하는 기본 흐름 |
| `front_left_only_open.rvc` | UC-03, FR-04, FR-05, FR-07, FR-16 | [R2-변경] 전방 interrupt 후 좌측이 열려 있을 때 좌회전 |
| `front_right_only_open.rvc` | UC-03, FR-04, FR-05, FR-08, FR-12, FR-13, FR-16 | [R2-변경] 좌측 blocked 후 `TurnRight`로 우측을 탐색하고 전방 센서가 clear이면 전진 재개 |
| `front_both_sides_open.rvc` | UC-03, FR-09, FR-16, NFR-08 | [R2-삭제] ~~좌우가 모두 열려 있을 때 결정적 회전 정책 확인~~. 현재는 R2 삭제 정책의 회귀 호환 확인 용도 |
| `repeated_front_interrupts_alternation.rvc` | UC-03, FR-09, FR-16, NFR-08 | [R2-삭제] ~~전방 interrupt 반복 시 좌우 선택 정책 확인~~. 현재는 삭제된 정책의 영향 범위 확인 용도 |
| `long_escape_right_exit_extreme.rvc` | UC-04, FR-10, FR-11, FR-12, FR-13, FR-16 | [R2-변경] 긴 후진 탈출 중 우측 탐색을 반복하고 우측 탐색 open 시 전진 재개 |
| `long_escape_left_exit_extreme.rvc` | UC-04, FR-10, FR-11, FR-12, FR-13, FR-16 | 긴 후진 탈출 끝에 좌측 탈출구가 열릴 때 좌회전으로 탈출 |
| `sealed_box_extreme.rvc` | UC-04, FR-10, FR-11, FR-16 | 완전 고립 상태에서 후진 명령은 반복되지만 위치가 유지되는지 확인 |
| `front_clears_but_sides_still_blocked.rvc` | UC-04, FR-11, FR-12, FR-16 | [R2-변경] Escaping 중 좌측 blocked이면 우측 탐색 결과를 기준으로 탈출 가능성을 판단 |
| `narrow_tunnel_sides_blocked_front_clear.rvc` | UC-01, UC-03, FR-03, FR-06 | [R2-변경] 좌측 periodic sensor가 blocked여도 전방이 열려 있으면 계속 전진해야 하는 조건 |
| `backward_escape.rvc` | UC-04, FR-10, FR-11, FR-12, FR-16 | 막힌 영역에서 후진 후 좌측 탈출구로 회전하는 기본 탈출 흐름 |
| `backward_escape2.rvc` | UC-04, FR-10, FR-11, FR-12, FR-13, FR-16 | 더 긴 탈출 경로에서 후진과 우측 탐색이 반복되는 흐름 |
| `continuous_backward.rvc` | UC-04, FR-10, FR-11, FR-16, FR-17 | 계속 막힌 상태에서 후진/우측 탐색 반복 로그가 안정적인지 확인 |
| `dust_trail_boost_refresh.rvc` | UC-06, FR-14, FR-15, FR-17 | [R2-기존] 먼지가 반복 감지될 때 fixed boost 잔여 tick이 갱신되는지 확인 |
| `dust_before_dead_end_escape.rvc` | UC-04, UC-06, FR-10, FR-11, FR-14, FR-16, FR-18 | [R2-기존] dust boost와 막다른 곳 탈출이 겹칠 때 회피/탈출 중 cleaner off 우선순위 확인 |
| `dust_and_interrupt.rvc` | UC-03, UC-06, FR-04, FR-14, FR-16, FR-17 | 먼지 감지와 전방 interrupt가 함께 있을 때 로그와 회피 흐름 확인 |
| `dense_dust_maze_extreme.rvc` | UC-03, UC-04, UC-06, FR-04, FR-14, FR-16, FR-17, FR-18 | 장애물과 먼지가 조밀할 때 R2 boost와 회피가 함께 나타나는 복합 지도 |
| `boundary_without_outer_wall.rvc` | UC-03, FR-04, FR-16, NFR-08 | 외곽 벽이 없는 지도에서 경계 밖을 장애물처럼 처리하는지 확인 |
| `ragged_map_edge_extreme.rvc` | UC-03, FR-04, FR-16, NFR-08 | 행 길이가 다른 지도에서 바깥 영역을 장애물처럼 처리하는지 확인 |
| `large_open_room_dust_sweep.rvc` | UC-01, UC-06, FR-03, FR-14, FR-16, FR-17 | 넓은 공간에서 장시간 전진, 먼지 감지, 벽 회피가 이어지는 상황 |
| `return_to_start_dust_refresh.rvc` | VS-01, FR-25, FR-26 | [R3/R4 후보] persistent dust 재감지와 시작 위치 복귀를 충분 tick으로 확인하기 위한 시나리오. 현재 구현 기준 테스트 연결 추가 필요 |

## 오류 기대 시나리오

| 파일 | 관련 항목 | 기대 결과 |
| --- | --- | --- |
| `error_cases/no_robot_invalid.rvc` | NFR-02, NFR-07 | 로봇 마커가 없어 실패 |
| `error_cases/multiple_robots_invalid.rvc` | NFR-02, NFR-07 | 로봇 마커가 둘 이상이라 실패 |
| `error_cases/empty_map_invalid.rvc` | NFR-02, NFR-07 | map 섹션에 지도 행이 없어 실패 |
| `error_cases/missing_map_invalid.rvc` | NFR-02, NFR-07 | map 섹션이 없어 실패 |
| `error_cases/negative_ticks_invalid.rvc` | NFR-02, NFR-07 | 음수 tick 실행이 거부되어 실패 |
| `error_cases/unknown_symbol_validation_gap.rvc` | NFR-02, NFR-07 | 현재는 통과할 수 있으나 입력 검증 강화 후 실패해야 하는 알려진 간극 |

## R3/R4 추가 필요 시나리오

| 요구사항 | 필요한 시나리오 | 검증 포인트 |
| --- | --- | --- |
| FR-19, FR-20 | 후진 중 rear obstacle interrupt 지도 | rear interrupt 발생 tick에서 `Backward` 지속 대신 후진 기준 회피 명령이 나오는지 확인 |
| FR-21, FR-24 | 회피/탈출/후진 중 cleaner 기본 출력 지도 | dust rotation이 아닌 모든 movement command에서 `cleaner=Normal`인지 확인 |
| FR-22 | 전진 중 dust 감지 지도 | clockwise in-place `Boost` 후 travel direction이 `Backward`로 전환되는지 확인 |
| FR-23 | 후진 중 dust 감지 지도 | counterclockwise in-place `Boost` 후 travel direction이 `Forward`로 전환되는지 확인 |
| FR-25 | 같은 dust cell 재방문 지도 | `Boost` 이후 dust가 제거되지 않고 재방문 시 다시 `dustPeriodic=detected`가 발생하는지 확인 |
| FR-26 | `return_to_start_dust_refresh.rvc` | 충분 tick 실행 후 시작 위치 복귀 여부를 최종 위치 또는 로그로 확인 |

## 놓치기 쉬운 위험 지점

| 위험 지점 | 관련 요구사항 | 시나리오 |
| --- | --- | --- |
| front interrupt와 좌측/dust periodic 판단 순서가 섞임 | FR-04, FR-05, FR-06 | `front_left_only_open.rvc`, `front_right_only_open.rvc` |
| 우측 탐색 실패 후 원래 진행 방향을 복구하지 않고 후진함 | FR-10, FR-11 | `front_right_only_open.rvc`, `long_escape_right_exit_extreme.rvc` |
| Escaping 중 우측 탐색 open을 놓치거나 전방 open만 보고 방향을 오판함 | FR-11, FR-12 | `front_clears_but_sides_still_blocked.rvc`, `long_escape_right_exit_extreme.rvc` |
| 후진 명령이 벽을 뚫고 위치를 이동시킴 | FR-10, FR-11 | `sealed_box_extreme.rvc` |
| [R3-변경] dust boost 중 회피/탈출에서 R2의 `Off` 정책이 남아 있음 | FR-21, FR-24 | R3 추가 필요 |
| [R3-변경] dust cell이 청소 후 제거되어 재방문 감지가 불가능함 | FR-25 | `return_to_start_dust_refresh.rvc` |
| 지도 경계 밖 또는 ragged row 접근에서 센서 계산이 깨짐 | FR-16, NFR-08 | `boundary_without_outer_wall.rvc`, `ragged_map_edge_extreme.rvc` |
| 잘못된 scenario 입력을 정상 입력처럼 받아들임 | NFR-02, NFR-07 | `error_cases/*` |
