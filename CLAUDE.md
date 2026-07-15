# CLAUDE.md

이 문서는 SampleOrderSystem(반도체 시료 생산주문관리 시스템) 개발 시 지켜야 할 규칙을
담는다. 요구사항/도메인 정의는 `PRD.md`가 단일 진실 소스(Single Source of Truth)이다.
이 문서와 PRD.md가 충돌하면 PRD.md를 우선한다.

**README.md 동기화**: `README.md`는 사용자·제3자가 이 저장소를 처음 볼 때 확인하는
문서다(기술 스택, 저장소 구조, 빌드 방법, 진행 상태 등). 이 문서(CLAUDE.md)나 PRD.md,
또는 실제 구현에서 **README.md에 이미 적힌 정보에 영향을 주는 변경**이 생기면(예:
저장소 구조 변경, 빌드 방법 변경, 새 기능 완료로 진행 상태가 바뀜, 기술 스택 변경 등)
같은 작업 안에서 README.md도 함께 갱신한다. 변경 사항이 README.md와 무관하다고
판단되면 갱신하지 않아도 되지만, 애매하면 사용자에게 확인한다 (6-1장).

## 1. 기술 스택

- 언어: C++ (C++17 이상)
- IDE/빌드: Visual Studio, MSBuild (`.vcxproj` / `.slnx`)
- 테스트: GoogleTest (GTest)
- 데이터 저장: JSON 파일 기반 Repository 패턴 (DataPersistence PoC 참고)

## 2. 저장소 구조

```
SampleOrderSystem/
├── PRD.md
├── CLAUDE.md
├── README.md
├── docs/
│   ├── tasks/                  # spec-writer가 작성한 기능별 TDD 태스크 목록 (사용자 승인 후 진행)
│   └── reviews/<작업-슬러그>/  # 4-1장 리뷰 흐름의 Task/기능별 리뷰 기록
├── SampleOrderSystem.slnx
├── SampleOrderSystem/          # 메인 애플리케이션 프로젝트 (MVC)
│   ├── Model/
│   ├── View/
│   ├── Controller/
│   └── ...
└── SampleOrderSystemTests/     # GTest 테스트 프로젝트 (구현 시작 시 추가)
```

- MVC 세부 패키지 구조는 `MVC skeleton` PoC(`../MVC skeleton`)를 참고하여 반영한다.
- 데이터 영속성 구현은 `DataPersistence` PoC(`../DataPersistence`)의 `JsonValue` /
  `Repository` 패턴을 그대로 따른다. 새로 발명하지 않는다.
- 데이터 모니터링 도구, 더미데이터 생성 도구가 필요한 기능(모니터링 메뉴, 테스트 데이터
  준비)은 각각 `DataMonitor`, `DummyDataGenerator` PoC를 참고한다.
- 위 PoC들은 이 프로젝트와 별도로 진행 중이다. PoC가 갱신되면 이 문서와 관련 skill도
  함께 최신화한다.

## 3. 빌드 & 테스트 (이 머신 기준)

```
MSBuild : "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe"
vstest  : "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\TestWindow\vstest.console.exe"
```

- 빌드: `MSBuild SampleOrderSystem.slnx /p:Configuration=Debug /p:Platform=x64`
- 테스트: 테스트 프로젝트 빌드 산출물(`*.dll` 또는 `*Tests.exe`)을 `vstest.console.exe`로
  실행한다.
- 정확한 커맨드는 `build-and-test` 스킬에 최신 상태로 유지한다. 경로가 바뀌면 스킬만
  수정하고 이 문서는 개념만 남긴다.

## 4. 개발 프로세스 (Agentic Engineering)

- 기능 구현 전 `cpp-tdd` 스킬(=Red-Green-Refactor)을 반드시 따른다. 예외 없음.
- 신규 기능/버그 수정에 착수하기 전 `PRD.md`의 관련 절과 상태 흐름을 다시 확인한다.
- 서브에이전트는 역할별로 구성한다 (설계/구현/리뷰). 도메인별로 에이전트를 쪼개지 않는다
  — 시료/주문/생산 라인은 상태로 강하게 얽혀 있어 역할별 에이전트가 전체 컨텍스트를
  일관되게 유지하는 편이 낫다.
