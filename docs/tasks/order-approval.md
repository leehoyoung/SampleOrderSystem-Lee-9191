## 기능: 주문 승인/거절 (PRD 7.4)

### 선행 조건

- PRD 7.3(시료 주문 접수/예약)은 완료되어 있다(`docs/tasks/order-intake.md`, 태스크
  1~11 구현/리뷰/PR 완료). `OrderModel::createOrder`는 `SampleModel&`을 주입받아 RESERVED
  주문을 생성한다. `OrderController`/`OrderView`/`IOrderView`로 메뉴 흐름(주문 접수/목록
  조회/뒤로가기)이 이미 있다.
- PRD 6장/6.1절을 정확히 따른다. 핵심 규칙 재정리:
  - `RESERVED --거절--> REJECTED`
  - `RESERVED --승인(재고 충분)--> CONFIRMED` (즉시 `재고 -= 주문 수량`)
  - `RESERVED --승인(재고 부족)--> PRODUCING` (재고 필드는 변경하지 않음)
  - 신규 주문의 충분/부족 판정은 판정 시점의 재고 필드 값을 그대로 쓰되, **동일 시료에
    대해 이미 `PRODUCING` 상태인 주문이 하나라도 존재하면, 재고 필드 값과 무관하게
    가용 재고를 0으로 간주해 신규 승인은 항상 "부족"으로 분류된다** (PRD 6.1 예시
    1~2단계: 재고 30, 1번 주문 qty=50 승인 → 부족 → `PRODUCING`(재고 그대로 30); 2번
    주문 qty=10 승인 시점에도 재고 필드는 여전히 30이지만, 1번 주문이 이미
    `PRODUCING`이므로 가용 재고 0으로 간주되어 2번 주문도 `PRODUCING`이 된다 — 30≥10
    임에도 단순 필드 비교만으로는 이 결과가 나오지 않으므로, 별도의 "동일 시료 +
    PRODUCING 존재 여부" 조회가 approveOrder 로직에 필요하다).
  - 재고 부족 승인 후 생산 완료 시점의 재고 갱신(`재고 = 재고 + 실생산량 − 주문 수량`,
    `PRODUCING → CONFIRMED` 전이)은 PRD 7.6(생산 라인) 범위다. **이번 스펙에서는
    구현하지 않는다** — 승인 시점에 `CONFIRMED` 또는 `PRODUCING`으로 전이하고 그 시점의
    재고 갱신 여부만 다룬다.
- 재사용:
  - `Order/OrderRepository`, `Sample/SampleRepository`, DataPersistence PoC의 Repository
    패턴을 그대로 따른다. 새 저장 포맷을 만들지 않는다.
  - Controller/View 테스트 전략은 order-intake에서 확정한 대로 `IOrderView` + 테스트
    더블(Fake/Mock) 주입 방식을 그대로 재사용한다 (표준입출력 리디렉션 방식 채택 안 함).
- 현재 코드에는 승인/거절에 필요한 다음 기능이 **아직 없다** (이번 스펙에서 새로 추가):
  - `OrderRepository`에는 `append`/`listAll`/`nextOrderNo`만 있고, 기존 주문의 상태를
    바꿔 저장하는 메서드가 없다.
  - `SampleModel`/`SampleRepository`에는 `addSample`/`getAll`/`findByName`/`nextId`만
    있고, 재고를 갱신(차감)하는 메서드가 없다.
  - `OrderModel`에는 `createOrder`/`getAll`만 있고, `approveOrder`/`rejectOrder`가 없다.
  - `IOrderView`/`OrderView`/`OrderController`에는 "접수된 주문 목록(RESERVED 필터)",
    "주문 승인", "주문 거절" 메뉴가 없다(order-intake 시점의 "목록 조회"는 전체 목록이었다).
- 순서: (1) 데이터 계층 기반 작업(재고 차감, 주문 상태 갱신 저장) → (2) `OrderModel`
  거절 로직 → (3) `OrderModel` 승인 로직(충분/부족/기존 PRODUCING 잠금 규칙) →
  (4) 예외/경계 → (5) Controller/View 메뉴 흐름. Controller/View 계층은 "확인 필요
  사항"의 메뉴 입출력 방식 결정 후 착수한다.

### TDD 태스크 목록 (순서대로)

1. **[기반] `SampleModel`에 재고 차감 메서드가 시료의 재고를 정확히 줄이고 영속화한다.**
   — 입력: 시료 `S-001`(재고 30)이 등록된 상태에서 신규 메서드(예:
   `decreaseStock("S-001", 10)`) 호출.
   기대 결과: 이후 `SampleModel::getAll()`/`SampleRepository`로 조회한 `S-001`의
   `stock`이 20. (메서드 이름/시그니처는 implementer 재량이나, "확인 필요 사항" 참고.)

2. **[기반, 경계] 존재하지 않는 시료 ID로 재고 차감을 시도하면 실패한다.**
   — 입력: 등록된 시료가 `S-001`뿐인 상태에서 `decreaseStock("S-999", 10)` 호출.
   기대 결과: 예외(또는 실패 신호) 발생, 어떤 시료의 재고도 변경되지 않음.

3. **[기반] `OrderRepository`에 기존 주문의 상태를 갱신하는 메서드가 상태를 바꾸고
   영속화한다.**
   — 입력: 주문 `ORD-20260101-0001`(상태 `RESERVED`)이 저장된 상태에서 신규 메서드(예:
   `updateStatus("ORD-20260101-0001", OrderStatus::Rejected)`) 호출.
   기대 결과: 이후 `listAll()`로 조회한 해당 주문의 `status`가 `REJECTED`로 바뀜, 나머지
   필드(`sampleId`/`customerName`/`quantity`)는 변화 없음.

4. **[기반, 경계] 존재하지 않는 주문번호로 상태 갱신을 시도하면 실패한다.**
   — 입력: 저장된 주문이 없는 상태에서 `updateStatus("ORD-NOPE", OrderStatus::Rejected)`
   호출.
   기대 결과: 예외(또는 실패 신호) 발생, 목록에 아무 변화 없음.

5. **`OrderModel::rejectOrder`는 RESERVED 주문을 REJECTED로 전이시키고 재고를 건드리지
   않는다.**
   — 입력: 시료 `S-001`(재고 30) 존재, 주문 `ORD-...-0001`(sampleId="S-001", quantity=10,
   status=RESERVED) 존재 상태에서 `rejectOrder("ORD-...-0001")` 호출.
   기대 결과: 해당 주문의 `status == REJECTED`, `S-001`의 `stock`은 여전히 30 (변화 없음).
   근거: PRD 6장 상태 흐름 표 — 거절은 재고 갱신 시점(6.1)에 포함되지 않는다.

6. **[경계] 존재하지 않는 주문번호로 거절을 시도하면 실패한다.**
   — 입력: 등록된 주문이 없는 상태에서 `rejectOrder("ORD-NOPE")` 호출.
   기대 결과: 예외(또는 실패 신호) 발생, 어떤 주문/재고도 변경되지 않음.

7. **[경계] RESERVED가 아닌 주문(이미 처리됨)을 거절하려 하면 실패한다.**
   — 입력: 상태가 이미 `CONFIRMED`인 주문 `ORD-...-0002`에 대해 `rejectOrder("ORD-...-0002")`
   호출.
   기대 결과: 예외(또는 실패 신호) 발생, 해당 주문의 상태가 `CONFIRMED`로 그대로 유지됨
   (이중 처리 방지). 근거: PRD 6장 상태 전이 다이어그램에서 REJECTED로 가는 화살표의
   출발 상태는 RESERVED뿐이다.

8. **`OrderModel::approveOrder`는 재고가 주문 수량보다 충분히 많으면 즉시 CONFIRMED로
   전이하고 그만큼 재고를 차감한다.**
   — 입력: 시료 `S-001`(재고 30), 주문 `ORD-...-0001`(sampleId="S-001", quantity=10,
   status=RESERVED)에 대해 `approveOrder("ORD-...-0001")` 호출.
   기대 결과: 해당 주문의 `status == CONFIRMED`, `S-001`의 `stock == 20` (30 − 10).
   근거: PRD 7.4 "재고 충분 → 즉시 CONFIRMED 전환", PRD 6.1 "재고 -= 주문 수량".

9. **[경계, 확정] 재고와 주문 수량이 정확히 일치하는 경우도 "충분"으로 처리되어
   즉시 CONFIRMED + 재고 0으로 차감된다.**
   — 입력: 시료 `S-001`(재고 10), 주문 quantity=10에 대해 `approveOrder` 호출.
   기대 결과: `status == CONFIRMED`, `stock == 0`.
   근거: 사용자 확정 — 재고 `>=` 주문 수량이면 "충분"(경계 포함).

10. **`OrderModel::approveOrder`는 재고가 주문 수량보다 부족하면 PRODUCING으로 전이하고
    재고를 변경하지 않는다.**
    — 입력: 시료 `S-001`(재고 30), 주문 `ORD-...-0001`(sampleId="S-001", quantity=50,
    status=RESERVED)에 대해 `approveOrder("ORD-...-0001")` 호출.
    기대 결과: 해당 주문의 `status == PRODUCING`, `S-001`의 `stock`은 여전히 30 (변화
    없음). 근거: PRD 7.4 "재고 부족 → 생산 라인 큐 자동 등록, PRODUCING 전환", PRD 6.1
    1단계 예시.

11. **[경계] 재고 0인 시료에 대한 승인도 PRODUCING으로 처리되며 재고는 0 그대로
    유지된다.**
    — 입력: 시료 `S-002`(재고 0), 주문 quantity=5에 대해 `approveOrder` 호출.
    기대 결과: `status == PRODUCING`, `S-002`의 `stock == 0` (그대로).

12. **[핵심 도메인 규칙] 동일 시료에 이미 PRODUCING 상태인 주문이 존재하면, 재고 필드
    값이 신규 주문 수량 이상이어도 신규 승인은 무조건 PRODUCING으로 분류되고 재고는
    변경되지 않는다.**
    — 입력(PRD 6.1 예시 1~2단계 그대로): 시료 `S-001`(재고 30). 1번 주문
    (sampleId="S-001", quantity=50)을 먼저 `approveOrder` → `PRODUCING`(재고 30 그대로,
    태스크 10과 동일 결과). 이어서 2번 주문(sampleId="S-001", quantity=10, 이 시점 재고
    필드는 여전히 30)에 대해 `approveOrder` 호출.
    기대 결과: 2번 주문의 `status == PRODUCING` (30 ≥ 10임에도 CONFIRMED가 아님),
    `S-001`의 `stock`은 여전히 30 (변화 없음).
    근거: PRD 6.1 "이미 PRODUCING인 주문이 존재해도... 재고 부족 승인 시점에 기존 재고를
    전혀 쓰지 못했다면... 향후 재고에 아무 영향도 주지 않는다" 및 뒤따르는 예시.
    이 태스크는 `approveOrder`가 "동일 sampleId를 가진 다른 주문 중 상태가 PRODUCING인
    것이 존재하는지"를 조회하는 로직을 요구한다 — 구현 방식(예: `OrderModel` 내부에서
    `getAll()` 필터링)은 implementer 재량이나, 이 조회 없이는 이 테스트를 통과할 수 없다.

