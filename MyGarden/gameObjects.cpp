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
                         int max_time_to_need_watering, char sprite, DryingState::PrevState prev_state)
    : GrowthState(0, 0, min_time_to_need_watering, max_time_to_need_watering, 0,
                  0, sprite), prev_state(prev_state) {}
RottenState::RottenState(char sprite) : GrowthState(0, 0, 0, 0, 0, 0, sprite) {}

UpdateResult GrowthState::update(GrowingObject& obj) {
    if (++obj.grow_iteration >= growing_time) {
        new_stage(obj);
        return UpdateResult(true);
    }
    if (--time_to_need_watering <= 0) {
        auto prev_state = DryingState::PrevState(get_type(), obj.grow_iteration);
        obj.set_new_state(obj.get_factory().create_drying(prev_state));
        return UpdateResult(true);
    }
    time_to_need_fertilizing = std::max(0, time_to_need_fertilizing - 1);
    return UpdateResult();
}
void PlantedState::new_stage(GrowingObject& obj) {
    obj.set_new_state(obj.get_factory().create_growing());
}
void GrowingState::new_stage(GrowingObject& obj) {
    obj.set_new_state(obj.get_factory().create_ready());
}
UpdateResult ReadyState::update(GrowingObject& obj) {
    if(RandomGenerator::randint(0, 1000)==0){
        return UpdateResult(false, true);
    }
    return UpdateResult(); 
}
UpdateResult DryingState::update(GrowingObject& obj) {
    if (--time_to_need_watering <= 0) {
        if(prev_state.type == GrowthStateType::Planted)
            return UpdateResult(false, false, true);
        if(prev_state.type == GrowthStateType::Growing)
            obj.set_new_state(obj.get_factory().create_rotten());
        return UpdateResult(true);
    }
    return UpdateResult();
}
void DryingState::watering(GrowingObject& obj) {
    int grow_iteration = prev_state.grow_iteration;
    if(prev_state.type == GrowthStateType::Planted)
        obj.set_new_state(obj.get_factory().create_planted());
    if(prev_state.type == GrowthStateType::Growing)
        obj.set_new_state(obj.get_factory().create_growing());
    obj.grow_iteration = grow_iteration;
}
UpdateResult RottenState::update(GrowingObject& obj) { return UpdateResult(); }

