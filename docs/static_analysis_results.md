# 정적 분석 결과 보고서 (Static Analysis Report)

## 1. 개요
* **실행일**: 2026-05-21
* **정적 분석 도구**: 
  * **Cppcheck**: v2.19.0
  * **Clang-Tidy**: LLVM v21.1.8
* **분석 대상 파일**:
  * [GridSimulator.cpp](file:///C:/Users/문재현/Desktop/대학/학기/3학년 2학기/객체지향개발방법론/HW_5/ooad-vibe/src/GridSimulator.cpp)
  * [RvcController.cpp](file:///C:/Users/문재현/Desktop/대학/학기/3학년 2학기/객체지향개발방법론/HW_5/ooad-vibe/src/RvcController.cpp)
  * [Types.cpp](file:///C:/Users/문재현/Desktop/대학/학기/3학년 2학기/객체지향개발방법론/HW_5/ooad-vibe/src/Types.cpp)
  * [main.cpp](file:///C:/Users/문재현/Desktop/대학/학기/3학년 2학기/객체지향개발방법론/HW_5/ooad-vibe/src/main.cpp)
  * `include/` 디렉터리 내 헤더 파일들

---

## 2. 분석 결과 요약

| 도구 | 결과 | 주요 내용 |
| --- | --- | --- |
| **Cppcheck** | 경고 2건 | `static` 함수로 선언 가능한 멤버 함수들에 대한 코드 스타일 경고 |
| **Clang-Tidy** | 통과 (Clean) | 감지된 경고 및 오류 없음 |

---

## 3. 상세 분석 결과

### 1) Cppcheck 결과
Cppcheck 분석 결과, 멤버 변수에 접근하지 않아 `static` 함수로 변경할 수 있는 멤버 함수에 대한 **스타일 경고(style)** 2건이 감지되었습니다.

* **경고 1**: `GridSimulator::adjacent` 함수 static 선언 가능
  * **파일 위치**: [GridSimulator.cpp:L239](file:///C:/Users/문재현/Desktop/대학/학기/3학년 2학기/객체지향개발방법론/HW_5/ooad-vibe/src/GridSimulator.cpp#L239)
  * **메시지**: `style: The member function 'rvc::GridSimulator::adjacent' can be static. [functionStatic]`
  * **설명**: 해당 함수는 `GridSimulator` 클래스의 멤버 변수를 사용하지 않고 독립적으로 동작하는 유틸리티적 성격의 함수이므로 `static` 멤버 함수로 전환하는 것이 권장됩니다.

* **경고 2**: `RvcController::makeCommand` 함수 static 선언 가능
  * **파일 위치**: [RvcController.cpp:L122](file:///C:/Users/문재현/Desktop/대학/학기/3학년 2학기/객체지향개발방법론/HW_5/ooad-vibe/src/RvcController.cpp#L122)
  * **메시지**: `style: The member function 'rvc::RvcController::makeCommand' can be static. [functionStatic]`
  * **설명**: 해당 함수도 클래스 내부의 인스턴스 멤버 변수를 참조하지 않기 때문에 `static` 멤버 함수로 구성하여 불필요한 `this` 포인터 전달을 줄일 수 있습니다.

### 2) Clang-Tidy 결과
Clang-Tidy 분석 결과, 소스 코드에서 추가로 탐지된 코딩 표준 위반 사례나 메모리 누수, 모호한 표현 등의 잠재적 오류는 발견되지 않았습니다.

---

## 4. 권장 조치 사항
Cppcheck에서 검출된 2건의 `functionStatic` 스타일 경고는 프로그램의 동작에 영향을 주는 치명적인 오류는 아닙니다. 그러나 코드의 품질과 최적화를 위해 아래와 같이 변경하는 것을 권장합니다.

1. **`GridSimulator::adjacent` 변경**:
   * [GridSimulator.hpp](file:///C:/Users/문재현/Desktop/대학/학기/3학년 2학기/객체지향개발방법론/HW_5/ooad-vibe/include/rvc/GridSimulator.hpp)에서 `Position adjacent(...)` 선언 앞에 `static`을 붙이고, [GridSimulator.cpp](file:///C:/Users/문재현/Desktop/대학/학기/3학년 2학기/객체지향개발방법론/HW_5/ooad-vibe/src/GridSimulator.cpp)의 구현부 맨 뒤에 붙은 `const` 키워드를 제거합니다.
2. **`RvcController::makeCommand` 변경**:
   * [RvcController.hpp](file:///C:/Users/문재현/Desktop/대학/학기/3학년 2학기/객체지향개발방법론/HW_5/ooad-vibe/include/rvc/RvcController.hpp)에서 `Command makeCommand(...)` 선언 앞에 `static`을 붙이고, [RvcController.cpp](file:///C:/Users/문재현/Desktop/대학/학기/3학년 2학기/객체지향개발방법론/HW_5/ooad-vibe/src/RvcController.cpp)의 구현부 맨 뒤에 붙은 `const` 키워드를 제거합니다.
