#include "GameObjects.h"

#include "Player.h"

Object::Object(char sprite, Color256 color) : sprite(sprite), color(color) {}

char Object::get_sprite() { return sprite; }
Color256 Object::get_color() { return color; }

bool Object::check_resources(ResourceMap& resources,
                             const ResourceMap& required_resources) {
    for (auto& [res, count] : required_resources) {
        if (!resources.contains(res)) return false;
        if (resources.at(res) < required_resources.at(res)) return false;
    }
    return true;
}
std::vector<PlayerActionTypes> Object::get_available_actions() { return {}; }
TerrainObject::TerrainObject(char sprite, Color256 color)
    : Object(sprite, color) {}
EntityObject::EntityObject(char sprite, Color256 color)
    : Object(sprite, color) {}
std::vector<BuildingTypes> TerrainObject::get_available_buildings() {
    return {};
}

GrowingObject::GrowingObject(Color256 color, GrowthStatePtr state)
    : EntityObject(state->get_sprite(), color),
      state(std::move(state)),
      grow_iteration(0) {}

GrowthState::GrowthState(int min_growing_time, int max_growing_time,
                         int min_time_to_need_watering,
                         int max_time_to_need_watering,
                         int min_time_to_need_fertilizing,
                         int max_time_to_need_fertilizing, char sprite)
    : sprite(sprite),
      min_time_to_need_watering(min_time_to_need_watering),
      max_time_to_need_watering(max_time_to_need_watering),
      min_time_to_need_fertilizing(min_time_to_need_fertilizing),
      max_time_to_need_fertilizing(max_time_to_need_fertilizing),
      growing_time(
          RandomGenerator::randint(min_growing_time, max_growing_time)),
      time_to_need_watering(RandomGenerator::randint(
          min_time_to_need_watering, max_time_to_need_watering)),
      time_to_need_fertilizing{0} {}

int GrowthState::get_growing_time() { return growing_time; }
char GrowthState::get_sprite() { return sprite; }
void GrowthState::watering(GrowingObject& obj) {
    time_to_need_watering = RandomGenerator::randint(min_time_to_need_watering,
                                                     max_time_to_need_watering);
}
void GrowthState::fertilizing() {
    if (time_to_need_fertilizing >= 0) return;
    growing_time -= RandomGenerator::randint(min_time_to_need_fertilizing,
                                             max_time_to_need_fertilizing);
    time_to_need_fertilizing = RandomGenerator::randint(
        min_time_to_need_fertilizing, max_time_to_need_fertilizing);
}
PlantedState::PlantedState(int min_growing_time, int max_growing_time,
                           int min_time_to_need_watering,
                           int max_time_to_need_watering,
                           int min_time_to_need_fertilizing,
                           int max_time_to_need_fertilizing, char sprite)
    : GrowthState(min_growing_time, max_growing_time, min_time_to_need_watering,
                  max_time_to_need_watering, min_time_to_need_fertilizing,
                  max_time_to_need_fertilizing, sprite) {}
GrowingState::GrowingState(int min_growing_time, int max_growing_time,
                           int min_time_to_need_watering,
                           int max_time_to_need_watering,
                           int min_time_to_need_fertilizing,
                           int max_time_to_need_fertilizing, char sprite)
    : GrowthState(min_growing_time, max_growing_time, min_time_to_need_watering,
                  max_time_to_need_watering, min_time_to_need_fertilizing,
                  max_time_to_need_fertilizing, sprite) {}
ReadyState::ReadyState(char sprite) : GrowthState(0, 0, 0, 0, 0, 0, sprite) {}
DryingState::DryingState(int min_time_to_need_watering,
                         int max_time_to_need_watering, char sprite)
    : GrowthState(0, 0, min_time_to_need_watering, max_time_to_need_watering, 0,
                  0, sprite) {}
RottenState::RottenState(char sprite) : GrowthState(0, 0, 0, 0, 0, 0, sprite) {}

