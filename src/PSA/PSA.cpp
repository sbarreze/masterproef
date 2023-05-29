//
// Created by sbarreze on 2/23/23.
//
#include "PSA.h"
#include <cmath>
#include <algorithm>
#include <unordered_set>
#include <cfloat>
#include <iostream>

void PSA::solve_cvrp() {

    DEBUG_LOGGER->info("Start PSA");

    double temperature = START_TEMP;
    double fraction = std::pow((END_TEMP / START_TEMP), (1.0 / (static_cast<double>(ITERATION_NUMBER) - 1.0)));
    std::vector<size_t> new_route(solution_size);
    COUNT_MATRIX = std::vector<size_t>(solution_size, 0);

    create_initial_population(population_size);
    double new_fitness, w, r;
    bool valid;

    for (int i=0; i < ITERATION_NUMBER; i++) {
        DEBUG_LOGGER->debug("---\nITERATION {}\n---", i);
        for (Single_Solution &s : population_vector) {
            valid = true;
            for (int j=0; j < NUMBER_OF_TRIALS; j++) {

                if (valid) r = random_percentage(rng);
                route_development(new_route, s, r);
                valid = fitness(new_route, new_fitness);

                DEBUG_LOGGER->debug("===\n{} VS {}\n===", s.routes.size(), new_route.size());

                if (valid) { // ONLY LOOK AT VALID OPTIONS
                    VALID++;
                    if (new_fitness < s.fitness) { // Better solution found
                        std::copy(new_route.begin(), new_route.end(), s.routes.begin());
                        s.fitness = new_fitness;
                    } else { // Check if old solutions needs to be updated
                        w = exp((-new_fitness + s.fitness) / temperature);
                        if (random_percentage(rng) < w) {
                            // TODO: line gives invalid write????
                            std::copy(new_route.begin(), new_route.end(), s.routes.begin());
                            s.fitness = new_fitness;
                        }
                    }
                } else { INVALID_TRIALS++; }
            }
            if (valid && s.fitness < best_solution.fitness) { // UPDATE BEST SOLUTION
                std::copy(s.routes.begin(), s.routes.end(), best_solution.routes.begin());
                best_solution.fitness = s.fitness;
            }
        }
        // TODO: make GIF
        if (i % 100 == 0)
            gif_data.emplace_back(fmt::format("epoch {}: {} [T={}]", i, best_solution.fitness, temperature), best_solution);

        chart_data.emplace_back(calculate_cost(best_solution));

        // Update temperature
        temperature *= fraction;
    }

    gif_data.emplace_back(fmt::format("epoch {}: {} [T={}]", ITERATION_NUMBER, best_solution.fitness, temperature), best_solution);

    // std::cout << "FAILED=" << INVALID_TRIALS << "\tvalid=" << VALID << std::endl;
    print_solution();

    DEBUG_LOGGER->debug("COUNT MATRIX: {}", fmt::join(COUNT_MATRIX.begin(), COUNT_MATRIX.end(), " "));

}

void PSA::create_initial_population(size_t nr_sol_created) {

    DEBUG_LOGGER->debug("---\nCreating population...\n---");

    std::vector<size_t> shuffled_customers(number_of_nodes - 1);
    std::iota(shuffled_customers.begin(), shuffled_customers.end(), 1);
    double curr_fit, best_fit;
    bool valid;

    while (population_vector.size() <= nr_sol_created) {

        DEBUG_LOGGER->debug(fmt::format("solution {}:", population_vector.size()));

        std::vector<size_t> new_solution(number_of_routes - 1, depot_id);

        random_shuffle(shuffled_customers);

        DEBUG_LOGGER->debug("{}", fmt::join(shuffled_customers.begin(), shuffled_customers.end(), " "));

        for (size_t c: shuffled_customers) {
            // Search the best place to store the random customer
            int best_pos = 0;
            best_fit = DBL_MAX;
            for (int j = 0; j <= new_solution.size(); j++) {

                new_solution.insert(new_solution.begin() + j, c);
                valid = fitness(new_solution, curr_fit);
                if (valid && curr_fit < best_fit) {
                    best_fit = curr_fit;
                    best_pos = j;
                }
                new_solution.erase(new_solution.begin() + j);

            }

            DEBUG_LOGGER->debug("best position={}", best_pos);

            // Add random customer to best place
            new_solution.insert(new_solution.begin() + best_pos, c);
        }

        valid = fitness(new_solution, curr_fit);
        DEBUG_LOGGER->debug("Solution created (f={}, valid={})\n{}", curr_fit, valid,
                      fmt::join(new_solution.begin(), new_solution.end(), " "));
        if (valid) {
            population_vector.emplace_back(new_solution, curr_fit, valid);
            if (curr_fit < best_solution.fitness) {
                std::copy(new_solution.begin(), new_solution.end(), best_solution.routes.begin());
                best_solution.fitness = curr_fit;
                best_solution.valid = valid;
            }
        } else {
            // std::cout << "invalid solution created in PSA creation of population";
            // exit(-5);
        }
    }
    DEBUG_LOGGER->debug("\nBest solution ({})", best_solution.fitness);
    DEBUG_LOGGER->debug("{}", fmt::join(best_solution.routes.begin(), best_solution.routes.end(), " "));

}

