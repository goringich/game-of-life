#include <iostream>
#include <string>
#include <windows.h>

int main() {
    // Set console output to UTF-8 encoding
    SetConsoleOutputCP(CP_UTF8);

    // Unicode representation of a bone emoji
    std::wstring boneUTF16 = L"\U0001F9A8";

    // Output the Unicode representation of the bone
    std::wcout << L"Unicode representation of bone: " << boneUTF16 << std::endl;

    return 0;
}
