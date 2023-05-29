//
// Created by sbarreze on 2/23/23.
//

#ifndef MASTERPROEF_PSA_H
#define MASTERPROEF_PSA_H

#include <set>
#include <random>
#include "../util/common.h"

class PSA : public virtual Solution {
protected:
    size_t ITERATION_NUMBER, NUMBER_OF_TRIALS, INVALID_TRIALS=0, VALID=0;
    double START_TEMP, END_TEMP;
    std::vector<size_t> COUNT_MATRIX;
public:
    PSA(size_t depot_id, int capacity, size_t number_of_nodes, size_t population_size,
        std::vector<Customer> *customers, std::vector<std::vector<double>> *weights,
        size_t iteration_number, size_t number_of_trials, double start_temp, double end_temp)
        : Solution(depot_id, capacity, number_of_nodes, population_size, customers, weights) {
        ITERATION_NUMBER = iteration_number;
        NUMBER_OF_TRIALS = number_of_trials;
        START_TEMP = start_temp;
        END_TEMP = end_temp;
    }

    void route_development(std::vector<size_t> &result, const Single_Solution &old_solution, double r);

    virtual void solve_cvrp() override;
    virtual void create_initial_population(size_t nr_sol_created) override;
    virtual bool fitness(const std::vector<size_t> &solution, double &fitness) override;
    virtual void run(size_t iterations) override;
    std::string name() override { return fmt::format("PSA\titer={}\tpop_size={}\ttrials={}\tstart_temp={}\tend_temp={}",
        ITERATION_NUMBER, population_size, NUMBER_OF_TRIALS, START_TEMP, END_TEMP);
    }

};

#endif //MASTERPROEF_PSA_H
