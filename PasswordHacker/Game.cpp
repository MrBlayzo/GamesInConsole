#include "Game.h"

#include <algorithm>

#include "Menu.h"

std::string PasswordGenerator::generate(int len, std::string vocab) {
    std::string pass = "";
    int max_ind = vocab.size() - 1;
    for (int i = 0; i < len; ++i) {
        pass.push_back(vocab.at(RandomGenerator::randint(0, max_ind)));
    }
    return pass;
}
std::string PasswordGenerator::generate(int len, int complexity) {
    return generate(len, vocabs[complexity]);
}

double MoneyCalculator::calculate_money(int len, int complexity) {
    int base = PasswordGenerator::vocabs.at(complexity).size();
    return std::pow(base, len) * std::pow(1.5, len);
}
PasswordSolver::PasswordSolver(SolverParams params,
                               std::atomic<double>& player_money,
                               std::atomic<bool>& hard_password_found)
    : params(params),
      combinations(std::pow(
          PasswordGenerator::vocabs.at(params.complexity).size(), params.len)),
      working(false),
      player_money(player_money),
      hard_password_found(hard_password_found) {}

PasswordSolver::~PasswordSolver() { stop(); }
void PasswordSolver::generate_new_target() {
    target = PasswordGenerator::generate(params.len, params.complexity);
}
void PasswordSolver::start_solving() {
    working.store(true);
    solve_thread = std::jthread(&PasswordSolver::solving, this);
}
void PasswordSolver::solving() {
    while (working.load()) {
        generate_new_target();
        // существует проблема нахождения большого количества паролей за единицу
        // времени (сейчас только 1)
        if (params.hashrate > 100000.0)
            solve_pass_by_prob();
        else
            solve_pass();
        calculate_money();
        check_hard_password();
    }
}
void PasswordSolver::stop() {
    working.store(false);
    if (solve_thread.joinable()) {
        solve_thread.join();
    }
}
void PasswordSolver::check_hard_password() {
    if (params.len == 16 && params.complexity == 3)
        hard_password_found.store(true);
}
void PasswordSolver::solve_pass() {
    using namespace std::chrono;

    const double BATCH_SIZE = std::max(1.0, params.hashrate / 10);
    const double batches_per_second = params.hashrate / BATCH_SIZE;
    const microseconds batch_interval{
        static_cast<int>(1000000 / batches_per_second)};
    auto next_batch_time = high_resolution_clock::now();

    while (working.load()) {
        for (int i = 0; i < BATCH_SIZE; ++i) {
            if (generate_new_pass() == target) {
                return;
            }
        }
        next_batch_time += batch_interval;
        auto now = high_resolution_clock::now();
        auto sleep_time = duration_cast<microseconds>(next_batch_time - now);

        if (sleep_time.count() > 0) {
            std::this_thread::sleep_for(sleep_time);
        }
    }
}

void PasswordSolver::calculate_money() {
    player_money +=
        MoneyCalculator::calculate_money(params.len, params.complexity);
}
RandomSolver::RandomSolver(SolverParams params,
                           std::atomic<double>& player_money,
                           std::atomic<bool>& hard_password_found)
    : PasswordSolver(params, player_money, hard_password_found) {}
std::string RandomSolver::generate_new_pass() {
    std::string vocab = PasswordGenerator::vocabs.at(params.complexity);
    std::string pass = "";
    for (int i = 0; i < params.len; ++i) {
        pass.push_back(vocab.at(RandomGenerator::randint(0, vocab.size() - 1)));
    }
    return pass;
}
double RandomSolver::get_solver_cost(SolverParams params) {
    return std::pow(1.3, params.len) *
           static_cast<double>(
               PasswordGenerator::vocabs.at(params.complexity).size()) *
           2.0 * params.hashrate;
}
void RandomSolver::solve_pass_by_prob() {
    using namespace std::chrono;
    const milliseconds update_interval{100};
    auto last_update = high_resolution_clock::now();

    while (working.load()) {
        auto now = high_resolution_clock::now();
        double delta_seconds = duration<double>(now - last_update).count();
        last_update = now;

        double attempts = params.hashrate * delta_seconds;

        if (RandomGenerator::rand() < get_prob(attempts)) return;
        std::this_thread::sleep_for(update_interval);
    }
}
double RandomSolver::get_prob(double attempts) {
    if (attempts >= combinations * 10) return 1.0;
    if (combinations > 1e10) {
        return 1.0 - std::exp(-attempts / combinations);
    }
    return 1.0 - std::pow(1.0 - 1.0 / combinations, attempts);
}

