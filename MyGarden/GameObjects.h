#pragma once
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "ConsoleEngine.h"
#include "RandomGenerator.h"

namespace PassabilityCoefs {
inline constexpr int ground = 2;
inline constexpr int soil = 4;
inline constexpr int grass = 8;
inline constexpr int path = 1;
inline constexpr int water = 16;
inline constexpr int rock = -1;
inline constexpr int bridge = 1;

};  // namespace PassabilityCoefs

namespace WeightCoefs {
inline constexpr int dirt = 2;
inline constexpr int stone = 5;
inline constexpr int wood = 4;
inline constexpr int water = 1;
inline constexpr int tree_plant = 2;
inline constexpr int seed = 0;

};  // namespace WeightCoefs

enum class PlayerActionTypes;
enum class BuildingTypes { House, Bridge };
inline std::string building_type_to_string(BuildingTypes building) {
    switch (building) {
        case BuildingTypes::House:
            return "House";
        case BuildingTypes::Bridge:
            return "Bridge";
        default:
            return "";
    }
}

enum class ResourceTypes {
    Dirt,
    Stone,
    Wood,
    Water,
    TreePlant,
    FlowerSeed,
    PotatoSeed,
    CarrotSeed,
    CucumberSeed,
    TomatoSeed,
    Fertilizer
};
using ResourceMap = std::unordered_map<ResourceTypes, int>;
inline std::string resource_type_to_string(ResourceTypes resource) {
    switch (resource) {
        case ResourceTypes::Dirt:
            return "Dirt";
        case ResourceTypes::Stone:
            return "Stone";
        case ResourceTypes::Wood:
            return "Wood";
        case ResourceTypes::Water:
            return "Water";
        case ResourceTypes::TreePlant:
            return "TreePlant";
        case ResourceTypes::FlowerSeed:
            return "FlowerSeed";
        case ResourceTypes::PotatoSeed:
            return "PotatoSeed";
        case ResourceTypes::CarrotSeed:
            return "CarrotSeed";
        case ResourceTypes::CucumberSeed:
            return "CucumberSeed";
        case ResourceTypes::TomatoSeed:
            return "TomatoSeed";
        case ResourceTypes::Fertilizer:
            return "Fertilizer";
        default:
            return "";
    }
}
inline int get_resourse_weight(ResourceTypes resource) {
    switch (resource) {
        case ResourceTypes::Dirt:
            return WeightCoefs::dirt;
        case ResourceTypes::Stone:
            return WeightCoefs::stone;
        case ResourceTypes::Wood:
            return WeightCoefs::wood;
        case ResourceTypes::Water:
            return WeightCoefs::water;
        case ResourceTypes::TreePlant:
            return WeightCoefs::tree_plant;
        case ResourceTypes::FlowerSeed:
            return WeightCoefs::seed;
        default:
            return 0;
    }
}
inline int get_resourse_weight(ResourceMap resources) {
    int mass = 0;
    for (auto& [res, count] : resources) {
        mass += count * get_resourse_weight(res);
    }
    return mass;
}

struct UpdateResult {
    bool redraw = false;
    bool seed = false;
    bool death = false;
};

class Object {
  public:
    Object(char sprite, Color256 color);
    virtual ~Object() = default;
    virtual UpdateResult update();
    char get_sprite();
    Color256 get_color();
    virtual const int get_passability() { return -1; };

    virtual std::vector<PlayerActionTypes> get_available_actions();
    virtual ResourceMap get_resources() { return {}; };

    static bool check_resources(ResourceMap& resources,
                                const ResourceMap& required_resources);
    virtual const ResourceMap get_required_resources() {
        static const ResourceMap empty;
        return empty;
    };

  protected:
    char sprite;
    Color256 color;

  private:
};

class TerrainObject : public Object {
  public:
    TerrainObject(char sprite, Color256 color);
    virtual std::vector<BuildingTypes> get_available_buildings();
};

class EntityObject : public Object {
  public:
    EntityObject(char sprite, Color256 color);
};

class Gardener : public Object {
  public:
    Gardener();
    UpdateResult update() override;

  private:
};

class Ground : public TerrainObject {
  public:
    static constexpr int passability = PassabilityCoefs::ground;
    Ground();

    const int get_passability() override { return passability; };
    std::vector<PlayerActionTypes> get_available_actions() override;
    std::vector<BuildingTypes> get_available_buildings() override;
    ResourceMap get_resources() override;

