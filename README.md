# RVC OOAD Project

이 프로젝트는 `docs/rvc.pdf`의 RVC Control SW 요구사항을 기반으로 작성한 OOAD 산출물과 C++20 구현이다.

## Documents

- `docs/requirements.md`: 유스케이스, FR, NFR, 핵심 제어 규칙
- `docs/ooa_ssd.md`: OOA SSD와 System Interface
- `docs/ooa_domain_diagram.md`: OOA Domain Diagram
- `docs/ood_sequence_diagrams.md`: OOD Sequence Diagrams
- `docs/ood_class_diagram.md`: OOD Class Diagram과 SOLID 분석
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

## Run Simulator

```powershell
build\Debug\rvc_simulator.exe --ticks 10
build\Debug\rvc_simulator.exe --scenario scenarios\dust_and_interrupt.rvc --quiet-map
build\Debug\rvc_simulator.exe --scenario scenarios\backward_escape.rvc --quiet-map
build\Debug\rvc_simulator.exe --scenario scenarios\continuous_backward.rvc --quiet-map
```

## Scenario Format

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
