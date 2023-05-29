#include <iostream>
#include <vector>
#include <algorithm>
#include <spdlog/spdlog.h>
#include <filesystem>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <fmt/format.h>
#include "src/util/common.h"
#include "src/util/TSPLIB95_parser.h"
#include "src/bee_colony/bee_colony.h"
#include "src/GELSGA/gelsga.h"
#include "src/PSA/PSA.h"
#include "src/Clarck-Wright/ClassicCW.h"
#include "src/Clarck-Wright/ImprovedCW.h"
#include "src/Experiments/GELGA_not_random_init.h"

namespace fs = std::filesystem;

void addAllFilesInFolder(const std::string &folderPath, std::vector<std::string> *benchmarkData) {
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            benchmarkData->push_back(folderPath + entry.path().filename().string());
        }
    }
}

int main(int argc, char *argv[]) {

    /*
     * Register LOGGERS
     */
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto debugLogger = std::make_shared<spdlog::logger>("debug_logger", console_sink);
    debugLogger->set_level(spdlog::level::off);
    debugLogger->set_pattern("%v"); // Just show txt

    auto gifLogger = std::make_shared<spdlog::logger>("gif_logger", console_sink);
    gifLogger->set_level(spdlog::level::off);
    gifLogger->set_pattern("%v"); // Just show txt

    auto chartLogger = std::make_shared<spdlog::logger>("chart_logger", console_sink);
    chartLogger->set_level(spdlog::level::off);
    chartLogger->set_pattern("%v"); // Just show txt

    auto visualLogger = std::make_shared<spdlog::logger>("visual_logger", console_sink);
    visualLogger->set_level(spdlog::level::off);
    visualLogger->set_pattern("%v"); // Just show txt

    auto objectiveLogger = std::make_shared<spdlog::logger>("objective_logger", console_sink);
    objectiveLogger->set_level(spdlog::level::off);
    objectiveLogger->set_pattern("%v"); // Just show txt

    spdlog::register_logger(debugLogger);
    spdlog::register_logger(gifLogger);
    spdlog::register_logger(chartLogger);
    spdlog::register_logger(visualLogger);
    spdlog::register_logger(objectiveLogger);

    std::vector<std::string> benchmark_data = {};

    int ITERATION_NR = 1;
    bool algorithms[9] = {false, false, false, false, false, false, false, false, false};
    size_t nrOfAlgorithms = 0;

    /*
     * PARSE PARAMETERS
     * TODO: use library to this
     */
    try {
        for (int i = 1; i < argc; i++) {
            std::string arg = std::string(argv[i]);
            if (arg == "--debug") { debugLogger->set_level(spdlog::level::info); }
            if (arg == "--gif") { gifLogger->set_level(spdlog::level::info); }
            if (arg == "--visual") { visualLogger->set_level(spdlog::level::info); }
            if (arg == "--chart") { chartLogger->set_level(spdlog::level::info); }
            if (arg == "--objective") { objectiveLogger->set_level(spdlog::level::info); }
            if (arg == "-data") { addAllFilesInFolder(std::string(argv[i + 1]), &benchmark_data); i++; }
            if (arg == "-i") { ITERATION_NR = std::stoi(std::string(argv[i + 1])); i++; }
            if (arg == "-ABC") { algorithms[0] = true; nrOfAlgorithms++; }
            if (arg == "-GELSGA") { algorithms[1] = true; nrOfAlgorithms++; }
            if (arg == "-PSA") { algorithms[2] = true; nrOfAlgorithms++; }
            if (arg == "-CW") { algorithms[3] = true; nrOfAlgorithms++; }
            if (arg == "-ICW") { algorithms[4] = true; nrOfAlgorithms++; }
            if (arg == "-ABCExp") { algorithms[5] = true; nrOfAlgorithms++; }
            if (arg == "-GELSGAExp") { algorithms[6] = true; nrOfAlgorithms++; }
            if (arg == "-PSAExp") { algorithms[7] = true; nrOfAlgorithms++; }
            if (arg == "-ICWExp") { algorithms[8] = true; nrOfAlgorithms++; }
        }
    } catch (int e) {
        std::cout << fmt::format("Error {}: Failed to parse command correctly.\nUsage: ./masterproef -data <folder_to_data> -i <nr of iterations> <-name algorithm-> --debug/--gif/--visual/--chart/--objective", e) << std::endl;
        exit(e);
    }

    std::vector<Solution *> solutions;
    visualLogger->info(nrOfAlgorithms); // Only for graph representation

    /*
     * EXECUTE ALGORITHMS
     */
    for (const std::string &file_name : benchmark_data) {

        solutions = {};

        std::vector<Customer> customers;
        std::vector<std::vector<double>> weights;

        Parser parser = Parser();
        parser.parse_problem(file_name, customers, weights);

        std::cout << file_name << std::endl;
        for (Customer c : customers) std::cout << c << " ";
        std::cout << std::endl;

        /**
         * ORIGINAL ALGORITHMS PROPOSED BY THE PAPERS
         */
        if (algorithms[0]) {
            solutions.push_back(new BeeColony(
                    parser.depot_id, parser.capacity, parser.number_of_nodes,
                    25, 0.1, 0.001, &customers, &weights, 2)
            );
        }
        if (algorithms[1]) {
            solutions.push_back(new GELS_GA(
                    parser.depot_id, parser.capacity, parser.number_of_nodes, 150, &customers, &weights,
                    1, 0, 1, 1, 300)
            );
        }
        if (algorithms[2]) {
            solutions.push_back(new PSA(
                    parser.depot_id, parser.capacity, parser.number_of_nodes, 20, &customers, &weights, 400,
                    40, 100, 0.00001)
            );
        }
        if (algorithms[3]) {
            solutions.push_back(new ClassicCW(
                    parser.depot_id, parser.capacity, parser.number_of_nodes, &customers, &weights)
            );
        }
        if (algorithms[4]) {
            solutions.push_back(new ImprovedCW(
                    parser.depot_id, parser.capacity, parser.number_of_nodes, &customers, &weights,3000, 1000)
            );
        }

        /**
         * BEST PERFORMING ALGORITHMS
        */

        if (algorithms[5]) {
            const double bestDelta = parser.number_of_nodes < 50 ? 0.01 : parser.number_of_nodes < 150 ? 0.0001 : 0.00008;
            solutions.push_back(new BeeColony(
                    parser.depot_id, parser.capacity, parser.number_of_nodes,
                    70, 1.2, bestDelta, &customers, &weights, 2)
            );
        }

        if (algorithms[6]) {
            solutions.push_back(new GELSGA_ABC_init(
                    parser.depot_id, parser.capacity, parser.number_of_nodes, 100, 200, &customers, &weights,
                    1, 0.1, 1, 1, 2000, 175, 1975
            ));
        }

        if (algorithms[7]) {
            solutions.push_back(new PSA(
                    parser.depot_id, parser.capacity, parser.number_of_nodes, 5, &customers, &weights, 10000,
                    1000, 200, 0.1)
            );
        }

        if (algorithms[8]) {
            solutions.push_back(new ImprovedCW(
                    parser.depot_id, parser.capacity, parser.number_of_nodes, &customers, &weights,10000, 5000, 2, 6)
            );
        }

        // Run algorithms
        for (auto sol : solutions) {
            sol->run(ITERATION_NR);
            delete sol;
        }

        solutions.clear();

    }

    return 0;
}
