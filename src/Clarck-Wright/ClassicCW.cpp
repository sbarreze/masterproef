//
// Created by sbarreze on 3/9/23.
//

#include <iostream>
#include <algorithm>
#include "ClassicCW.h"
#include "../util/Route.h"

void ClassicCW::solve_cvrp() {

    solve_classic_manner();

    best_solution.routes.pop_back();
    best_solution.routes.erase(best_solution.routes.begin());
    best_solution.valid = true;
    print_solution();

}

int ClassicCW::not_interior(std::vector<size_t> &routes, int idx) {
    if (idx == -1) return -1;
    if (routes[idx-1] == depot_id) return 0;
    if (routes[idx+1] == depot_id) return 1;
    return -1;
}

int ClassicCW::find_route_begin(std::vector<size_t> &routes, int idx) {
    if (idx <= 0) return -1; // No valid idx, can happen when the route of a not added customer is asked
    int j = -1;
    while (routes[idx+j] != depot_id) j--; // walk to the left until you are at the depot
    return idx+j;
}

int ClassicCW::find_route_end(std::vector<size_t> &routes, int idx) {
    if (idx <= 0) return -1; // No valid idx, can happen when the route of a not added customer is asked
    int j = 1;
    while (routes[idx+j] != depot_id) j++; // walk to the right until you are at the depot
    return idx+j;
}


void ClassicCW::merge_routes(std::vector<size_t> &routes, int end_i, int start_j) {
    size_t route_load = 0;
    int i = 1;
    while (routes[end_i - i] != depot_id) { route_load += (*customers)[routes[end_i - i]].demand; i++; }
    int j = 1;
    while (routes[start_j + j] != depot_id) { route_load += (*customers)[routes[start_j + j]].demand; j++; }
    if (route_load > capacity) return; // Invalid to merge 2 routes

    routes.erase(routes.begin() + end_i); // Remove depot visit
    std::vector<size_t> tmp(routes.begin() + start_j, routes.begin() + start_j + j);
    routes.insert(routes.begin() + end_i, tmp.begin(), tmp.end());
    routes.erase(routes.begin() + start_j + j, routes.begin() + start_j + j + j);
    for (int k=end_i; k < routes.size(); k++) {
        customer_idxs[routes[k]] = k;
    }

}

void ClassicCW::merge_customer_in_route(std::vector<size_t> &routes, size_t customer, int i, int interiorI) {
    size_t route_load = (*customers)[routes[i]].demand;
    int j = -1;
    while (routes[i+j] != depot_id) { route_load += (*customers)[routes[i+j]].demand; j--; }
    j = 1;
    while (routes[i+j] != depot_id) { route_load += (*customers)[routes[i+j]].demand; j++; }
    if (route_load + (*customers)[customer].demand <= capacity) {
        routes.insert(routes.begin() + i + interiorI, customer);
        customer_idxs[customer] = i + interiorI;
        for (int c = i + interiorI + 1; c < routes.size(); c++) customer_idxs[routes[c]]++;
    }
}

void ClassicCW::solve_classic_manner() {

    // Calculate savings list
    for (size_t i=1; i < number_of_nodes; i++) {
        for (size_t j=i+1; j < number_of_nodes; j++) {
                savingList.emplace_back(i, j, (*weights)[i][0] + (*weights)[j][0] - (*weights)[j][i]);
        }
    }

    std::sort(savingList.begin(), savingList.end()); // Sort in decreasing order.

    RoutePlanning solution(number_of_nodes, customers);

    // Run over saving list and build solution
    for (const Saving &saving : savingList) {

        int routeI = solution.getCustomerRoute(saving.i);
        int routeJ = solution.getCustomerRoute(saving.j);

        if (routeI == routeJ) continue; // If i and j are in the same route, skip
        if (solution.getRouteLoad(routeI) + solution.getRouteLoad(routeJ) > capacity) continue;

        DEBUG_LOGGER->info("{}", fmt::join(solution.routes.begin(), solution.routes.end(), " "));
        DEBUG_LOGGER->info("I={}\tJ={}", saving.i, saving.j);

        if (solution.isLastInRoute(saving.i) && solution.isFirstInRoute(saving.j)) {
            solution.mergeRoutes(routeI, routeJ);
        } else if (solution.isLastInRoute(saving.j) && solution.isFirstInRoute(saving.i)) {
            solution.mergeRoutes(routeJ, routeI);
        }
    }

    best_solution.routes = std::move(solution.routes);

}

void ClassicCW::reset() {
    Solution::reset();
    savingList.clear();
}

void ClassicCW::run(size_t iterations) {
    Solution::run(iterations);
    OBJECTIVE_LOGGER->info("{}\t({})\t{}\t{}", name(), bestSolution, avgSolution, avgDuration);
}