  private:
};
class Soil : public TerrainObject {
  public:
    static constexpr int passability = PassabilityCoefs::soil;
    Soil();

    const int get_passability() override { return passability; };
    std::vector<PlayerActionTypes> get_available_actions() override;
    std::vector<BuildingTypes> get_available_buildings() override;
    ResourceMap get_resources() override;

  private:
};

class Grass : public EntityObject {
  public:
    static constexpr int passability = PassabilityCoefs::grass;
    Grass();

    const int get_passability() override { return passability; };
    std::vector<PlayerActionTypes> get_available_actions() override;

  private:
};
class Path : public TerrainObject {
  public:
    static constexpr int passability = PassabilityCoefs::path;
    Path();

    const int get_passability() override { return passability; };
    std::vector<PlayerActionTypes> get_available_actions() override;
    std::vector<BuildingTypes> get_available_buildings() override;

  private:
};
class Water : public TerrainObject {
  public:
    static constexpr int passability = PassabilityCoefs::water;
    Water();

    const int get_passability() override { return passability; };
    std::vector<PlayerActionTypes> get_available_actions() override;
    std::vector<BuildingTypes> get_available_buildings() override;
    ResourceMap get_resources() override;

  private:
};
class Rock : public TerrainObject {
  public:
    Rock();

    std::vector<PlayerActionTypes> get_available_actions() override;
    ResourceMap get_resources() override;

  private:
    ResourceMap resources;
};

class Dump : public EntityObject {
  public:
    Dump();

    std::vector<PlayerActionTypes> get_available_actions() override;
    ResourceMap get_resources() override;

    ResourceMap resources;

  private:
};

class BuildingObject;

struct BuildState {
  public:
    BuildState(char sprite, int passability, ResourceMap required_resources,
               std::vector<PlayerActionTypes> available_actions);
    virtual ~BuildState() = default;
    const ResourceMap get_required_resources();
    char get_sprite();
    const int get_passability();
    std::vector<PlayerActionTypes> get_available_actions();
    ResourceMap get_resources();
    bool build(BuildingObject& obj, ResourceMap resources);
    virtual void new_stage(BuildingObject& obj) { return; };

  protected:
    const char sprite;
    const ResourceMap required_resources;
    ResourceMap invested_resources{};
    const int passability;
    const std::vector<PlayerActionTypes> available_actions;
};

class BuildingState : public BuildState {
  public:
    BuildingState(char sprite, int passability, ResourceMap required_resources,
                  std::vector<PlayerActionTypes> available_actions);
    void new_stage(BuildingObject& obj) override;
};
class BuildedState : public BuildState {
  public:
    BuildedState(char sprite, int passability, ResourceMap required_resources,
                 std::vector<PlayerActionTypes> available_actions);
};

using BuildStatePtr = std::unique_ptr<BuildState>;

class BuildStateFactory {
  public:
    virtual ~BuildStateFactory() = default;
    virtual BuildStatePtr create_building() const = 0;
    virtual BuildStatePtr create_builded() const = 0;
};

class HouseStateFactory : public BuildStateFactory {
  public:
    BuildStatePtr create_building() const override;
    BuildStatePtr create_builded() const override;
};

class BridgeStateFactory : public BuildStateFactory {
  public:
    BuildStatePtr create_building() const override;
    BuildStatePtr create_builded() const override;
};

class BuildingObject : public EntityObject {
  public:
    BuildingObject(Color256 color, BuildStatePtr state);

    virtual const BuildStateFactory& get_factory() const = 0;
    void set_new_state(BuildStatePtr new_state);
    std::vector<PlayerActionTypes> get_available_actions() override;
    bool build(ResourceMap resources);
    const int get_passability() override;
    ResourceMap get_resources() override;
    const ResourceMap get_required_resources() override;
    virtual const ResourceMap get_start_build_resources() = 0;

  protected:
    BuildStatePtr state;
};

class House : public BuildingObject {
  public:
    static const HouseStateFactory state_factory;
    House();
    static const ResourceMap& get_required_resources_static();
    static bool check_resources(ResourceMap& resources);
    const BuildStateFactory& get_factory() const override;
    const ResourceMap get_start_build_resources() override;

  private:
    static const ResourceMap required_resources;
};

class Bridge : public BuildingObject {
  public:
    static const BridgeStateFactory state_factory;
    Bridge();

    static const ResourceMap& get_required_resources_static();
    static bool check_resources(ResourceMap& resources);
    const BuildStateFactory& get_factory() const override;
    const ResourceMap get_start_build_resources() override;

