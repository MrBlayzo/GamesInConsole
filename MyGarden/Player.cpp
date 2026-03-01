#include "Player.h"

#include "Game.h"
#include "Menu.h"
#include "PathFinder.h"

PlayerAction::PlayerAction(Map& map, Player& player, Point pos)
    : map(map),
      player(player),
      execute_iteration(0),
      is_executed(false),
      pos(pos) {}
DigAction::DigAction(Map& map, Player& player, Point pos)
    : PlayerAction(map, player, pos) {}
PlaceAction::PlaceAction(Map& map, Player& player, Point pos,
                         std::unique_ptr<GrowingObject> new_object)
    : PlayerAction(map, player, pos), new_object(std::move(new_object)) {}
StartBuildAction::StartBuildAction(Map& map, Player& player, Point pos,
                                   std::unique_ptr<BuildingObject> new_object)
    : PlayerAction(map, player, pos), new_object(std::move(new_object)) {}
BuildAction::BuildAction(Map& map, Player& player, Point pos,
                         ResourceMap resources)
    : PlayerAction(map, player, pos), resources(resources) {}
DestroyAction::DestroyAction(Map& map, Player& player, Point pos)
    : PlayerAction(map, player, pos) {}
ExtractResourcesAction::ExtractResourcesAction(Map& map, Player& player,
                                               Point pos)
    : PlayerAction(map, player, pos) {}
DumpResourcesAction::DumpResourcesAction(Map& map, Player& player, Point pos,
                                         ResourceMap resources)
    : PlayerAction(map, player, pos), resources(resources) {}
DropResourcesAction::DropResourcesAction(Map& map, Player& player, Point pos,
                                         ResourceMap resources)
    : PlayerAction(map, player, pos), resources(resources) {}
GetResourcesFromDumpAction::GetResourcesFromDumpAction(Map& map, Player& player,
                                                       Point pos,
                                                       ResourceMap resources)
    : PlayerAction(map, player, pos), resources(resources) {}
WateringAction::WateringAction(Map& map, Player& player, Point pos)
    : PlayerAction(map, player, pos) {}
FertilizingAction::FertilizingAction(Map& map, Player& player, Point pos)
    : PlayerAction(map, player, pos) {}
void PlayerAction::execute() {
    if (is_executed) return;
    if (++execute_iteration < get_execution_time()) return;
    finish();
    is_executed = true;
}
bool PlayerAction::executed() { return is_executed; }

void DigAction::finish() {
    for (auto& [res, count] : map.get(pos.x, pos.y).entity->get_resources()) {
        player.resources[res] += count;
    }
    map.reset_entity(pos.x, pos.y);
    map.redraw(pos.x, pos.y);
}
void DestroyAction::finish() {
    for (auto& [res, count] : map.get(pos.x, pos.y).entity->get_resources()) {
        player.resources[res] += count;
    }
    map.reset_entity(pos.x, pos.y);
    map.redraw(pos.x, pos.y);
}
void ExtractResourcesAction::finish() {
    for (auto& [res, count] : map.get(pos.x, pos.y).terrain->get_resources()) {
        player.resources[res] += count;
    }
}
void PlaceAction::finish() {
    for (auto& [res, count] : new_object->get_required_resources()) {
        player.resources[res] -= count;
    }
    map.set_new_entity(pos.x, pos.y, std::move(new_object));
    map.redraw(pos.x, pos.y);
}
void StartBuildAction::finish() {
    for (auto& [res, count] : new_object->get_start_build_resources()) {
        player.resources[res] -= count;
    }
    map.set_new_entity(pos.x, pos.y, std::move(new_object));
    map.redraw(pos.x, pos.y);
}
void BuildAction::finish() {
    if (dynamic_cast<BuildingObject*>(map.get(pos).entity.get())
            ->build(resources))
        map.redraw(pos.x, pos.y);
    for (auto& [res, count] : resources) {
        player.resources[res] -= count;
    }
}
void DumpResourcesAction::finish() {
    auto dump = dynamic_cast<Dump*>(map.get(pos.x, pos.y).entity.get());
    if (dump) {
        for (auto& [res, count] : resources) {
            player.resources[res] -= count;
            dump->resources[res] += count;
        }
    } else {
        auto new_object = std::make_unique<Dump>();
        for (auto& [res, count] : resources) {
            player.resources[res] -= count;
            new_object->resources[res] += count;
        }
        map.set_new_entity(pos.x, pos.y, std::move(new_object));
    }
}
void DropResourcesAction::finish() {
    for (auto& [res, count] : resources) {
        player.resources[res] -= count;
    }
}
void GetResourcesFromDumpAction::finish() {
    auto dump = dynamic_cast<Dump*>(map.get(pos.x, pos.y).entity.get());
    int count_all = 0;
    for (auto& [res, count] : resources) {
        player.resources[res] += count;
        dump->resources[res] -= count;
        count_all += dump->resources[res];
    }
    if (count_all <= 0) map.reset_entity(pos.x, pos.y);
}
void WateringAction::finish() {
    player.resources[ResourceTypes::Water] -= 1;
    dynamic_cast<GrowingObject*>(map.get(pos).entity.get())->watering();
}
void FertilizingAction::finish() {
    player.resources[ResourceTypes::Fertilizer] -= 1;
    dynamic_cast<GrowingObject*>(map.get(pos).entity.get())->fertilizing();
}
Player::Player(Map& map) : map(map), cursor_pos(0, 0) { calculate_weight(); }
bool Player::update() {
    if (active_path.has_value() && !active_path.value().empty()) {
        auto next_point = active_path.value().front();
        if (++walk_iteration < map.get_passability(next_point) +
                                   std::max(0, current_weight - max_weight))
            return false;

        walk_iteration = 0;
        active_path.value().pop_front();
        pos = next_point;
        return true;
    }
    if (active_action) {
        active_action->execute();
        if (active_action->executed()) {
            map.get(active_action->pos).is_on_work = false;
            map.redraw(active_action->pos);
            active_action.reset();
            calculate_weight();
            return true;
        }
    }

    return false;
}

