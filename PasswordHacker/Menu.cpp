#include "Menu.h"

#include <format>

Menu::Menu(ConsoleEngine& engine, int width, int height, int pos_x, int pos_y)
    : engine(engine),
      width(width),
      height(height),
      current_option(0),
      pos_x(pos_x),
      pos_y(pos_y) {}

void Menu::set_relative_pos(int x, int y) {
    engine.set_cursor_to_pos(pos_x + x, pos_y + y);
}

void Menu::draw_frame() {
    for (int x = 0; x < width; ++x) {
        set_relative_pos(x, 0);
        engine.print('#');
    }
    for (int x = 0; x < width; ++x) {
        set_relative_pos(x, height - 1);
        engine.print('#');
    }
    for (int y = 0; y < height; ++y) {
        set_relative_pos(0, y);
        engine.print('#');
    }
    for (int y = 0; y < height; ++y) {
        set_relative_pos(width - 1, y);
        engine.print('#');
    }
    for (int y = 1; y < height - 1; ++y) {
        set_relative_pos(1, y);
        engine.print(std::string(width - 2, ' '));
    }
}

void Menu::draw() {
    draw_frame();
    draw_options();
}

void Menu::draw_options() {
    for (int option = 0; option < get_options_size(); ++option) {
        draw_option(option, option == current_option);
    }
}

void Menu::select_option(int option) {
    option = std::max(0, std::min(get_options_size() - 1, option));
    if (option == current_option) return;
    draw_option(current_option, false);
    current_option = option;
    draw_option(option, true);
}

std::optional<int> MenuSingle::show_options_menu(
    ConsoleEngine& engine, int width, int heigth, int pos_x, int pos_y,
    std::vector<MenuOption> options) {
    return MenuSingle(engine, width, heigth, pos_x, pos_y, options)
        .get_option();
}
MenuSingle::MenuSingle(ConsoleEngine& engine, int width, int height, int pos_x,
                       int pos_y, std::vector<MenuOption> options)
    : Menu(engine, width, height, pos_x, pos_y), options(options) {
    draw();
}

void MenuSingle::draw_option(int option, bool is_select) {
    set_relative_pos((width - 2 - options[option].param.size()) / 2 + 1,
                     option + 1);
    if (is_select) {
        engine.set_background_color(Colors256::Gray50);
        engine.print(options[option].param);
        engine.reset_styles();
    } else {
        engine.print(options[option].param);
    }
}

int MenuSingle::get_options_size() { return options.size(); }

std::optional<int> MenuSingle::get_option() {
    char c;
    do {
        c = engine.get_no_wait();
        if (c == 'w') {
            select_option(current_option - 1);
        } else if (c == 's') {
            select_option(current_option + 1);
        } else if (c == '\r') {
            return current_option;
        } else if (c == 27) {
            return std::nullopt;
        }
    } while (true);
    return std::nullopt;
}

std::optional<std::vector<MenuCountOption>> MenuCount::show_options_menu(
    ConsoleEngine& engine, int width, int heigth, int pos_x, int pos_y,
    std::vector<MenuCountOption> options) {
    return MenuCount(engine, width, heigth, pos_x, pos_y, options).get_option();
}
MenuCount::MenuCount(ConsoleEngine& engine, int width, int height, int pos_x,
                     int pos_y, std::vector<MenuCountOption> options)
    : Menu(engine, width, height, pos_x, pos_y), options(options) {
    draw();
}

void MenuCount::draw_option(int option, bool is_select) {
    std::string option_str =
        options[option].param + " " + std::to_string(options[option].count);
    set_relative_pos((width - 2 - option_str.size()) / 2 + 1, option + 1);
    if (is_select) {
        engine.set_background_color(Colors256::Gray50);
        engine.print(option_str);
        engine.reset_styles();
    } else {
        engine.print(option_str);
    }
}

int MenuCount::get_options_size() { return options.size(); }

std::optional<std::vector<MenuCountOption>> MenuCount::get_option() {
    char c;
    do {
        c = engine.get_no_wait();
        if (c == 'w') {
            select_option(current_option - 1);
        } else if (c == 's') {
            select_option(current_option + 1);
        } else if (c == 'r') {
            options[current_option].count =
                std::min(options[current_option].count + 1,
                         options[current_option].max_count);
            draw_option(current_option, true);
        } else if (c == 'f') {
            options[current_option].count =
                std::max(options[current_option].count - 1, 0.0);
            draw_option(current_option, true);
        } else if (c == '\r') {
            return options;
        } else if (c == 27) {
            return std::nullopt;
        }
    } while (true);
    return std::nullopt;
}

