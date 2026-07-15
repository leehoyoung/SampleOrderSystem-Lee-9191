# order-intake PR 전체 리뷰 기록 (태스크 1~11)

## 대상 커밋
- `cc680a6` test: createOrder가 시료 재고를 변경하지 않음을 재검증 (태스크 1)
- `bbbc7d7` feat: createOrder에 시료 존재/수량 유효성 검증 추가 (태스크 2·3·3-1·4)
- `e9f8a29` feat: 주문 관리 메뉴 흐름(접수/목록조회/뒤로가기) 구현 (태스크 5~11)
- (이 리뷰 이후 수정) `OrderView::showMenu()` 잔여 문구 제거

## requirements-auditor 결과 (PR 단위 1회)
- PRD 5.1(등록된 시료만 주문 가능), 6.1(재고는 승인/생산완료 시점에만 갱신), 7.3(주문
  접수/예약 메뉴 흐름) 전체를 태스크 1~11 각각에 대해 확인.
- 커버리지 체크리스트: 태스크 1~11 전부 코드:라인/테스트명 근거와 함께 구현됨 확인.
- 불일치 / 스코프 초과(7.4·7.6·6.1 승인 이후 갱신·7.5·7.7 등 제외 대상) / 확인 필요:
  모두 **해당 없음**.

## cpp-code-reviewer 결과 (PR 단위 1회)
- Spec 준수: requirements-auditor 판단에 동의. 태스크 1~11 각각과 실제 구현/테스트가
  1:1 대응함을 직접 확인.
- **Important 발견 → 수정 완료**: `SampleOrderSystem/Order/OrderView.cpp`의
  `showMenu()`에 카피 잔재("\n[2] 주문 접수\n")가 실제 메뉴 문구 위에 중복 출력되고
  있었음. 태스크 5는 선택지 개수/의미만 고정하고 문구는 자유라 GREEN 테스트로는
  걸러지지 않았으나, 실제 콘솔 UI 결함이므로 컨트롤러가 직접 제거.
- Minor(참고 의견, 리팩터링 강제 안 함): `OrderModel::createOrder`의 시료 존재 검증이
  `SampleModel::findById` 대신 `getAll()` + `std::any_of` 순회 방식을 택함 — 태스크
  문서가 "구현 방식은 implementer 재량"으로 명시적으로 위임했고 현재 규모에서 성능
  문제는 아니므로, 향후 조회 기능이 더 필요해질 때 `findById` 도입을 검토하는 것으로
  남겨둠.
- Clean Code 전반 양호: `IOrderView` 계층 분리, 예외→UI 메시지 변환 책임 분리,
  `enum class` 사용, 헤더/소스 분리, WHY 주석 확인.
- 테스트 품질 양호: 태스크당 하나의 행동만 검증, `FakeOrderView`가 `RouterTest.cpp`의
  `SpyController` 관례를 따름.
- **결론: 위 Important 수정 반영 후 APPROVE**

## 컨트롤러 직접 검증
- `showMenu()` 수정 후 `MSBuild SampleOrderSystem.slnx /p:Configuration=Debug /p:Platform=x64`
  (전체 솔루션) → 경고/오류 없음.
- `x64\Debug\SampleOrderSystemTests.exe` 전체 실행 → `[  PASSED  ] 41 tests.`
  (order-intake 신규 15개 + 기존 26개 전부 GREEN, 회귀 없음).

## 부수 발견 및 조치
- 태스크 2~4 구현 시 테스트 프로젝트만 빌드해 검증한 탓에 `Application.cpp`가
  `OrderModel(OrderRepository&, SampleModel&)` 시그니처 변경을 따라가지 못해 메인
  애플리케이션 빌드가 깨진 채로 넘어간 사고가 있었음 — 태스크 5~11 배치 중 발견해
  즉시 수정(`e9f8a29`에 포함). 이후 CLAUDE.md 4-1장에 "컨트롤러 직접 검증은 전체
  솔루션을 빌드한다"는 규칙을 추가해 재발을 방지함(`chore/process-overhead-reduction`
  브랜치, PR #4).

## 결론
승인 완료(Important 수정 반영). 사용자의 PR 단위 최종 승인 대기 중.