bool GrowthState::update(GrowingObject& obj) {
    if (++obj.grow_iteration >= growing_time) {
        new_stage(obj);
        return true;
    }
    if (--time_to_need_watering <= 0) {
        obj.set_new_state(obj.get_factory().create_drying());
        return true;
    }
    time_to_need_fertilizing = std::max(0, time_to_need_fertilizing - 1);
    return false;
}
void PlantedState::new_stage(GrowingObject& obj) {
    obj.set_new_state(obj.get_factory().create_growing());
}
void GrowingState::new_stage(GrowingObject& obj) {
    obj.set_new_state(obj.get_factory().create_ready());
}
bool ReadyState::update(GrowingObject& obj) { return false; }
bool DryingState::update(GrowingObject& obj) {
    if (--time_to_need_watering <= 0) {
        obj.set_new_state(obj.get_factory().create_rotten());
        return true;
    }
    return false;
}
void DryingState::watering(GrowingObject& obj) {
    obj.set_new_state(obj.get_factory().create_growing());
}
bool RottenState::update(GrowingObject& obj) { return false; }

GrowthStatePtr VegetableStateFactory::create_planted() const {
    return std::make_unique<PlantedState>(50, 100, 50, 100, 1, 100, 'c');
}
GrowthStatePtr VegetableStateFactory::create_growing() const {
    return std::make_unique<GrowingState>(50, 100, 50, 100, 1, 100, 'c');
}
GrowthStatePtr VegetableStateFactory::create_ready() const {
    return std::make_unique<ReadyState>('C');
}
GrowthStatePtr VegetableStateFactory::create_drying() const {
    return std::make_unique<DryingState>(50, 100, 'o');
}
GrowthStatePtr VegetableStateFactory::create_rotten() const {
    return std::make_unique<RottenState>('o');
}

GrowthStatePtr FlowerStateFactory::create_planted() const {
    return std::make_unique<PlantedState>(50, 100, 50, 100, 1, 100, 'f');
}
GrowthStatePtr FlowerStateFactory::create_growing() const {
    return std::make_unique<GrowingState>(50, 100, 50, 100, 1, 100, 'f');
}
GrowthStatePtr FlowerStateFactory::create_ready() const {
    return std::make_unique<ReadyState>('F');
}
GrowthStatePtr FlowerStateFactory::create_drying() const {
    return std::make_unique<DryingState>(50, 100, '/');
}
GrowthStatePtr FlowerStateFactory::create_rotten() const {
    return std::make_unique<RottenState>('r');
}

GrowthStatePtr TreeStateFactory::create_planted() const {
    return std::make_unique<PlantedState>(50, 300, 50, 100, 1, 100, 'i');
}
GrowthStatePtr TreeStateFactory::create_growing() const {
    return std::make_unique<GrowingState>(1000, 5000, 50, 100, 1, 100, 't');
}
GrowthStatePtr TreeStateFactory::create_ready() const {
    return std::make_unique<ReadyState>('T');
}
GrowthStatePtr TreeStateFactory::create_drying() const {
    return std::make_unique<DryingState>(50, 100, '!');
}
GrowthStatePtr TreeStateFactory::create_rotten() const {
    return std::make_unique<RottenState>('r');
}

BuildingObject::BuildingObject(Color256 color, BuildStatePtr state)
    : EntityObject(state->get_sprite(), color), state(std::move(state)) {}

void BuildingObject::set_new_state(BuildStatePtr new_state) {
    state = std::move(new_state);
    sprite = state->get_sprite();
}

std::vector<PlayerActionTypes> BuildingObject::get_available_actions() {
    return state->get_available_actions();
}
ResourceMap BuildingObject::get_resources() { return state->get_resources(); }
bool BuildingObject::build(ResourceMap resources) {
    return state->build(*this, resources);
}
const int BuildingObject::get_passability() { return state->get_passability(); }
BuildState::BuildState(char sprite, int passability,
                       ResourceMap required_resources,
                       std::vector<PlayerActionTypes> available_actions)
    : sprite(sprite),
      passability(passability),
      required_resources(required_resources),
      available_actions(available_actions) {}
