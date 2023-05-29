//
// Created by stijn on 29.10.22.
//

#include <iostream>
#include <algorithm>
#include <random>
#include <fmt/format.h>
#include "common.h"

#define ROUND_2_INT(f) ((int)(f >= 0.0 ? (f + 0.5) : (f - 0.5)))

size_t Solution::calculate_cost(const Single_Solution &s) {

    /**
     * BEST SOLUTION SHOULD NOT CONTAIN FIRST ZERO AND LAST ZERO
     */

    if (!s.valid) return 0; // Invalid solutions should return 0

    size_t totalCost = 0;
    size_t prev = depot_id; // Routes always start at depot

    for (size_t customer : s.routes) {

        totalCost += (prev < customer) ? ROUND_2_INT((*weights)[customer][prev]) : ROUND_2_INT((*weights)[prev][customer]);
        prev = customer;

    }

    // Add distance from last visited customer to depot
    totalCost += ROUND_2_INT((*weights)[prev][depot_id]);

    return totalCost;

}


Solution::Solution(size_t depot_id, int capacity, size_t number_of_nodes, size_t population_size, std::vector<Customer> *customers, std::vector<std::vector<double>> *weights)
    : depot_id(depot_id), capacity(capacity), number_of_nodes(number_of_nodes), population_size(population_size),
      rng(std::random_device{}()), customers(customers), weights(weights) {

    // General computations
    int total_demands = 0;
    for (Customer c : (*customers))
        total_demands += c.demand;
    number_of_routes = std::ceil((double)total_demands / capacity);

    if (population_size > number_of_nodes) { // Edge case when there are too few customers
        population_size = number_of_nodes;
    }

    solution_size = number_of_nodes + number_of_routes - 2; // All customers + zeros to specify routes -2 because skip first and last zero

    rand_customer = std::uniform_int_distribution<std::mt19937::result_type>(1, number_of_nodes - 1);
    rand_idx_in_solution = std::uniform_int_distribution<std::mt19937::result_type>(0, solution_size - 1);
    rand_idx_in_population = std::uniform_int_distribution<std::mt19937::result_type>(0, population_size - 1);

    best_solution.routes = std::vector<size_t>(solution_size);
    gif_data = {}; // Epoch data
}

void Solution::random_shuffle(std::vector<size_t> &solution) {
    std::shuffle(std::begin(solution), std::end(solution), rng);
}

void Solution::print_solution() {
    // Normal graph to visualise route
    VISUAL_LOGGER->info("{}", fmt::join(best_solution.routes, " "));

    // Gif data to visualise route updates
    for (auto it: gif_data) {
        GIF_LOGGER->info("{}", std::get<0>(it));
        GIF_LOGGER->info("{}", fmt::join(std::get<1>(it).routes, " "));
    }

    // Chart data to visualise evolution in fitness values
    CHART_LOGGER->info("{}", name());
    CHART_LOGGER->info("{}", fmt::join(chart_data.begin(), chart_data.end(), " "));
}

void Solution::reset() {
    population_vector.clear();
    best_solution.fitness = DBL_MAX;
}

void Solution::run(size_t iterations) {

    size_t validIterations = 0;

    for (int i=0; i < iterations; i++) {

        std::chrono::steady_clock::time_point begin, end;
        begin = std::chrono::steady_clock::now();
        solve_cvrp();
        end = std::chrono::steady_clock::now();

        auto time = std::chrono::duration_cast<std::chrono::seconds> (end - begin).count();

        size_t f = calculate_cost(best_solution);

        if (f > 0) {
            avgDuration += time;
            avgSolution += f;

            if (f < bestSolution) {
                bestSolution = f;
            }
            validIterations++;
        }

        reset();

    }

    if (validIterations > 0) {
        avgDuration /= validIterations;
        avgSolution /= validIterations;
    }

    VISUAL_LOGGER->info("{} ({}) with time: {}", name(), avgSolution, avgDuration);

}
