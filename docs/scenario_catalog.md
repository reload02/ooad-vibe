# 시나리오 카탈로그

이 문서는 `docs/requirements.md`의 Use Case, FR, NFR을 기준으로 오류가 생기기 쉬운 지점을 시뮬레이터 입력 시나리오로 정리한다. 정상 실행 시나리오는 `scenarios/`에, 입력 오류를 기대하는 시나리오는 `scenarios/error_cases/`에 둔다.

## 정상 실행 시나리오

| 파일 | 관련 항목 | 의도 |
| --- | --- | --- |
| `clear_corridor_forward.rvc` | UC-01, FR-01, FR-03 | 장애물이 없을 때 계속 전진 청소하는 기본 흐름 |
| `front_left_only_open.rvc` | UC-03, FR-04, FR-05, FR-07 | 전방 인터럽트 후 좌측만 열려 있을 때 좌회전 |
| `front_right_only_open.rvc` | UC-03, FR-04, FR-05, FR-08 | 전방 인터럽트 후 우측만 열려 있을 때 우회전 |
| `front_both_sides_open.rvc` | UC-03, FR-09, NFR-08 | 좌우가 모두 열려 있을 때 결정적 회전 정책 확인 |
| `repeated_front_interrupts_alternation.rvc` | UC-03, FR-09, NFR-08 | 전방 인터럽트가 반복될 때 좌우 선택 정책이 흔들리지 않는지 확인 |
| `long_escape_right_exit_extreme.rvc` | UC-04, FR-10, FR-11, FR-12, FR-13 | 긴 후진 탈출 끝에 우측 탈출구가 열리는 극단 상황 |
| `long_escape_left_exit_extreme.rvc` | UC-04, FR-10, FR-11, FR-12, FR-13 | 긴 후진 탈출 끝에 좌측 탈출구가 열리는 극단 상황 |
| `sealed_box_extreme.rvc` | UC-04, FR-10, FR-11 | 완전 고립 상태에서 후진 명령은 유지되지만 위치가 유지되는지 확인 |
| `front_clears_but_sides_still_blocked.rvc` | UC-04, FR-11, FR-12 | Escaping 중 전방이 열려 보여도 좌우가 막히면 탈출 가능으로 보지 않는 조건 |
| `narrow_tunnel_sides_blocked_front_clear.rvc` | UC-01, UC-03, FR-03, FR-06 | 좌우 periodic 센서가 막혀도 전방이 열려 있으면 전진해야 하는 조건 |
| `dust_trail_boost_refresh.rvc` | UC-05, FR-14, FR-15 | 먼지가 반복 감지될 때 boost 유지 시간이 갱신되는지 확인 |
| `dust_before_dead_end_escape.rvc` | UC-04, UC-05, FR-10, FR-11, FR-14 | 먼지 boost와 막다른 곳 탈출이 겹치는 복합 상황 |
| `dense_dust_maze_extreme.rvc` | UC-03, UC-05, UC-06, FR-04, FR-14, FR-16, FR-17 | 장애물과 먼지가 조밀한 복합 지도 |
| `boundary_without_outer_wall.rvc` | UC-03, UC-06, FR-04, FR-16 | 외곽 벽이 없는 지도에서 경계 밖을 장애물처럼 처리하는지 확인 |
| `ragged_map_edge_extreme.rvc` | UC-06, FR-16, FR-17 | 행 길이가 다른 지도에서 바깥 영역 처리 확인 |
| `large_open_room_dust_sweep.rvc` | UC-01, UC-05, UC-06, FR-03, FR-14, FR-16, FR-17 | 넓은 공간에서 장시간 전진, 먼지 감지, 벽 회피가 이어지는 상황 |

## 오류 기대 시나리오

| 파일 | 관련 항목 | 기대 결과 |
| --- | --- | --- |
| `error_cases/no_robot_invalid.rvc` | UC-06, FR-16 | 로봇 마커가 없어 실패 |
| `error_cases/multiple_robots_invalid.rvc` | UC-06, FR-16 | 로봇 마커가 둘 이상이라 실패 |
| `error_cases/empty_map_invalid.rvc` | UC-06, FR-16 | map 섹션에 지도 행이 없어 실패 |
| `error_cases/missing_map_invalid.rvc` | UC-06, FR-16 | map 섹션이 없어 실패 |
| `error_cases/negative_ticks_invalid.rvc` | UC-06, FR-17 | 음수 tick 실행이 거부되어 실패 |
| `error_cases/unknown_symbol_validation_gap.rvc` | NFR-02, NFR-07 | 현재는 통과할 수 있으나, 입력 검증 강화 시 실패해야 할 후보 |

## 특히 오류가 나기 쉬운 지점

| 위험 지점 | 관련 요구사항 | 시나리오 |
| --- | --- | --- |
| 전방 interrupt와 periodic 좌우 센서 판단 순서가 뒤섞임 | FR-04, FR-05, FR-06 | `front_left_only_open.rvc`, `front_right_only_open.rvc` |
| 좌우가 모두 열린 경우 회전 선택이 비결정적으로 바뀜 | FR-09, NFR-08 | `front_both_sides_open.rvc`, `repeated_front_interrupts_alternation.rvc` |
| Escaping 중 전방이 열린 것만 보고 탈출했다고 오판 | FR-11, FR-12 | `front_clears_but_sides_still_blocked.rvc` |
| 후진 명령이 벽을 뚫고 위치를 이동시킴 | FR-10, FR-11, FR-16 | `sealed_box_extreme.rvc` |
| 먼지 boost 지속 시간이 새 먼지 감지로 갱신되지 않음 | FR-14, FR-15 | `dust_trail_boost_refresh.rvc` |
| 지도 경계 밖 또는 ragged row 접근에서 센서 계산이 깨짐 | FR-16, FR-17 | `boundary_without_outer_wall.rvc`, `ragged_map_edge_extreme.rvc` |
| 잘못된 scenario 입력을 정상 입력처럼 받아들임 | NFR-02, NFR-07 | `error_cases/*` |