const ResourceMap BuildState::get_required_resources() {
    ResourceMap current_required_resources;
    for (auto& [res, count] : required_resources) {
        if (invested_resources.contains(res)) {
            current_required_resources[res] =
                count - invested_resources.at(res);
        } else {
            current_required_resources[res] = count;
        }
    }
    return current_required_resources;
}
char BuildState::get_sprite() { return sprite; }
std::vector<PlayerActionTypes> BuildState::get_available_actions() {
    return available_actions;
}
ResourceMap BuildState::get_resources() {
    ResourceMap return_resources;
    for (auto& [res, count] : invested_resources) {
        return_resources[res] = RandomGenerator::randint(0, count);
    }
    return return_resources;
}
bool BuildState::build(BuildingObject& obj, ResourceMap resources) {
    for (auto& [res, count] : resources) {
        invested_resources[res] += count;
    }
    for (auto& [res, count] : required_resources) {
        if (!invested_resources.contains(res)) return false;
        if (invested_resources.at(res) < count) return false;
    }
    new_stage(obj);
    return true;
}
const int BuildState::get_passability() { return passability; }
BuildingState::BuildingState(char sprite, int passability,
                             ResourceMap required_resources,
                             std::vector<PlayerActionTypes> available_actions)
    : BuildState(sprite, passability, required_resources, available_actions) {}
BuildedState::BuildedState(char sprite, int passability,
                           ResourceMap required_resources,
                           std::vector<PlayerActionTypes> available_actions)
    : BuildState(sprite, passability, required_resources, available_actions) {}

void BuildingState::new_stage(BuildingObject& obj) {
    obj.set_new_state(obj.get_factory().create_builded());
}

BuildStatePtr HouseStateFactory::create_building() const {
    return std::make_unique<BuildingState>(
        'h', -1,
        ResourceMap{{ResourceTypes::Wood, 10}, {ResourceTypes::Stone, 10}},
        std::vector<PlayerActionTypes>{
            PlayerActionTypes::Move, PlayerActionTypes::Build,
            PlayerActionTypes::Destroy, PlayerActionTypes::DropResources});
}
BuildStatePtr HouseStateFactory::create_builded() const {
    return std::make_unique<BuildedState>(
        'H', -1, ResourceMap{},
        std::vector<PlayerActionTypes>{PlayerActionTypes::Move,
                                       PlayerActionTypes::Destroy,
                                       PlayerActionTypes::DropResources});
}

BuildStatePtr BridgeStateFactory::create_building() const {
    return std::make_unique<BuildingState>(
        '-', -1, ResourceMap{{ResourceTypes::Wood, 5}},
        std::vector<PlayerActionTypes>{
            PlayerActionTypes::Move, PlayerActionTypes::Build,
            PlayerActionTypes::Destroy, PlayerActionTypes::DropResources});
}
BuildStatePtr BridgeStateFactory::create_builded() const {
    return std::make_unique<BuildedState>(
        '=', PassabilityCoefs::bridge, ResourceMap{},
        std::vector<PlayerActionTypes>{PlayerActionTypes::Move,
                                       PlayerActionTypes::Destroy,
                                       PlayerActionTypes::DropResources});
}

Gardener::Gardener() : Object('@', Colors256::Yellow) {}
Ground::Ground() : TerrainObject('.', Colors256::GrayBrown) {}
Soil::Soil() : TerrainObject('#', Colors256::LightBrown) {}
Grass::Grass() : EntityObject('"', Colors256::DarkGreen) {}
Path::Path() : TerrainObject(':', Color256(130)) {}
Water::Water() : TerrainObject('~', Colors256::Blue) {}
Rock::Rock() : TerrainObject('^', Color256(242)) {}
Dump::Dump() : EntityObject('%', Color256(230)) {}
Bridge::Bridge()
    : BuildingObject(Colors256::OrangeBrown, get_factory().create_building()) {}