13. **[경계] 동일 시료에 PRODUCING 주문이 있어도 다른 시료의 승인 판정에는 영향을 주지
    않는다.**
    — 입력: 시료 `S-001`(재고 30, PRODUCING 주문 1건 존재)과 시료 `S-002`(재고 20,
    PRODUCING 주문 없음)가 있는 상태에서, `S-002`에 대한 신규 주문(quantity=5)을
    `approveOrder` 호출.
    기대 결과: `status == CONFIRMED`, `S-002`의 `stock == 15` (20 − 5). `S-001`의 재고나
    PRODUCING 주문 상태는 변화 없음. (태스크 12의 조회 로직이 sampleId로 정확히 필터링
    되는지 확인하는 회귀 테스트.)

14. **[경계] 존재하지 않는 주문번호로 승인을 시도하면 실패한다.**
    — 입력: 등록된 주문이 없는 상태에서 `approveOrder("ORD-NOPE")` 호출.
    기대 결과: 예외(또는 실패 신호) 발생, 어떤 주문/재고도 변경되지 않음.

15. **[경계] RESERVED가 아닌 주문(이미 처리됨)을 승인하려 하면 실패한다.**
    — 입력: 상태가 이미 `REJECTED`인 주문에 대해 `approveOrder` 호출.
    기대 결과: 예외(또는 실패 신호) 발생, 해당 주문 상태와 관련 시료 재고 모두 변화 없음
    (이중 승인/재고 이중 차감 방지).

