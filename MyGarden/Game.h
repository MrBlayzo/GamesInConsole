#pragma once
#include <array>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <queue>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ConsoleEngine.h"
#include "GameObjects.h"
#include "RandomGenerator.h"
#include "Point.h"
#include "PathFinder.h"

class Cell {
  public:
    std::unique_ptr<TerrainObject> terrain;
    std::unique_ptr<Object> entity;
    bool is_selected;
    bool is_on_path;

    Cell(int x, int y, std::unique_ptr<TerrainObject> terrain);
    void draw(ConsoleEngine& engine);

  private:
    int pos_x;
    int pos_y;
};

enum class PlayerActionTypes { Move, Dig, Place, Build };

class Map;
class PathFinder;

class PlayerAction {
  public:
    PlayerAction(Map& map, Point pos);
    virtual ~PlayerAction() = default;
    void execute();
    virtual void finish() = 0;
    bool executed();

    virtual constexpr int get_execution_time() = 0;
    Point pos;

  protected:
    Map& map;

  private:
    int execute_iteration = 0;
    bool is_executed = false;
};

class DigAction : public PlayerAction {
  public:
    static constexpr int execution_time = 10;
    DigAction(Map& map, Point pos);
    void finish() override;
    constexpr int get_execution_time() override { return execution_time; };
};

class PlaceAction : public PlayerAction {
  public:
    static constexpr int execution_time = 10;
    PlaceAction(Map& map, Point pos, std::unique_ptr<GrowingObject> new_object);
    void finish() override;
    constexpr int get_execution_time() override { return execution_time; };

    std::unique_ptr<GrowingObject> new_object;
};

class BuildAction : public PlayerAction {
  public:
    static constexpr int execution_time = 10;
    BuildAction(Map& map, Point pos, std::unique_ptr<TerrainObject> new_object);
    void finish() override;
    constexpr int get_execution_time() override { return execution_time; };

    std::unique_ptr<TerrainObject> new_object;
};

class Player {
  public:
    Player(Map& map);
    Point pos;
    Point cursor_pos;
    std::optional<std::deque<Point>> active_path;
    std::unique_ptr<PlayerAction> active_action;
    int walk_iteration;

    bool update();
    void create_path();
    void create_path_to_area();
    void new_action();

  private:
    Map& map;
};

class Map {
  public:
    Map(int width, int height);
    Cell& get(int x, int y);
    void update();
    double get_passability(int x, int y);
    double get_passability(Point p);

    void set_new_terrain(int x, int y, std::unique_ptr<TerrainObject> terrain);
    void set_new_entity(int x, int y, std::unique_ptr<Object> entity);
    void reset_entity(int x, int y);

    void clear_path();
    void draw_path();
    void redraw(int x, int y);
    void redraw_all();

    int width;
    int height;

    ConsoleEngine engine;

  private:
    std::vector<std::vector<Cell>> map;
    Player player;

    void generate();
    void generate_lakes();
    void generate_rivers();
    void generate_rocks();
    void generate_objects();
    void add_gardener();

    void get_player_control();
    void player_move();
};

class Menu {
  public:
    static int show_options_menu(ConsoleEngine& engine, int width, int heigth,
                                 int pos_x, int pos_y,
                                 std::vector<std::string> options);

  private:
    Menu(ConsoleEngine& engine, int width, int height, int pos_x, int pos_y,
         std::vector<std::string> options);
    void draw();
    int get_option();
    void select_option(int option);
    void set_relative_pos(int x, int y);
    ConsoleEngine& engine;
    int width;
    int height;
    int pos_x;
    int pos_y;
    std::vector<std::string> options;
    int current_option;
};

class MyGarden {
  public:
    MyGarden(int width = 100, int height = 20);
    void play();

  private:
    Map map;
};