//
// Created by sbarreze on 3/11/23.
//

#ifndef MASTERREPO_IMPROVEDCW_H
#define MASTERREPO_IMPROVEDCW_H


#include <unordered_set>
#include "ClassicCW.h"
#include "../util/Route.h"

class ImprovedCW : public ClassicCW {
protected:
    RoutePlanning currRoutes;
    std::vector<Saving> newSavingList;
    std::uniform_int_distribution<int> random_tournament_size;
    int MAX_NUMBER_ITERATIONS, MAX_NUMBER_ITERATIONS_NO_IMPROVEMENT;
    int firstImprovement = -1, lastImprovement = 0, nr_of_improvements=0;
public:
    ImprovedCW(size_t depot_id, int capacity, size_t number_of_nodes, std::vector<Customer> *customers,
               std::vector<std::vector<double>> *weights,  int max_it, int max_no_imp, int minTournamentSize=3, int maxTournamentSize=8)
               : ClassicCW(depot_id, capacity, number_of_nodes, customers, weights),
               MAX_NUMBER_ITERATIONS(max_it), MAX_NUMBER_ITERATIONS_NO_IMPROVEMENT(max_no_imp) {
        random_tournament_size = std::uniform_int_distribution<int>(minTournamentSize, maxTournamentSize);
    }

    void create_new_savinglist();
    void build_routes();

    void solve_cvrp() override;
    bool fitness(const std::vector<size_t> &routes, double &fitness) override;
    void create_initial_population(size_t nr_sol_created) override { } // Not used
    std::string name() override { return fmt::format("Improved CW\titer={}\tpop_size={}\tmax_no_imp={}\ttournament=({},{})",
        MAX_NUMBER_ITERATIONS, population_size, MAX_NUMBER_ITERATIONS_NO_IMPROVEMENT, random_tournament_size.min(), random_tournament_size.max());
    }
    void run(size_t iter) override;
    void reset() override;
};


#endif //MASTERREPO_IMPROVEDCW_H
