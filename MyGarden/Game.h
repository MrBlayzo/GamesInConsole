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
#include "Player.h"
#include "Point.h"
#include "RandomGenerator.h"

class Cell {
  public:
    std::unique_ptr<TerrainObject> terrain;
    std::unique_ptr<EntityObject> entity;
    std::unique_ptr<Gardener> gardener;
    bool is_selected;
    bool is_on_path;

    Cell(int x, int y, std::unique_ptr<TerrainObject> terrain);
    void draw(ConsoleEngine& engine);

  private:
    int pos_x;
    int pos_y;
};

class Map {
  public:
    Map(int width, int height);
    Cell& get(int x, int y);
    void update();
    double get_passability(int x, int y);
    double get_passability(Point p);

    void set_new_terrain(int x, int y, std::unique_ptr<TerrainObject> terrain);
    void set_new_entity(int x, int y, std::unique_ptr<EntityObject> entity);
    void reset_entity(int x, int y);

    void clear_path();
    void draw_path();
    void redraw(int x, int y);
    void redraw_all();

    std::vector<PlayerActionTypes> get_available_action(int x, int y);
    std::vector<BuildingTypes> get_available_buildings(int x, int y);

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

class MyGarden {
  public:
    MyGarden(int width = 100, int height = 20);
    void play();

  private:
    Map map;
};