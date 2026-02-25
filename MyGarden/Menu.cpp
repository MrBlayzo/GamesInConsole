#include "Menu.h"

Menu::Menu(ConsoleEngine& engine, int width, int height, int pos_x, int pos_y)
    : engine(engine),
      width(width),
      pos_x(pos_x),
      pos_y(pos_y),
      height(height),
      current_option(0) {}

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
                std::max(options[current_option].count - 1, 0);
            draw_option(current_option, true);
        } else if (c == '\r') {
            return options;
        } else if (c == 27) {
            return std::nullopt;
        }
    } while (true);
    return std::nullopt;
}

std::optional<std::vector<MenuMassOption>> MenuMass::show_options_menu(
    ConsoleEngine& engine, int width, int heigth, int pos_x, int pos_y,
    std::vector<MenuMassOption> options, bool is_control) {
    return MenuMass(engine, width, heigth, pos_x, pos_y, options, is_control).get_option();
}
MenuMass::MenuMass(ConsoleEngine& engine, int width, int height, int pos_x,
                   int pos_y, std::vector<MenuMassOption> options, bool is_control)
    : Menu(engine, width, height, pos_x, pos_y), options(options), is_control(is_control){
    draw();
    draw_header();
}

void MenuMass::draw_option(int option, bool is_select) {
    std::string option_str =
        options[option].param + " " + std::to_string(options[option].count);
    std::string weight_str =
        std::to_string(get_resourse_weight(options[option].return_param) *
                       options[option].count);

    set_relative_pos(1, option + 3);
    int start_offset =
        (width - 3 - second_col_width - option_str.size()) / 2 - 1;
    engine.print(std::string(start_offset, ' '));

    if (is_select) engine.set_background_color(Colors256::Gray50);
    engine.print(option_str);
    if (is_select) engine.reset_styles();

    int second_offset = width - 3 - second_col_width +
                        (second_col_width - weight_str.size()) / 2 -
                        (start_offset + option_str.size()) - 1;
    engine.print(std::string(second_offset, ' '));

    if (is_select) engine.set_background_color(Colors256::Gray50);
    engine.print(weight_str);
    if (is_select) engine.reset_styles();

    engine.print(std::string(
        second_col_width -
            ((second_col_width - weight_str.size()) / 2 + weight_str.size()),
        ' '));
}

int MenuMass::get_options_size() { return options.size(); }

std::optional<std::vector<MenuMassOption>> MenuMass::get_option() {
    char c;
    do {
        c = engine.get_no_wait();
        if (c == 'w') {
            select_option(current_option - 1);
        } else if (c == 's') {
            select_option(current_option + 1);
        } else if (c == 'r' && is_control) {
            options[current_option].count =
                std::min(options[current_option].count + 1,
                         options[current_option].max_count);
            draw_option(current_option, true);
        } else if (c == 'f' && is_control) {
            options[current_option].count =
                std::max(options[current_option].count - 1, 0);
            draw_option(current_option, true);
        } else if (c == '\r') {
            return options;
        } else if (c == 27) {
            return std::nullopt;
        }
    } while (true);
    return std::nullopt;
}

void MenuMass::draw_header() {
    std::string first_col = "Resource";
    std::string second_col = "Weight";

    set_relative_pos((width - 3 - second_col_width - first_col.size()) / 2 + 1,
                     1);
    engine.print(first_col);

    set_relative_pos(width - 3 - second_col_width +
                         (second_col_width - second_col.size()) / 2,
                     1);
    engine.print(second_col);
    set_relative_pos(1, 2);
    engine.print(std::string(width - 2, '-'));
}