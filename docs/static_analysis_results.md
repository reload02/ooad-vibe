# 정적 분석 결과 보고서

## 1. 개요

| 항목 | 값 |
| --- | --- |
| 마지막 정적 분석 실행일 | 2026-05-21 |
| 문서 갱신일 | 2026-06-19 |
| 정적 분석 도구 | Cppcheck v2.19.0, Clang-Tidy LLVM v21.1.8 |
| 분석 기준 | R2 구현 기준 |
| R3/R4 상태 | 후방 interrupt, dust rotation, persistent dust 구현 후 재실행 필요 |

## 2. 분석 대상

| 영역 | 대상 |
| --- | --- |
| Core source | `src/GridSimulator.cpp`, `src/Rvc.cpp`, `src/RvcController.cpp`, `src/NavigationPolicy.cpp`, `src/CleaningPowerPolicy.cpp`, `src/SimulatedHardwareAdapter.cpp`, `src/Types.cpp`, `src/main.cpp` |
| Headers | `include/rvc/*.hpp` |
| Tests | 필요 시 `tests/*.cpp` |

## 3. 마지막 분석 결과 요약

| 도구 | 결과 | 주요 내용 |
| --- | --- | --- |
| Cppcheck | 스타일 경고 2건 | 멤버 변수를 사용하지 않는 함수에 대한 `functionStatic` 권고 |
| Clang-Tidy | 통과 | 감지된 경고 및 오류 없음 |

## 4. R3/R4 이후 재분석 포인트

| 변경 요구사항 | 확인할 정적 분석 포인트 |
| --- | --- |
| FR-19, FR-20 | rear interrupt pending 상태와 travel direction 전이가 unused field 또는 dead branch 없이 연결되는지 확인 |
| FR-21, FR-24 | cleaner `Normal` 기본 정책 변경 후 R2 `Off` 분기 잔재가 unreachable 또는 inconsistent branch로 남지 않는지 확인 |
| FR-22, FR-23 | dust rotation duration, direction toggle 상태가 초기화 누락 없이 사용되는지 확인 |
| FR-25 | persistent dust 전환 후 `dustCleaned` 카운터와 `remainingDust` 의미가 문서/코드에서 모순되지 않는지 확인 |
| FR-26 | return-to-start 시나리오 tick 증가가 overflow나 음수 tick 처리와 충돌하지 않는지 확인 |

## 5. 권장 조치

R3/R4 구현 후 `cmake --build build-ninja`와 전체 테스트를 먼저 통과시킨 뒤 Cppcheck와 Clang-Tidy를 재실행한다. 현재 문서는 마지막 R2 정적 분석 결과를 보존하되, 최신 요구사항 기준의 정적 분석 완료 상태로 간주하지 않는다.
