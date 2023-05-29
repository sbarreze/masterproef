//
// Created by sbarreze on 4/13/23.
//

#ifndef MASTERREPO_GELGA_NOT_RANDOM_INIT_H
#define MASTERREPO_GELGA_NOT_RANDOM_INIT_H

#include "../GELSGA/gelsga.h"
#include "../PSA/PSA.h"
#include "../bee_colony/bee_colony.h"

class GELSGA_PSA_init : public GELS_GA, PSA  {

private:
    size_t populationSizeRandom, populationSizePSA;

public:

    GELSGA_PSA_init(size_t depot_id, int capacity, size_t number_of_nodes, size_t population_size_random, size_t population_size_PSA,
                          std::vector<Customer> *customers, std::vector<std::vector<double>> *weights,
                          double alpha, double delta, double crossoverProb=1, double mutationProb=1, size_t iterations=300,
                          size_t endLocalOptimisations=0, size_t startLocalOptimisations=0)
        : Solution(depot_id, capacity, number_of_nodes, population_size_random + population_size_PSA, customers, weights),
          GELS_GA(depot_id, capacity, number_of_nodes, 0, customers, weights, alpha, delta, crossoverProb, mutationProb, iterations, endLocalOptimisations, startLocalOptimisations),
          PSA(depot_id, capacity, number_of_nodes, 0, customers, weights, 0, 0, 0, 0),
          populationSizePSA(population_size_PSA), populationSizeRandom(population_size_random) {}

    void create_initial_population(size_t nr_sol_created) override;
    bool fitness(const std::vector<size_t> &solution, double &f) override;
    void solve_cvrp() override;
    void run(size_t iterations) override;
    void reset() override { GELS_GA::reset(); }
    std::string name() override { return fmt::format(
                "GELS-GA-PSA-INIT\titer={}\tpop_size=random:{} + PSA:{}\talpha={}\tdelta={}\tcrossOver={}\tmutation={}\tlocalSearchEnd={}\tlocalSearchStart={}",
                iterations, populationSizeRandom, populationSizePSA, ORIGINAL_ALPHA, GELS_GA::DELTA, crossoverProb, mutationProb, endLocalOptimisations, startLocalOptimisations);
    }

};

class GELSGA_ABC_init : public GELS_GA, BeeColony  {

private:
    size_t populationSizeRandom, populationSizeABC;

public:

    GELSGA_ABC_init (size_t depot_id, int capacity, size_t number_of_nodes, size_t population_size_random, size_t population_size_ABC,
                          std::vector<Customer> *customers, std::vector<std::vector<double>> *weights,
                          double alpha, double delta, double crossoverProb=1, double mutationProb=1, size_t iterations=300,
                          size_t endLocalOptimisations=0, size_t startLocalOptimisations=0)
            : Solution(depot_id, capacity, number_of_nodes, population_size_random + population_size_ABC, customers, weights),
            GELS_GA(depot_id, capacity, number_of_nodes, 0, customers, weights, alpha, delta, crossoverProb, mutationProb, iterations, endLocalOptimisations, startLocalOptimisations),
            BeeColony(depot_id, capacity, number_of_nodes, 0, alpha, delta, customers, weights, 2),
            populationSizeRandom(population_size_random), populationSizeABC(population_size_ABC) {}

    void create_initial_population(size_t nr_sol_created) override;
    bool fitness(const std::vector<size_t> &solution, double &f) override;
    void solve_cvrp() override;
    void run(size_t iterations) override;
    void reset() override { GELS_GA::reset(); }
    std::string name() override { return fmt::format(
                "GELS-GA-ABC-INIT(iter={}, pop_size=random:{} + ABC:{}, alpha={}, delta={}, crossOver={}, mutation={}, localSearchEnd={}, localSearchStart={})",
                iterations, populationSizeRandom, populationSizeABC, ORIGINAL_ALPHA, GELS_GA::DELTA, crossoverProb, mutationProb, endLocalOptimisations, startLocalOptimisations);
    }

};


#endif //MASTERREPO_GELGA_NOT_RANDOM_INIT_H