House::House()
    : BuildingObject(Colors256::OrangeBrown, get_factory().create_building()) {}

Vegetable::Vegetable()
    : GrowingObject(Colors256::Red, get_factory().create_planted()) {}
Vegetable::Vegetable(GrowthStatePtr state)
    : GrowingObject(Colors256::Red, std::move(state)) {}
Flower::Flower()
    : GrowingObject(Colors256::Purple, get_factory().create_planted()) {}
Flower::Flower(GrowthStatePtr state)
    : GrowingObject(Colors256::Purple, std::move(state)) {}
Tree::Tree() : GrowingObject(Color256(28), get_factory().create_planted()) {}
Tree::Tree(GrowthStatePtr state)
    : GrowingObject(Color256(28), std::move(state)) {}

bool Object::update() { return false; }
bool Gardener::update() { return false; }

void GrowingObject::set_new_state(GrowthStatePtr new_state) {
    state = std::move(new_state);
    grow_iteration = 0;
    sprite = state->get_sprite();
}
bool GrowingObject::update() { return state->update(*this); }

void GrowingObject::watering() { state->watering(*this); }
void GrowingObject::fertilizing() { state->fertilizing(); }
const HouseStateFactory House::state_factory{};
const BridgeStateFactory Bridge::state_factory{};
const VegetableStateFactory Vegetable::state_factory{};
const FlowerStateFactory Flower::state_factory{};
const TreeStateFactory Tree::state_factory{};

const BuildStateFactory& House::get_factory() const { return state_factory; }
const BuildStateFactory& Bridge::get_factory() const { return state_factory; }
const GrowthStateFactory& Vegetable::get_factory() const {
    return state_factory;
}
const GrowthStateFactory& Flower::get_factory() const { return state_factory; }
const GrowthStateFactory& Tree::get_factory() const { return state_factory; }

std::vector<PlayerActionTypes> Ground::get_available_actions() {
    return {
        PlayerActionTypes::Move,          PlayerActionTypes::Place,
        PlayerActionTypes::StartBuild,    PlayerActionTypes::ExtractResources,
        PlayerActionTypes::DumpResources, PlayerActionTypes::DropResources};
}
std::vector<PlayerActionTypes> Soil::get_available_actions() {
    return {
        PlayerActionTypes::Move,          PlayerActionTypes::Place,
        PlayerActionTypes::StartBuild,    PlayerActionTypes::ExtractResources,
        PlayerActionTypes::DumpResources, PlayerActionTypes::DropResources};
}
std::vector<PlayerActionTypes> Grass::get_available_actions() {
    return {PlayerActionTypes::Move, PlayerActionTypes::Dig,
            PlayerActionTypes::DropResources};
}
std::vector<PlayerActionTypes> Path::get_available_actions() {
    return {
        PlayerActionTypes::Move,          PlayerActionTypes::Place,
        PlayerActionTypes::StartBuild,    PlayerActionTypes::ExtractResources,
        PlayerActionTypes::DumpResources, PlayerActionTypes::DropResources};
}
std::vector<PlayerActionTypes> Water::get_available_actions() {
    return {PlayerActionTypes::Move, PlayerActionTypes::StartBuild,
            PlayerActionTypes::ExtractResources,
            PlayerActionTypes::DropResources};
}
std::vector<PlayerActionTypes> Rock::get_available_actions() {
    return {PlayerActionTypes::ExtractResources,
            PlayerActionTypes::DropResources};
}
std::vector<PlayerActionTypes> Dump::get_available_actions() {
    return {PlayerActionTypes::DumpResources,
            PlayerActionTypes::GetResourcesFromDump,
            PlayerActionTypes::DropResources};
}
std::vector<PlayerActionTypes> GrowingObject::get_available_actions() {
    return {PlayerActionTypes::Move, PlayerActionTypes::Dig,
            PlayerActionTypes::Watering, PlayerActionTypes::Fertilizing,
            PlayerActionTypes::DropResources};
}

