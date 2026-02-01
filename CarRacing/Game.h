#pragma once
#include <vector>
#include <list>
#include <array>
#include <concepts>
#include <random>
#include <chrono>
#include "ConsoleEngine.h"

class SpriteRepository {
public:
    static constexpr int width = 3;
    static constexpr int height = 4;
    using Sprite = std::array<std::array<char, width>, height>;
    static const Sprite& get_player() {
        static const Sprite sprite = {{
            {{'.', '0', '.'}},
            {{'0', '0', '0'}},
            {{'.', '0', '.'}},
            {{'0', '0', '0'}}
        }};
        return sprite;
    }

    static const Sprite& get_car() {
        static const Sprite sprite = {{
            {{'C', 'C', 'C'}},
            {{'C', ' ', 'C'}},
            {{'C', 'C', 'C'}},
            {{'.', '.', '.'}}
        }};
        return sprite;
    }

    static const Sprite& get_broken_player() {
        static const Sprite sprite = {{
            {{' ', '&', ' '}},
            {{'&', '0', '&'}},
            {{' ', '0', ' '}},
            {{'&', '0', '&'}}
        }};
        return sprite;
    }

    static const Sprite& get_truck() {
        static const Sprite sprite = {{
            {{'T', 'T', 'T'}},
            {{'T', ' ', 'T'}},
            {{'T', 'T', 'T'}},
            {{'T', ' ', 'T'}}
        }};
        return sprite;
    }

private:
    SpriteRepository() = delete;
};

class Object
{
public:
    static constexpr int width = 3;
    static constexpr int height = 4;

    Object(int pos_x, int pos_y, int relative_speed);
    virtual ~Object() = default;
    virtual const SpriteRepository::Sprite& get_sprite() = 0;
    void update_pos();
    int right_border();
    int left_border();
    int front_border();
    int back_border();
    int get_pos_x();
    int get_pos_y();
    int get_relative_speed();
    void set_relative_speed(int new_speed);
protected:
    int pos_x;
    int pos_y;
    int relative_speed;
};
class Player: public Object
{
public:
    static constexpr int width = 3;
    static constexpr int height = 4;

    Player(int pos_x, int pos_y, int relative_speed=0);
    ~Player();
    const SpriteRepository::Sprite& get_sprite() override;
    void set_pos_x(int new_pos);

private:
};
class Car: public Object
{
public:
    static constexpr int width = 3;
    static constexpr int height = 3;

    Car(int pos_x, int pos_y, int relative_speed=1);
    ~Car();
    const SpriteRepository::Sprite& get_sprite() override;

private:
};
class Truck: public Object
{
public:
    static constexpr int width = 3;
    static constexpr int height = 4;

    Truck(int pos_x, int pos_y, int relative_speed=2);
    ~Truck();
    const SpriteRepository::Sprite& get_sprite() override;

private:
};

class Road{
public:
    static constexpr int width = 5*SpriteRepository::width;
    static constexpr int height = 5*SpriteRepository::height;
    Road();
    template <typename T>
        requires std::derived_from<T, Object>
    void add_object(int pos_x, int pos_y){
        objects.push_back(std::make_unique<T>(pos_x, pos_y));
    }
    bool update();
    void clear();
    void draw();
    void render_objs();
    bool render_player();
    void render_broken_player();
    std::vector<int> free_pos();
    int get_score();
    void set_max_dist(int new_max_dist);

    Player player;
private:
    ConsoleEngine engine;
    std::list<std::unique_ptr<Object>> objects;
    std::array<std::string, height> road;
    int dist;
    int max_dist;

};

class CarGenerator{
public:
    static void generate(Road &road);

private:
    static std::random_device rd;
    static std::mt19937 gen;
    static std::uniform_int_distribution<int> dist;
};

class CarRacing{
public:
    CarRacing();
    ~CarRacing();
    void play();
    void save_high_score();
    int load_high_score();
private:
    static constexpr int max_x_move = 3;
    Road road;
    bool is_running;

    ConsoleEngine engine;
    const std::chrono::milliseconds FRAME_DURATION = std::chrono::milliseconds(250);
    const std::string highscore_filename = "highscore.txt";

    void get_player_commands();
};