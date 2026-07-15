# PoC 통합 스캐폴딩 설계

## 1. 배경 / 목표

`MVC skeleton`, `DataPersistence`, `DummyDataGenerator`, `DataMonitor` PoC가 모두
완료되었다. 이 문서는 네 PoC의 산출물을 `SampleOrderSystem` 본 프로젝트에 어떻게
반영할지에 대한 **스캐폴딩(아키텍처/폴더 구조/데이터 모델/Repository/MVC Core)
통합 계획**이다.

**범위**: 이 문서는 스캐폴딩 반영 계획만 다룬다. PRD 7.2~7.7의 개별 기능(시료
관리/주문/승인·거절/생산 라인/모니터링/출고)에 대한 구체적인 동작 구현은
`CLAUDE.md` 4장의 파이프라인(spec-writer → test-writer → implementer)이 기능별로
따로 진행한다.

## 2. 아키텍처 / 폴더 구조

MVC skeleton의 `Core/`(Router·Application·IModel/IView/IController)를 그대로
포팅하고, `Sample/`·`Order/`·`Monitoring/` 세 기능을 먼저 채운다. 나머지 기능
(생산 라인/출고)은 PoC에 선례가 없어 이번 스캐폴딩에 포함하지 않고, 해당 기능
착수 시점에 spec-writer가 필요한 폴더를 새로 만든다.

```
SampleOrderSystem/
  Core/        IModel.h, IView.h, IController.h, Router.h/cpp, Application.h/cpp,
               StringUtil.h                         (MVC skeleton 그대로 포팅)
  Json/        JsonValue.h/cpp                       (DataPersistence 원본이 아니라
                                                       DummyDataGenerator가 확장한
                                                       버전 포팅 — makeDouble/asDouble 포함)
  Sample/      Sample.h, SampleRepository.h/cpp,
               SampleModel.h/cpp, SampleView.h/cpp,
               SampleController.h/cpp
  Order/       Order.h(+OrderStatus enum),
               OrderRepository.h/cpp, OrderModel.h/cpp,
               OrderView.h/cpp, OrderController.h/cpp
  Monitoring/  MonitorModel.h/cpp, MonitorView.h/cpp,
               MonitorController.h/cpp               (DataMonitor의 테이블/대시보드
                                                       렌더링 스타일만 참고, 아래
                                                       4장 참조)
  data/        samples.json, orders.json              (초기값 "[]", DummyDataGenerator와는
                                                        별도 경로)
  main.cpp
SampleOrderSystemTests/   GTest 콘솔 실행 프로젝트, gtest_main 링크
                          (build-and-test skill 컨벤션 그대로)
```

## 3. 데이터 모델 & 상태 enum

- `Sample`은 DummyDataGenerator의 필드(`id`/`name`/`avgProductionTimeMin`/`yield`/
  `stock`)와 `toJson()`/`fromJson()`을 그대로 이식한다.
- `Order`는 DummyDataGenerator 구조를 이식하되, `status`를 `std::string` 대신
  `enum class OrderStatus { Reserved, Rejected, Producing, Confirmed, Released }`로
  바꾼다. `Order.h` 안에 `toString(OrderStatus)` / `orderStatusFromString(...)` 변환
  함수를 함께 둔다 (기존 PoC가 구조체 자신에게 JSON 변환 책임을 맡긴 패턴을 그대로
  따름).

## 4. 포팅 vs TDD 경계 (CLAUDE.md 6-1 관련 — 확인 완료)

- **그대로 포팅(TDD 없이)**: `JsonValue`, `Router`, `Application`,
  `IModel`/`IView`/`IController`, `Sample.h`/`Order.h`의 `toJson`/`fromJson`,
  `SampleRepository`/`OrderRepository`의 `load`/`save`/`listAll` 같은 순수 CRUD
  골격 — 이미 PoC에서 동작이 검증된 인프라이므로 재검증 없이 이식한다.
- **TDD 대상**: PRD 고유의 도메인 규칙(재고 갱신/선점, 승인 시 상태 분기, 생산량
  계산식, FIFO 생산 큐, 생산 완료 판정 등) — PoC에 전례가 없으므로 이번 스캐폴딩
  단계에서는 구현하지 않는다. 해당 PRD 절 착수 시 spec-writer → test-writer →
  implementer가 처리한다.