std::vector<BuildingTypes> Ground::get_available_buildings() {
    return {BuildingTypes::House};
}
std::vector<BuildingTypes> Soil::get_available_buildings() {
    return {BuildingTypes::House};
}
std::vector<BuildingTypes> Path::get_available_buildings() {
    return {BuildingTypes::House};
}
std::vector<BuildingTypes> Water::get_available_buildings() {
    return {BuildingTypes::Bridge};
}

ResourceMap Ground::get_resources() {
    return {{ResourceTypes::Dirt, RandomGenerator::randint(2, 6)}};
}
ResourceMap Soil::get_resources() {
    return {{ResourceTypes::Dirt, RandomGenerator::randint(5, 10)}};
}
ResourceMap Water::get_resources() { return {{ResourceTypes::Water, 1}}; }
ResourceMap Rock::get_resources() {
    return {{ResourceTypes::Stone, RandomGenerator::randint(5, 10)}};
}
ResourceMap Flower::get_resources() {
    // TODO: передать ответственность состояниям
    // TODO: от сгнивших растений удобрения, от нормальных - другие
    if (dynamic_cast<GrowingState*>(state.get())) {
        return {{ResourceTypes::FlowerPlant, RandomGenerator::randint(1, 2)}};
    } else if (dynamic_cast<ReadyState*>(state.get())) {
        return {{ResourceTypes::FlowerPlant, RandomGenerator::randint(2, 4)}};
    }

    return {{ResourceTypes::FlowerPlant, 1}};
}
ResourceMap Tree::get_resources() {
    // TODO: передать ответственность состояниям
    if (dynamic_cast<GrowingState*>(state.get())) {
        return {{ResourceTypes::TreePlant, RandomGenerator::randint(1, 3)}};
    } else if (dynamic_cast<ReadyState*>(state.get())) {
        return {{ResourceTypes::TreePlant, RandomGenerator::randint(1, 3)},
                {ResourceTypes::Wood, RandomGenerator::randint(1, 3)}};
    }

    return {{ResourceTypes::TreePlant, 1}};
}
ResourceMap Dump::get_resources() { return {}; }

const ResourceMap Bridge::required_resources{{ResourceTypes::Wood, 1}};
const ResourceMap House::required_resources{{ResourceTypes::Wood, 1},
                                            {ResourceTypes::Stone, 1}};
const ResourceMap Flower::required_resources{{ResourceTypes::FlowerPlant, 1}};
const ResourceMap Tree::required_resources{{ResourceTypes::TreePlant, 1}};

const ResourceMap& Bridge::get_required_resources_static() {
    return required_resources;
}
const ResourceMap& House::get_required_resources_static() {
    return required_resources;
}
const ResourceMap& Flower::get_required_resources_static() {
    return required_resources;
}
const ResourceMap& Tree::get_required_resources_static() {
    return required_resources;
}

bool Bridge::check_resources(ResourceMap& resources) {
    return Object::check_resources(resources, get_required_resources_static());
}
bool House::check_resources(ResourceMap& resources) {
    return Object::check_resources(resources, get_required_resources_static());
}
bool Flower::check_resources(ResourceMap& resources) {
    return Object::check_resources(resources, get_required_resources_static());
}
bool Tree::check_resources(ResourceMap& resources) {
    return Object::check_resources(resources, get_required_resources_static());
}

const ResourceMap BuildingObject::get_required_resources() {
    return state->get_required_resources();
}
const ResourceMap House::get_start_build_resources() {
    return get_required_resources_static();
}
const ResourceMap Bridge::get_start_build_resources() {
    return get_required_resources_static();
}
const ResourceMap Flower::get_required_resources() {
    return get_required_resources_static();
}
const ResourceMap Tree::get_required_resources() {
    return get_required_resources_static();
}