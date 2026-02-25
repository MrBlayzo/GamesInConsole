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
                         char sprite)
    : min_growing_time(min_growing_time),
      max_growing_time(max_growing_time),
      sprite(sprite),
      growing_time(
          RandomGenerator::randint(min_growing_time, max_growing_time)) {}
int GrowthState::get_growing_time() { return growing_time; }
char GrowthState::get_sprite() { return sprite; }

PlantedState::PlantedState(int min_growing_time, int max_growing_time,
                           char sprite)
    : GrowthState(min_growing_time, max_growing_time, sprite) {}
GrowingState::GrowingState(int min_growing_time, int max_growing_time,
                           char sprite)
    : GrowthState(min_growing_time, max_growing_time, sprite) {}
ReadyState::ReadyState(int min_growing_time, int max_growing_time, char sprite)
    : GrowthState(min_growing_time, max_growing_time, sprite) {}

bool PlantedState::update(GrowingObject& obj) {
    if (++obj.grow_iteration >= growing_time) {
        obj.set_new_state(obj.get_factory().create_growing());
        return true;
    }
    return false;
}
bool GrowingState::update(GrowingObject& obj) {
    if (++obj.grow_iteration >= growing_time) {
        obj.set_new_state(obj.get_factory().create_ready());
        return true;
    }
    return false;
}
bool ReadyState::update(GrowingObject& obj) { return false; }

GrowthStatePtr VegetableStateFactory::create_planted() const {
    return std::make_unique<PlantedState>(50, 100, 'c');
}
GrowthStatePtr VegetableStateFactory::create_growing() const {
    return std::make_unique<GrowingState>(50, 100, 'c');
}
GrowthStatePtr VegetableStateFactory::create_ready() const {
    return std::make_unique<ReadyState>(50, 100, 'C');
}

GrowthStatePtr FlowerStateFactory::create_planted() const {
    return std::make_unique<PlantedState>(50, 100, 'f');
}
GrowthStatePtr FlowerStateFactory::create_growing() const {
    return std::make_unique<GrowingState>(50, 100, 'f');
}
GrowthStatePtr FlowerStateFactory::create_ready() const {
    return std::make_unique<ReadyState>(50, 100, 'F');
}

GrowthStatePtr TreeStateFactory::create_planted() const {
    return std::make_unique<PlantedState>(50, 300, 'i');
}
GrowthStatePtr TreeStateFactory::create_growing() const {
    return std::make_unique<GrowingState>(1000, 5000, 't');
}
GrowthStatePtr TreeStateFactory::create_ready() const {
    return std::make_unique<ReadyState>(50, 100, 'T');
}

Gardener::Gardener() : Object('@', Colors256::Yellow) {}
Ground::Ground() : TerrainObject('.', Colors256::GrayBrown) {}
Soil::Soil() : TerrainObject('#', Colors256::LightBrown) {}
Grass::Grass() : EntityObject('"', Colors256::DarkGreen) {}
Path::Path() : TerrainObject(':', Color256(130)) {}
Water::Water() : TerrainObject('~', Colors256::Blue) {}
Rock::Rock() : TerrainObject('^', Color256(242)) {}
Dump::Dump() : EntityObject('%', Color256(230)) {}
Bridge::Bridge() : EntityObject('=', Colors256::OrangeBrown) {}
House::House() : EntityObject('H', Colors256::OrangeBrown) {}

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

bool Vegetable::update() { return state->update(*this); }
bool Flower::update() { return state->update(*this); }
bool Tree::update() { return state->update(*this); }

const VegetableStateFactory Vegetable::state_factory{};
const FlowerStateFactory Flower::state_factory{};
const TreeStateFactory Tree::state_factory{};

const GrowthStateFactory& Vegetable::get_factory() const {
    return state_factory;
}
const GrowthStateFactory& Flower::get_factory() const { return state_factory; }
const GrowthStateFactory& Tree::get_factory() const { return state_factory; }

std::vector<PlayerActionTypes> Ground::get_available_actions() {
    return {
        PlayerActionTypes::Move,          PlayerActionTypes::Place,
        PlayerActionTypes::Build,         PlayerActionTypes::ExtractResources,
        PlayerActionTypes::DumpResources, PlayerActionTypes::DropResources};
}
std::vector<PlayerActionTypes> Soil::get_available_actions() {
    return {
        PlayerActionTypes::Move,          PlayerActionTypes::Place,
        PlayerActionTypes::Build,         PlayerActionTypes::ExtractResources,
        PlayerActionTypes::DumpResources, PlayerActionTypes::DropResources};
}
std::vector<PlayerActionTypes> Grass::get_available_actions() {
    return {PlayerActionTypes::Move, PlayerActionTypes::Dig,
            PlayerActionTypes::DropResources};
}
std::vector<PlayerActionTypes> Path::get_available_actions() {
    return {
        PlayerActionTypes::Move,          PlayerActionTypes::Place,
        PlayerActionTypes::Build,         PlayerActionTypes::ExtractResources,
        PlayerActionTypes::DumpResources, PlayerActionTypes::DropResources};
}
std::vector<PlayerActionTypes> Bridge::get_available_actions() {
    return {PlayerActionTypes::Move, PlayerActionTypes::Destroy,
            PlayerActionTypes::DropResources};
}
std::vector<PlayerActionTypes> Water::get_available_actions() {
    return {PlayerActionTypes::Move, PlayerActionTypes::Build,
            PlayerActionTypes::ExtractResources,
            PlayerActionTypes::DropResources};
}
std::vector<PlayerActionTypes> Rock::get_available_actions() {
    return {PlayerActionTypes::ExtractResources,
            PlayerActionTypes::DropResources};
}
std::vector<PlayerActionTypes> House::get_available_actions() {
    return {PlayerActionTypes::Move, PlayerActionTypes::Destroy,
            PlayerActionTypes::DropResources};
}
std::vector<PlayerActionTypes> Dump::get_available_actions() {
    return {PlayerActionTypes::DumpResources,
            PlayerActionTypes::GetResourcesFromDump,
            PlayerActionTypes::DropResources};
}
std::vector<PlayerActionTypes> GrowingObject::get_available_actions() {
    return {PlayerActionTypes::Move, PlayerActionTypes::Dig,
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
ResourceMap Bridge::get_resources() {
    return {{ResourceTypes::Wood, RandomGenerator::randint(2, 5)}};
}
ResourceMap Water::get_resources() { return {{ResourceTypes::Water, 1}}; }
ResourceMap Rock::get_resources() {
    return {{ResourceTypes::Stone, RandomGenerator::randint(5, 10)}};
}
ResourceMap House::get_resources() {
    return {{ResourceTypes::Wood, RandomGenerator::randint(3, 6)},
            {ResourceTypes::Stone, RandomGenerator::randint(2, 4)}};
}
ResourceMap Flower::get_resources() {
    // TODO: передать ответственность состояниям
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

const ResourceMap Bridge::required_resources{{ResourceTypes::Wood, 5}};
const ResourceMap House::required_resources{{ResourceTypes::Wood, 10},
                                            {ResourceTypes::Stone, 10}};
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

const ResourceMap& Bridge::get_required_resources() {
    return get_required_resources_static();
}
const ResourceMap& House::get_required_resources() {
    return get_required_resources_static();
}
const ResourceMap& Flower::get_required_resources() {
    return get_required_resources_static();
}
const ResourceMap& Tree::get_required_resources() {
    return get_required_resources_static();
}