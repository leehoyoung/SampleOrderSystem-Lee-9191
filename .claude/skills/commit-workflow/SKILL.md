---
name: commit-workflow
description: Use whenever you are about to create a git commit in SampleOrderSystem — checks the user-approval gate, the commit message format, commit granularity, and the .gitignore checklist before running `git commit`.
---

# 커밋 워크플로우

## 승인 게이트 (하드 게일 — CLAUDE.md 6장)

**어떤 에이전트도 스스로 판단해서 커밋하지 않는다.** 기능 하나(TDD 사이클 또는
cpp-code-reviewer 리뷰까지 끝난 논리적 단위) 또는 문서/설정 변경이 끝나면:

1. 변경 사항을 요약해서 사용자에게 보여준다 (`git diff` / `git status` 결과 포함).
2. 사용자의 명시적 승인을 받는다.
3. 승인 후에만 아래 형식으로 커밋한다.

승인 전에는 작업 디렉터리에 변경된 상태로 남겨둔다. "사용자가 이미 비슷한 걸
승인했으니 이번에도 괜찮겠지"라고 생략하지 않는다 — 매번 확인한다.

## 커밋 메시지 형식

```
<type>: <설명>
```

- **type**: `docs`(문서), `feat`(기능), `test`(테스트만 추가/수정), `fix`(버그 수정),
  `refactor`(동작 변경 없는 구조 개선), `chore`(설정/도구/빌드 관련)
- **설명**: 한국어. "무엇을 했는지"보다 **"왜"** 를 담는다 (예: "재고 판정 규칙 변경"
  보다 "재고 부족 판정 로직이 선점 규칙과 어긋나 있어 수정" 이 낫다).
- 본문(body)이 필요하면 빈 줄 하나 띄우고 이어 쓴다. PRD.md의 어느 절과 관련 있는지
  언급하면 추적에 도움이 된다.

## 커밋 단위

- TDD 사이클(RED→GREEN→REFACTOR) 또는 하나의 논리적 기능 단위로 작게 나눈다.
- 테스트가 실패하는 상태(RED)로 커밋하지 않는다.
- 문서/설정 작업(PRD.md, CLAUDE.md, `.claude/agents`, `.claude/skills` 등)은 코드
  변경과 섞지 않고 `docs`/`chore`로 별도 커밋한다.
- 같은 세션에서 여러 차례 수정한 문서라도, 최종 상태 하나로 합쳐서 커밋해도 된다
  (중간 수정 이력을 그대로 재현할 필요는 없다) — 단, 성격이 다른 변경(예: 문서 작성과
  에이전트/스킬 설계)은 커밋을 나누는 편이 추적하기 쉽다.
- `docs/tasks/<기능-슬러그>.md`(spec-writer 산출물)는 **사용자 승인**을 받은 뒤
  `docs` 타입으로 커밋한다. 이 승인은 커밋 승인 게이트와는 별개로, 태스크 목록 자체가
  test-writer로 넘어가기 전에 이미 받아야 하는 게이트다 (`CLAUDE.md` 4장). 보통 이
  승인 시점에 문서도 함께 커밋하면 된다.

## 커밋 전 .gitignore 체크리스트

이 프로젝트는 Visual Studio 프로젝트이며 Public GitHub repo로 제출될 예정이다. 커밋
전에 다음이 `.gitignore`에 포함되어 있는지 확인한다:

- `.vs/` (Visual Studio 캐시)
- `x64/`, `Debug/`, `Release/`, `ARM/`, `ARM64/` (빌드 산출물)
- `*.user` (사용자별 IDE 설정)
- `*.sdf`, `*.opendb`, `ipch/` (IntelliSense 캐시)
- 데이터 저장 파일(JSON 등 실행 시 생성되는 실데이터)은 스킴/샘플 데이터가 아니라면
  제외 대상인지 판단 — 애매하면 사용자에게 확인 (`CLAUDE.md` 6-1장).

## 애매한 경우

커밋 범위나 메시지 타입이 애매하면(`CLAUDE.md` 6-1장 참고) 임의로 정하지 말고
사용자에게 확인한다.
