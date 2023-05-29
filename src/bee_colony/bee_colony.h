//
// Created by stijn on 31.10.22.
//

#ifndef MASTERREPO_BEE_COLONY_H
#define MASTERREPO_BEE_COLONY_H

#include "../util/common.h"
#include "random"

class BeeColony : public virtual Solution {

private:
    double ALPHA = 0.1, originalAlpha; // parameter in the cost function
    const double DELTA; // parameter in the cost function
    const size_t TAU; // Number of food sources = number of employed bees = onlookers
    const size_t limit; // Limitation on the duration of exploring non-improving food sources
    const size_t MAX_ITERATIONS;
    int constraint_violation = 0; // Counts how many of the food sources are violating the capacity constraint
    std::uniform_int_distribution<std::mt19937::result_type> rand_gen;
    std::uniform_int_distribution<short> random_neighborhood = std::uniform_int_distribution<short>(1, 7);
    std::mt19937 rng;
    std::vector<size_t> shuffledCustomers;
    std::string nameStr;
    const unsigned short improvedVersion;

public:
    BeeColony(size_t depot_id, int capacity, size_t number_of_nodes, size_t population_size, double ALPHA, double DELTA,
              std::vector<Customer> *customers, std::vector<std::vector<double>> *weights, unsigned short improvedVersion=2)
            : Solution(depot_id, capacity, number_of_nodes, population_size, customers, weights),
             limit(50*number_of_nodes), MAX_ITERATIONS(1000*number_of_nodes), TAU(population_size), ALPHA(ALPHA),
             originalAlpha(ALPHA), DELTA(DELTA), improvedVersion(improvedVersion) {
        std::random_device dev;
        this->rng = std::mt19937(dev());
        this->rand_gen = std::uniform_int_distribution<std::mt19937::result_type>(1, number_of_nodes - 1);
        nameStr = fmt::format("ABC(improv={}, TAU={}, ALPHA={}, DELTA={}, iter={}, limit={})", improvedVersion, TAU, originalAlpha, DELTA, MAX_ITERATIONS, limit);
        shuffledCustomers = std::vector<size_t>(number_of_nodes - 1); // Make a vector of customers
        std::iota(shuffledCustomers.begin(), shuffledCustomers.end(), 1);
    }

    bool fitness(const std::vector<size_t> &solution, double &fitness) override;
    void solve_cvrp() override;
    void create_initial_population(size_t nr_sol_created) override;
    void run(size_t iterations) override;
    void reset() override;

    void neighborhood(std::vector<size_t> &old_food_source);
    void createNewSolution(std::vector<size_t> &shuffledCustomers, std::vector<size_t> &newSolution, bool &valid, double &fitness);

    std::string name() override { return nameStr; };

};

#endif //MASTERREPO_BEE_COLONY_H
