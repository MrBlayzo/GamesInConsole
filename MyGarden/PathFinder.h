#pragma once
#include <array>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Point.h"
#include "Game.h"

class Map;

class PathFinder {
  public:
    static std::optional<std::deque<Point>> create_path_to_point(Map& map,
                                                                 Point start,
                                                                 Point end);
    static std::optional<std::deque<Point>> create_path_to_area(Map& map,
                                                                Point start,
                                                                Point end);

  private:
    struct PathPoint {
        Point pos;
        Point parent_pos;
        double cost;
        double target_cost;
        bool calculated = false;
    };
    static constexpr std::array<Point, 4> dirr{
        {{-1, 0}, {0, -1}, {1, 0}, {0, 1}}};
    struct ComparePoints {
        std::unordered_map<Point, PathPoint>& points_map;
        ComparePoints(std::unordered_map<Point, PathPoint>& points_map)
            : points_map(points_map) {};
        bool operator()(const Point& a, const Point& b) const {
            return points_map[a].cost + points_map[a].target_cost >
                   points_map[b].cost + points_map[b].target_cost;
        }
    };

    PathFinder(Map& map, Point start, Point end,
               std::function<bool(Point)> is_target);
    std::optional<std::deque<Point>> find_path();
    void explore_point(Point current_pos);
    void look_at_new(Point new_pos, Point current_pos, double current_cost);
    std::deque<Point> convert_path();
    double dist_to_target(const Point& p) const;

    Map& map_;
    const Point start_;
    const Point end_;
    std::unordered_map<Point, PathPoint> points_;
    ComparePoints comparator_;
    std::priority_queue<Point, std::vector<Point>, ComparePoints> pending_;
    std::optional<Point> found_target_;
    std::function<bool(Point)> is_target_;
};