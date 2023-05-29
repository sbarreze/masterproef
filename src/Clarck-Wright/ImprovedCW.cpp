//
// Created by sbarreze on 3/11/23.
//

#include <iostream>
#include <stdexcept>
#include "ImprovedCW.h"

void ImprovedCW::solve_cvrp() {

    // Do a classic CW run
    best_solution.valid = true;
    solve_classic_manner();
    int not_better_iterations = 0;

    // Create new saving list and new route based on this new list
    newSavingList = std::vector<Saving>(savingList);
    auto best = DBL_MAX;

    for (int i=0; i < MAX_NUMBER_ITERATIONS; i++) {

        double new_cost;
        create_new_savinglist();
        build_routes();
        bool valid = fitness(currRoutes.routes, new_cost);
        if (new_cost < best) { // New route is better than best route so far

            if (firstImprovement == -1) firstImprovement = i;
            lastImprovement = i;
            nr_of_improvements++;

            // Update saving list
            std::copy(newSavingList.begin(), newSavingList.end(), savingList.begin());
            not_better_iterations = 0;

            if (!valid) { // Infeasible acceptance technique
                best = DBL_MAX;
                std::cout << "Invalid solution was created" << std::endl;
                exit(5);
            } else { // Update best solution
                best = new_cost;
                best_solution.fitness = new_cost;
                std::copy(currRoutes.routes.begin(), currRoutes.routes.end(), best_solution.routes.begin());
            }

        } else { not_better_iterations++; }

        if (i % 100 == 0) {
            std::cout << i << "\t" << nr_of_improvements << std::endl;
        }

        if (not_better_iterations > MAX_NUMBER_ITERATIONS_NO_IMPROVEMENT) { break; }

        chart_data.emplace_back(calculate_cost(best_solution));

    }

    // Remove first and last zero == depot id
    best_solution.routes.erase(best_solution.routes.begin());
    best_solution.routes.pop_back();
    best_solution.valid = true;
    print_solution();

}

void ImprovedCW::create_new_savinglist() {

#ifdef DEBUG
    std::cout << "creating new saving list ..." << std::endl;
#endif

    std::copy(savingList.begin(), savingList.end(), newSavingList.begin());

    for (int i=0; i < savingList.size(); i++) {

        int tournament_size = (int)savingList.size() - i < 9 ? (int)savingList.size() - i : random_tournament_size(rng);
        double sum_s = 0;
        for (int j=0; j < tournament_size; j++) sum_s += newSavingList[i+j].s;

        double p = random_percentage(rng);
        double qn = newSavingList[i].s / sum_s;
        int j = 1;
        while (p >= qn) {
            qn += newSavingList[i+j].s / sum_s;
            j++;
        }

        newSavingList.insert(newSavingList.begin() + i, newSavingList[i+j-1]);
        newSavingList.erase(newSavingList.begin() + i + j);

    }

}

void ImprovedCW::build_routes() {

    currRoutes = RoutePlanning(number_of_nodes, customers);

    for (const Saving &saving : newSavingList) {

        int routeI = currRoutes.getCustomerRoute(saving.i);
        int routeJ = currRoutes.getCustomerRoute(saving.j);

        if (routeI == routeJ) continue; // If i and j are in the same route, skip
        if (currRoutes.getRouteLoad(routeI) + currRoutes.getRouteLoad(routeJ) > capacity) continue;

        DEBUG_LOGGER->info("{}", fmt::join(currRoutes.routes.begin(), currRoutes.routes.end(), " "));
        DEBUG_LOGGER->info("I={}\tJ={}", saving.i, saving.j);

        if (currRoutes.isLastInRoute(saving.i) && currRoutes.isFirstInRoute(saving.j)) {
            currRoutes.mergeRoutes(routeI, routeJ);
        } else if (currRoutes.isLastInRoute(saving.j) && currRoutes.isFirstInRoute(saving.i)) {
            currRoutes.mergeRoutes(routeJ, routeI);
        }

    }
}

bool ImprovedCW::fitness(const std::vector<size_t> &routes, double &fitness) {
    bool valid = true;
    size_t route_load = 0, prev_c = depot_id;
    fitness = 0;
    for (size_t c: routes) {
        if (c == depot_id) {
            if (route_load > capacity) valid = false;
            route_load = 0;
        }
        fitness += (prev_c < c) ? (*weights)[c][prev_c] : (*weights)[prev_c][c];
        route_load += (*customers)[c].demand;
        prev_c = c;
    }

    return valid;

}

void ImprovedCW::run(size_t iter) {
    Solution::run(iter);
    OBJECTIVE_LOGGER->info("{}\t({})\t{}\t{}", name(), bestSolution, avgSolution, avgDuration);
    OBJECTIVE_LOGGER->info("F={}, L={}", firstImprovement, lastImprovement);
}

void ImprovedCW::reset() {
    ClassicCW::reset();
    newSavingList.clear();
}
