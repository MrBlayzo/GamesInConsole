#pragma once
#include <any>
#include <functional>
#include <optional>
#include <vector>

#include "ConsoleEngine.h"

struct MenuOption {
    std::string param;
    std::any return_param;
};

struct MenuCountOption {
    std::string param;
    std::any return_param;
    double count;
    double max_count;
};

struct MenuSolverOption {
    std::string param;
    double count;
    double max_count;
};

class Menu {
  public:
    static constexpr int default_width = 64;
    static constexpr int default_height = 10;

  protected:
    Menu(ConsoleEngine& engine, int width, int height, int pos_x, int pos_y);

    virtual void draw_option(int option, bool is_select) = 0;
    virtual int get_options_size() = 0;
    void set_relative_pos(int x, int y);
    void select_option(int option);
    void draw();

    ConsoleEngine& engine;
    int width;
    int height;
    int current_option;

  private:
    void draw_frame();
    void draw_options();

    int pos_x;
    int pos_y;
};

class MenuSingle : public Menu {
  public:
    static std::optional<int> show_options_menu(
        ConsoleEngine& engine, int width, int heigth, int pos_x, int pos_y,
        std::vector<MenuOption> options);

  private:
    MenuSingle(ConsoleEngine& engine, int width, int height, int pos_x,
               int pos_y, std::vector<MenuOption> options);
    void draw_option(int option, bool is_select) override;
    int get_options_size() override;
    std::optional<int> get_option();

    std::vector<MenuOption> options;
};

class MenuCount : public Menu {
  public:
    static std::optional<std::vector<MenuCountOption>> show_options_menu(
        ConsoleEngine& engine, int width, int heigth, int pos_x, int pos_y,
        std::vector<MenuCountOption> options);

  private:
    MenuCount(ConsoleEngine& engine, int width, int height, int pos_x,
              int pos_y, std::vector<MenuCountOption> options);
    void draw_option(int option, bool is_select) override;
    int get_options_size() override;
    std::optional<std::vector<MenuCountOption>> get_option();

    std::vector<MenuCountOption> options;
};

class MenuSolver : public Menu {
  public:
    static std::optional<std::vector<MenuSolverOption>> show_options_menu(
        ConsoleEngine& engine, int width, int heigth, int pos_x, int pos_y,
        std::function<double(double, double, double)> cost_func,
        double current_money);

  private:
    MenuSolver(ConsoleEngine& engine, int width, int height, int pos_x,
               int pos_y,
               std::function<double(double, double, double)> cost_func,
               double current_money);
    void draw_option(int option, bool is_select) override;
    void draw_cost();
    void draw_current_money();
    double calculate_cost();
    int get_options_size() override;
    std::optional<std::vector<MenuSolverOption>> get_option();

    std::vector<MenuSolverOption> options;
    std::function<double(double, double, double)> cost_func;
    double current_money;
};