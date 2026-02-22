#pragma once
#include <deque>
#include <optional>
#include <unordered_map>

#include "GameObjects.h"
#include "Point.h"

enum class PlayerActionTypes {
    Move,
    Dig,
    Place,
    Build,
    Destroy,
    ExtractResources,
    DumpResources,
    GetResourcesFromDump
};

inline std::string action_to_string(PlayerActionTypes action) {
    switch (action) {
        case PlayerActionTypes::Move:
            return "Move";
        case PlayerActionTypes::Dig:
            return "Dig";
        case PlayerActionTypes::Place:
            return "Place";
        case PlayerActionTypes::Build:
            return "Build";
        case PlayerActionTypes::Destroy:
            return "Destroy";
        case PlayerActionTypes::ExtractResources:
            return "Extract resources";
        case PlayerActionTypes::DumpResources:
            return "Dump resources";
        case PlayerActionTypes::GetResourcesFromDump:
            return "Get resources from dump";
        default:
            return "";
    }
}

class Map;
class Player;

class PlayerAction {
  public:
    PlayerAction(Map& map, Player& player, Point pos);
    virtual ~PlayerAction() = default;
    void execute();
    virtual void finish() = 0;
    bool executed();

    virtual constexpr int get_execution_time() = 0;
    Point pos;

  protected:
    Map& map;
    Player& player;

  private:
    int execute_iteration = 0;
    bool is_executed = false;
};

class DigAction : public PlayerAction {
  public:
    static constexpr int execution_time = 10;
    DigAction(Map& map, Player& player, Point pos);
    void finish() override;
    constexpr int get_execution_time() override { return execution_time; };
};

class PlaceAction : public PlayerAction {
  public:
    static constexpr int execution_time = 10;
    PlaceAction(Map& map, Player& player, Point pos,
                std::unique_ptr<GrowingObject> new_object);
    void finish() override;
    constexpr int get_execution_time() override { return execution_time; };

    std::unique_ptr<GrowingObject> new_object;
};

class BuildAction : public PlayerAction {
  public:
    static constexpr int execution_time = 10;
    BuildAction(Map& map, Player& player, Point pos,
                std::unique_ptr<EntityObject> new_object);
    void finish() override;
    constexpr int get_execution_time() override { return execution_time; };

    std::unique_ptr<EntityObject> new_object;
};

class DestroyAction : public PlayerAction {
  public:
    static constexpr int execution_time = 20;
    DestroyAction(Map& map, Player& player, Point pos);
    void finish() override;
    constexpr int get_execution_time() override { return execution_time; };
};

class ExtractResourcesAction : public PlayerAction {
  public:
    static constexpr int execution_time = 20;
    ExtractResourcesAction(Map& map, Player& player, Point pos);
    void finish() override;
    constexpr int get_execution_time() override { return execution_time; };
};

class DumpResourcesAction : public PlayerAction {
  public:
    static constexpr int execution_time = 5;
    DumpResourcesAction(Map& map, Player& player, Point pos, ResourceMap resources);
    void finish() override;
    constexpr int get_execution_time() override { return execution_time; };

    ResourceMap resources;
};

class GetResourcesFromDumpAction : public PlayerAction {
  public:
    static constexpr int execution_time = 5;
    GetResourcesFromDumpAction(Map& map, Player& player, Point pos, ResourceMap resources);
    void finish() override;
    constexpr int get_execution_time() override { return execution_time; };

    ResourceMap resources;
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
    void see_resouces();
    bool check_resources(PlayerActionTypes action);

    ResourceMap resources{
      {ResourceTypes::FlowerPlant, 2}
    };

  private:
    Map& map;

    void new_build_action();
    void new_place_action();
    void new_dump_resources_action();
    void new_get_resources_from_dump_action();
};