#include "Core/Application.h"

#include <windows.h>

int main() {
    // 소스가 /utf-8로 컴파일되어 문자열 리터럴이 UTF-8 바이트로 저장되므로,
    // 콘솔 코드페이지도 UTF-8(65001)로 맞추지 않으면 한글이 깨져 보인다.
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    Application app;
    app.run();
    return 0;
}