  private:
    static const ResourceMap required_resources;
};

class GrowingObject;

enum class GrowthStateType { Planted, Growing, Ready, Drying, Rotten };

class GrowthState {
  public:
    GrowthState(int min_growing_time, int max_growing_time,
                int min_time_to_need_watering, int max_time_to_need_watering,
                int min_time_to_need_fertilizing,
                int max_time_to_need_fertilizing, char sprite);
    virtual ~GrowthState() = default;
    virtual UpdateResult update(GrowingObject& obj);
    int get_growing_time();
    char get_sprite();
    virtual void new_stage(GrowingObject& obj) { return; };
    virtual void watering(GrowingObject& obj);
    void fertilizing();
    virtual GrowthStateType get_type() = 0;

  protected:
    const char sprite;
    int growing_time;
    const int min_time_to_need_watering, max_time_to_need_watering;
    const int min_time_to_need_fertilizing, max_time_to_need_fertilizing;
    int time_to_need_watering;
    int time_to_need_fertilizing;
};

using GrowthStatePtr = std::unique_ptr<GrowthState>;

class PlantedState : public GrowthState {
  public:
    PlantedState(int min_growing_time, int max_growing_time,
                 int min_time_to_need_watering, int max_time_to_need_watering,
                 int min_time_to_need_fertilizing,
                 int max_time_to_need_fertilizing, char sprite);
    void new_stage(GrowingObject& obj) override;
    GrowthStateType get_type() override { return GrowthStateType::Planted; }
};
class GrowingState : public GrowthState {
  public:
    GrowingState(int min_growing_time, int max_growing_time,
                 int min_time_to_need_watering, int max_time_to_need_watering,
                 int min_time_to_need_fertilizing,
                 int max_time_to_need_fertilizing, char sprite);
    void new_stage(GrowingObject& obj) override;
    GrowthStateType get_type() override { return GrowthStateType::Growing; }
};
class ReadyState : public GrowthState {
  public:
    ReadyState(char sprite);
    UpdateResult update(GrowingObject& obj) override;
    GrowthStateType get_type() override { return GrowthStateType::Ready; }
};

class DryingState : public GrowthState {
  public:
    struct PrevState {
        GrowthStateType type = GrowthStateType::Planted;
        int grow_iteration = 0;
    };

    DryingState(int min_time_to_need_watering, int max_time_to_need_watering,
                char sprite, DryingState::PrevState prev_state);
    UpdateResult update(GrowingObject& obj) override;
    GrowthStateType get_type() override { return GrowthStateType::Drying; }
    void watering(GrowingObject& obj) override;

    PrevState prev_state;
};

class RottenState : public GrowthState {
  public:
    RottenState(char sprite);
    UpdateResult update(GrowingObject& obj) override;
    GrowthStateType get_type() override { return GrowthStateType::Rotten; }
};

class GrowthStateFactory {
  public:
    virtual ~GrowthStateFactory() = default;
    virtual GrowthStatePtr create_planted() const = 0;
    virtual GrowthStatePtr create_growing() const = 0;
    virtual GrowthStatePtr create_ready() const = 0;
    virtual GrowthStatePtr create_drying(
        DryingState::PrevState prev_state) const = 0;
    virtual GrowthStatePtr create_rotten() const = 0;
};

class PotatoStateFactory : public GrowthStateFactory {
  public:
    GrowthStatePtr create_planted() const override;
    GrowthStatePtr create_growing() const override;
    GrowthStatePtr create_ready() const override;
    GrowthStatePtr create_drying(
        DryingState::PrevState prev_state) const override;
    GrowthStatePtr create_rotten() const override;
};
class CarrotStateFactory : public GrowthStateFactory {
  public:
    GrowthStatePtr create_planted() const override;
    GrowthStatePtr create_growing() const override;
    GrowthStatePtr create_ready() const override;
    GrowthStatePtr create_drying(
        DryingState::PrevState prev_state) const override;
    GrowthStatePtr create_rotten() const override;
};
class CucumberStateFactory : public GrowthStateFactory {
  public:
    GrowthStatePtr create_planted() const override;
    GrowthStatePtr create_growing() const override;
    GrowthStatePtr create_ready() const override;
    GrowthStatePtr create_drying(
        DryingState::PrevState prev_state) const override;
    GrowthStatePtr create_rotten() const override;
};
class TomatoStateFactory : public GrowthStateFactory {
  public:
    GrowthStatePtr create_planted() const override;
    GrowthStatePtr create_growing() const override;
    GrowthStatePtr create_ready() const override;
    GrowthStatePtr create_drying(
        DryingState::PrevState prev_state) const override;
    GrowthStatePtr create_rotten() const override;
};

