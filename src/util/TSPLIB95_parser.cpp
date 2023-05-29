//
// Created by stijn on 29.10.22.
//

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <sstream>
#include <algorithm>
#include "TSPLIB95_parser.h"

// #define verbose

void Parser::parse_problem(const std::string &file_name, std::vector<Customer> &customers, std::vector<std::vector<double>> &edges) {

    // Open file.
    std::ifstream in_file(file_name, std::ios_base::in);

    // If failed to open file, exit the program
    if (!in_file.is_open()) {
        std::cout << "Failed to open file " << file_name << std::endl;
        exit(1);
    }

    // Start reading line by line and parse the file.
    std::string line, command;

    // Parse commands
    while (std::getline(in_file, line)) {

        // Extract command from line and possible data
        size_t separator = line.find(':');
        command = line.substr(0, separator);
        command.erase(std::remove_if(command.begin(), command.end(), isspace), command.end());

        // TODO: remove spaces by comment in demand

        if (command == "DIMENSION") parse_dimension(line, separator, customers);
        else if (command == "EDGE_WEIGHT_TYPE") edge_weight_type = line.substr(separator + 2);
        else if (command == "EDGE_WEIGHT_FORMAT") edge_format = line.substr(separator + 2);
        else if (command == "CAPACITY") capacity = std::stoi(line.substr(separator + 2));
        else if (command == "NODE_COORD_SECTION") parse_node_coords(in_file, customers);
        else if (command == "DEMAND_SECTION") parse_demands(in_file, customers);
        else if (command == "DEPOT_SECTION") depot_id = parse_depots(in_file);
        else if (command == "EDGE_WEIGHT_SECTION") parse_edges(in_file, customers, edges);
#ifdef verbose
        else std::cout << command << " not implemented!" << std::endl;
#endif
    }

    // If edge_weight_type was explicit, calculate the edges
    // TODO: in_file not used now, maybe make separate function
    if (edge_weight_type != "EXPLICIT") parse_edges(in_file, customers, edges);

    // Close file
    in_file.close();

}

bool Parser::parse_demands(std::ifstream &in_file, std::vector<Customer> &customers) {

    std::string line;
    int id, demand;

    for (int i=0; i < number_of_nodes; i++) {

        std::getline(in_file, line);

        std::stringstream line_stream(line);
        line_stream >> id >> demand;

        if (!in_file || !line_stream) {
            std::cout << "Failed to parse demands." << std::endl;
            exit(5); // Failed to parse
        }

        customers[id - 1].demand = demand;

    }

    return true; // Should never get here
}

bool Parser::parse_node_coords(std::ifstream &in_file, std::vector<Customer> &customers) {

    std::string line;
    int id;
    double x, y;

    for(int i=0; i < number_of_nodes; i++) {

        std::getline(in_file, line);
        std::stringstream line_stream(line);

        if (!in_file || !line_stream) {
            std::cout << "Failed to parse node coordinates." << std::endl;
            exit(5); // Failed to parse
        }

        line_stream >> id >> x >> y;
        customers[id - 1].position = {x, y};
    }


    return true;
}

int Parser::parse_depots(std::ifstream &in_file) {

    std::vector<int> depots;
    std::string line;

    while (std::getline(in_file, line)) {
        line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
        if (line == "-1") return depots[0];
        depots.push_back(std::stoi(line) - 1);
    }

    return -1; // Should never get here
}

bool Parser::parse_dimension(const std::string &line, size_t separator, std::vector<Customer> &customers) {

    try {
        number_of_nodes = std::stoi(line.substr(separator + 1));
    } catch (std::exception &e) {
        std::cout << "Could not parse \"" << line << "\"!" << std::endl;
        exit(5);
    }

    for (int i=0; i < number_of_nodes; i++)
        customers.emplace_back();

    return true;
}

bool Parser::parse_edges(std::ifstream &in_file, const std::vector<Customer> &customers, std::vector<std::vector<double>> &edges) {

    edge_weight_type.erase(std::remove_if(edge_weight_type.begin(), edge_weight_type.end(), isspace), edge_weight_type.end());
    if (edge_weight_type == "EUC_2D") {

        for (int x=0; x < number_of_nodes; x++) {

            int y = 0;
            edges.emplace_back(); // add new row to edges

            while (y <= x) {
                Customer c1 = customers[x];
                Customer c2 = customers[y];
                double dx = c1.position.x - c2.position.x;
                double dy = c1.position.y - c2.position.y;
                double euc_d = std::sqrt((dx * dx) + (dy * dy));
                // euc_d = round(euc_d);
                edges[x].push_back(euc_d);
                y++;
            }
        }

        return true;

    }

    edge_format.erase(std::remove_if(edge_format.begin(), edge_format.end(), isspace), edge_format.end());

    if (edge_format == "LOWER_ROW") {

        edges.emplace_back(1, 0); // edge_weight(0, 0) == 0

        std::string line;
        for (int x=1; x < number_of_nodes; x++) {

            edges.emplace_back(); // add new row to edges
            std::getline(in_file, line);
            std::stringstream line_stream(line);
            double edge_weight;

            while (line_stream >> edge_weight)
                edges[x].push_back(edge_weight);

            edges[x].push_back(0);

        }
    }

    return false;
}