double RandomSolver::get_income() {
    return MoneyCalculator::calculate_money(params.len, params.complexity) *
           params.hashrate / combinations;
}

BruteSolver::BruteSolver(SolverParams params, std::atomic<double>& player_money,
                         std::atomic<bool>& hard_password_found)
    : PasswordSolver(params, player_money, hard_password_found) {
    current = "";
    std::string vocab = PasswordGenerator::vocabs.at(params.complexity);
    for (int i = 0; i < params.len; ++i) {
        current.push_back(vocab.at(0));
    }
}
std::string BruteSolver::generate_new_pass() {
    std::string vocab = PasswordGenerator::vocabs.at(params.complexity);
    std::string return_pass = current;
    int current_ind = current.size() - 1;
    while (current_ind >= 0 &&
           vocab.find(current[current_ind]) == vocab.size() - 1) {
        current[current_ind] = vocab[0];
        current_ind -= 1;
    }
    if (current_ind >= 0)
        current[current_ind] = vocab[vocab.find(current[current_ind]) + 1];

    return return_pass;
}
double BruteSolver::get_solver_cost(SolverParams params) {
    return std::pow(1.3, params.len) *
           static_cast<double>(
               PasswordGenerator::vocabs.at(params.complexity).size()) *
           10.0 * params.hashrate;
}
double BruteSolver::get_prob(double attempts) {
    return std::min(1.0, attempts / combinations);
}
void BruteSolver::solve_pass_by_prob() {
    using namespace std::chrono;
    const milliseconds update_interval{100};
    auto last_update = high_resolution_clock::now();
    progress = 0.0;

    while (working.load()) {
        auto now = high_resolution_clock::now();
        double delta_seconds = duration<double>(now - last_update).count();
        last_update = now;

        double attempts = params.hashrate * delta_seconds;
        progress += attempts;
        if (progress >= combinations) return;

        if (RandomGenerator::rand() < get_prob(attempts)) return;

        std::this_thread::sleep_for(update_interval);
    }
}
double BruteSolver::get_income() {
    return MoneyCalculator::calculate_money(params.len, params.complexity) *
           2.0 * params.hashrate / combinations;
}

PasswordHacker::PasswordHacker()
    : engine(),
      target_len(1),
      target_complexity(0),
      target(""),
      current(""),
      money(0.0),
      hard_password_found(false),
      hard_password(PasswordGenerator::generate(16, 3)) {}
