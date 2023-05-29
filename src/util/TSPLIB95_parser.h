//
// Created by stijn on 29.10.22.
//

#ifndef MASTERREPO_TSPLIB95_PARSER_H
#define MASTERREPO_TSPLIB95_PARSER_H

#include "vector"
#include "string"
#include "common.h"

class Parser {
    /*
     *
     */

private:
    std::vector<int> demands; // vector that contains all demands of all customers
    std::string edge_weight_type; // Type of how edge-distances can be calculated from the data
    std::string edge_format; // If edge_weight_type=EXPLICIT, how are these formatted


public:
    int number_of_nodes; // The number of nodes
    int depot_id; // id of node that will behave as depot
    int capacity; // Capacity of the truck
    bool parse_node_coords(std::ifstream &in_file, std::vector<Customer> &customers);
    bool parse_demands(std::ifstream &in_file, std::vector<Customer> &customers);
    int parse_depots(std::ifstream &in_file);
    bool parse_dimension(const std::string &line, size_t separator, std::vector<Customer> &customers);
    bool parse_edges(std::ifstream &in_file, const std::vector<Customer> &customers, std::vector<std::vector<double>> &edges);
    void parse_problem(const std::string &file_name, std::vector<Customer> &customers, std::vector<std::vector<double>> &edges);
};

#endif //MASTERREPO_TSPLIB95_PARSER_H
