# SampleOrderSystem — 반도체 시료 생산주문관리 시스템

가상의 반도체 회사 "S-Semi"를 위한 콘솔 기반 시료(Sample) 생산·주문 관리 시스템이다.
시료 등록, 주문 접수/승인/거절, 생산 라인 관리, 재고 모니터링, 출고 처리를 하나의
콘솔 애플리케이션에서 수행한다.

요구사항 전체는 [`PRD.md`](./PRD.md), 개발 규칙은 [`CLAUDE.md`](./CLAUDE.md)를 참고한다.

## 기술 스택

- 언어: C++ (C++17 이상)
- IDE/빌드: Visual Studio, MSBuild (`.vcxproj` / `.slnx`)
- 테스트: GoogleTest (GTest)
- 데이터 저장: JSON 파일 기반 Repository 패턴 (별도 진행 중인 DataPersistence PoC 참고)

## 저장소 구조

```
SampleOrderSystem/
├── README.md                  # 이 문서
├── PRD.md                     # 요구사항 명세 (단일 진실 소스)
├── CLAUDE.md                  # 개발 규칙 (빌드, Clean Code, 커밋, 에스컬레이션 기준)
├── docs/
│   ├── specs/                  # 스캐폴딩/포팅 설계 근거 문서
│   └── reviews/                # 서브에이전트 구현·리뷰 기록 (CLAUDE.md 4-1장)
├── SampleOrderSystem.slnx
├── SampleOrderSystem/          # 메인 애플리케이션 프로젝트 (MVC)
│   ├── Core/                   # Router/Application/IModel/IView/IController
│   ├── Json/                   # JsonValue 파서/직렬화
│   ├── Sample/                 # 시료 관리 — 등록/목록/검색 동작
│   ├── Order/                  # 주문 도메인 — Controller는 스텁(승인/거절은 이후 TDD)
│   ├── Monitoring/              # 모니터링 스켈레톤 (전부 스텁)
│   └── data/                    # samples.json / orders.json
├── SampleOrderSystemTests/     # GTest+gmock, 포팅 코드에 대한 사후 검증 테스트
└── .claude/
    ├── agents/                 # 역할별 서브에이전트 (spec-writer, test-writer, implementer, requirements-auditor, cpp-code-reviewer)
    └── skills/                 # 절차 문서 (cpp-tdd, build-and-test, commit-workflow)
```

## 개발 프로세스 (Agentic Engineering)

이 프로젝트는 Claude Code 서브에이전트 + 스킬로 TDD 개발 파이프라인을 구성한다.

```
spec-writer          PRD.md 기능 → 단일 행동 단위 TDD 태스크 목록을
                       docs/tasks/<기능-슬러그>.md 로 저장
   ↓
사용자 리뷰/승인       ← 승인 전에는 test-writer가 시작하지 않음
   ↓
test-writer (RED)     테스트 1개 + 트리비얼 스텁 작성, GTest FAILED 확인
   ↓
implementer (GREEN)   스텁 본문에 최소 로직 채움, 전체 테스트 GREEN 확인
   ↓
requirements-auditor   PRD.md 요구사항과 구현이 실제로 일치하는지 추적/감사
   ↓
cpp-code-reviewer      Clean Code 리뷰 + 리팩터링 요청 라우팅
   ↓
사용자 리뷰/승인 → 커밋
```

각 역할의 상세 정의는 `.claude/agents/`, 공유 절차(RED/GREEN 정의, 빌드/테스트
명령어, 커밋 워크플로우)는 `.claude/skills/`에 있다. 자세한 내용은 `CLAUDE.md`를
참고한다.

## 빌드 및 실행

빌드(레포 루트에서 실행):

```
"C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" SampleOrderSystem.slnx /p:Configuration=Debug /p:Platform=x64
```

메인 애플리케이션 실행 (콘솔 메뉴 진입):

```
x64\Debug\SampleOrderSystem.exe
```

테스트 실행:

```
x64\Debug\SampleOrderSystemTests.exe
```

`x64\Debug\`는 빌드 산출물이라 `.gitignore` 처리되어 있으므로, 위 두 실행 파일은
먼저 빌드해야 생성된다. MSBuild 경로가 이 머신과 다르거나 정확한 빌드/테스트
명령어가 더 필요하면 [`.claude/skills/build-and-test/SKILL.md`](./.claude/skills/build-and-test/SKILL.md)를 참고한다.

## 관련 PoC

이 프로젝트는 별도로 진행 중인 다음 PoC들의 산출물을 참고하여 구현한다:

- `ConsoleMVC` (MVC 스켈레톤 구조)
- `DataPersistence` (JSON 파일 기반 Repository 패턴)
- `DataMonitor` (데이터 모니터링 도구)
- `DummyDataGenerator` (더미 데이터 생성 도구)

## 현재 진행 상태

- [x] PRD.md / CLAUDE.md 작성
- [x] Agentic Engineering 파이프라인(서브에이전트/스킬) 구성
- [x] 빈 Visual Studio 프로젝트 스켈레톤
- [x] MVC 스켈레톤 반영 (Core/Sample/Order/Monitoring 폴더 구조, `docs/specs/poc-integration-design.md` 참고)
- [x] 데이터 영속성 반영 (JsonValue + Repository 패턴, data/samples.json, data/orders.json)
- [x] 모니터링/더미 데이터 도구 검토 반영 (DataMonitor는 렌더링 스타일만 Monitoring/ 폴더로 흡수, DummyDataGenerator는 독립 도구로 유지)
- [x] SampleOrderSystemTests(GTest+gmock) 프로젝트 구성
- [x] 포팅된 Core/Json/Sample/Order 코드에 대한 사후 검증(characterization) 테스트 28개 추가
- [ ] 기능 구현 (TDD) — PRD 7.3(주문 승인/거절)·7.5(모니터링 집계)·7.6(생산 라인)·7.7(출고)
