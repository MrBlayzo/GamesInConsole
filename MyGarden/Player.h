#pragma once
#include <optional>
#include <deque>
#include "Point.h"
#include "GameObjects.h"

enum class PlayerActionTypes { Move, Dig, Place, Build };

inline std::string action_to_string(PlayerActionTypes action){
    switch (action)
    {
    case PlayerActionTypes::Move:
        return "Move";
    case PlayerActionTypes::Dig:
        return "Dig";
    case PlayerActionTypes::Place:
        return "Place";
    case PlayerActionTypes::Build:
        return "Build";
    default:
        return "";
    }
}

class Map;

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