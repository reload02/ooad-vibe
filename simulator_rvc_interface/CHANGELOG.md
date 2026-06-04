# Simulator RVC Interface 변경사항

## 목적
- 새 `Simulator/` 코드가 기대하는 `Class/*`, `HDWARE/*` 인터페이스를 별도 호환 계층으로 제공한다.
- 기존 RVC 핵심 알고리즘은 `rvc::RvcController`를 그대로 사용한다.
- `Simulator/` 폴더 내부 파일은 수정하지 않는다.

## 추가된 인터페이스
- `Class/ISensor.h`: 센서 입력을 위한 `detect()` 추상 인터페이스.
- `HDWARE/Motor.h`: 전진, 후진, 좌회전, 우회전 모터 추상 인터페이스.
- `HDWARE/HwCleaner.h`: 청소기 전원 및 PowerUp 제어 추상 인터페이스.
- `Class/EventBus.*`: 센서 데이터, 전방 장애물 이벤트, RVC 컨트롤러 tick, 청소기 명령 적용을 묶는 최소 공유 상태 계층.
- `Class/SensorController.*`: 좌측 장애물과 먼지 센서를 읽고 전방 장애물 이벤트를 전달한다.
- `Class/MotorController.*`: `rvc::Command`를 모터 동작으로 변환한다.
- `Class/CleanerController.*`: 청소기 하드웨어를 EventBus에 연결한다.
- `Class/PowerController.*`: 전원 상태와 정지 명령을 EventBus에 반영한다.
- `Class/Timer.*`: 새 Simulator가 호출하는 타이머 동기화 API를 제공한다.

## 유지한 부분
- 기존 `rvc::RvcController`의 회피, 우측 탐색, 후진 탈출, 먼지 Boost 판단 로직은 변경하지 않았다.
- 기존 `rvc_core`, 기존 CLI 시뮬레이터, 기존 테스트는 계속 같은 API를 사용한다.

## CMake 연결
- `rvc_simulator_interface` 라이브러리를 추가해 새 호환 계층을 기본 빌드에 포함했다.
- `rvc_imported_simulator` 타깃을 `EXCLUDE_FROM_ALL`로 추가해, 기본 빌드는 깨지지 않으면서 새 `Simulator/` 전체 컴파일을 별도로 확인할 수 있게 했다.
- `rvc_imported_simulator_cli` 타깃을 추가해 새 `Simulator/`의 `RvcSimulator::run()`을 실행할 수 있게 했다.
- `SimulatorRvcInterfaceSmoke` 테스트를 추가해 전진, 회피, 먼지 Boost, 전원 OFF 흐름을 호환 계층만으로 검증한다.

## Simulator 컴파일 오류 수정
- `Simulator/` 원본 파일은 변경하지 않았다.
- MSVC가 UTF-8 소스를 CP949로 해석하면서 닫히지 않은 문자열처럼 컴파일하던 문제를 `rvc_imported_simulator` 타깃의 `/utf-8` 옵션으로 해결했다.

## 실행
- 빌드: `cmake --build build --config Debug --target rvc_imported_simulator_cli`
- 실행: `.\build\simulator_rvc_interface\Debug\rvc_imported_simulator_cli.exe`
