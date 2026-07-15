# Task 4 리뷰: Order/ 도메인 (Model+Repository+View+Controller, Controller는 스텁)

- 리뷰 대상 커밋: chore: 주문 도메인(Order/OrderRepository/OrderModel) 스캐폴딩 — 승인/거절 로직은 이후 TDD
- 리뷰 방식: superpowers:subagent-driven-development (task reviewer subagent, model: sonnet)

## Spec 준수: ✅

Order/* 9개 파일, data/orders.json, Application.h/.cpp, vcxproj 세 ItemGroup 모두 브리핑
(Step 1~13)과 바이트 단위로 일치. `enum class OrderStatus` 값(Reserved/Rejected/Producing/
Confirmed/Released), toString/fromString 매핑, Task1/2/3 기존 항목(Core, Json, Sample)
전부 보존 확인.

## 코드 품질: Approved

- RAII: 스마트 포인터/값 타입 준수, `new`/`delete` 없음.
- 헤더/소스 분리: 준수(Order.h의 toJson/fromJson 인라인 정의만 예외이나 브리핑 스펙 그대로이며
  DummyDataGenerator 패턴 계승).
- enum class: `OrderStatus`로 정확히 표현, 매직 스트링 없음.
- 주석: 모두 WHY 위주 — WHAT 설명 없음.
- Minor(조치 불필요): `OrderModel`이 `IModel`을 상속하지만 오버라이드하는 가상 함수가 없음 —
  향후 TDD 단계에서 채워질 부분이라 현 단계 지적 대상 아님.

Critical/Important 항목 없음.

## 런타임 검증 관련 메모

구현자가 최초 보고서에서 "입력 2 선택 시 라우팅 실패"를 우려로 보고했으나, 컨트롤러(메인
세션)가 클린 리빌드 + 파일 리다이렉션(`< input.txt`)으로 직접 재현한 결과 정상 동작함을
확인했다. 구현자가 사용한 PowerShell 문자열 파이프(`"2\`r\`n" | exe`) 입력 방식의 인코딩
문제로 인한 테스트 아티팩트였으며, 실제 코드/빌드에는 결함이 없다.
