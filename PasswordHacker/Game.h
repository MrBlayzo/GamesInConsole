#pragma once

#include <math.h>

#include <atomic>
#include <chrono>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "ConsoleEngine.h"
#include "RandomGenerator.h"

class PasswordGenerator {
  public:
    inline static const std::vector<std::string> vocabs{
        "1234567890", "1234567890abcdefghijklmnopqrstuvwxyz",
        "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
        "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]{};:'"
        "\"<,.>/?-_=+`~!@#$%^&*()\\|"};

    static std::string generate(int len, std::string vocab);
    static std::string generate(int len, int complexity);
};

class MoneyCalculator {
  public:
    static double calculate_money(int len, int complexity);
};

struct SolverParams {
    int len, complexity;
    double hashrate;
};

class PasswordSolver {
  public:
    PasswordSolver(SolverParams params, std::atomic<double>& player_money,
                   std::atomic<bool>& hard_password_found);
    virtual ~PasswordSolver();
    void generate_new_target();
    void start_solving();
    void solving();
    void stop();
    void solve_pass();
    void check_hard_password();
    virtual void solve_pass_by_prob() = 0;
    void calculate_money();
    virtual std::string generate_new_pass() = 0;
    virtual double get_income() = 0;

  protected:
    SolverParams params;
    const double combinations;
    std::atomic<bool> working;

  private:
    std::atomic<double>& player_money;
    std::atomic<bool>& hard_password_found;
    std::string target;
    std::jthread solve_thread;
};

class RandomSolver : public PasswordSolver {
  public:
    RandomSolver(SolverParams params, std::atomic<double>& player_money,
                 std::atomic<bool>& hard_password_found);
    std::string generate_new_pass() override;
    static double get_solver_cost(SolverParams params);
    void solve_pass_by_prob() override;
    double get_prob(double attempts);
    double get_income() override;
};

class BruteSolver : public PasswordSolver {
  public:
    BruteSolver(SolverParams params, std::atomic<double>& player_money,
                std::atomic<bool>& hard_password_found);
    std::string generate_new_pass() override;
    static double get_solver_cost(SolverParams params);
    void solve_pass_by_prob() override;
    double get_prob(double attempts);
    double get_income() override;

  private:
    std::string current;
    double progress = 0.0;
};

class PasswordHacker {
  public:
    PasswordHacker();
    void play();

  private:
    ConsoleEngine engine;
    int target_len, target_complexity;
    std::string target, current;
    std::atomic<double> money;
    std::atomic<bool> hard_password_found;
    std::string hard_password;

    std::vector<std::unique_ptr<PasswordSolver>> solvers;
    double max_money_from_solvers = 0.0;

    void print_mask();
    void print_current();
    void print_money();
    void open_menu();
    void open_complexity_menu();
    void open_shop_menu();
    std::optional<SolverParams> get_solver_params();
    void buy_random_solver();
    void buy_Brute_solver();
    void redraw();
    bool check_win();
    void optimise_solvers();
};