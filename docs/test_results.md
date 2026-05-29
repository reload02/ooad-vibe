# RVC 테스트 결과

## 실행 환경

| 항목 | 값 |
| --- | --- |
| 빌드 | `cmake --build build --config Debug` |
| 테스트 | `ctest --test-dir build -C Debug --output-on-failure` |
| 결과 | [변경] 60개 테스트 모두 통과 |

초기 빌드는 Windows SDK/AppData 접근 제한으로 샌드박스 안에서 실패했으며, 승인된 권한으로 동일 빌드를 다시 실행해 성공했다.

## 테스트 범위

| 그룹 | 검증 내용 |
| --- | --- |
| `RvcControllerTest` | [변경] 우측 센서 없는 controller 계약, 좌측 우선 회피, 후진 우측 probe, dust boost, cleaner off 규칙 |
| `RvcSubsystemTest` | [변경] `SensorFusion`, `NavigationPolicy`, `CleaningPolicy`, `CommandComposer` 단위 책임 |
| `RvcFacadeTest` | `Rvc` facade가 controller 계약과 같은 최종 command를 반환하는지 확인 |
| `RvcSystemTest` | [변경] `GridSimulator`가 `rightPeriodic` 없이 front interrupt, left/dust periodic 입력과 command 적용을 처리하는지 확인 |
| `RvcScenarioRegressionTest` | [변경] 우측 probe tick 증가와 새 로그 fragment를 반영한 scenario file 결과 검증 |
| CLI CTest | simulator CLI 실행 호환성 검증 |

## 변경 검증 요약

| Tag | 검증 |
| --- | --- |
| [삭제] | `rightObstacle` 필드와 `rightPeriodic` 로그 참조가 코드/테스트 계약에서 제거되었다. |
| [삭제] | 좌우 모두 열림 교대 정책 테스트는 제거되고 좌측 우선 및 우측 probe 테스트로 대체되었다. |
| [신규] | `Backward -> TurnRight -> Forward` 우측 열림 경로를 단위/컨트롤러/시나리오 테스트로 검증했다. |
| [신규] | `Backward -> TurnRight -> TurnLeft -> Backward` 우측 막힘 및 원복 경로를 검증했다. |
| [변경] | 회전 명령이 각각 tick을 소비하므로 scenario 기대 tick별 위치/방향/명령 수를 갱신했다. |
