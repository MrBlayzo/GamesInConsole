#include <algorithm>
#include <thread>
#include <fstream>
#include "Game.h"

Object::Object(int pos_x, int pos_y, int relative_speed)
    : pos_x(pos_x), pos_y(pos_y), relative_speed(relative_speed) {}
void Object::update_pos()
{
    pos_y += relative_speed;
}
int Object::right_border()
{
    return pos_x + width - 1;
}
int Object::left_border()
{
    return pos_x;
}
int Object::front_border()
{
    return pos_y;
}
int Object::back_border()
{
    return pos_y + height - 1;
}
int Object::get_pos_x(){
    return pos_x;
}
int Object::get_pos_y(){
    return pos_y;
}
int Object::get_relative_speed(){
    return relative_speed;
}
void Object::set_relative_speed(int new_speed){
    relative_speed = new_speed;
}
Player::Player(int pos_x, int pos_y, int relative_speed) : Object(pos_x, pos_y, relative_speed) {}
Player::~Player() {}
void Player::set_pos_x(int new_pos){
    pos_x = new_pos;
}
const SpriteRepository::Sprite &Player::get_sprite()
{
    return SpriteRepository::get_player();
}

Car::Car(int pos_x, int pos_y, int relative_speed) : Object(pos_x, pos_y, relative_speed) {}
Car::~Car() {}
const SpriteRepository::Sprite &Car::get_sprite()
{
    return SpriteRepository::get_car();
}

Truck::Truck(int pos_x, int pos_y, int relative_speed) : Object(pos_x, pos_y, relative_speed) {}
Truck::~Truck() {}
const SpriteRepository::Sprite &Truck::get_sprite()
{
    return SpriteRepository::get_truck();
}

Road::Road() : player(2 * SpriteRepository::width, height - SpriteRepository::height), engine(), objects(), dist(0)
{
    clear();
    engine.clear();
}
std::vector<int> Road::free_pos()
{
    std::vector<int> pos;
    std::string &first_row = road[0];
    for (int i = 0; i < first_row.size(); ++i)
    {
        bool free = true;
        for (int j = i; j < i + SpriteRepository::width && j < first_row.size(); ++j)
        {
            if (first_row[j] != '.')
            {
                free = false;
                break;
            }
        }
        if (free)
        {
            pos.push_back(i);
        }
    }
    return pos;
}
bool Road::update()
{
    CarGenerator::generate(*this);

    for (auto it = objects.begin(); it != objects.end();)
    {
        auto &obj = *it;
        for (auto &another : objects)
        {
            if (obj == another)
                continue;
            int dist = std::min(another->right_border(), obj->right_border()) - std::max(another->left_border(), obj->left_border());
            if (another->back_border() + 1 == obj->front_border() && another->get_relative_speed() > obj->get_relative_speed() &&
                dist >= 0)
            {
                obj->set_relative_speed(another->get_relative_speed());
                break;
            }
        }
        obj->update_pos();

        if (obj->get_pos_y() >= height)
        {
            it = objects.erase(it);
        }
        else
        {
            ++it;
        }
    }
    clear();
    render_objs();
    bool is_colision = render_player();
    if (is_colision)
    {
        render_broken_player();
    }
    if(!is_colision)
        ++dist;
    return is_colision;
}
void Road::clear()
{
    std::string empty_line(width, '.'); // строка из `width` точек
    for (auto &line : road)
    {
        line = empty_line;
    }
}
void Road::draw()
{
    engine.set_cursor_to_zero();

    engine.print("Best score: ", max_dist, "\n");
    engine.print("Score: ", dist, "\n");
    for (auto &row : road)
    {
        engine.print(row, "\n");
    }
}
void Road::render_objs()
{
    for (auto &obj : objects)
    {
        const SpriteRepository::Sprite &sprite = obj->get_sprite();
        for (int y = std::max(obj->get_pos_y(), 0); y < std::min(height, obj->get_pos_y() + obj->height); ++y)
        {
            for (int x = obj->get_pos_x(); x < std::min(width, obj->get_pos_x() + obj->width); ++x)
            {
                road[y][x] = sprite[y - obj->get_pos_y()][x - obj->get_pos_x()];
            }
        }
    }
}

bool Road::render_player()
{
    bool is_colision = false;
    const SpriteRepository::Sprite &sprite = player.get_sprite();
    for (int y = player.get_pos_y(); y < std::min(height, player.get_pos_y() + player.height); ++y)
    {
        for (int x = player.get_pos_x(); x < std::min(width, player.get_pos_x() + player.width); ++x)
        {
            if (road[y][x] != '.' && sprite[y - player.get_pos_y()][x - player.get_pos_x()] != '.')
            {
                return true;
            }
            road[y][x] = sprite[y - player.get_pos_y()][x - player.get_pos_x()];
        }
    }
    return is_colision;
}

void Road::render_broken_player()
{
    const SpriteRepository::Sprite &sprite = SpriteRepository::get_broken_player();
    for (int y = player.get_pos_y(); y < std::min(height, player.get_pos_y() + player.height); ++y)
    {
        for (int x = player.get_pos_x(); x < std::min(width, player.get_pos_x() + player.width); ++x)
        {
            road[y][x] = sprite[y - player.get_pos_y()][x - player.get_pos_x()];
        }
    }
}

int Road::get_score(){
    return dist;
}

void Road::set_max_dist(int new_max_dist){
    max_dist = new_max_dist;
}

std::random_device CarGenerator::rd;
std::mt19937 CarGenerator::gen{CarGenerator::rd()};
std::uniform_int_distribution<int> CarGenerator::dist(0, 8);

void CarGenerator::generate(Road& road){
    std::vector<int> free = road.free_pos();
    int randint = dist(gen);
    if (free.size() > 0 && randint < 2)
    {
        std::uniform_int_distribution<int> pos_dist(0, free.size()-1);
        int randpos = pos_dist(gen);
        if (randint == 0)
        {
            road.add_object<Car>(free[randpos], -Car::height);
        }
        else if (randint == 1)
        {
            road.add_object<Truck>(free[randpos], -Truck::height);
        }
    }
}

void CarRacing::get_player_commands()
{
    if (engine.key_pressed('A'))
    {
        road.player.set_pos_x(std::max(0, road.player.get_pos_x() - 1));
    }
    else if (engine.key_pressed('D'))
    {
        road.player.set_pos_x(std::min(road.width - road.player.width, road.player.get_pos_x() + 1));
    }
}

CarRacing::CarRacing() : road(), engine() {}
CarRacing::~CarRacing() {}
void CarRacing::play()
{
    road.set_max_dist(load_high_score());
    is_running = true;
    auto last_frame = std::chrono::steady_clock::now();
    while (is_running)
    {
        get_player_commands();

        auto now = std::chrono::steady_clock::now();
        if (now - last_frame >= FRAME_DURATION)
        {
            bool is_colision = road.update();
            road.draw();
            is_running = !is_colision;
            last_frame = now;
        }

        // Маленькая пауза, чтобы не грузить CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    save_high_score();
}

void CarRacing::save_high_score() {
    int current = road.get_score();
    int best = load_high_score();

    if (current > best) {
        std::ofstream file(highscore_filename, std::ios::trunc);
        file << current;
    }
}

int CarRacing::load_high_score() {
    std::ifstream file(highscore_filename);
    int score = 0;
    if (file) {
        file >> score;
    }
    return score;
}