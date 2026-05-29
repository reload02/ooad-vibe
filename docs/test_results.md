# RVC 테스트 결과

## 실행 환경

| 항목 | 값 |
| --- | --- |
| 빌드 | `cmake --build build --config Debug` |
| 테스트 | `ctest --test-dir build -C Debug --output-on-failure` |
| 결과 | 62개 테스트 모두 통과 |

첫 빌드는 Windows SDK/AppData 접근 제한으로 실패했고, 승인된 권한으로 동일 빌드를 다시 실행해 성공했다. 이후 CTest 전체 회귀 테스트가 통과했다.

## 테스트 범위

| 그룹 | 검증 내용 |
| --- | --- |
| `RvcControllerTest` | 기존 controller public 계약, 회피, 탈출, dust boost, cleaner off 규칙 |
| `RvcSubsystemTest` | `SensorFusion`, `NavigationPolicy`, `CleaningPolicy`, `CommandComposer` 단위 책임 |
| `RvcFacadeTest` | `Rvc` facade가 기존 controller 계약과 같은 최종 command를 반환하는지 |
| `RvcSystemTest` | `GridSimulator`가 위치/방향/먼지 상태를 소유하고 RVC command를 적용하는지 |
| `RvcScenarioRegressionTest` | 기존 scenario file 결과와 로그 의미가 유지되는지 |
| CLI CTest | 기존 simulator CLI 실행 호환성 |

## 요약

- 새 RVC 내부 SOLID/OOD 구조에서도 기존 controller/system 동작은 유지된다.
- 새 subsystem 단위 테스트가 감지 결합, 이동 판단, 청소 정책, command 조립 책임을 직접 검증한다.
- 기존 CLI, scenario format, log fragment 기반 회귀 테스트가 통과했다.
