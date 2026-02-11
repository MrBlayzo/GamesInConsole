#include "PathFinder.h"

std::optional<std::deque<Point>> PathFinder::create_path_to_point(Map& map,
                                                                  Point start,
                                                                  Point end) {
    return PathFinder(map, start, end, [end](Point p) { return p == end; })
        .find_path();
}
std::optional<std::deque<Point>> PathFinder::create_path_to_area(Map& map,
                                                                 Point start,
                                                                 Point end) {
    return PathFinder(
               map, start, end,
               [end](Point p) {
                   return std::abs(p.x - end.x) + std::abs(p.y - end.y) == 1;
               })
        .find_path();
}

PathFinder::PathFinder(Map& map, Point start, Point end,
                       std::function<bool(Point)> is_target)
    : map_(map),
      start_(start),
      end_(end),
      points_(),
      comparator_(points_),
      pending_(comparator_),
      is_target_(std::move(is_target)) {}

std::optional<std::deque<Point>> PathFinder::find_path() {
    points_[start_] = PathPoint(start_, start_, 0, dist_to_target(start_));
    pending_.push(start_);

    while (!pending_.empty()) {
        auto current_pos = pending_.top();
        pending_.pop();
        if (is_target_(current_pos)) {
            found_target_ = current_pos;
            return convert_path();
        }
        explore_point(current_pos);
    }

    return std::nullopt;
}
void PathFinder::explore_point(Point current_pos) {
    auto& current = points_[current_pos];
    if (current.calculated) return;
    current.calculated = true;
    for (auto& d : dirr) {
        look_at_new({current_pos + d}, current_pos, current.cost);
    }
}
void PathFinder::look_at_new(Point new_pos, Point current_pos,
                             double current_cost) {
    if (new_pos.x < 0 || new_pos.x >= map_.width || new_pos.y < 0 ||
        new_pos.y >= map_.height) {
        return;
    }
    if (map_.get_passability(new_pos) <= 0) {
        return;
    }
    if (points_.contains(new_pos) && points_[new_pos].calculated) {
        return;
    }

    double new_cost = current_cost + map_.get_passability(new_pos);

    if (!points_.contains(new_pos) || new_cost < points_[new_pos].cost) {
        points_[new_pos] =
            PathPoint(new_pos, current_pos, new_cost, dist_to_target(new_pos));

        pending_.push(new_pos);
    }
}
double PathFinder::dist_to_target(const Point& p) const {
    return std::abs(end_.x - p.x) + std::abs(end_.y - p.y);
}

std::deque<Point> PathFinder::convert_path() {
    std::deque<Point> path;
    auto current = found_target_.value();
    while (current != start_) {
        path.push_front(current);
        current = points_[current].parent_pos;
    }
    return path;
}

