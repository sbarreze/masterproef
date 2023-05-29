//
// Created by stijn on 25.11.22.
//

#include "gelsga.h"
#include <set>
#include <algorithm>
#include <iostream>
#include <unordered_set>

void GELS_GA::create_initial_population(size_t nr_sol_created) {

    DEBUG_LOGGER->info("===\nGenerating initial population...\n===");

    std::vector<size_t> random_solution(number_of_routes - 1, depot_id);
    for (size_t c=1; c < number_of_nodes; c++) random_solution.push_back(c);

    double f;
    bool v;

    population_vector.reserve(population_size + 3 * population_size);

    while (population_vector.size() < nr_sol_created) {

        random_shuffle(random_solution); // Start from really random solutions
        v = fitness(random_solution, f);
        population_vector.emplace_back(random_solution, f, v);

        DEBUG_LOGGER->info("Chromosome {}: {}", population_vector.size(), f);
        DEBUG_LOGGER->info("{}", fmt::join(random_solution.begin(), random_solution.end(), " "));
    }

}

bool GELS_GA::fitness(const std::vector<size_t> &solution, double &f) {

    int overload = 0;
    int route_capacity = 0;
    f = 0;

    size_t prev_customer = depot_id;

    for (size_t customer : solution) {

        route_capacity += (*customers)[customer].demand;
        f += (prev_customer < customer) ? (*weights)[customer][prev_customer] : (*weights)[prev_customer][customer];

        if (customer == depot_id) {
            overload += UTIL_MAX(0, route_capacity - capacity);
            route_capacity = 0;
        }

        prev_customer = customer;
    }

    // Final route: last customer -> depot
    f += (*weights)[prev_customer][depot_id];
    overload += UTIL_MAX(0, route_capacity - capacity);

    f += ALPHA * overload;

    return (overload == 0);

}

void GELS_GA::crossover(const Single_Solution &original) {

    if (random_percentage(rng) > crossoverProb) return;

    double f;
    bool v;

    auto random_it = population_vector.begin();
    size_t rand_i = rand_idx_in_population(rng);
    std::advance(random_it, rand_i);

    if (random_percentage(rng) < 0.5) { // Type 1 crossover: order from 1 parent, # customers per route from other

        statistics[0]++;

        DEBUG_LOGGER->info("Crossover type 1:");
        DEBUG_LOGGER->info("p1({})={}", original.fitness, fmt::join(original.routes.begin(), original.routes.end(), " "));
        DEBUG_LOGGER->info("p2({})={}", (*random_it).fitness, fmt::join((*random_it).routes.begin(), (*random_it).routes.end(), " "));

        std::vector<size_t> child_1, child_2;

        int j = 0, j2 = 0;
        while (original.routes[j] == depot_id) { j++; }
        while ((*random_it).routes[j2] == depot_id) { j2++; }
        size_t n; // Contains new element to add to child
        for (int i=0; i < solution_size; i++) {

            // Child 1
            n = (*random_it).routes[i] == 0 ? 0 : original.routes[j];
            child_1.push_back(n);
            if ((*random_it).routes[i] != 0) {
                do { j++; } while (j < original.routes.size() && original.routes[j] == depot_id);
            }

            // Child 2
            n = original.routes[i] == 0 ? 0 : (*random_it).routes[j2];
            child_2.push_back(n);
            if (original.routes[i] != 0) {
                do { j2++; } while (j2 < (*random_it).routes.size() && (*random_it).routes[j2] == depot_id);
            }
        }

        // Add children to the population
        v = fitness(child_1, f);
        population_vector.emplace_back(child_1, f, v);
        // children.emplace(child_1, f, v);
        DEBUG_LOGGER->info("c1({})={}", f, fmt::join(child_1.begin(), child_1.end(), " "));

        v = fitness(child_2, f);
        population_vector.emplace_back(child_2, f, v);
        DEBUG_LOGGER->info("c2({})={}", f, fmt::join(child_2.begin(), child_2.end(), " "));

        return;

    } else { // Type 2: one part from one parent, one part from other

        statistics[1]++;

        size_t random_index;
        do {random_index = rand_idx_in_solution(rng);} while (random_index == 0 || random_index == solution_size - 1);

        DEBUG_LOGGER->info("Crossover type 2: BREAKPOINT={}", original.routes[random_index]);
        DEBUG_LOGGER->info("p1({})={}", original.fitness, fmt::join(original.routes.begin(), original.routes.end(), " "));
        DEBUG_LOGGER->info("p2({})={}", (*random_it).fitness, fmt::join((*random_it).routes.begin(), (*random_it).routes.end(), " "));

        std::vector<size_t> child(solution_size);
        std::unordered_set<size_t> dubbles, notIncluded;
        int routeCounter = 0;

        // First part
        for (size_t i=0; i <= random_index; i++) {
            if (original.routes[i] == depot_id) routeCounter++;
            if ((*random_it).routes[i] == depot_id) routeCounter--;

            if (notIncluded.find(original.routes[i]) == notIncluded.end()) dubbles.insert(original.routes[i]);
            else notIncluded.erase(original.routes[i]);

            if (dubbles.find((*random_it).routes[i]) == dubbles.end()) notIncluded.insert((*random_it).routes[i]);
            else dubbles.erase((*random_it).routes[i]);

            child[i] = original.routes[i];
        }

        notIncluded.erase(depot_id);
        dubbles.erase(depot_id);

        // Second part
        size_t newElem;
        for (size_t i=random_index + 1; i < solution_size; i++) {

            newElem = (*random_it).routes[i];
            if (dubbles.find(newElem) != dubbles.end()) {
                if (routeCounter < 0) {
                    newElem = depot_id;
                    routeCounter++;
                } else {
                    newElem = *notIncluded.begin();
                    notIncluded.erase(notIncluded.begin());
                }
            }

            if (routeCounter > 0 && newElem == depot_id) {
                newElem = *notIncluded.begin();
                notIncluded.erase(notIncluded.begin());
                routeCounter--;
            }

            child[i] = newElem;

        }

        v = fitness(child, f);

        population_vector.emplace_back(child, f, v);

        DEBUG_LOGGER->info("c1({})={}", f, fmt::join(child.begin(), child.end(), " "));

        return;

    }

}