void PSA::route_development(std::vector<size_t> &result, const Single_Solution &old_solution, double r) {

    DEBUG_LOGGER->debug("\n\n=> {}", fmt::join(old_solution.routes.begin(), old_solution.routes.end(), " "));
    std::copy(old_solution.routes.begin(), old_solution.routes.end(), result.begin());

    long random_point_1, random_point_2;
    random_point_1 = static_cast<long>(rand_idx_in_solution(rng));
    do { random_point_2 = static_cast<long>(rand_idx_in_solution(rng)); } while (random_point_1 == random_point_2);
    if (random_point_1 > random_point_2) std::swap(random_point_1, random_point_2);
    COUNT_MATRIX[random_point_1]++;
    COUNT_MATRIX[random_point_2]++;

    short extra_random_point_1 = random_point_1 + 1 < random_point_2 && random_percentage(rng) > 0.5 ? 1 : 0;
    short extra_random_point_2 = random_point_2 < solution_size - 1 && random_percentage(rng) > 0.5 ? 1 : 0;

    std::vector<size_t> sublist1(result.begin() + random_point_1, result.begin() + random_point_1 + extra_random_point_1 + 1);
    std::vector<size_t> sublist2(result.begin() + random_point_2, result.begin() + random_point_2 + extra_random_point_2 + 1);

    DEBUG_LOGGER->debug("r={}, p1={}, p2={}, e1={}, e2={}", r, random_point_1, random_point_2, extra_random_point_1, extra_random_point_2);

    if (r < 0.3333) { // EXCHANGE OPERATOR
        result.erase(result.begin() + random_point_2, result.begin() + random_point_2 + 1 + extra_random_point_2);
        result.erase(result.begin() + random_point_1, result.begin() + random_point_1 + 1 + extra_random_point_1);
        result.insert(result.begin() + random_point_1, sublist2.begin(), sublist2.end());
        result.insert(result.begin() + random_point_2 + extra_random_point_2 - extra_random_point_1, sublist1.begin(), sublist1.end());
    } else if (r < 0.6666) { // INSERT OPERATOR
        result.erase(result.begin() + random_point_2, result.begin() + random_point_2 + 1 + extra_random_point_2);
        result.insert(result.begin() + random_point_1, sublist2.begin(), sublist2.end());
    } else { // REVERSE OPERATOR
        std::reverse(result.begin() + random_point_1, result.begin() + random_point_2 + extra_random_point_2);
    }

    DEBUG_LOGGER->debug("<= {}", fmt::join(result.begin(), result.end(), " "));

}

bool PSA::fitness(const std::vector<size_t> &solution, double &f) {
    size_t prev = depot_id;
    int route_weight = 0;
    f = 0;

    for (size_t customer : solution) {
        f += (prev < customer) ? (*weights)[customer][prev] : (*weights)[prev][customer];
        route_weight += (*customers)[customer].demand;
        if (customer == depot_id) { // Invalid solution => regenerate
            if (route_weight > capacity) {
                return false;
            }
            route_weight = 0;
        }
        prev = customer;
    }

    f += (*weights)[prev][depot_id];

    return route_weight <= capacity;
}

void PSA::run(size_t iterations) {
    Solution::run(iterations);
    OBJECTIVE_LOGGER->info("{}\t({})\t{}\t{}", name(), bestSolution, avgSolution, avgDuration);
    // OBJECTIVE_LOGGER->info("{}\t{}\t{}", START_TEMP, END_TEMP, avgSolution);
    // OBJECTIVE_LOGGER->info("{}\t{}\t{}", ITERATION_NUMBER, NUMBER_OF_TRIALS, avgSolution);
    // OBJECTIVE_LOGGER->info("n{}\t{}\t{}", avgDuration, population_size, avgSolution);

}