- 각 서브에이전트가 공유해야 할 도메인 규칙(상태 전이, 생산량 계산식 등)은 코드 주석이
  아니라 PRD.md와 skill 문서에 둔다.

### 파이프라인 (`.claude/agents/`)

```
spec-writer           PRD.md 기능 → 단일 행동 단위 TDD 태스크 목록을
                       docs/tasks/<기능-슬러그>.md 로 저장
   ↓
사용자 리뷰/승인       ← 이 문서를 승인하기 전에는 test-writer가 시작하지 않는다
   ↓
test-writer (RED)     테스트 1개 + 트리비얼 스텁 작성, GTest FAILED 확인
   ↓
implementer (GREEN)   스텁 본문에 최소 로직 채움, 전체 테스트 GREEN 확인
   ↓
requirements-auditor   PRD.md 요구사항과 구현이 실제로 일치하는지 추적/감사 (리포트만 생성)
   ↓
cpp-code-reviewer      위 리포트 + Clean Code 관점 리뷰 → 필요시 test-writer/implementer에게
                       리팩터링 요청 → 반영 후 재검증
   ↓
사용자 리뷰/승인 → 커밋
```

- spec-writer의 태스크 목록은 **문서로 남기고 사용자 승인을 받는 것 자체가 하나의
  게이트**다 (6-1장의 "애매한 판단 확인"과 별개로, 태스크 목록 자체는 항상 승인을
  거친다). 승인되지 않은 태스크 목록으로 test-writer를 진행시키지 않는다.
- `requirements-auditor`와 `cpp-code-reviewer`는 모두 **읽기 전용**이다. 코드를 직접
  고치지 않고, 실행은 항상 test-writer/implementer가 한다.

## 4-1. 서브에이전트 구현·리뷰 공통 규칙 (모든 구현 작업에 적용)

이 절의 규칙은 **어떤 파이프라인으로 구현하든** — PRD 기능 TDD(위 파이프라인의
test-writer/implementer)든, 스캐폴딩/포팅처럼 Task 단위 실행 계획을 세워 진행하는
작업(예: PoC 통합)이든 — 서브에이전트가 코드를 만드는 모든 구현 작업에 공통으로
적용되는 실행 방식이다. 구체적인 실행은 `superpowers:subagent-driven-development`
스킬(Task/브리핑 단위가 명확한 스캐폴딩·포팅 작업)을 기본으로 삼되, 위 파이프라인의
이름 붙은 에이전트(test-writer, implementer 등)를 쓰는 경우에도 아래 규칙은 동일하게
지킨다.

1. **서브에이전트는 절대 커밋하지 않는다** — 6장의 커밋 게이트가 여기도 예외 없이
   적용된다. 서브에이전트는 구현(+해당 파이프라인이 요구하는 테스트/빌드 확인)까지만
   하고 보고서를 남긴다. 커밋은 항상 컨트롤러(메인 세션)가 사용자 승인 후 수행한다.
2. 구현이 끝나면 리뷰를 거친다 — PRD 기능 파이프라인이면 `requirements-auditor` +
   `cpp-code-reviewer`, 스캐폴딩/포팅 작업이면 task reviewer 서브에이전트. 리뷰는 반드시
   **Spec 준수**(요구사항/실행 계획과 실제 구현이 일치하는지)와 **코드 품질**(5장 Clean
   Code 규칙) 두 가지 verdict를 모두 보고해야 한다. 하나라도 Critical/Important
   이슈가 있으면 fix 서브에이전트 → 재리뷰 루프를 승인될 때까지 반복한다.
3. 리뷰가 승인되면 컨트롤러가 **직접 빌드(및 필요시 실행)를 재실행**해 서브에이전트의
   성공 주장을 검증한다 (서브에이전트 보고서를 곧이곧대로 믿지 않는다).
4. 리뷰 verdict는 대화에만 남기지 않고 **`docs/reviews/<작업-슬러그>/<태스크-또는-기능-
   슬러그>-review.md`로 파일 기록**한 뒤, 해당 변경의 커밋에 **함께 포함**시킨다(커밋
   메시지에 리뷰 기록 경로를 한 줄로 남긴다). `<작업-슬러그>`는 스캐폴딩/포팅 작업이면
   그 작업 이름, PRD 기능이면 `docs/tasks/`와 동일한 기능 슬러그를 쓴다. 이렇게 하면
   나중에 커밋 히스토리만 봐도 각 변경이 어떤 리뷰를 거쳤는지 추적할 수 있다.
