# 시나리오 카탈로그

이 문서는 `scenarios/` 입력 파일이 어떤 요구사항과 위험 지점을 검증하는지 정리한다.

## 정상 실행 시나리오

| 파일 | 관련 항목 | 의도 |
| --- | --- | --- |
| `clear_corridor_forward.rvc` | UC-01, FR-01, FR-03, FR-19 | 장애물이 없을 때 cleaner normal 상태로 계속 전진 청소한다. |
| `front_left_only_open.rvc` | UC-03, FR-04, FR-06, FR-18 | [변경] 전방 interrupt 후 좌측이 열려 있으면 우측 센서 없이 좌회전한다. |
| `front_right_only_open.rvc` | UC-03, UC-04, FR-10, FR-15, FR-18 | [변경] 초기에는 좌측이 막혀 후진하지만, 후진 후 좌측이 열리면 좌측 우선 탈출을 수행한다. |
| `front_both_sides_open.rvc` | UC-03, FR-06, FR-18 | [삭제] 좌우 모두 열림 교대 정책 검증은 제거되었고, 현재는 좌측 우선 회피를 확인한다. |
| `repeated_front_interrupts_alternation.rvc` | UC-03, UC-04, FR-06, FR-12, FR-15, FR-18 | [변경] 반복 front interrupt에서 좌측 우선과 우측 probe가 결정적으로 동작하는지 확인한다. |
| `long_escape_right_exit_extreme.rvc` | UC-04, FR-10, FR-12, FR-14, FR-15, FR-18 | [변경] 긴 막힌 통로에서 우측 probe와 원복 회전이 tick 단위로 반복되는지 확인한다. |
| `long_escape_left_exit_extreme.rvc` | UC-04, FR-10, FR-12, FR-14, FR-15, FR-18 | [변경] 긴 막힌 통로에서 좌측 센서와 우측 probe cycle이 충돌하지 않는지 확인한다. |
| `sealed_box_extreme.rvc` | UC-04, FR-10, FR-12, FR-14, FR-15, FR-18 | [변경] 완전 고립 상태에서 `Backward/TurnRight/TurnLeft` cycle이 벽을 통과하지 않는지 확인한다. |
| `front_clears_but_sides_still_blocked.rvc` | UC-04, FR-10, FR-12, FR-14, FR-15 | [변경] escaping 중 probe 방향의 front interrupt를 기준으로 우측 막힘을 판정한다. |
| `narrow_tunnel_sides_blocked_front_clear.rvc` | UC-01, FR-03, FR-05 | [변경] 우측 주기 센서 없이도 전방 interrupt가 없으면 전진한다. |
| `dust_trail_boost_refresh.rvc` | UC-05, FR-16, FR-17 | 먼지 감지가 반복될 때 boost 유지 시간이 갱신되는지 확인한다. |
| `dust_before_dead_end_escape.rvc` | UC-04, UC-05, FR-10, FR-16, FR-18 | 먼지 boost와 탈출이 겹칠 때 회피/탈출 중 cleaner off가 우선하는지 확인한다. |
| `dense_dust_maze_extreme.rvc` | UC-03, UC-04, UC-05, FR-04, FR-10, FR-18 | [변경] 장애물과 먼지가 조밀할 때 우측 probe cycle과 cleaner off 우선순위를 함께 확인한다. |
| `boundary_without_outer_wall.rvc` | UC-03, UC-06, FR-04, FR-19 | 외곽 벽이 없는 지도에서 경계 밖을 장애물처럼 처리하는지 확인한다. |
| `ragged_map_edge_extreme.rvc` | UC-06, FR-19 | 행 길이가 다른 지도에서 바깥 영역 처리를 확인한다. |
| `large_open_room_dust_sweep.rvc` | UC-01, UC-05, UC-06, FR-03, FR-16, FR-19 | [변경] 넓은 공간에서 우측 센서 제거 후 전진, 먼지, front interrupt 회피가 함께 동작하는지 확인한다. |

## 오류 기대 시나리오

| 파일 | 관련 항목 | 기대 결과 |
| --- | --- | --- |
| `error_cases/no_robot_invalid.rvc` | UC-06, FR-19 | 로봇 marker가 없어 실패한다. |
| `error_cases/multiple_robots_invalid.rvc` | UC-06, FR-19 | 로봇 marker가 둘 이상이라 실패한다. |
| `error_cases/empty_map_invalid.rvc` | UC-06, FR-19 | map section 뒤 지도 행이 없어 실패한다. |
| `error_cases/missing_map_invalid.rvc` | UC-06, FR-19 | map section이 없어 실패한다. |
| `error_cases/negative_ticks_invalid.rvc` | UC-06, FR-19 | 음수 tick 실행을 거부한다. |
| `error_cases/unknown_symbol_validation_gap.rvc` | NFR-02 | 현재는 통과하지만 입력 검증 강화 시 실패해야 할 후보이다. |

## 위험 지점

| 위험 지점 | 관련 요구사항 | 시나리오 |
| --- | --- | --- |
| [삭제] 좌우 periodic 센서 판단 순서가 뒤섞임 | FR-05-old | 우측 주기 센서가 제거되어 더 이상 적용되지 않는다. |
| [삭제] 좌우 모두 열린 경우 교대 선택이 흔들림 | FR-08-old | 교대 정책이 제거되어 더 이상 적용되지 않는다. |
| [신규] 우측 probe 회전이 tick을 소비하지 않는 것처럼 처리됨 | FR-12, FR-15 | `front_right_only_open.rvc`, `continuous_backward.rvc` |
| [신규] 이전 tick front interrupt가 probe 평가 tick에 남아 우측을 막힘으로 오판 | FR-04, FR-12, FR-13 | controller and subsystem tests |
| [변경] 완전 고립 상태에서 probe cycle이 벽을 통과함 | FR-10, FR-14, FR-19 | `sealed_box_extreme.rvc` |
| 회피/탈출 이동 중 cleaner가 계속 켜짐 | FR-18 | `front_left_only_open.rvc`, `front_right_only_open.rvc`, `long_escape_right_exit_extreme.rvc` |
