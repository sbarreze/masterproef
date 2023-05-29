//
// Created by stijn on 31.10.22.
//

#include <cmath>
#include <iostream>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <random>
#include "bee_colony.h"

bool BeeColony::fitness(const std::vector<size_t> &food_source, double &f) {

    int route_demand=0, total_violation=0;
    f = 0;
    size_t prev_visited = depot_id;

    for (size_t customer_id : food_source) {

        if (customer_id == depot_id) { // End of route: add violation if there is any
            total_violation += UTIL_MAX(0, route_demand - capacity);
            route_demand = 0;
        }

        // Keep track of route demand
        route_demand += (*customers)[customer_id].demand;

        // Keep track of travel cost
        if (prev_visited < customer_id) f += (*weights)[customer_id][prev_visited];
        else f += (*weights)[prev_visited][customer_id];

        prev_visited = customer_id;

    }

    total_violation += UTIL_MAX(0, route_demand - capacity); // add violation of final route
    f += (*weights)[prev_visited][depot_id]; // add travel cost from final point to depot

    f += ALPHA * total_violation; // + BETHA * time_constraint

    return total_violation == 0;

}

void BeeColony::solve_cvrp() {

    /**
     * Variables
     */
    double p, best_f, new_f, curr_f;
    bool valid, best_valid;
    int v = 0;
    std::vector<int> l = std::vector<int>(TAU, 0);
    std::vector<size_t> old_food_source(solution_size), new_food_source(solution_size), best(solution_size);
    std::vector<std::set<std::vector<size_t>>> g(TAU);
    std::set<int> sorted_by_l;

    // Generate TAU random solutions
    create_initial_population(TAU);

    while (v < MAX_ITERATIONS) {

        best_f = DBL_MAX;

        // Recalculate the fitness's based on adapted ALPHA
        for (int j=0; j < TAU; j++)
            population_vector[j].valid = fitness(population_vector[j].routes, population_vector[j].fitness);

        // EMPLOYED BEES PHASE:
        // For each food source, apply neighborhood operator, if better replace else mark as not improving
        int i=0;
        for (Single_Solution &s : population_vector) {

            std::copy(s.routes.begin(), s.routes.end(), old_food_source.begin());
            neighborhood(s.routes);

            valid = fitness(s.routes, new_f);
            if (new_f > s.fitness) { // Worse fitting solution
                l[i]++;
                std::copy(old_food_source.begin(), old_food_source.end(), s.routes.begin());
            } else { // Improvement, x[i] is already set because of using a reference
                l[i] = 0;
                s.fitness = new_f;
                s.valid = valid;
            }

            i++;
        }

        // ONLOOKER BEES PHASE:

        // Select food source using fitness-based roulette wheel selection, apply neighborhood operators
        double probability, sum_fx = 0;
        for (int j=0; j < TAU; j++) {
            sum_fx += (1 / population_vector[j].fitness);
            g[j].clear();
        }

        for (int j=0; j < TAU; j++) { // Each onlooker picks a "random" food source
            probability = 0;
            p = random_percentage(rng);
            int locked_id = -1;
            while (p > probability) {
                locked_id++;
                probability += (1 / population_vector[locked_id].fitness) / sum_fx;
            }

            // Onlooker bee has picked a food source and applies a neighborhood operation
            std::copy(population_vector[locked_id].routes.begin(), population_vector[locked_id].routes.end(), new_food_source.begin());
            neighborhood(new_food_source);
            g[locked_id].insert(new_food_source);

        }

        // Find the best food source among all food sources near the old food source
        for (int j=0; j < TAU; j++) {
            if (!g[j].empty()) {

                best_f = DBL_MAX;
                for (auto &x_hat : g[j]) {
                    valid = fitness(x_hat, curr_f);
                    if (curr_f < best_f) {
                        best_f = curr_f;
                        std::copy(x_hat.begin(), x_hat.end(), best.begin());
                        best_valid = valid;
                    }
                }

                if (improvedVersion == 0) { // Original version
                    if (best_f < population_vector[j].fitness) {
                        population_vector[j].fitness = best_f;
                        population_vector[j].valid = best_valid;
                        std::copy(best.begin(), best.end(), population_vector[j].routes.begin());
                        l[j] = 0;
                    } else {
                        l[j]++;
                    }
                } else { // Optimized version

                    if (best_f < population_vector[j].fitness) {

                        sorted_by_l.clear();
                        sorted_by_l.insert(l.begin(), l.end());

                        while (!sorted_by_l.empty()) {

                            int max_l = *sorted_by_l.rbegin();
                            sorted_by_l.erase(std::prev(sorted_by_l.end())); // Pop last element
                            size_t index_of_max_l = std::find(l.begin(), l.end(), max_l) - l.begin();

                            if (best_f < population_vector[index_of_max_l].fitness) {
                                population_vector[j].fitness = best_f;
                                population_vector[j].valid = best_valid;
                                std::copy(best.begin(), best.end(), population_vector[j].routes.begin());
                                l[j] = 0;
                                break;
                            }
                        }

                    }

                }

            }
        }

        // Scout for new random solutions for solutions that have not improved for far too long.
        for (int j=0; j < TAU; j++) {
            if (l[j] >= limit) {
                if (improvedVersion == 2) {
                    neighborhood(population_vector[j].routes);
                    population_vector[j].valid = fitness(population_vector[j].routes, population_vector[j].fitness);
                } else {
                    population_vector[j].routes.resize(number_of_routes - 1, depot_id);
                    createNewSolution(shuffledCustomers, population_vector[j].routes, population_vector[j].valid, population_vector[j].fitness);
                    l[j] = 0;
                }
            }
        }

        // Count how many food sources are violating the capacity constraint
        constraint_violation = 0;
        for (const Single_Solution &s : population_vector) {
            if (!s.valid) constraint_violation++;
        }

        // If too many solutions are violating the capacity constraints, multiply ALPHA by something
        // This will cause a lower fitness for solutions that are violating this constraint
        ALPHA = (constraint_violation <= TAU / 2) ? ALPHA / (1 + DELTA) : ALPHA * (1 + DELTA);

        v++;

        // Update the best solution if needed
        for (i=0; i < TAU; i++) {
            if (population_vector[i].valid && static_cast<double>(calculate_cost(population_vector[i])) < best_solution.fitness) {
                best_solution.fitness = static_cast<double>(calculate_cost(population_vector[i]));
                std::copy(population_vector[i].routes.begin(), population_vector[i].routes.end(), best_solution.routes.begin());
                best_solution.valid = true;
            }
        }

        if (v % 200 == 0) gif_data.emplace_back(fmt::format("epoch {}: {} [a={}]", v, best_solution.fitness, ALPHA), best_solution);
        chart_data.emplace_back(calculate_cost(best_solution));

    }

    gif_data.emplace_back(fmt::format("epoch {}: {} [a={}]", v, best_solution.fitness, ALPHA), best_solution);
    chart_data.emplace_back(calculate_cost(best_solution));
    print_solution();

}