5. 진행 상황은 `.superpowers/sdd/progress.md`(로컬 스크래치, git-ignore) 원장에 작업
   단위로 구현/리뷰/커밋 상태를 기록해 컨텍스트 압축·세션 재개에 대비한다.
6. 리뷰어가 지적한 내용이 **실행 계획/요구사항에 명시된 내용과 충돌**하면(예: 계획이나
   PRD가 의도적으로 지시한 패턴을 리뷰어가 결함으로 본 경우), 에이전트가 임의로
   판단하지 않고 6-1장 기준대로 사용자에게 확인한다.

## 5. Clean Code 규칙 (C++)

- RAII 원칙을 지킨다. `new`/`delete` 직접 사용 대신 스마트 포인터(`unique_ptr`,
  `shared_ptr`) 또는 값 타입(컨테이너)을 우선한다.
- 헤더(.h)에는 선언, 소스(.cpp)에는 구현을 분리한다. 템플릿 등 불가피한 경우만 예외.
- 주문 상태 등 열거형 값은 매직 넘버/문자열이 아니라 `enum class`로 표현한다.
- 주석은 WHY(숨은 제약, 트릭, 특이한 이유)만 남긴다. WHAT은 이름으로 설명한다.
- 함수/클래스는 한 가지 책임만 지도록 작게 유지한다 (SRP).
- 사용하지 않는 코드, TODO 방치, 임시 디버그 출력은 커밋 전 제거한다.

## 6. 커밋 규칙

**어떤 에이전트도 스스로 판단해서 커밋하지 않는다.** 변경이 끝나면 결과를 사용자에게
요약해서 보여주고 **사용자의 리뷰·승인을 받은 뒤에만** 커밋한다. 승인 전에는 작업
디렉터리에 변경 상태로 남겨둔다. 이건 예외 없는 게이트다.

메시지 형식, 커밋 단위, `.gitignore` 체크리스트 등 상세 절차는
`.claude/skills/commit-workflow/SKILL.md`를 따른다 (커밋하려 할 때만 참고하면 되므로
여기에 반복하지 않는다). 4-1장의 리뷰 흐름을 거친 작업은 리뷰 기록 파일을 해당 커밋에
함께 포함한다.

## 6-1. 애매한 판단은 사용자에게 확인

다음과 같이 **PRD.md/CLAUDE.md만으로 명확히 답이 나오지 않는 경우**, 에이전트는 임의로
결정하지 말고 작업을 멈추고 사용자에게 확인을 요청한다. (spec-writer는 이런 항목을
"확인 필요 사항"으로 미리 표시해 두어야 한다.)

- PRD.md의 요구사항이 여러 방식으로 해석될 수 있는 경우 (예: 예시 UI와 실제 요구사항이
  다르게 읽히는 경우)
- 기존 PoC(DataPersistence 등) 패턴을 그대로 따르기 애매하거나, PoC에 아직 없는 기능을
  새로 설계해야 하는 경우
- 테스트가 잘못된 것 같아서 implementer가 수정이 필요하다고 판단한 경우 (test-writer
  본인이 판단하기 애매한 경우 포함)
- cpp-code-reviewer가 리팩터링을 요청했지만 그 리팩터링이 동작이나 성능에 영향을 줄
  수 있다고 판단되는 경우
- 상태 흐름/계산식(재고 선점, 실생산량 등)처럼 한 번 정하면 되돌리기 번거로운 도메인
  규칙에 새로운 케이스가 발견된 경우

## 7. 상태 흐름 (요약 — 상세는 PRD.md 6장)

```
RESERVED --거절--> REJECTED
RESERVED --승인(재고충분)--> CONFIRMED --출고--> RELEASED
RESERVED --승인(재고부족)--> PRODUCING --생산완료--> CONFIRMED --출고--> RELEASED
```

## 8. 과제 진행 유의사항

- 사용 모델: Sonnet, Effort: Medium 고정 (Project_Requirement.pdf 3페이지 기준).
