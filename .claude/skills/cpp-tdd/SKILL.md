---
name: cpp-tdd
description: Use whenever implementing any behavior in SampleOrderSystem (new feature, bugfix, refactor) with C++/GTest/Visual Studio — before writing a test or production code. Defines what RED/GREEN mean for a compiled, statically-typed language, the stub convention that keeps the test-writer/implementer role boundary clean, and how to test time-based production-line logic without waiting in real time.
---

# C++ TDD (GTest / Visual Studio) — SampleOrderSystem 전용

이 skill은 [superpowers의 test-driven-development skill](https://github.com/)의
Red-Green-Refactor 원칙을 기반으로, C++(정적 컴파일 언어)과 이 프로젝트의 역할 분리
(test-writer / implementer / cpp-code-reviewer 에이전트)에 맞게 새로 작성한 규칙이다.

**핵심 원칙은 동일하다: 테스트를 먼저 쓰고, 실패를 직접 보고, 최소 코드로 통과시킨다.
규칙의 문구를 어기는 것은 규칙의 취지를 어기는 것이다.**

## Iron Law

```
실패하는 테스트 없이 프로덕션 코드(스텁 본문 로직)를 작성하지 않는다.
```

## RED의 정의 — 왜 C++에서는 특히 중요한가

동적 언어라면 "아직 함수가 없다"는 사실 자체가 곧바로 런타임 에러로 나타나 RED처럼
보이기 쉽다. 하지만 C++은 컴파일/링크 단계가 있어서, 자칫 **"빌드 에러"를 RED로
착각**하기 쉽다. 이 프로젝트에서는 그렇게 하지 않는다.

**RED는 오직 다음 상태만을 의미한다:**

1. 빌드가 성공한다 (컴파일 에러 없음)
2. 링크가 성공한다 (미해결 심볼 없음)
3. 테스트 실행 파일이 크래시 없이 끝까지 실행된다
4. GTest가 assertion 불일치로 인해 **FAILED**를 출력한다

빌드/링크 에러, 혹은 실행 중 크래시(미처리 예외, 세그폴트 등)는 RED가 아니다. 이런
상태는 "테스트가 검증하려는 로직이 맞는지"를 전혀 알려주지 못하기 때문이다.

### 스텁(Stub) 컨벤션

위 RED를 만들려면, 테스트가 호출하는 타입/함수가 **이미 컴파일·링크·실행 가능한 형태로
존재**해야 한다. 그래서 테스트 작성과 동시에 다음과 같은 트리비얼 스텁을 만든다:

- 선언(시그니처)은 최종적으로 필요한 모양 그대로 만든다.
- 본문은 **조건 분기나 계산 없이 고정된 기본값만 반환**한다.
  - 예: `int actualQuantity() const { return 0; }`
  - 예: `bool isApproved() const { return false; }`
  - 예: `std::vector<Order> pendingOrders() const { return {}; }`
- 스텁은 "구현"이 아니라 "테스트가 실행 가능하도록 만드는 API 골격"이다. 이 스텁을
  만드는 것은 test-writer(RED)의 역할이고, 스텁의 본문에 실제 로직을 채우는 것은
  implementer(GREEN)의 역할이다. 두 역할 모두 상대방의 영역(테스트 내용 / 시그니처)을
  침범하지 않는다.

## Red-Green-Refactor 사이클

### RED (test-writer 담당)

1. 검증할 행동(behavior) 하나를 정한다. "and"가 필요하면 쪼갠다.
2. 필요한 타입/함수의 선언 + 트리비얼 스텁을 만든다.
3. 그 행동을 검증하는 GTest 테스트를 작성한다. PRD.md의 수치 예시를 그대로 assertion에
   반영한다.
4. 빌드하고 테스트를 실행한다.
   - 빌드/링크 실패 → 스텁이 불완전한 것. 스텁을 보완한다.
   - 크래시 → 스텁 반환값이 실행을 막고 있는 것. 안전한 기본값으로 조정한다.
   - GTest FAILED (assertion 불일치) → 성공적인 RED. 실패 메시지가 기대한 이유(스텁의
     트리비얼 값 vs 기대값)로 나왔는지 확인한다.

### GREEN (implementer 담당)

1. RED 상태의 스텁 본문에 실제 로직을 채운다. 시그니처는 바꾸지 않는다.
2. 테스트를 통과시키는 데 필요한 최소한만 구현한다 (YAGNI). 옵션, 유연성, 미래 대비
   추상화를 넣지 않는다.
3. 빌드하고 테스트를 실행한다.
   - 대상 테스트 통과, 다른 모든 테스트도 여전히 통과, 출력에 경고/에러 없음(pristine)
     을 확인한다.

### REFACTOR (cpp-code-reviewer가 판단, test-writer/implementer가 실행)

- GREEN 상태에서만 진행한다.
- 새 동작을 추가하지 않는다. 순수 구조 개선(중복 제거, 이름 개선, 헬퍼 추출)만 한다.
- 리팩터링 후 반드시 전체 테스트를 다시 실행해 여전히 GREEN인지 확인한다.
- 테스트 코드 리팩터링은 test-writer가, 프로덕션 코드 리팩터링은 implementer가
  실행한다 (`.claude/agents/cpp-code-reviewer.md` 참고).

## 실시간(현실 시간) 로직 테스트하기 — 생산 라인

PRD.md의 생산 라인은 "생산 시간이 현실 시간을 기준으로 진행되고, 프로그램이 꺼져도
생산은 계속된 것으로 간주"해야 한다. 이런 로직을 테스트할 때 실제로 수십 분을 기다릴
수는 없다.

- 시간을 얻는 부분(예: `now()`)은 **주입 가능한 인터페이스**로 추상화한다 (예: `Clock`
  인터페이스를 만들고, 프로덕션에서는 `std::chrono::system_clock`을 감싼 구현체를,
  테스트에서는 임의의 시각을 반환하는 `FakeClock`을 주입).
- 테스트는 `FakeClock`의 시각을 원하는 만큼 앞으로 돌려서 "생산 완료 여부", "재시작 후
  경과 시간 반영" 같은 시나리오를 실제 대기 없이 검증한다.
- 이 `Clock` 추상화 자체도 하나의 행동(behavior)으로 TDD 사이클을 거쳐 만든다 —
  미리 설계해두고 예외로 취급하지 않는다.

## 테스트 품질 기준

| 기준 | 좋음 | 나쁨 |
|------|------|------|
| 최소성 | 행동 하나만 검증 | "and"가 들어간 테스트 이름 |
| 명확성 | 이름이 행동을 설명 | `Test1`, `WorksCorrectly` |
| 실제성 | 실제 객체 사용 | 불필요한 mock (GTest gmock은 외부 경계에서만) |
| PRD 반영 | PRD 수치 예시를 그대로 assertion에 사용 | 임의로 지어낸 숫자 |

## 흔한 착각

| 착각 | 실제 |
|------|------|
| "컴파일 에러도 실패니까 RED 아냐?" | 아니다. 로직 검증을 못 한다. 스텁으로 컴파일/링크/실행까지 되게 만들고 assertion FAILED를 봐야 RED다. |
| "스텁에 if문 하나 정도는 괜찮겠지" | 그 순간 구현이 된다. implementer 영역 침범. 항상 고정값만 반환. |
| "생산 라인은 실시간이라 테스트에서 sleep으로 기다리면 되지" | 느리고 불안정하다. Clock을 주입해서 시간을 조작한다. |
| "일단 통과시키고 나중에 정리하자" | REFACTOR는 GREEN 상태에서만, 그리고 cpp-code-reviewer 판단 후에 한다. |

## 완료 전 체크리스트

- [ ] 새 함수/메서드마다 테스트가 있다
- [ ] 각 테스트가 RED(GTest FAILED, 빌드/크래시 아님) 상태를 직접 확인했다
- [ ] 실패 메시지가 기대한 이유(assertion 불일치)였다
- [ ] 스텁 본문에는 최소 코드만 채웠다 (GREEN)
- [ ] 모든 테스트가 통과하고 출력이 깨끗하다
- [ ] mock은 불가피한 경우에만 사용했다
- [ ] PRD.md의 엣지 케이스가 커버되었다

## 빌드/테스트 실행 방법

정확한 명령어는 `build-and-test` skill을 참고한다.