void BeeColony::neighborhood(std::vector<size_t> &old_food_source) {

    long i = static_cast<long>(rand_idx_in_solution(rng));
    while (i == 0) { i = static_cast<long>(rand_idx_in_solution(rng)); }
    long j = static_cast<long>(rand_idx_in_solution(rng));
    while (j == 0 || j == i) {j = static_cast<long>(rand_idx_in_solution(rng));}

    short neighborhood_type = random_neighborhood(rng); // Choose a random neighborhood swap
    short add = i > j ? 0 : 1;

    /*
     * Single element neighborhood operations
     */

    if (neighborhood_type == 1) { // Random swap
        std::swap(old_food_source[i], old_food_source[j]);
        return;
    }

    if (neighborhood_type == 3) { // Random insertion
        old_food_source.insert(old_food_source.begin() + i, old_food_source[j]);
        old_food_source.erase(old_food_source.begin() + j + add);
        return;
    }

    /*
     * Subsequence neighborhood operations
     */

    if (neighborhood_type == 5) {
        int length = int(random() % (old_food_source.size() - i)) + 1;
        std::reverse(old_food_source.begin() + i, old_food_source.begin() + i + length);
        return;
    }

    if (j < i) std::swap(i, j); // Make sure 'i' is the smallest index.
    int l1 = int(rand_gen(rng) % (j - i)) + 1;
    int l2 = int(rand_gen(rng) % (old_food_source.size() - j)) + 1;
    std::vector<size_t> sublist_1(old_food_source.begin() + i, old_food_source.begin() + i + l1);
    std::vector<size_t> sublist_2(old_food_source.begin() + j, old_food_source.begin() + j + l2);

    if (neighborhood_type == 2 || neighborhood_type == 6) { // Random swaps of (reversed) subsequences

        if (neighborhood_type == 6 && random() % 2) std::reverse(sublist_1.begin(), sublist_1.end()); // TODO: chance can be slightly less or more than 50%
        if (neighborhood_type == 6 && random() % 2) std::reverse(sublist_2.begin(), sublist_2.end());

        old_food_source.insert(old_food_source.begin() + i, sublist_2.begin(), sublist_2.end());
        old_food_source.erase(old_food_source.begin() + j + l2, old_food_source.begin() + j + l2 + l2);

        old_food_source.insert(old_food_source.begin() + j + l2, sublist_1.begin(), sublist_1.end());
        old_food_source.erase(old_food_source.begin() + i + l2, old_food_source.begin() + i + l2 + l1);
        return;
    }

    if (neighborhood_type == 4 || neighborhood_type == 7) { // Random insertion of a (reversed) subsequence

        if (neighborhood_type == 7 && random() % 2) std::reverse(sublist_2.begin(), sublist_2.end());

        old_food_source.insert(old_food_source.begin() + i, sublist_2.begin(), sublist_2.end());
        old_food_source.erase(old_food_source.begin() + j + l2, old_food_source.begin() + j + l2 + l2);
        return;
    }

}

