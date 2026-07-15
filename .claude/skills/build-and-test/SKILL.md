---
name: build-and-test
description: Use whenever you need to build the SampleOrderSystem C++ solution or run its GTest suite from the command line (during RED verification, GREEN verification, or after a refactor). Documents the exact MSBuild/GTest commands for this machine — update this file, not CLAUDE.md, when paths or project names change.
---

# 빌드 & 테스트 실행 (Visual Studio / MSBuild / GTest)

## 전제

- 이 머신에는 Visual Studio 18 (Community)가 설치되어 있다.
- 테스트 프로젝트(`SampleOrderSystemTests`)는 **콘솔 실행 파일**로 빌드하고
  `gtest_main`을 링크한다. 이렇게 하면 GTest 자체의 `--gtest_filter` 등 플래그를 바로
  쓸 수 있어, Visual Studio의 Google Test 어댑터 확장 설치 여부와 무관하게 CLI에서
  빠르게 반복 실행할 수 있다.

## 경로 (이 머신 기준 — 바뀌면 여기만 수정)

```
MSBuild : "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe"
```

## 빌드

솔루션 전체 빌드:

```
"C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ^
  "SampleOrderSystem.slnx" /p:Configuration=Debug /p:Platform=x64
```

테스트 프로젝트만 빌드 (더 빠름):

```
"C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ^
  "SampleOrderSystemTests\SampleOrderSystemTests.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

## 테스트 실행

빌드 산출물(`x64\Debug\SampleOrderSystemTests.exe`)을 직접 실행한다.

전체 테스트 실행:

```
x64\Debug\SampleOrderSystemTests.exe
```

특정 테스트/스위트만 실행 (RED/GREEN 검증 시 반복 실행에 사용):

```
x64\Debug\SampleOrderSystemTests.exe --gtest_filter=OrderApprovalTest.*
x64\Debug\SampleOrderSystemTests.exe --gtest_filter=OrderApprovalTest.ApprovesImmediatelyWhenStockSufficient
```

사용 가능한 테스트 목록만 확인:

```
x64\Debug\SampleOrderSystemTests.exe --gtest_list_tests
```

## RED/GREEN 판정 기준 (cpp-tdd skill과 함께 사용)

- 명령 자체가 0이 아닌 종료 코드로 끝나면서 **빌드 실패 로그**가 보이면 → RED가 아니라
  스텁이 불완전한 상태. 스텁을 보완한다.
- 실행이 되다가 크래시(액세스 위반, 미처리 예외 등)로 비정상 종료하면 → RED가 아니라
  스텁 반환값 문제. 스텁을 조정한다.
- `[  FAILED  ]`가 출력되고 프로세스가 정상 종료하면 → 유효한 RED.
- `[  PASSED  ]`만 출력되고 프로세스가 정상 종료하면 → GREEN.

## 참고

- 테스트 프로젝트가 아직 없다면(`SampleOrderSystemTests` 디렉터리 부재), 첫 기능 구현
  시작 시 GTest를 링크하는 콘솔 프로젝트로 새로 만든다. `CLAUDE.md` 2장의 저장소 구조를
  따른다.
- vcxproj/vstest 관련 경로가 바뀌거나 GTest NuGet 패키지 참조 방식이 바뀌면 이 파일만
  수정한다. `CLAUDE.md`와 `cpp-tdd` skill은 "정확한 명령은 이 skill을 참고"라고만
  가리키므로 수정할 필요가 없다.