void PasswordHacker::play() {
    engine.hide_cursor();
    engine.clear();
    target = PasswordGenerator::generate(target_len, target_complexity);
    while (true) {
        print_mask();
        while (current.size() < target.size()) {
            if (check_win()) return;
            print_money();
            auto new_input = engine.get_no_wait();
            if (new_input == '\0') continue;
            if (new_input == 27) {
                open_menu();
                continue;
            }
            if (new_input == '\b' && current.size() > 0)
                current.pop_back();
            else
                current.push_back(new_input);
            print_current();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        if (current == target) {
            if (target_len == 16 && target_complexity == 3) {
                hard_password_found.store(true);
                check_win();
            }
            engine.clear();
            money +=
                MoneyCalculator::calculate_money(target_len, target_complexity);
            target = PasswordGenerator::generate(target_len, target_complexity);
        } else {
            engine.clear();
            current.clear();
        }
    }
}

void PasswordHacker::print_mask() {
    engine.set_cursor_to_zero();
    for (int i = 0; i < target.length(); ++i) {
        engine.print("_ ");
    }
}

void PasswordHacker::print_current() {
    engine.set_cursor_to_zero();
    for (auto& c : current) {
        engine.print(c, " ");
    }
    for (int i = current.size(); i < target.length(); ++i) {
        engine.print("_ ");
    }
}

void PasswordHacker::print_money() {
    engine.set_cursor_to_pos(0, 1);
    std::string raw = std::format("{:.0f}", money.load());

    int len = static_cast<int>(raw.length());
    for (int i = len - 3; i > 0; i -= 3) {
        raw.insert(i, " ");
    }
    engine.print("Money: ", raw, std::string(64, ' '));
}

void PasswordHacker::open_menu() {
    std::string complexity_option{"Password complexity"},
        buy_option{"Buy solver"};
    std::vector<MenuOption> menu_options{
        MenuOption(complexity_option, complexity_option),
        MenuOption(buy_option, buy_option)};

    auto chose = MenuSingle::show_options_menu(
        engine, Menu::default_width, Menu::default_height, 0, 0, menu_options);

    redraw();

    if (!chose.has_value()) return;
    std::string chosed =
        std::any_cast<std::string>(menu_options[chose.value()].return_param);

    if (chosed == complexity_option) {
        open_complexity_menu();

    } else {
        open_shop_menu();
    }
}

void PasswordHacker::open_complexity_menu() {
    std::vector<MenuCountOption> menu_options{
        MenuCountOption("Len", "Len", target_len, 64),
        MenuCountOption("Complexity",  //(0-10, 1-36, 2-62, 3-94)
                        "Complexity", target_complexity, 3)};

    auto chose = MenuCount::show_options_menu(
        engine, Menu::default_width, Menu::default_height, 0, 0, menu_options);

    redraw();

    if (!chose.has_value()) return;

    target_len = chose.value()[0].count;
    target_complexity = chose.value()[1].count;

    if (target_len == 16 && target_complexity == 3)
        target = hard_password;
    else
        target = PasswordGenerator::generate(target_len, target_complexity);

    current.clear();
    redraw();
}

void PasswordHacker::open_shop_menu() {
    std::string random_option{"Random solver"}, Brute_option{"Brute solver"};
    std::vector<MenuOption> menu_options{
        MenuOption(random_option, random_option),
        MenuOption(Brute_option, Brute_option)};

    auto chose = MenuSingle::show_options_menu(
        engine, Menu::default_width, Menu::default_height, 0, 0, menu_options);

    redraw();

    if (!chose.has_value()) return;
    std::string chosed =
        std::any_cast<std::string>(menu_options[chose.value()].return_param);

    if (chosed == random_option) {
        buy_random_solver();

    } else {
        buy_Brute_solver();
    }
}

std::optional<SolverParams> PasswordHacker::get_solver_params() {
    return std::nullopt;
}

void PasswordHacker::buy_random_solver() {
    auto chose = MenuSolver::show_options_menu(
        engine, Menu::default_width, Menu::default_height, 0, 0,
        [](double len, double complexity, double hashrate) {
            return RandomSolver::get_solver_cost(
                SolverParams(len, complexity, hashrate));
        },
        money);

    redraw();

    if (!chose.has_value()) return;
    SolverParams params(chose.value()[0].count, chose.value()[1].count,
                        chose.value()[2].count);
    if (params.len <= 0 || params.hashrate <= 0) return;
    auto solver_cost = RandomSolver::get_solver_cost(params);
    if (money < solver_cost) return;
    money -= solver_cost;
    solvers.emplace_back(
        std::make_unique<RandomSolver>(params, money, hard_password_found));
    max_money_from_solvers =
        std::max(max_money_from_solvers, solvers.back()->get_income());
    optimise_solvers();
    solvers.back()->start_solving();
}

void PasswordHacker::buy_Brute_solver() {
    auto chose = MenuSolver::show_options_menu(
        engine, Menu::default_width, Menu::default_height, 0, 0,
        [](double len, double complexity, double hashrate) {
            return BruteSolver::get_solver_cost(
                SolverParams(len, complexity, hashrate));
        },
        money);

    redraw();

    if (!chose.has_value()) return;
    SolverParams params(chose.value()[0].count, chose.value()[1].count,
                        chose.value()[2].count);
    if (params.len <= 0 || params.hashrate <= 0) return;
    auto solver_cost = BruteSolver::get_solver_cost(params);
    if (money < solver_cost) return;
    money -= solver_cost;
    solvers.emplace_back(
        std::make_unique<BruteSolver>(params, money, hard_password_found));
    max_money_from_solvers =
        std::max(max_money_from_solvers, solvers.back()->get_income());
    solvers.back()->start_solving();
    optimise_solvers();
}

void PasswordHacker::redraw() {
    engine.clear();
    print_mask();
    print_money();
    print_current();
}

bool PasswordHacker::check_win() {
    if (hard_password_found.load()) {
        engine.set_cursor_to_pos(0, 3);
        engine.print(
            "You have found the most difficult password for the goal!!! \n",
            "Your target password: ", hard_password, "\nCongratulations!!!");
        return true;
    }
    return false;
}

void PasswordHacker::optimise_solvers() {
    auto iter = solvers.begin();
    while (iter != solvers.end()) {
        if ((*iter)->get_income() * 1000000 < max_money_from_solvers) {
            (*iter)->stop();
            iter = solvers.erase(iter);
            continue;
        }
        ++iter;
    }
}