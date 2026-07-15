---
name: commit-workflow
description: Use whenever you are about to create a git commit in SampleOrderSystem — checks the user-approval gate, the commit message format, commit granularity, and the .gitignore checklist before running `git commit`.
---

# 커밋 워크플로우

## 승인 게이트 (CLAUDE.md 6장)

**어떤 서브에이전트도 스스로 판단해서 커밋하지 않는다.** 커밋은 항상 컨트롤러(메인
세션)가 수행한다.

- **태스크(로컬 커밋) 단위**: 사용자 승인을 기다리지 않고 컨트롤러가 바로 커밋한다
  (로컬 커밋은 되돌리기 쉬운 작업이므로). 다만 6-1장에 해당하는 위험한 판단이 그
  태스크에서 발견됐다면 커밋 전 즉시 사용자에게 확인한다.
- **PR(기능 브랜치) 단위 — 예외 없는 게이트**: 4-1장 기준으로 해당 PR의 모든 태스크
  구현·리뷰·컨트롤러 직접 검증이 끝나면, PR 전체의 `git diff`/커밋 로그를 한 번에
  보여주고 병합 전 사용자의 명시적 승인을 받는다. "이전 PR에서 비슷한 걸 승인했으니
  이번에도 괜찮겠지"라고 생략하지 않는다 — PR마다 확인한다.
- 문서/설정 전용 변경(코드 변경이 없는 `docs`/`chore` 커밋, 예: spec-writer 산출물
  승인)은 위 PR 흐름과 무관하게 그 자리에서 승인받고 바로 커밋해도 된다.

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
