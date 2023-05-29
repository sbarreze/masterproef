//
// Created by sbarreze on 3/9/23.
//

#ifndef MASTERREPO_CLASSICCW_H
#define MASTERREPO_CLASSICCW_H

#include "../util/common.h"

struct Saving {

    size_t i, j;
    double s;

    friend bool operator< (const struct Saving &a, const struct Saving &b) {
        return a.s > b.s;
    }

    friend std::ostream& operator<<(std::ostream& os, Saving const& arg) {
        return os << "(" << arg.i << "," << arg.j << ")\t\t" << arg.s << std::endl;
    }

    explicit Saving(size_t i, size_t j, double s) {
        this->i = i;
        this->j = j;
        this->s = s;
    }
};


class ClassicCW : public Solution {
protected:
    std::vector<Saving> savingList = std::vector<Saving>();
    std::vector<int> customer_idxs;
public:
    ClassicCW(size_t depot_id, int capacity, size_t number_of_nodes,
              std::vector<Customer> *customers, std::vector<std::vector<double>> *weights)
    : Solution(depot_id, capacity, number_of_nodes, 0, customers, weights){
        best_solution.routes = std::vector<size_t> (1, depot_id);
        customer_idxs = std::vector<int>(number_of_nodes, -1);
    }

    void solve_classic_manner();
    int not_interior(std::vector<size_t> &routes, int idx);
    int find_route_begin(std::vector<size_t> &routes, int idx);
    int find_route_end(std::vector<size_t> &routes, int idx);
    void merge_routes(std::vector<size_t> &routes, int end_i, int start_j);
    void merge_customer_in_route(std::vector<size_t> &routes, size_t customer, int idxI, int interiorIdxI);

    void solve_cvrp() override;
    bool fitness(const std::vector<size_t> &s, double &f) override { return 0; } // Not used
    void create_initial_population(size_t nr_sol_created) override { } // Not used
    void reset() override;
    void run(size_t iterations) override;
    std::string name() override { return "Classic CW"; }
};


#endif //MASTERREPO_CLASSICCW_H
