# Task 6 리뷰: SampleOrderSystemTests 프로젝트 (GTest+gmock)

- 리뷰 대상 커밋: chore: SampleOrderSystemTests 프로젝트 추가 — GTest+gmock(NuGet) 연동, 빌드/링크 확인용 SmokeTest 포함
- 리뷰 방식: superpowers:subagent-driven-development (task reviewer subagent, model: sonnet)

## Spec 준수: ✅

packages.config, SmokeTest.cpp, vcxproj, vcxproj.filters, slnx 등록 모두 브리핑 코드와
diff가 바이트 단위로 일치. `..\packages\gmock.1.11.0\...` 상대경로도 실제 구조(레포 루트
형제 디렉터리)에 맞게 유효함을 컨트롤러가 사전 확인. gmock_main.cc 명시적 ClCompile
포함, gmock.targets ExtensionTargets/Exists 조건, EnsureNuGetPackageBuildImports 에러
타겟 모두 그대로 반영됨.

## 코드 품질: Approved

- `/utf-8` 옵션: 4개 구성(Debug/Release × Win32/x64) `AdditionalOptions`에 모두 존재.
- gmock.targets 참조 경로가 실제 packages 위치와 정확히 일치.
- gmock은 SampleOrderSystemTests.vcxproj에만 참조되고 메인 앱(SampleOrderSystem.vcxproj)엔
  변경 없음 — 격리 유지됨.
- 브리핑 대비 불필요한 추가/누락 없음.

## OutDir 차이 관련 (구현자 우려사항 검토)

구현자가 "솔루션 빌드와 프로젝트 단독 빌드에서 OutDir 위치가 다르게 나타난다"고 보고했으나,
실제 결함이 아니다. vcxproj가 OutDir을 별도로 오버라이드하지 않으므로 MSBuild 기본 규칙
(`$(SolutionDir)$(Platform)\$(Configuration)\`, 솔루션 컨텍스트 없이 단독 빌드 시
`SolutionDir`이 프로젝트 디렉터리로 대체됨)에 따른 정상 동작이다. 코드 수정 불필요.

Critical/Important 항목 없음.

## 컨트롤러 빌드/테스트 재확인

- `MSBuild SampleOrderSystem.slnx /p:Configuration=Debug /p:Platform=x64` — 전체 솔루션
  빌드 성공 (SampleOrderSystem.exe, SampleOrderSystemTests.exe 모두 생성).
- `x64\Debug\SampleOrderSystemTests.exe` 직접 실행 — `[  PASSED  ] 1 test.` 확인.
