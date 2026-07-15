# Task 3 리뷰: Sample/ 도메인 (Model+Repository+View+Controller)

- 리뷰 대상 커밋: feat: 시료 관리(등록/목록/검색) 스캐폴딩 — MVC skeleton + DataPersistence 패턴 포팅
- 리뷰 방식: superpowers:subagent-driven-development (task reviewer subagent, model: sonnet)

## Spec 준수: ✅

- Sample.h/SampleRepository.h/.cpp/SampleModel.h/.cpp/SampleView.h/.cpp/SampleController.h/.cpp
  (Steps 1–9): 브리핑 코드와 정확히 일치.
- data/samples.json: `[]` 일치 (Step 10).
- Application.h/.cpp (Step 11–12): 멤버 선언 순서 `router_ → sampleRepository_ → sampleModel_
  → sampleView_`로 Controller가 참조하는 Model/View보다 먼저 선언/생성됨 — 소멸 순서 안전.
- vcxproj (Step 13): ClInclude 5개, ClCompile 4개, 신규 `<None Include="data\samples.json" />`
  모두 브리핑과 일치하며 Task1/2 기존 항목 보존됨.
- 빌드 독립 재현: 컨트롤러가 직접 MSBuild 실행, `SampleOrderSystem.exe` 정상 생성 확인.

## 코드 품질: Approved

RAII, 헤더/소스 분리, WHY 주석(Controller 소멸 순서 주석 등) 모두 규칙 준수. enum class
미사용은 도메인상 불필요(상태 값 없음)하므로 문제 없음.

Minor: 구현자 보고서의 자가검토 서술("unique_ptr 사용...")이 실제로는 값 타입 멤버로
구현되어 있어 부정확하지만, 코드 자체는 더 단순하고 적절한 선택이라 문제 없음.

Critical/Important 항목 없음.

## Global Constraints 관련 사전 확인 (사용자 승인 완료)

SampleRepository와 (Task 4에서 추가될) OrderRepository 간 load/save/append 로직 중복은
CLAUDE.md 2장 "DataPersistence PoC 패턴을 그대로 따르고 새로 발명하지 않는다"에 따른
의도적 설계로, 실행 전 사용자에게 확인 후 "플랜 그대로 중복 유지"로 결정됨.