void GELS_GA::mutate(const Single_Solution &original) {

    if (random_percentage(rng) > mutationProb) return;

    double f;
    bool v;
    std::vector<size_t> mutant(solution_size);
    std::copy(original.routes.begin(), original.routes.end(), mutant.begin());

    if (random_percentage(rng) < 0.5) { // Type 1 mutation: swap 2 random positions

        statistics[2]++;

        size_t pos1 = rand_idx_in_solution(rng), pos2 = rand_idx_in_solution(rng);
        while (pos2 == pos1) { pos2 = rand_idx_in_solution(rng); }

        DEBUG_LOGGER->info("Mutate type 1: x={}\ty={}", mutant[pos1], mutant[pos2]);
        DEBUG_LOGGER->info("p({})={}", original.fitness, fmt::join(mutant.begin(), mutant.end(), " "));

        std::swap(mutant[pos1], mutant[pos2]);
        v = fitness(mutant, f);

        DEBUG_LOGGER->info("c1({})={}", f, fmt::join(mutant.begin(), mutant.end(), " "));

        population_vector.emplace_back(mutant, f, v);
        return;

    } else { // Type 2 mutation: random pivot and mutate route containing pivot

        statistics[3]++;

        size_t pivot = rand_idx_in_solution(rng);
        while (mutant[pivot] == depot_id) {
            pivot = rand_idx_in_solution(rng);
        }

        DEBUG_LOGGER->info("Mutate type 2: pivot={}", mutant[pivot]);
        DEBUG_LOGGER->info("p({})={}", original.fitness, fmt::join(mutant.begin(), mutant.end(), " "));

        std::vector<size_t> new_route;
        // right side of pivot
        int i = 1;
        int element_cnt = 1;
        while (pivot + i < mutant.size() && mutant[pivot + i] != depot_id) {
            new_route.push_back(mutant[pivot + i]);
            i++;
            element_cnt++;
        }

        // Pivot
        new_route.push_back(mutant[pivot]);

        // Left side of pivot
        i = 1;
        auto start_pos = new_route.begin() + element_cnt;
        const int tmp = element_cnt;
        while ((int)(pivot) - i >= 0 && mutant[pivot - i] != 0) {
            new_route.insert(start_pos, mutant[pivot - i]);
            i++;
            element_cnt++;
            start_pos = new_route.begin() + tmp;
        }

        // Replace in actual vector
        for (int j = 1; j <= element_cnt; j++) {
            mutant[pivot - i + j] = new_route[j - 1];
        }

        v = fitness(mutant, f);
        population_vector.emplace_back(mutant, f, v);

        DEBUG_LOGGER->info("p({})={}", f, fmt::join(mutant.begin(), mutant.end(), " "));

    }

}


