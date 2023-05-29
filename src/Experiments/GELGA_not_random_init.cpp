//
// Created by sbarreze on 4/13/23.
//

#include "GELGA_not_random_init.h"

/**
 * PSA INIT
 */
void GELSGA_PSA_init::create_initial_population(size_t nr_sol_created) {
    PSA::create_initial_population(populationSizePSA);
    GELS_GA::create_initial_population(populationSizeRandom + populationSizePSA);
}

bool GELSGA_PSA_init::fitness(const std::vector<size_t> &solution, double &f) {
    return GELS_GA::fitness(solution, f);
}

void GELSGA_PSA_init::solve_cvrp() {
    GELS_GA::solve_cvrp();
}

void GELSGA_PSA_init::run(size_t iterations) {
    GELS_GA::run(iterations);
}

/**
 * ABC INIT
 */
bool GELSGA_ABC_init::fitness(const std::vector<size_t> &solution, double &f) {
    return GELS_GA::fitness(solution, f);
}

void GELSGA_ABC_init::create_initial_population(size_t nr_sol_created) {
    BeeColony::create_initial_population(populationSizeABC);
    GELS_GA::create_initial_population(populationSizeRandom + populationSizeABC);
}

void GELSGA_ABC_init::solve_cvrp() {
    GELS_GA::solve_cvrp();
}

void GELSGA_ABC_init::run(size_t iterations) {
    GELS_GA::run(iterations);
}
