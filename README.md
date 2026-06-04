# RVC OOAD Project

이 프로젝트는 `docs/rvc.pdf`의 RVC Control SW 요구사항을 기반으로 작성한 OOAD 산출물과 C++20 구현이다.

## Documents

- `docs/requirements.md`: 유스케이스, FR, NFR, 핵심 제어 규칙
- `docs/ooa_ssd.md`: OOA SSD와 System Interface
- `docs/ooa_domain_diagram.md`: OOA Domain Diagram
- `docs/ood_sequence_diagrams.md`: OOD Sequence Diagrams
- `docs/ood_class_diagram.md`: OOD Class Diagram과 SOLID 분석
- `docs/sdd.md`: Software Design Description
- `docs/traceability.md`: 요구사항, 설계, 테스트 추적성

## Build

```powershell
cmake -S . -B build
cmake --build build --config Debug
```

## Test

```powershell
ctest --test-dir build -C Debug --output-on-failure
```

## 기존 시뮬레이션 실행

프로젝트 루트(`C:\Users\re-ro\Desktop\ooad-vibe`)에서 PowerShell로 실행한다. 이미 `build\Debug\rvc_simulator.exe`가 있으면 빌드 없이 바로 실행할 수 있다.

```powershell
build\Debug\rvc_simulator.exe --ticks 10
```

시나리오 파일을 지정해서 실행하려면 `--scenario` 옵션을 사용한다.

```powershell
build\Debug\rvc_simulator.exe --scenario scenarios\dust_and_interrupt.rvc --quiet-map
build\Debug\rvc_simulator.exe --scenario scenarios\backward_escape.rvc --quiet-map
build\Debug\rvc_simulator.exe --scenario scenarios\continuous_backward.rvc --quiet-map
build\Debug\rvc_simulator.exe --scenario scenarios\long_escape_right_exit_extreme.rvc --quiet-map
```

`--quiet-map`을 빼면 초기 맵과 최종 맵도 함께 출력된다.

실행 파일이 없거나 코드를 수정한 뒤 다시 빌드해야 하면 아래 명령을 먼저 실행한다.

```powershell
cmake -S . -B build
cmake --build build --config Debug
```

사용 가능한 옵션은 다음과 같다.

- `--ticks N`: N틱 동안 기본 맵으로 시뮬레이션 실행
- `--scenario FILE`: 지정한 `.rvc` 시나리오 파일로 시뮬레이션 실행
- `--quiet-map`: 초기/최종 맵 출력 생략
- `--help`: 사용법 출력

## 기존/외부 인터페이스 시뮬레이터 실행

`Simulator/RvcSimulator.h`를 사용하는 두 번째 시뮬레이터는 `rvc_imported_simulator_cli` 타깃이다. 기본 전체 빌드에서는 제외되어 있으므로 필요하면 타깃을 지정해서 빌드한다.

```powershell
cmake --build build --config Debug --target rvc_imported_simulator_cli
```

빌드된 실행 파일은 아래 경로에 생성된다.

```powershell
build\simulator_rvc_interface\Debug\rvc_imported_simulator_cli.exe
```

실행하면 메뉴가 표시된다.

```text
===== RVC Simulator =====
1. 기본 모드
2. 테스트 모드
3. 종료
선택>
```

기본 모드는 `1`을 입력한 뒤 명령어로 조작한다.

```text
on
step
auto 10
status
quit
```

테스트 모드는 `2`를 입력한 뒤 `1`~`30` 중 하나의 테스트 케이스 번호를 입력하거나, `A`를 입력해 전체 테스트를 실행한다. `Q`를 입력하면 테스트 모드를 종료한다.

## 시나리오 형식

```text
ticks=10
map:
##########
#..*.....#
#.>..*#..#
##########
```

- `#`: obstacle
- `.`: empty cell
- `*`: dust
- `R`: robot facing north
- `^`, `>`, `v`, `<`: robot with direction

추가 시나리오 목록과 FR/NFR/Use Case 매핑은 `docs/scenario_catalog.md`를 참고한다.