void BeeColony::create_initial_population(size_t nr_sol_created) {

    bool valid;
    double f;

    for (size_t i=0; i < nr_sol_created; i++) {

        std::vector<size_t> newSolution(number_of_routes - 1, depot_id);
        createNewSolution(shuffledCustomers, newSolution, valid, f);
        population_vector.emplace_back(newSolution, f, valid);

    }
}

void BeeColony::createNewSolution(std::vector<size_t> &shuffledCustomers, std::vector<size_t> &newSolution, bool &valid, double &f) {

    double currFit, bestFit;
    int bestPos;

    random_shuffle(shuffledCustomers);

    for (size_t c : shuffledCustomers) {
        // Search the best place to store the random customer
        bestPos = 0;
        bestFit = DBL_MAX;
        for (int j = 0; j <= newSolution.size(); j++) {

            newSolution.insert(newSolution.begin() + j, c);
            fitness(newSolution, currFit);
            if (currFit < bestFit) {
                bestFit = currFit;
                bestPos = j;
            }
            newSolution.erase(newSolution.begin() + j);

        }

        // Add random customer to best place
        newSolution.insert(newSolution.begin() + bestPos, c);
    }

    valid = fitness(newSolution, bestFit);

}

void BeeColony::run(size_t iterations) {
    Solution::run(iterations);
    // OBJECTIVE_LOGGER->info("{}\t{}\t{}\t{}", TAU, bestSolution, avgSolution, avgDuration); // Objective results
    OBJECTIVE_LOGGER->info("{}\t({})\t{}\t{}", name(), bestSolution, avgSolution, avgDuration); // Draw scatter plot for alpha beta parameters
}

void BeeColony::reset() {
    Solution::reset();
    ALPHA = originalAlpha;

}
