//
// Created by sbarreze on 5/15/23.
//

#ifndef MASTERREPO_ROUTE_H
#define MASTERREPO_ROUTE_H

#include <vector>
#include "common.h"

using namespace std;

struct Route {
    int start, end;
    size_t load;
    Route(int start, int end, size_t load) : start(start), end(end), load(load) {}
    Route() : start(-1), end(-1), load(0) {}
};


/**
 * Klasse die de routeplanning beschrijft
 */
class RoutePlanning {

private:
    size_t depotId = 0, routeNumber = 0;
    vector<pair<int, int>> customerList; // List of {idxInRoute, routeIdx}
    vector<Route> routeList; // List of {startIdxRoute, EndIdxRoute}
    vector<Customer> *customers = nullptr;

public:
    vector<size_t> routes; // RoutePlanning
    RoutePlanning(vector<size_t> &routes, size_t customerNumber, vector<Customer> *customers);
    RoutePlanning(size_t customerNumber, vector<Customer> *customers);
    RoutePlanning() = default;
    int getCustomerRoute(size_t customer);
    int getCustomerIdx(size_t customer);
    int getRouteStart(size_t route);
    int getRouteEnd(size_t route);
    size_t getCustomerInRoute(int route, int customerIdx);
    size_t getRouteSize(size_t route);
    bool isLastInRoute(size_t customer);
    bool isFirstInRoute(size_t customer);
    void mergeRoutes(size_t route1, size_t route2);
    size_t getRouteLoad(size_t route);

};


#endif //MASTERREPO_ROUTE_H
