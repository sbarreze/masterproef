//
// Created by stijn on 25.11.22.
//

#ifndef MASTERREPO_GELSGA_H
#define MASTERREPO_GELSGA_H

#include <random>
#include <set>
#include <utility>
#include "../util/common.h"


class GELS_GA : public virtual Solution {
protected:
    double crossoverProb, mutationProb;
    const double G = 6.672; // gravitational constant
    double *velocity, *mass; // Matrices used in GELS
    std::vector<size_t> statistics = {0, 0, 0, 0}; // C1, C2, M1, M2
    size_t localOptimizations=0, endLocalOptimisations, startLocalOptimisations;
    const size_t iterations;
    double ALPHA, DELTA, ORIGINAL_ALPHA;

public:
    GELS_GA(size_t depot_id, int capacity, size_t number_of_nodes, size_t population_size,
            std::vector<Customer> *customers, std::vector<std::vector<double>> *weights,
            double alpha, double delta, double crossoverProb=1, double mutationProb=1, size_t iterations=300,
            size_t endLocalOptimisations=0, size_t startLocalOptimisations=0)
        : Solution(depot_id, capacity, number_of_nodes, population_size, customers, weights),
        crossoverProb(crossoverProb), mutationProb(mutationProb), ORIGINAL_ALPHA(alpha), iterations(iterations),
        ALPHA(alpha), DELTA(delta),
        startLocalOptimisations(startLocalOptimisations), endLocalOptimisations(endLocalOptimisations) {

        // Allocate one block of memory for the 2D array but saved as 1D
        velocity = new double[number_of_nodes * number_of_nodes];
        mass = new double[number_of_nodes * number_of_nodes];

        // Initialization
        double dist;
        for (int i=0; i < number_of_nodes; i++) {
            for (int j=0; j < number_of_nodes; j++) {
                velocity[i*number_of_nodes + j] = (i == j) ? 0 : 100;
                dist = (i > j) ? (*weights)[i][j] : (*weights)[j][i];
                mass[i*number_of_nodes + j] = (i == j) ? 0 :  0.6 * dist;
            }
        }
    }

    virtual ~GELS_GA() {
        delete[] velocity;
        delete[] mass;
    }

    void crossover(const Single_Solution &original);
    void mutate(const Single_Solution &original);
    void localSearch(const Single_Solution &ch, Single_Solution &CU, Single_Solution &CA);

    void create_initial_population(size_t nr_sol_created) override;
    bool fitness(const std::vector<size_t> &solution, double &f) override;
    void solve_cvrp() override;
    void run(size_t iterations) override;
    void reset() override;
    std::string name() override { return fmt::format(
            "GELS-GA\titer={}\tpop_size={}\talpha={}\tdelta={}\tcrossOver={}\tmutation={}\tlocalSearchEnd={}\tlocalSearchStart={}",
             iterations, population_size, ORIGINAL_ALPHA, DELTA, crossoverProb, mutationProb, endLocalOptimisations, startLocalOptimisations);
    }
};

#endif //MASTERREPO_GELSGA_H
