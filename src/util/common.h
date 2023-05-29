//
// Created by stijn on 29.10.22.
//

#ifndef MASTERREPO_COMMON_H
#define MASTERREPO_COMMON_H

#define UTIL_MAX(a, b) (((a) > (b)) ? (a): (b))

#include <float.h>
#include <ostream>
#include <utility>
#include <vector>
#include <random>
#include <set>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/stdout_color_sinks.h>

struct Coord {

    double x, y;

    friend std::ostream& operator<<(std::ostream& os, Coord const& arg) {
        return os  << arg.x << "," << arg.y;
    }

};

struct Customer {

    int demand=0;
    struct Coord position={0, 0};

    explicit Customer(int demand=0, struct Coord position={0, 0}) {
        this->demand = demand;
        this->position = position;
    }

    friend std::ostream& operator<<(std::ostream& os, Customer const& arg) {
        return os << arg.position << " ";
        // return os << "Customer " << arg.position << " with demand=" << arg.demand;
    }

};

struct Single_Solution {
    std::vector<size_t> routes;
    double fitness = -1, distance = 0, overload = 0;
    bool valid = false;

    friend bool operator< (const struct Single_Solution &a, const struct Single_Solution &b) {
        return a.fitness < b.fitness;
    }

    explicit Single_Solution(std::vector<size_t> routes) : overload(0), distance(0) {
        this->routes = std::move(routes);
        this->fitness = -1;
        this->valid = false;
    }

    explicit Single_Solution(std::vector<size_t> routes, double fitness, bool valid) {
        this->routes = std::move(routes);
        this->fitness = fitness;
        this->valid = valid;
    }

    explicit Single_Solution(std::vector<size_t> routes, double fitness, double distance, double overload) {
        this->routes = std::move(routes);
        this->fitness = fitness;
        this->valid = overload == 0;
        this->overload = overload;
        this->distance = distance;
    }

    friend std::ostream& operator<<(std::ostream& os, Single_Solution const& arg) {
        os << "[";
        for (const size_t c : arg.routes) os << c << " ";
        os << "] with fitness=" << arg.fitness << std::endl;
        return os;
    }
};

class Solution {

protected:
    // Basic things needed to use solve_cvrp
    size_t depot_id, population_size, solution_size, number_of_nodes;
    int capacity, number_of_routes=-1;
    std::vector<std::tuple<std::string, Single_Solution>> gif_data;
    std::vector<size_t> chart_data;
    std::vector<struct Single_Solution> population_vector;
    // Random generators
    std::uniform_int_distribution<std::mt19937::result_type> rand_customer, rand_idx_in_solution, rand_idx_in_population;
    std::uniform_real_distribution<double> random_percentage = std::uniform_real_distribution<double>(0.0, 1.0);
    std::mt19937 rng;
    std::vector<Customer> *customers = nullptr; // Customer data
    std::vector<std::vector<double>> *weights = nullptr; // Distance between customers -> lower triangle matrix
    // Logger
    std::shared_ptr<spdlog::logger> DEBUG_LOGGER = spdlog::get("debug_logger");
    std::shared_ptr<spdlog::logger> VISUAL_LOGGER = spdlog::get("visual_logger");
    std::shared_ptr<spdlog::logger> CHART_LOGGER = spdlog::get("chart_logger");
    std::shared_ptr<spdlog::logger> GIF_LOGGER = spdlog::get("gif_logger");
    std::shared_ptr<spdlog::logger> OBJECTIVE_LOGGER = spdlog::get("objective_logger");
    // record result
    struct Single_Solution best_solution = Single_Solution(std::vector<size_t>(0), DBL_MAX, false);
    long double avgSolution = 0, avgDuration = 0;
    size_t bestSolution = SIZE_MAX;

    void random_shuffle(std::vector<size_t> &solution);

public:
    Solution(size_t depot_id, int capacity, size_t number_of_nodes, size_t population_size, std::vector<Customer> *customers, std::vector<std::vector<double>> *weights);
    size_t calculate_cost(const Single_Solution &s);
    void print_solution();
    virtual void reset();
    virtual void run(size_t iterations);
    // To be implemented by derived classes
    virtual void solve_cvrp() = 0;
    virtual std::string name() = 0;
    virtual bool fitness(const std::vector<size_t> &solution, double &fitness) = 0;
    virtual void create_initial_population(size_t nr_sol_created) = 0;
    virtual ~Solution() = default;
};

#endif //MASTERREPO_COMMON_H
