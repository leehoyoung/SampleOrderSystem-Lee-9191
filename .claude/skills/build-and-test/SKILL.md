---
name: build-and-test
description: Use whenever you need to build the SampleOrderSystem C++ solution or run its GTest suite from the command line (during RED verification, GREEN verification, or after a refactor). Documents the exact MSBuild/GTest commands for this machine — update this file, not CLAUDE.md, when paths or project names change.
---

# 빌드 & 테스트 실행 (Visual Studio / MSBuild / GTest)

## 전제

- 이 머신에는 Visual Studio 18 (Community)가 설치되어 있다.
- **테스트 프로젝트를 따로 두지 않는다.** `SampleOrderSystem` 프로젝트 하나가 앱과
  테스트를 모두 담는다. `main.cpp`가 빌드 구성에 따라 진입점을 스위칭한다:
  - `_DEBUG` 정의됨(Debug 구성) → `RUN_ALL_TESTS()`를 실행 (테스트 모드)
  - 그 외(Release 구성) → 실제 콘솔 메뉴 앱을 실행
  - 따라서 개발 중 실제 앱을 직접 실행/디버깅하고 싶으면 **Release 구성으로
    실행**해야 한다. Debug 구성은 테스트 전용이다.
- GTest/gmock은 NuGet 패키지(gmock 1.11.0)로 설치되어 있고, `SampleOrderSystem`
  프로젝트 자체에 링크되어 있다. `cpp-tdd` skill 기준대로 mock 사용은 외부 경계
  (예: 생산 라인의 `Clock` 추상화)로 한정하고, 나머지 도메인 로직 테스트는 실제
  객체를 사용한다.
- `--gtest_filter` 등 커맨드라인 플래그가 동작하려면 `main.cpp`의 테스트 진입점이
  `::testing::InitGoogleMock(&argc, &argv)`처럼 `argc`/`argv`를 넘기는 오버로드를
  써야 한다 (인자 없는 `InitGoogleMock()`은 플래그를 못 읽는다).

## 경로 (이 머신 기준 — 바뀌면 여기만 수정)

```
MSBuild : "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe"
```

## 빌드

테스트 실행용 (Debug):

```
"C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ^
  "SampleOrderSystem.slnx" /p:Configuration=Debug /p:Platform=x64
```

실제 앱 실행용 (Release):

```
"C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ^
  "SampleOrderSystem.slnx" /p:Configuration=Release /p:Platform=x64
```

## 테스트 실행

Debug 빌드 산출물(`x64\Debug\SampleOrderSystem.exe`)을 그대로 실행하면 앱 대신
GTest가 돈다.

전체 테스트 실행:

```
x64\Debug\SampleOrderSystem.exe
```

특정 테스트/스위트만 실행 (RED/GREEN 검증 시 반복 실행에 사용):

```
x64\Debug\SampleOrderSystem.exe --gtest_filter=OrderApprovalTest.*
x64\Debug\SampleOrderSystem.exe --gtest_filter=OrderApprovalTest.ApprovesImmediatelyWhenStockSufficient
```

사용 가능한 테스트 목록만 확인:

```
x64\Debug\SampleOrderSystem.exe --gtest_list_tests
```

## RED/GREEN 판정 기준 (cpp-tdd skill과 함께 사용)

- 명령 자체가 0이 아닌 종료 코드로 끝나면서 **빌드 실패 로그**가 보이면 → RED가 아니라
  스텁이 불완전한 상태. 스텁을 보완한다.
- 실행이 되다가 크래시(액세스 위반, 미처리 예외 등)로 비정상 종료하면 → RED가 아니라
  스텁 반환값 문제. 스텁을 조정한다.
- `[  FAILED  ]`가 출력되고 프로세스가 정상 종료하면 → 유효한 RED.
- `[  PASSED  ]`만 출력되고 프로세스가 정상 종료하면 → GREEN.

## 실제 앱 수동 확인

콘솔 메뉴를 직접 눌러보며 확인하고 싶으면 Release 빌드 산출물을 실행한다:

```
x64\Release\SampleOrderSystem.exe
```

## 참고

- 테스트 파일은 `SampleOrderSystem/Tests/` 아래에 두고, `SampleOrderSystem.vcxproj`의
  `ClCompile` 항목에 추가해 같은 프로젝트로 컴파일되게 한다. 별도 vcxproj를 새로
  만들지 않는다.
- vcxproj 경로나 NuGet 패키지 참조 방식이 바뀌면 이 파일만 수정한다. `CLAUDE.md`와
  `cpp-tdd` skill은 "정확한 명령은 이 skill을 참고"라고만 가리키므로 수정할 필요가
  없다.
