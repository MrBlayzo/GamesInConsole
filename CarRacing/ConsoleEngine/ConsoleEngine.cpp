#include <iostream>
#include "ConsoleEngine.h"

ConsoleEngine::ConsoleEngine()
{
    enableAnsiColors();
}
ConsoleEngine::~ConsoleEngine()
{
    reset_styles();
    flush_input_buffer();
}

void ConsoleEngine::clear()
{
    std::cout << "\033[2J\033[H" << std::flush;
}

void ConsoleEngine::set_cursor_to_zero()
{
    std::cout << "\033[H";
}

std::string ConsoleEngine::get()
{
    std::string input;
    std::getline(std::cin, input);
    std::cout << "\033[1A\033[2K\033[G" << std::flush;
    return input;
}

char ConsoleEngine::get_no_wait()
{
    if (::uni_kbhit())
    {
        return ::uni_getch();
    }
    return '\0';
}

void ConsoleEngine::reset_styles()
{
    std::cout << "\033[" << static_cast<int>(ConsoleStyle::Reset) << "m";
}
void ConsoleEngine::set_style(ConsoleStyle style)
{
    std::cout << "\033[" << static_cast<int>(style) << "m";
}
void ConsoleEngine::set_color(ConsoleTextColors text_color)
{
    std::cout << "\033[" << static_cast<int>(text_color) << "m";
}
void ConsoleEngine::set_color(ConsoleBkgColors background_color)
{
    std::cout << "\033[" << static_cast<int>(background_color) << "m";
}

#ifdef _WIN32
void ConsoleEngine::flush_input_buffer()
{
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    if (h != INVALID_HANDLE_VALUE)
    {
        FlushConsoleInputBuffer(h);
    }
}
#else
void ConsoleEngine::flush_input_buffer()
{
    // NOP on non-Windows
}
#endif

#ifdef _WIN32
bool ConsoleEngine::key_pressed(char key)
{
    return GetAsyncKeyState(key) & 0x8000;
}
#else
bool ConsoleEngine::key_pressed(char key)
    // TODO: сделать поддержку
    return get_no_wait() == key;
}
#endif

#ifdef _WIN32
void ConsoleEngine::enableAnsiColors()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
        return;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
        return;

    // Включаем поддержку ANSI escape-кодов
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}
#else
void ConsoleEngine::enableAnsiColors() {}
#endif