16. **[경계, 방어적] 승인 대상 주문의 sampleId가 더 이상 등록된 시료 목록에 없으면
    실패한다.**
    — 입력: 주문은 sampleId="S-999"로 저장되어 있으나 현재 등록된 시료 목록에 `S-999`가
    없는 상태(데이터 불일치 시나리오)에서 `approveOrder` 호출.
    기대 결과: 예외(또는 실패 신호) 발생, 해당 주문 상태 변화 없음. (order-intake의
    createOrder 시점 검증과 별개로, 승인 시점에도 방어적으로 검증한다.)

17. **[확정] 주문 관리 메뉴의 기존 "접수된 주문 목록"(2번) 메뉴가 RESERVED 상태
    주문만 필터링해 보여주도록 좁혀 교체된다.**
    — 입력: 주문 4건 존재(상태 RESERVED 2건, CONFIRMED 1건, REJECTED 1건) 상태에서
    "접수된 주문 목록" 메뉴 선택.
    기대 결과: 화면에 RESERVED 2건만 표시되고 나머지 2건은 표시되지 않음. 근거: PRD 7.4
    "접수된 주문 목록: RESERVED 상태 주문 목록 표시". 사용자 확정 — 기존 메뉴 항목을
    RESERVED 필터로 교체한다(전체 이력 조회는 이번 스펙 범위 밖, 추후 모니터링/조회
    기능에서 다룸). `OrderControllerTest`의 기존 전체 목록 조회 테스트는 이 의미 변경에
    맞춰 함께 수정한다.