void Player::create_path() {
    map.clear_path();
    walk_iteration = 0;

    if (map.get(cursor_pos.x, cursor_pos.y).entity)
        active_path = PathFinder::create_path_to_area(map, pos, cursor_pos);
    else
        active_path = PathFinder::create_path_to_point(map, pos, cursor_pos);

    map.draw_path();
}

void Player::create_path_to_area(Point target) {
    map.clear_path();
    walk_iteration = 0;

    active_path = PathFinder::create_path_to_area(map, pos, target);

    map.draw_path();
}

void Player::create_path_to_area() { create_path_to_area(cursor_pos); }

void Player::new_action() {
    std::vector<PlayerActionTypes> actions =
        map.get_available_action(cursor_pos.x, cursor_pos.y);
    if (current_weight > max_weight) {
        int i = 0;
        while (i < actions.size()) {
            if (actions[i] != PlayerActionTypes::Move &&
                actions[i] != PlayerActionTypes::DumpResources &&
                actions[i] != PlayerActionTypes::DropResources) {
                actions.erase(actions.begin() + i);
            } else
                ++i;
        }
    }
    if (!resources.contains(ResourceTypes::Water) ||
        resources.at(ResourceTypes::Water) <= 0) {
        std::erase_if(actions, [](PlayerActionTypes action) {
            return action == PlayerActionTypes::Watering;
        });
    }
    if (!resources.contains(ResourceTypes::Fertilizer) ||
        resources.at(ResourceTypes::Fertilizer) <= 0) {
        std::erase_if(actions, [](PlayerActionTypes action) {
            return action == PlayerActionTypes::Fertilizing;
        });
    }

    std::vector<MenuOption> menu_options;
    for (auto& action : actions) {
        menu_options.emplace_back(action_to_string(action), action);
    }

    auto chose = MenuSingle::show_options_menu(
        map.engine, Menu::default_width, Menu::default_height,
        (map.width - Menu::default_width) / 2,
        (map.height - Menu::default_height) / 2, menu_options);
    map.redraw_all();

    if (!chose.has_value()) return;
    PlayerActionTypes chosed = std::any_cast<PlayerActionTypes>(
        menu_options[chose.value()].return_param);
    if (chosed == PlayerActionTypes::Move) {
        create_path();
    } else if (chosed == PlayerActionTypes::Place) {
        new_place_action();
    } else if (chosed == PlayerActionTypes::StartBuild) {
        new_start_build_action();
    } else if (chosed == PlayerActionTypes::Build) {
        new_continue_build_action();
    } else if (chosed == PlayerActionTypes::Dig) {
        create_path_to_area();
        active_action = std::make_unique<DigAction>(map, *this, cursor_pos);
    } else if (chosed == PlayerActionTypes::Destroy) {
        create_path_to_area();
        active_action = std::make_unique<DestroyAction>(map, *this, cursor_pos);
    } else if (chosed == PlayerActionTypes::ExtractResources) {
        create_path_to_area();
        active_action =
            std::make_unique<ExtractResourcesAction>(map, *this, cursor_pos);
    } else if (chosed == PlayerActionTypes::DumpResources) {
        new_dump_resources_action(cursor_pos);

    } else if (chosed == PlayerActionTypes::GetResourcesFromDump) {
        new_get_resources_from_dump_action();
    } else if (chosed == PlayerActionTypes::DropResources) {
        new_drop_resources_action();
    } else if (chosed == PlayerActionTypes::Watering) {
        create_path_to_area();
        active_action =
            std::make_unique<WateringAction>(map, *this, cursor_pos);
    } else if (chosed == PlayerActionTypes::Fertilizing) {
        create_path_to_area();
        active_action =
            std::make_unique<FertilizingAction>(map, *this, cursor_pos);
    }
    if (active_action) {
        map.get(active_action->pos).is_on_work = true;
        map.redraw(active_action->pos);
    }
}