- 이번 스캐폴딩의 `SampleModel`/`OrderModel`은 MVC skeleton의 `SampleModel`과
  동등한 수준(등록/전체조회/검색 같은 순수 CRUD 래핑)까지만 채우고, PRD 7.3~7.7의
  승인/생산/출고 로직은 넣지 않는다. `OrderController`는 MVC skeleton의 스텁
  ("준비 중입니다")을 그대로 유지한다.
- `Monitoring/`도 동일한 원칙을 따른다: DataMonitor에서 가져오는 것은 **테이블/
  대시보드 렌더링 스타일(`MonitorView`의 출력 포맷)뿐**이다. DataMonitor 고유의
  파일 mtime 폴링(2초 간격 감시)과 비동기 키 입력(`_kbhit`/`_getch`)은 도입하지
  않는다 — DataMonitor는 별도 프로세스가 남의 데이터 파일을 감시하는 구조였지만,
  `Monitoring/`은 SampleOrderSystem과 같은 프로세스 안에서 이미 메모리에 있는
  `SampleModel`/`OrderModel` 데이터를 메뉴 진입 시점에 즉시 계산해 보여주면 되므로
  폴링이 필요 없다. PRD 7.5의 실제 집계 로직(상태별 주문 수, 재고 여유/부족/고갈
  판정)은 PoC에 전례가 없는 도메인 규칙이므로 이번 스캐폴딩에서는 구현하지 않고,
  `MonitorController`도 `OrderController`와 같은 방식으로 스텁("준비 중입니다")
  으로 시작한다. 실제 로직은 해당 기능 착수 시 spec-writer → test-writer →
  implementer가 채운다.

## 5. 테스트 프로젝트

`SampleOrderSystemTests`를 GTest 콘솔 실행 파일로 새로 만들어 솔루션에 등록한다
(`build-and-test` skill 컨벤션). gmock은 이미 NuGet 패키지로 설치되어 있으므로,
테스트 프로젝트는 `gtest_main`뿐 아니라 gmock도 함께 링크해둔다 — 단
`cpp-tdd` skill 기준대로 실제 사용은 외부 경계(예: 생산 라인의 `Clock` 추상화)로
한정하고, 도메인 로직 검증에는 실제 객체를 쓴다. 이번 단계의 스캐폴딩 코드는
포팅 코드라 TDD 대상이 아니므로, 테스트는 빈 상태(또는 프로젝트가 정상적으로
빌드·링크되는지 확인하는 최소 placeholder 1개)로 시작하고, 실제 테스트는 이후
Sample/Order 도메인 로직 기능 TDD부터 채워진다.

## 6. DummyDataGenerator / DataMonitor와의 관계

- **DummyDataGenerator**: 독립 실행 도구로 유지한다 (SampleOrderSystem 메뉴에는
  통합하지 않음). `data/` 경로와 JSON 형식을 SampleOrderSystem과 별도로 유지한다
  — DummyDataGenerator는 `sampleId`, `status`를 문자열로 저장하는 반면,
  SampleOrderSystem은 `OrderStatus` enum을 쓰므로 형식이 다르다. 더미 데이터가
  필요하면 DummyDataGenerator 산출물을 SampleOrderSystem의
  `data/samples.json`/`orders.json` 형식에 맞게 수동으로 옮긴다 (자동 변환
  스크립트는 이번 범위 밖).
- **DataMonitor**: 별도 실행 파일로 남기지 않는다. 2장에서 설명한 대로 렌더링
  스타일만 `Monitoring/` 폴더로 가져와 SampleOrderSystem 메인 메뉴 안의 기능
  (PRD 7.5)으로 통합하고, 원본 PoC의 프로세스/exe 자체는 더 이상 사용하지 않는다.

## 7. 제외 범위 (이번 계획에서 다루지 않음)

- PRD 7.2~7.7 개별 기능의 실제 동작 구현 (spec-writer 파이프라인에서 별도 진행)
- DummyDataGenerator ↔ SampleOrderSystem 간 데이터 자동 변환/공유
- DataMonitor의 파일 mtime 폴링·비동기 키 입력 메커니즘 도입 (같은 프로세스라
  불필요하다고 판단, 4장 참조)
- 생산 라인의 `Clock` 추상화 등 신규 도메인 설계 (해당 기능 착수 시 cpp-tdd skill
  안내에 따라 진행)