class FlowerStateFactory : public GrowthStateFactory {
  public:
    GrowthStatePtr create_planted() const override;
    GrowthStatePtr create_growing() const override;
    GrowthStatePtr create_ready() const override;
    GrowthStatePtr create_drying(
        DryingState::PrevState prev_state) const override;
    GrowthStatePtr create_rotten() const override;
};

class TreeStateFactory : public GrowthStateFactory {
  public:
    GrowthStatePtr create_planted() const override;
    GrowthStatePtr create_growing() const override;
    GrowthStatePtr create_ready() const override;
    GrowthStatePtr create_drying(
        DryingState::PrevState prev_state) const override;
    GrowthStatePtr create_rotten() const override;
};

class GrowingObject : public EntityObject {
  public:
    GrowingObject(Color256 color, GrowthStatePtr state);

    virtual const GrowthStateFactory& get_factory() const = 0;
    void set_new_state(GrowthStatePtr state);
    std::vector<PlayerActionTypes> get_available_actions() override;
    UpdateResult update() override;
    void watering();
    void fertilizing();
    virtual std::unique_ptr<GrowingObject> create_seed() = 0;

    int grow_iteration;

  protected:
    GrowthStatePtr state;
};

class Potato : public GrowingObject {
  public:
    static const PotatoStateFactory state_factory;
    Potato();
    Potato(GrowthStatePtr state);
    const GrowthStateFactory& get_factory() const override;
    ResourceMap get_resources() override;
    static const ResourceMap& get_required_resources_static();
    static bool check_resources(ResourceMap& resources);
    const ResourceMap get_required_resources() override;
    std::unique_ptr<GrowingObject> create_seed() override;

  private:
    static const ResourceMap required_resources;
};
class Carrot : public GrowingObject {
  public:
    static const CarrotStateFactory state_factory;
    Carrot();
    Carrot(GrowthStatePtr state);
    const GrowthStateFactory& get_factory() const override;
    ResourceMap get_resources() override;
    static const ResourceMap& get_required_resources_static();
    static bool check_resources(ResourceMap& resources);
    const ResourceMap get_required_resources() override;
    std::unique_ptr<GrowingObject> create_seed() override;

  private:
    static const ResourceMap required_resources;
};
class Cucumber : public GrowingObject {
  public:
    static const CucumberStateFactory state_factory;
    Cucumber();
    Cucumber(GrowthStatePtr state);
    const GrowthStateFactory& get_factory() const override;
    ResourceMap get_resources() override;
    static const ResourceMap& get_required_resources_static();
    static bool check_resources(ResourceMap& resources);
    const ResourceMap get_required_resources() override;
    std::unique_ptr<GrowingObject> create_seed() override;

  private:
    static const ResourceMap required_resources;
};
class Tomato : public GrowingObject {
  public:
    static const TomatoStateFactory state_factory;
    Tomato();
    Tomato(GrowthStatePtr state);
    const GrowthStateFactory& get_factory() const override;
    ResourceMap get_resources() override;
    static const ResourceMap& get_required_resources_static();
    static bool check_resources(ResourceMap& resources);
    const ResourceMap get_required_resources() override;
    std::unique_ptr<GrowingObject> create_seed() override;

  private:
    static const ResourceMap required_resources;
};
class Flower : public GrowingObject {
  public:
    static const FlowerStateFactory state_factory;
    Flower();
    Flower(GrowthStatePtr state);
    const GrowthStateFactory& get_factory() const override;
    ResourceMap get_resources() override;
    static const ResourceMap& get_required_resources_static();
    static bool check_resources(ResourceMap& resources);
    const ResourceMap get_required_resources() override;
    std::unique_ptr<GrowingObject> create_seed() override;

  private:
    static const ResourceMap required_resources;
};
class Tree : public GrowingObject {
  public:
    static const TreeStateFactory state_factory;
    Tree();
    Tree(GrowthStatePtr state);
    const GrowthStateFactory& get_factory() const override;
    ResourceMap get_resources() override;
    static const ResourceMap& get_required_resources_static();
    static bool check_resources(ResourceMap& resources);
    const ResourceMap get_required_resources() override;
    std::unique_ptr<GrowingObject> create_seed() override;

  private:
    static const ResourceMap required_resources;
};