std::optional<std::vector<MenuSolverOption>> MenuSolver::show_options_menu(
    ConsoleEngine& engine, int width, int heigth, int pos_x, int pos_y,
    std::function<double(double, double, double)> cost_func,
    double current_money) {
    return MenuSolver(engine, width, heigth, pos_x, pos_y, cost_func,
                      current_money)
        .get_option();
}
MenuSolver::MenuSolver(ConsoleEngine& engine, int width, int height, int pos_x,
                       int pos_y,
                       std::function<double(double, double, double)> cost_func,
                       double current_money)
    : Menu(engine, width, height, pos_x, pos_y),
      options({MenuSolverOption("Len", 1, 16),
               MenuSolverOption("Complexity", 0, 3),
               MenuSolverOption("Hashrate", 1.0, -1)}),
      cost_func(cost_func),
      current_money(current_money) {
    draw();
}

void MenuSolver::draw_option(int option, bool is_select) {
    std::string raw_count = std::format("{:.0f}", options[option].count);
    int len = static_cast<int>(raw_count.length());
    for (int i = len - 3; i > 0; i -= 3) {
        raw_count.insert(i, " ");
    }
    std::string option_str = options[option].param + " " + raw_count;
    set_relative_pos(1, option + 1);
    int start_offset = (width - 2 - option_str.size()) / 2;
    engine.print(std::string(start_offset, ' '));

    if (is_select) {
        engine.set_background_color(Colors256::Gray50);
        engine.print(option_str);
        engine.reset_styles();
    } else {
        engine.print(option_str);
    }

    engine.print(
        std::string(width - 3 - (start_offset + option_str.size()), ' '));
}

int MenuSolver::get_options_size() { return options.size(); }

std::optional<std::vector<MenuSolverOption>> MenuSolver::get_option() {
    draw_cost();
    draw_current_money();
    char c;
    do {
        c = engine.get_no_wait();
        if (c == 'w') {
            select_option(current_option - 1);
        } else if (c == 's') {
            select_option(current_option + 1);
        } else if (c == 'r') {
            double div = 1;
            while (div * 10 <= options[current_option].count) {
                div *= 10;
            }
            div = std::max(1.0, div / 10);
            options[current_option].count = options[current_option].count + div;
            if (options[current_option].max_count > 0)
                options[current_option].count =
                    std::min(options[current_option].count,
                             options[current_option].max_count);

            draw_option(current_option, true);
            draw_cost();
        } else if (c == 'f') {
            double div = 1;
            while (div * 10 <= options[current_option].count) {
                div *= 10;
            }
            div = std::max(1.0, div / 10);
            options[current_option].count =
                std::max(options[current_option].count - div, 0.0);
            draw_option(current_option, true);
            draw_cost();
        } else if (c == 't') {
            options[current_option].count = options[current_option].count * 10;
            if (options[current_option].max_count > 0)
                options[current_option].count =
                    std::min(options[current_option].count,
                             options[current_option].max_count);

            draw_option(current_option, true);
            draw_cost();
        } else if (c == 'g') {
            options[current_option].count = options[current_option].count / 10;
            if (options[current_option].max_count > 0)
                options[current_option].count =
                    std::min(options[current_option].count,
                             options[current_option].max_count);

            draw_option(current_option, true);
            draw_cost();
        } else if (c == '\r') {
            return options;
        } else if (c == 27) {
            return std::nullopt;
        }
    } while (true);
    return std::nullopt;
}

void MenuSolver::draw_cost() {
    std::string raw = std::format("{:.0f}", calculate_cost());

    int len = static_cast<int>(raw.length());
    for (int i = len - 3; i > 0; i -= 3) {
        raw.insert(i, " ");
    }
    std::string cost_str = "Cost:  " + raw;

    set_relative_pos(1, options.size() + 1);
    int start_offset = (width - 2 - cost_str.size()) / 2;
    engine.print(std::string(start_offset, ' '));

    engine.print(cost_str);

    engine.print(
        std::string(width - 3 - (start_offset + cost_str.size()), ' '));
}

void MenuSolver::draw_current_money() {
    std::string raw = std::format("{:.0f}", current_money);

    int len = static_cast<int>(raw.length());
    for (int i = len - 3; i > 0; i -= 3) {
        raw.insert(i, " ");
    }
    std::string money_str = "Money: " + raw;

    set_relative_pos((width - 2 - money_str.size()) / 2 + 1,
                     options.size() + 2);
    engine.print(money_str);
}

double MenuSolver::calculate_cost() {
    return cost_func(options[0].count, options[1].count, options[2].count);
}