GrowthStatePtr PotatoStateFactory::create_planted() const {
    return std::make_unique<PlantedState>(50, 100, 50, 100, 1, 100, ',');
}
GrowthStatePtr PotatoStateFactory::create_growing() const {
    return std::make_unique<GrowingState>(50, 100, 50, 100, 1, 100, 'i');
}
GrowthStatePtr PotatoStateFactory::create_ready() const {
    return std::make_unique<ReadyState>('o');
}
GrowthStatePtr PotatoStateFactory::create_drying(DryingState::PrevState prev_state) const {
    return std::make_unique<DryingState>(50, 100, '\'', prev_state);
}
GrowthStatePtr PotatoStateFactory::create_rotten() const {
    return std::make_unique<RottenState>('r');
}
GrowthStatePtr CarrotStateFactory::create_planted() const {
    return std::make_unique<PlantedState>(50, 100, 50, 100, 1, 100, ',');
}
GrowthStatePtr CarrotStateFactory::create_growing() const {
    return std::make_unique<GrowingState>(50, 100, 50, 100, 1, 100, 'i');
}
GrowthStatePtr CarrotStateFactory::create_ready() const {
    return std::make_unique<ReadyState>('c');
}
GrowthStatePtr CarrotStateFactory::create_drying(DryingState::PrevState prev_state) const {
    return std::make_unique<DryingState>(50, 100, '\'', prev_state);
}
GrowthStatePtr CarrotStateFactory::create_rotten() const {
    return std::make_unique<RottenState>('r');
}
GrowthStatePtr CucumberStateFactory::create_planted() const {
    return std::make_unique<PlantedState>(50, 100, 50, 100, 1, 100, ',');
}
GrowthStatePtr CucumberStateFactory::create_growing() const {
    return std::make_unique<GrowingState>(50, 100, 50, 100, 1, 100, 'i');
}
GrowthStatePtr CucumberStateFactory::create_ready() const {
    return std::make_unique<ReadyState>('C');
}
GrowthStatePtr CucumberStateFactory::create_drying(DryingState::PrevState prev_state) const {
    return std::make_unique<DryingState>(50, 100, '\'', prev_state);
}
GrowthStatePtr CucumberStateFactory::create_rotten() const {
    return std::make_unique<RottenState>('r');
}
GrowthStatePtr TomatoStateFactory::create_planted() const {
    return std::make_unique<PlantedState>(50, 100, 50, 100, 1, 100, ',');
}
GrowthStatePtr TomatoStateFactory::create_growing() const {
    return std::make_unique<GrowingState>(50, 100, 50, 100, 1, 100, 'i');
}
GrowthStatePtr TomatoStateFactory::create_ready() const {
    return std::make_unique<ReadyState>('O');
}
GrowthStatePtr TomatoStateFactory::create_drying(DryingState::PrevState prev_state) const {
    return std::make_unique<DryingState>(50, 100, '\'', prev_state);
}
GrowthStatePtr TomatoStateFactory::create_rotten() const {
    return std::make_unique<RottenState>('r');
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
GrowthStatePtr FlowerStateFactory::create_drying(DryingState::PrevState prev_state) const {
    return std::make_unique<DryingState>(50, 100, '/', prev_state);
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
GrowthStatePtr TreeStateFactory::create_drying(DryingState::PrevState prev_state) const {
    return std::make_unique<DryingState>(50, 100, '!', prev_state);
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

Flower::Flower()
    : GrowingObject(Color256(RandomGenerator::randint(1, 230)), get_factory().create_planted()) {}
Flower::Flower(GrowthStatePtr state)
    : GrowingObject(Color256(RandomGenerator::randint(1, 230)), std::move(state)) {}
Potato::Potato()
    : GrowingObject(Color256(172), get_factory().create_planted()) {}
Potato::Potato(GrowthStatePtr state)
    : GrowingObject(Color256(172), std::move(state)) {}
Carrot::Carrot()
    : GrowingObject(Color256(208), get_factory().create_planted()) {}
Carrot::Carrot(GrowthStatePtr state)
    : GrowingObject(Color256(208), std::move(state)) {}
Cucumber::Cucumber()
    : GrowingObject(Color256(46), get_factory().create_planted()) {}
Cucumber::Cucumber(GrowthStatePtr state)
    : GrowingObject(Color256(46), std::move(state)) {}
Tomato::Tomato()
    : GrowingObject(Color256(196), get_factory().create_planted()) {}
Tomato::Tomato(GrowthStatePtr state)
    : GrowingObject(Color256(196), std::move(state)) {}
Tree::Tree() : GrowingObject(Color256(28), get_factory().create_planted()) {}
Tree::Tree(GrowthStatePtr state)
    : GrowingObject(Color256(28), std::move(state)) {}

UpdateResult Object::update() { return UpdateResult(); }
UpdateResult Gardener::update() { return UpdateResult(); }

void GrowingObject::set_new_state(GrowthStatePtr new_state) {
    state = std::move(new_state);
    grow_iteration = 0;
    sprite = state->get_sprite();
}
UpdateResult GrowingObject::update() { return state->update(*this); }

void GrowingObject::watering() { state->watering(*this); }
void GrowingObject::fertilizing() { state->fertilizing(); }
const HouseStateFactory House::state_factory{};
const BridgeStateFactory Bridge::state_factory{};
const FlowerStateFactory Flower::state_factory{};
const PotatoStateFactory Potato::state_factory{};
const CarrotStateFactory Carrot::state_factory{};
const CucumberStateFactory Cucumber::state_factory{};
const TomatoStateFactory Tomato::state_factory{};
const TreeStateFactory Tree::state_factory{};

const BuildStateFactory& House::get_factory() const { return state_factory; }
const BuildStateFactory& Bridge::get_factory() const { return state_factory; }
const GrowthStateFactory& Flower::get_factory() const { return state_factory; }
const GrowthStateFactory& Potato::get_factory() const { return state_factory; }
const GrowthStateFactory& Carrot::get_factory() const { return state_factory; }
const GrowthStateFactory& Cucumber::get_factory() const { return state_factory; }
const GrowthStateFactory& Tomato::get_factory() const { return state_factory; }
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
        return {{ResourceTypes::FlowerSeed, RandomGenerator::randint(1, 2)}};
    } else if (dynamic_cast<ReadyState*>(state.get())) {
        return {{ResourceTypes::FlowerSeed, RandomGenerator::randint(2, 4)}};
    } else if (dynamic_cast<RottenState*>(state.get())) {
        return {{ResourceTypes::FlowerSeed, RandomGenerator::randint(0, 1)}, 
                {ResourceTypes::Fertilizer, RandomGenerator::randint(0, 2)}};
    }


    return {{ResourceTypes::FlowerSeed, 1}};
}
ResourceMap Potato::get_resources() {
    // TODO: передать ответственность состояниям
    // TODO: от сгнивших растений удобрения, от нормальных - другие
    if (dynamic_cast<GrowingState*>(state.get())) {
        return {{ResourceTypes::PotatoSeed, RandomGenerator::randint(1, 2)}};
    } else if (dynamic_cast<ReadyState*>(state.get())) {
        return {{ResourceTypes::PotatoSeed, RandomGenerator::randint(2, 4)}};
    } else if (dynamic_cast<RottenState*>(state.get())) {
        return {{ResourceTypes::PotatoSeed, RandomGenerator::randint(0, 1)}, 
                {ResourceTypes::Fertilizer, RandomGenerator::randint(0, 2)}};
    }

    return {{ResourceTypes::PotatoSeed, 1}};
}
ResourceMap Carrot::get_resources() {
    // TODO: передать ответственность состояниям
    // TODO: от сгнивших растений удобрения, от нормальных - другие
    if (dynamic_cast<GrowingState*>(state.get())) {
        return {{ResourceTypes::CarrotSeed, RandomGenerator::randint(1, 2)}};
    } else if (dynamic_cast<ReadyState*>(state.get())) {
        return {{ResourceTypes::CarrotSeed, RandomGenerator::randint(2, 4)}};
    } else if (dynamic_cast<RottenState*>(state.get())) {
        return {{ResourceTypes::CarrotSeed, RandomGenerator::randint(0, 1)}, 
                {ResourceTypes::Fertilizer, RandomGenerator::randint(0, 2)}};
    }

    return {{ResourceTypes::CarrotSeed, 1}};
}
ResourceMap Cucumber::get_resources() {
    // TODO: передать ответственность состояниям
    // TODO: от сгнивших растений удобрения, от нормальных - другие
    if (dynamic_cast<GrowingState*>(state.get())) {
        return {{ResourceTypes::CucumberSeed, RandomGenerator::randint(1, 2)}};
    } else if (dynamic_cast<ReadyState*>(state.get())) {
        return {{ResourceTypes::CucumberSeed, RandomGenerator::randint(2, 4)}};
    } else if (dynamic_cast<RottenState*>(state.get())) {
        return {{ResourceTypes::CucumberSeed, RandomGenerator::randint(0, 1)}, 
                {ResourceTypes::Fertilizer, RandomGenerator::randint(0, 2)}};
    }

    return {{ResourceTypes::CucumberSeed, 1}};
}
ResourceMap Tomato::get_resources() {
    // TODO: передать ответственность состояниям
    // TODO: от сгнивших растений удобрения, от нормальных - другие
    if (dynamic_cast<GrowingState*>(state.get())) {
        return {{ResourceTypes::TomatoSeed, RandomGenerator::randint(1, 2)}};
    } else if (dynamic_cast<ReadyState*>(state.get())) {
        return {{ResourceTypes::TomatoSeed, RandomGenerator::randint(2, 4)}};
    } else if (dynamic_cast<RottenState*>(state.get())) {
        return {{ResourceTypes::TomatoSeed, RandomGenerator::randint(0, 1)}, 
                {ResourceTypes::Fertilizer, RandomGenerator::randint(0, 2)}};
    }

    return {{ResourceTypes::TomatoSeed, 1}};
}
ResourceMap Tree::get_resources() {
    // TODO: передать ответственность состояниям
    if (dynamic_cast<GrowingState*>(state.get())) {
        return {{ResourceTypes::TreePlant, RandomGenerator::randint(1, 3)}};
    } else if (dynamic_cast<ReadyState*>(state.get())) {
        return {{ResourceTypes::TreePlant, RandomGenerator::randint(1, 3)},
                {ResourceTypes::Wood, RandomGenerator::randint(1, 3)}};
    } else if (dynamic_cast<RottenState*>(state.get())) {
        return {{ResourceTypes::TreePlant, RandomGenerator::randint(0, 1)}, 
                {ResourceTypes::Fertilizer, RandomGenerator::randint(0, 2)}};
    }

    return {{ResourceTypes::TreePlant, 1}};
}
ResourceMap Dump::get_resources() { return {}; }

const ResourceMap Bridge::required_resources{{ResourceTypes::Wood, 1}};
const ResourceMap House::required_resources{{ResourceTypes::Wood, 1},
                                            {ResourceTypes::Stone, 1}};
const ResourceMap Flower::required_resources{{ResourceTypes::FlowerSeed, 1}};
const ResourceMap Potato::required_resources{{ResourceTypes::PotatoSeed, 1}};
const ResourceMap Carrot::required_resources{{ResourceTypes::CarrotSeed, 1}};
const ResourceMap Cucumber::required_resources{{ResourceTypes::CucumberSeed, 1}};
const ResourceMap Tomato::required_resources{{ResourceTypes::TomatoSeed, 1}};
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
const ResourceMap& Potato::get_required_resources_static() {
    return required_resources;
}
const ResourceMap& Carrot::get_required_resources_static() {
    return required_resources;
}
const ResourceMap& Cucumber::get_required_resources_static() {
    return required_resources;
}
const ResourceMap& Tomato::get_required_resources_static() {
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
bool Potato::check_resources(ResourceMap& resources) {
    return Object::check_resources(resources, get_required_resources_static());
}
bool Carrot::check_resources(ResourceMap& resources) {
    return Object::check_resources(resources, get_required_resources_static());
}
bool Cucumber::check_resources(ResourceMap& resources) {
    return Object::check_resources(resources, get_required_resources_static());
}
bool Tomato::check_resources(ResourceMap& resources) {
    return Object::check_resources(resources, get_required_resources_static());
}
bool Tree::check_resources(ResourceMap& resources) {
    return Object::check_resources(resources, get_required_resources_static());
}

std::unique_ptr<GrowingObject> Flower::create_seed(){
    return std::make_unique<Flower>();
}
std::unique_ptr<GrowingObject> Potato::create_seed(){
    return std::make_unique<Potato>();
}
std::unique_ptr<GrowingObject> Carrot::create_seed(){
    return std::make_unique<Carrot>();
}
std::unique_ptr<GrowingObject> Cucumber::create_seed(){
    return std::make_unique<Cucumber>();
}
std::unique_ptr<GrowingObject> Tomato::create_seed(){
    return std::make_unique<Tomato>();
}
std::unique_ptr<GrowingObject> Tree::create_seed(){
    return std::make_unique<Tree>();
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
const ResourceMap Potato::get_required_resources() {
    return get_required_resources_static();
}
const ResourceMap Carrot::get_required_resources() {
    return get_required_resources_static();
}
const ResourceMap Cucumber::get_required_resources() {
    return get_required_resources_static();
}
const ResourceMap Tomato::get_required_resources() {
    return get_required_resources_static();
}
const ResourceMap Tree::get_required_resources() {
    return get_required_resources_static();
}