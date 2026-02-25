#pragma once
#include <memory>
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
inline constexpr int water = 4;
inline constexpr int tree_plant = 2;
inline constexpr int flower_plant = 1;

};  // namespace MassCoefs

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

enum class ResourceTypes { Dirt, Stone, Wood, Water, TreePlant, FlowerPlant };
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
        case ResourceTypes::FlowerPlant:
            return "FlowerPlant";
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
        case ResourceTypes::FlowerPlant:
            return WeightCoefs::flower_plant;
        default:
            return 0;
    }
}
inline int get_resourse_weight(ResourceMap resources) {
    int mass = 0;
    for (auto& [res, count]: resources) {
        mass += count * get_resourse_weight(res);
    }
    return mass;
}

class Object {
  public:
    Object(char sprite, Color256 color);
    virtual ~Object() = default;
    virtual bool update();
    char get_sprite();
    Color256 get_color();
    virtual constexpr int get_passability() { return -1; };

    virtual std::vector<PlayerActionTypes> get_available_actions();
    virtual ResourceMap get_resources() { return {}; };

    static bool check_resources(ResourceMap& resources,
                                const ResourceMap& required_resources);
    virtual const ResourceMap& get_required_resources() {
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
    bool update() override;

  private:
};

class Ground : public TerrainObject {
  public:
    static constexpr int passability = PassabilityCoefs::ground;
    Ground();

    constexpr int get_passability() override { return passability; };
    std::vector<PlayerActionTypes> get_available_actions() override;
    std::vector<BuildingTypes> get_available_buildings() override;
    ResourceMap get_resources() override;

  private:
};
class Soil : public TerrainObject {
  public:
    static constexpr int passability = PassabilityCoefs::soil;
    Soil();

    constexpr int get_passability() override { return passability; };
    std::vector<PlayerActionTypes> get_available_actions() override;
    std::vector<BuildingTypes> get_available_buildings() override;
    ResourceMap get_resources() override;

  private:
};

class Grass : public EntityObject {
  public:
    static constexpr int passability = PassabilityCoefs::grass;
    Grass();

    constexpr int get_passability() override { return passability; };
    std::vector<PlayerActionTypes> get_available_actions() override;

  private:
};
class Path : public TerrainObject {
  public:
    static constexpr int passability = PassabilityCoefs::path;
    Path();

    constexpr int get_passability() override { return passability; };
    std::vector<PlayerActionTypes> get_available_actions() override;
    std::vector<BuildingTypes> get_available_buildings() override;

  private:
};
class Bridge : public EntityObject {
  public:
    static constexpr int passability = PassabilityCoefs::bridge;
    Bridge();

    constexpr int get_passability() override { return passability; };
    std::vector<PlayerActionTypes> get_available_actions() override;
    ResourceMap get_resources() override;
    static const ResourceMap& get_required_resources_static();
    static bool check_resources(ResourceMap& resources);
    const ResourceMap& get_required_resources() override;

  private:
    static const ResourceMap required_resources;
};
class Water : public TerrainObject {
  public:
    static constexpr int passability = PassabilityCoefs::water;
    Water();

    constexpr int get_passability() override { return passability; };
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
class House : public EntityObject {
  public:
    House();
    std::vector<PlayerActionTypes> get_available_actions() override;
    ResourceMap get_resources() override;
    static const ResourceMap& get_required_resources_static();
    static bool check_resources(ResourceMap& resources);
    const ResourceMap& get_required_resources() override;

  private:
    static const ResourceMap required_resources;
};

class Dump : public EntityObject {
  public:
    Dump();

    std::vector<PlayerActionTypes> get_available_actions() override;
    ResourceMap get_resources() override;

    ResourceMap resources;

  private:
};

class GrowingObject;

struct GrowthState {
  public:
    GrowthState(int min_growing_time, int max_growing_time, char sprite);
    virtual ~GrowthState() = default;
    virtual bool update(GrowingObject& obj) = 0;
    int get_growing_time();
    char get_sprite();

  protected:
    const int min_growing_time;
    const int max_growing_time;
    const char sprite;
    const int growing_time;
};

class PlantedState : public GrowthState {
  public:
    PlantedState(int min_growing_time, int max_growing_time, char sprite);
    bool update(GrowingObject& obj) override;
};
class GrowingState : public GrowthState {
  public:
    GrowingState(int min_growing_time, int max_growing_time, char sprite);
    bool update(GrowingObject& obj) override;
};
class ReadyState : public GrowthState {
  public:
    ReadyState(int min_growing_time, int max_growing_time, char sprite);
    bool update(GrowingObject& obj) override;
};

using GrowthStatePtr = std::unique_ptr<GrowthState>;

class GrowthStateFactory {
  public:
    virtual ~GrowthStateFactory() = default;
    virtual GrowthStatePtr create_planted() const = 0;
    virtual GrowthStatePtr create_growing() const = 0;
    virtual GrowthStatePtr create_ready() const = 0;
};

class VegetableStateFactory : public GrowthStateFactory {
  public:
    GrowthStatePtr create_planted() const override;
    GrowthStatePtr create_growing() const override;
    GrowthStatePtr create_ready() const override;
};

class FlowerStateFactory : public GrowthStateFactory {
  public:
    GrowthStatePtr create_planted() const override;
    GrowthStatePtr create_growing() const override;
    GrowthStatePtr create_ready() const override;
};

class TreeStateFactory : public GrowthStateFactory {
  public:
    GrowthStatePtr create_planted() const override;
    GrowthStatePtr create_growing() const override;
    GrowthStatePtr create_ready() const override;
};

class GrowingObject : public EntityObject {
  public:
    GrowingObject(Color256 color, GrowthStatePtr state);

    virtual const GrowthStateFactory& get_factory() const = 0;
    void set_new_state(GrowthStatePtr state);
    std::vector<PlayerActionTypes> get_available_actions() override;

    int grow_iteration;

  protected:
    GrowthStatePtr state;
};

class Vegetable : public GrowingObject {
  public:
    static const VegetableStateFactory state_factory;
    Vegetable();
    Vegetable(GrowthStatePtr state);
    bool update() override;
    const GrowthStateFactory& get_factory() const override;

  private:
};
class Flower : public GrowingObject {
  public:
    static const FlowerStateFactory state_factory;
    Flower();
    Flower(GrowthStatePtr state);
    bool update() override;
    const GrowthStateFactory& get_factory() const override;
    ResourceMap get_resources() override;
    static const ResourceMap& get_required_resources_static();
    static bool check_resources(ResourceMap& resources);
    const ResourceMap& get_required_resources() override;

  private:
    static const ResourceMap required_resources;
};
class Tree : public GrowingObject {
  public:
    static const TreeStateFactory state_factory;
    Tree();
    Tree(GrowthStatePtr state);
    bool update() override;
    const GrowthStateFactory& get_factory() const override;
    ResourceMap get_resources() override;
    static const ResourceMap& get_required_resources_static();
    static bool check_resources(ResourceMap& resources);
    const ResourceMap& get_required_resources() override;

  private:
    static const ResourceMap required_resources;
};