void GELS_GA::solve_cvrp() {

    Single_Solution CU(std::vector<size_t>(solution_size), 0, false), CA(std::vector<size_t>(solution_size), 0, false);

    create_initial_population(population_size);

    std::sort(population_vector.begin(), population_vector.end());

    for (int generation=1; generation <= iterations; generation++) {

        DEBUG_LOGGER->info("===\ngeneration {}: \n===", generation);

        // Mutate & crossover
        for (int i = 0; i < population_size; i++) {
            DEBUG_LOGGER->info("chromosome size:{}", population_vector[i].routes.size());
            crossover(population_vector[i]);
            mutate(population_vector[i]);
            if (population_vector[i].valid && static_cast<double>(calculate_cost(population_vector[i])) < best_solution.fitness) {
                std::copy(population_vector[i].routes.begin(), population_vector[i].routes.end(), best_solution.routes.begin());
                best_solution.fitness = static_cast<double>(calculate_cost(population_vector[i]));
                best_solution.valid = true;
            }
        }

        // Selection: only keep the best chromosomes
        DEBUG_LOGGER->info("===\nSelection\n===");
        std::sort(population_vector.begin(), population_vector.end());
        population_vector.erase(population_vector.begin() + static_cast<int>(population_size), population_vector.end());

        // Local search: GELS
        DEBUG_LOGGER->info("===\nLocal search\n===");
        if (generation > startLocalOptimisations || generation <= endLocalOptimisations) {
            for (Single_Solution &chromosome: population_vector) {

                DEBUG_LOGGER->info("Chromosome: {}",
                                   fmt::join(chromosome.routes.begin(), chromosome.routes.end(), " "));

                std::copy(chromosome.routes.begin(), chromosome.routes.end(), CU.routes.begin());
                CU.fitness = chromosome.fitness;

                localSearch(chromosome, CU, CA);

                if (CU.fitness < chromosome.fitness) {
                    localOptimizations++;
                    DEBUG_LOGGER->info("Found a better solution jippie :)");
                    std::copy(CU.routes.begin(), CU.routes.end(), chromosome.routes.begin());
                    chromosome.fitness = CU.fitness;
                    chromosome.valid = CU.valid;
                }

            }
        }

        size_t numberOfInfeasibleSolutions = 0;
        for (const Single_Solution& s: population_vector) {
            if (!s.valid) numberOfInfeasibleSolutions++;
        }

        // Update ALPHA parameter used in fitness function
        ALPHA = (numberOfInfeasibleSolutions <= population_size / 2) ? ALPHA / (1 + DELTA) : ALPHA * (1 + DELTA);

        // Recalculate fitness values based on new parameter
        for (Single_Solution &s: population_vector) {
            fitness(s.routes, s.fitness);
        }

        chart_data.emplace_back(calculate_cost(best_solution));

    }

    DEBUG_LOGGER->info("STATISTICS [C1, C2, M1, M2]");
    DEBUG_LOGGER->info("{}", fmt::join(statistics.begin(), statistics.end(), " "));

    print_solution();

    // OBJECTIVE_LOGGER->info("number of successful local searches: {}", localOptimizations);

}

void GELS_GA::localSearch(const Single_Solution &ch, Single_Solution &CU, Single_Solution &CA) {

    double dist, dist_2, F;
    size_t g_j, g_i;
    std::copy(ch.routes.begin(), ch.routes.end(), CU.routes.begin());
    CU.fitness = ch.fitness;
    std::vector<size_t> sortedCustomers;

    int i = -1;
    do {i++;} while (ch.routes[i] == depot_id); // Run until first non-depot element

    for (int j=i+1; j < solution_size - 1; j++) { // Generate CA's

        DEBUG_LOGGER->info("place {} VS solution size {}", j, solution_size);
        if (ch.routes[j] == depot_id) continue;

        g_j = ch.routes[j];
        g_i = ch.routes[i];

        std::copy(ch.routes.begin(), ch.routes.end(), CA.routes.begin());

        // Sort based on mass (j, :)
        sortedCustomers.clear();
        sortedCustomers.push_back(CA.routes[j]);
        for (int k=j+1; k < solution_size; k++) {
            if (CA.routes[k] == depot_id) continue;

            int l = 0;
            while (l < sortedCustomers.size() && mass[g_j*number_of_nodes + CA.routes[k]] > mass[g_j*number_of_nodes + sortedCustomers[l]]) l++;
            sortedCustomers.insert(sortedCustomers.begin() + l, ch.routes[k]);
        }

        DEBUG_LOGGER->info("Sorted after j: {}", fmt::join(sortedCustomers.begin(), sortedCustomers.end(), " "));

        int idx = 0;
        for (int k=j; k < solution_size; k++) {
            if (CA.routes[k] == depot_id) continue;
            std::swap(CA.routes[k], sortedCustomers[idx]);
            idx++;
        }

        CA.valid = fitness(CA.routes, CA.fitness);

        if (CA.fitness < CU.fitness) { // Update mass and velocity matrices

            // F = G * (fitness(CU) - fitness(CA)) / dist(i, j) ^ 2
            dist = (g_i > g_j) ? (*weights)[g_i][g_j] : (*weights)[g_j][g_i];
            dist_2 = dist * dist;
            F = G * (CU.fitness - CA.fitness) / dist_2;

            // velocity(i, j) += F
            // mass(i, j) = (dist(i, j) / velocity(i, j)) * 60
            velocity[g_i*number_of_nodes + g_j] += F;
            mass[g_i*number_of_nodes + g_j] = 60 * (dist / velocity[g_i*number_of_nodes + g_j]);

            // Accept CA as CU
            std::copy(CA.routes.begin(), CA.routes.end(), CU.routes.begin());
            CU.fitness = CA.fitness;
            CU.valid = CA.valid;

        }
        i = j;
    }

}

void GELS_GA::run(size_t it) {
    Solution::run(it);
    OBJECTIVE_LOGGER->info("{}\t({})\t{}\t{}", name(), bestSolution, avgSolution, avgDuration);
    // OBJECTIVE_LOGGER->info("{}\t{}\t{}", ORIGINAL_ALPHA, DELTA, avgSolution);
}

void GELS_GA::reset() {
    Solution::reset();
    ALPHA = ORIGINAL_ALPHA;
    localOptimizations = 0;
    statistics = {0, 0, 0, 0};
}