void Player::new_start_build_action() {
    std::vector<BuildingTypes> buildings =
        map.get_available_buildings(cursor_pos.x, cursor_pos.y);

    std::vector<MenuOption> menu_buildings_options;
    for (auto& building : buildings) {
        if (building == BuildingTypes::Bridge &&
            Bridge::check_resources(resources))
            menu_buildings_options.emplace_back(
                building_type_to_string(building), building);
        else if (building == BuildingTypes::House &&
                 House::check_resources(resources))
            menu_buildings_options.emplace_back(
                building_type_to_string(building), building);
    }

    auto chose_object_ind = MenuSingle::show_options_menu(
        map.engine, Menu::default_width, Menu::default_height,
        (map.width - Menu::default_width) / 2,
        (map.height - Menu::default_height) / 2, menu_buildings_options);
    map.redraw_all();
    if (!chose_object_ind.has_value()) return;
    BuildingTypes chose_object = std::any_cast<BuildingTypes>(
        menu_buildings_options[chose_object_ind.value()].return_param);
    create_path_to_area();
    if (chose_object == BuildingTypes::Bridge)
        active_action = std::make_unique<StartBuildAction>(
            map, *this, cursor_pos, std::make_unique<Bridge>());
    else if (chose_object == BuildingTypes::House)
        active_action = std::make_unique<StartBuildAction>(
            map, *this, cursor_pos, std::make_unique<House>());
}
void Player::new_continue_build_action() {
    std::vector<MenuMassOption> menu_options;
    const ResourceMap& required_resources =
        map.get(cursor_pos).entity->get_required_resources();
    for (auto& [res, count] : resources) {
        if (!required_resources.contains(res)) continue;
        menu_options.emplace_back(resource_type_to_string(res), res, 0,
                                  std::min(count, required_resources.at(res)));
    }

    auto chosed_options = MenuMass::show_options_menu(
        map.engine, Menu::default_width, Menu::default_height,
        (map.width - Menu::default_width) / 2,
        (map.height - Menu::default_height) / 2, menu_options);
    map.redraw_all();

    if (!chosed_options.has_value()) return;
    ResourceMap chosed_resources;
    int count = 0;
    for (auto& option : chosed_options.value()) {
        chosed_resources[std::any_cast<ResourceTypes>(option.return_param)] =
            option.count;
        count += option.count;
    }
    if (count >= 0) {
        create_path_to_area();
        active_action = std::make_unique<BuildAction>(map, *this, cursor_pos,
                                                      chosed_resources);
    }
}
void Player::new_place_action() {
    std::vector<MenuOption> place_menu_options;
    if (Flower::check_resources(resources)) {
        place_menu_options.emplace_back("Flower", "Flower");
    }
    if (Tree::check_resources(resources)) {
        place_menu_options.emplace_back("Tree", "Tree");
    }
    auto chose_object = MenuSingle::show_options_menu(
        map.engine, Menu::default_width, Menu::default_height,
        (map.width - Menu::default_width) / 2,
        (map.height - Menu::default_height) / 2, place_menu_options);
    map.redraw_all();
    if (!chose_object.has_value()) return;
    create_path_to_area();
    if (place_menu_options[chose_object.value()].param == "Flower")
        active_action = std::make_unique<PlaceAction>(
            map, *this, cursor_pos, std::make_unique<Flower>());
    else if (place_menu_options[chose_object.value()].param == "Tree")
        active_action = std::make_unique<PlaceAction>(map, *this, cursor_pos,
                                                      std::make_unique<Tree>());
}
void Player::new_dump_resources_action(Point action_pos) {
    std::vector<MenuMassOption> menu_options;
    for (auto& [res, count] : resources) {
        if (count <= 0) continue;
        menu_options.emplace_back(resource_type_to_string(res), res, 0, count);
    }

    auto chosed_options = MenuMass::show_options_menu(
        map.engine, Menu::default_width, Menu::default_height,
        (map.width - Menu::default_width) / 2,
        (map.height - Menu::default_height) / 2, menu_options);
    map.redraw_all();

    if (!chosed_options.has_value()) return;
    ResourceMap chosed_resources;
    int count = 0;
    for (auto& option : chosed_options.value()) {
        chosed_resources[std::any_cast<ResourceTypes>(option.return_param)] =
            option.count;
        count += option.count;
    }
    if (count >= 0) {
        create_path_to_area();
        active_action = std::make_unique<DumpResourcesAction>(
            map, *this, action_pos, chosed_resources);
    }
}
void Player::new_drop_resources_action() {
    std::vector<MenuMassOption> menu_options;
    for (auto& [res, count] : resources) {
        if (count <= 0) continue;
        menu_options.emplace_back(resource_type_to_string(res), res, 0, count);
    }

    auto chosed_options = MenuMass::show_options_menu(
        map.engine, Menu::default_width, Menu::default_height,
        (map.width - Menu::default_width) / 2,
        (map.height - Menu::default_height) / 2, menu_options);
    map.redraw_all();

    if (!chosed_options.has_value()) return;
    ResourceMap chosed_resources;
    int count = 0;
    for (auto& option : chosed_options.value()) {
        chosed_resources[std::any_cast<ResourceTypes>(option.return_param)] =
            option.count;
        count += option.count;
    }
    if (count >= 0) {
        create_path_to_area();
        active_action = std::make_unique<DropResourcesAction>(
            map, *this, cursor_pos, chosed_resources);
    }
}
void Player::new_get_resources_from_dump_action() {
    std::vector<MenuMassOption> menu_options;
    auto dump =
        dynamic_cast<Dump*>(map.get(cursor_pos.x, cursor_pos.y).entity.get());
    for (auto& [res, count] : dump->resources) {
        if (count <= 0) continue;
        menu_options.emplace_back(resource_type_to_string(res), res, 0, count);
    }

    auto chosed_options = MenuMass::show_options_menu(
        map.engine, Menu::default_width, Menu::default_height,
        (map.width - Menu::default_width) / 2,
        (map.height - Menu::default_height) / 2, menu_options);
    map.redraw_all();

    if (!chosed_options.has_value()) return;
    ResourceMap chosed_resources;
    int count = 0;
    for (auto& option : chosed_options.value()) {
        chosed_resources[std::any_cast<ResourceTypes>(option.return_param)] =
            option.count;
        count += option.count;
    }
    if (count >= 0) {
        create_path_to_area();
        active_action = std::make_unique<GetResourcesFromDumpAction>(
            map, *this, cursor_pos, chosed_resources);
    }
}
void Player::see_resouces() {
    std::vector<MenuMassOption> menu_options;
    for (auto& [res, count] : resources) {
        if (count <= 0) continue;
        menu_options.emplace_back(resource_type_to_string(res), res, count,
                                  count);
    }

    MenuMass::show_options_menu(
        map.engine, Menu::default_width, Menu::default_height,
        (map.width - Menu::default_width) / 2,
        (map.height - Menu::default_height) / 2, menu_options, false);
    map.redraw_all();
}

bool Player::check_resources(PlayerActionTypes action) {
    switch (action) {
        case PlayerActionTypes::Move:
            return true;
        case PlayerActionTypes::Dig:
            return true;
        case PlayerActionTypes::Place:
            return true;
        case PlayerActionTypes::Build:
            return true;
        case PlayerActionTypes::Destroy:
            return true;
        case PlayerActionTypes::ExtractResources:
            return true;

        default:
            return false;
    }
}

void Player::calculate_weight() {
    current_weight = get_resourse_weight(resources);
}

void Player::clear_action() {
    map.clear_path();
    if (active_action) {
        map.get(active_action->pos).is_on_work = false;
        map.redraw(active_action->pos);
    }
    active_path.reset();
    active_action.reset();
}