18. **[경계] RESERVED 주문이 하나도 없을 때 "접수된 주문 목록"은 빈 상태를 오류 없이
    표시한다.**
    — 입력: 주문은 존재하지만 전부 CONFIRMED/REJECTED 상태인 경우 "접수된 주문 목록"
    선택.
    기대 결과: 크래시/예외 없이 "RESERVED 주문 없음" 등 빈 상태 안내(0건 목록 출력도
    허용).

19. **주문 관리 메뉴에서 "주문 승인" 선택 → 주문번호 입력 → 승인 처리 후 전이된 상태를
    포함한 완료 메시지가 표시된다.**
    — 입력: 시료 `S-001`(재고 30), 주문 `ORD-...-0001`(RESERVED, quantity=10) 존재.
    사용자가 "주문 승인" 메뉴 선택 후 주문번호로 `ORD-...-0001` 입력.
    기대 결과: `OrderModel`에서 해당 주문 `status == CONFIRMED`로 바뀜, 화면에 전이된
    상태(CONFIRMED)를 알 수 있는 완료 메시지 출력.

20. **주문 승인 시 재고 부족으로 PRODUCING 전이된 경우에도 그 사실이 화면에 표시된다.**
    — 입력: 시료 `S-001`(재고 5), 주문 `ORD-...-0002`(RESERVED, quantity=20) 존재.
    "주문 승인" → 주문번호 `ORD-...-0002` 입력.
    기대 결과: 해당 주문 `status == PRODUCING`, 화면에 PRODUCING 전이임을 알 수 있는
    메시지 출력(예: "재고 부족으로 생산 라인에 등록되었습니다" 류 — 문구는 자유, CONFIRMED
    메시지와 구분 가능해야 함만 고정).

21. **주문 관리 메뉴에서 "주문 거절" 선택 → 주문번호 입력 → 거절 처리 후 완료 메시지가
    표시된다.**
    — 입력: 주문 `ORD-...-0003`(RESERVED) 존재. "주문 거절" → 주문번호 입력.
    기대 결과: 해당 주문 `status == REJECTED`, 화면에 거절 완료 메시지 출력.

22. **[경계] 승인/거절 메뉴에서 존재하지 않는 주문번호를 입력하면 실패 메시지가
    표시되고 메뉴로 복귀한다(프로그램 종료 없음).**
    — 입력: 등록된 주문이 없는 상태에서 "주문 승인" → 주문번호 `ORD-NOPE` 입력.
    기대 결과: 태스크 14 로직 재사용으로 승인 실패, 화면에 실패 메시지 출력, 메뉴 루프
    유지(크래시 없음). (거절 메뉴도 동일 패턴이므로 승인/거절 중 하나만 대표로 우선
    작성하고, 나머지는 회귀 테스트로 짧게 추가한다.)

23. **[경계] 승인/거절 메뉴에서 이미 처리된 주문번호를 입력하면 실패 메시지가 표시된다.**
    — 입력: 상태가 이미 `CONFIRMED`인 주문번호를 "주문 거절" 메뉴에 입력.
    기대 결과: 태스크 7 로직 재사용으로 거절 실패, 화면에 실패 메시지 출력, 해당 주문
    상태는 `CONFIRMED`로 유지.

### 확인 필요 사항 (결정 완료)

- **재고 == 주문 수량 경계 — 확정.** 재고 `>=` 주문 수량이면 "충분"으로 처리한다(경계
  포함, 재고 0까지 차감 허용). 태스크 9 참고.

- **재고 차감/갱신 로직을 둘 계층 — 확정.** `SampleRepository`에 "재고만 델타로
  변경"하는 전용 메서드(예: `applyStockDelta(sampleId, delta)`)를 추가한다. 구체적
  시그니처는 implementer 재량이나, 델타 방식이어야 한다(시료 객체 전체를 덮어쓰는
  범용 `update()` 방식은 채택하지 않음). PRD 7.6(생산 완료 시 재고 갱신)에서도 이
  메서드를 재사용할 것을 염두에 둔다.

- **`OrderRepository` 상태 갱신 메서드의 실패 시 동작(예외 vs 반환값)** — 이번 문서에서
  의도적으로 정하지 않았다. "무엇을 검증해야 하는가"(상태가 바뀐다/안 바뀐다)만 고정하고
  구체적 방식은 implementer 재량으로 남긴다.

- **"접수된 주문 목록" 메뉴 — 확정.** 기존 "접수된 주문 목록(2번)" 메뉴를 RESERVED
  필터로 좁혀 교체한다(대체, 신규 추가 아님). 전체 이력 조회는 이번 스펙 범위 밖(추후
  모니터링/조회 기능에서 다룸). 태스크 17 참고. order-intake에서 작성된
  `OrderControllerTest.ListingOrdersShowsAllCreatedOrders`(전체 목록 조회 검증)는 이
  의미 변경에 맞춰 함께 수정해야 한다 — cpp-tdd 스킬 기준 "테스트 코드 리팩터링"이
  아니라 요구사항 자체가 바뀐 것이므로, 새 태스크(태스크 17)의 RED 사이클 안에서
  test-writer가 함께 고친다.

- **승인/거절 메뉴의 입력 방식 — 확정.** 주문번호를 직접 입력하는 방식으로 한다
  (인덱스 선택 방식 채택 안 함). 태스크 19~23 그대로 진행한다.

- **Controller/View 테스트 전략은 order-intake에서 이미 확정한 `IOrderView` + Fake/Mock
  주입 방식을 그대로 재사용한다.** 재확인 완료.
