//
// Created by sbarreze on 5/15/23.
//

#include "Route.h"

RoutePlanning::RoutePlanning(vector<size_t> &routes, const size_t customerNumber, vector<Customer> *customers)
    : customerList(customerNumber, {-1, -1}), routeList(customerNumber - 1), customers(customers) {

    this->routes = std::move(routes);

    size_t start = 0;
    for (size_t i = 1; i < this->routes.size(); i++) {
        if (this->routes[i] == depotId) {
            routeList[routeNumber].start = static_cast<int>(start);
            routeList[routeNumber].end = static_cast<int>(i);
            start = i;
            routeNumber++;
        } else {
            customerList[this->routes[i]] = {i - start, routeNumber};
            routeList[routeNumber].load += (*customers)[this->routes[i]].demand;
        }
    }

}

RoutePlanning::RoutePlanning(size_t customerNumber, vector<Customer> *customers)
        : customerList(customerNumber, {-1, -1}), routeList(customerNumber - 1), customers(customers) {

    const size_t routesSize = 2 * customerNumber - 1;
    routes.reserve(routesSize);
    size_t c = 1;
    routes.push_back(depotId);
    for (size_t i=1; i < routesSize; i += 2) {
        routes.insert(routes.end(), {c, depotId});
        routeList[routeNumber].start = static_cast<int>(i) - 1;
        routeList[routeNumber].end = static_cast<int>(i) + 1;
        routeList[routeNumber].load = (*customers)[c].demand;
        customerList[c] = {1, routeNumber};
        routeNumber++;
        c++;
    }

}

int RoutePlanning::getCustomerRoute(size_t customer) {
    return customerList[customer].second;
}

int RoutePlanning::getCustomerIdx(size_t customer) {
    return customerList[customer].first;
}

int RoutePlanning::getRouteStart(size_t route) {
    return routeList[route].start;
}

int RoutePlanning::getRouteEnd(size_t route) {
    return routeList[route].end;
}

bool RoutePlanning::isLastInRoute(size_t customer) {
    int route = getCustomerRoute(customer);
    return getCustomerIdx(customer) == getRouteSize(route);
}

bool RoutePlanning::isFirstInRoute(size_t customer) {
    return getCustomerIdx(customer) == 1;
}

size_t RoutePlanning::getRouteSize(size_t route) {
    return routeList[route].end - routeList[route].start - 1;
}

void RoutePlanning::mergeRoutes(size_t route1, size_t route2) {

    if (getRouteStart(route1) < getRouteStart(route2)) {
        for (int j = getRouteEnd(route1) - 1; j > getRouteStart(route1); j--) {
            routes.insert(routes.begin() + getRouteStart(route2) + 1, routes[j]);
        }
        routes.erase(routes.begin() + getRouteStart(route1), routes.begin() + getRouteEnd(route1));
    } else {
        for (int j = getRouteEnd(route2) - 1; j > getRouteStart(route2); j--) {
            routes.insert(routes.begin() + getRouteEnd(route1), routes[j]);
        }
        routes.erase(routes.begin() + getRouteStart(route2), routes.begin() + getRouteEnd(route2));
    }

    if (getRouteStart(route1) > getRouteStart(route2)) swap(route1, route2);

    // Update route 2
    routeList[route2].start = getRouteStart(route2) - static_cast<int>(getRouteSize(route1)) - 1;
    routeList[route2].end = getRouteEnd(route2) - 1;
    routeList[route2].load = getRouteLoad(route1) + getRouteLoad(route2);

    int idxInRoute = 1;
    for (int j = getRouteStart(route2) + 1; j < getRouteEnd(route2); j++) { // Update customers in route
        customerList[routes[j]] = {idxInRoute, route2};
        idxInRoute++;
    }


    // Update routes
    int newStart = getRouteStart(route1);
    for (size_t r = route1 + 1; r < route2; r++) {
        if (getRouteStart(r) < 0) continue; // Route does not exists

        int w = static_cast<int>(getRouteSize(r));
        routeList[r].start = newStart;
        routeList[r].end = newStart + w + 1;
        newStart = routeList[r].end;
    }

    newStart = routeList[route2].end;
    for (size_t r = route2 + 1; r < routeNumber; r++) {
        if (getRouteStart(r) < 0) continue; // Route does not exists

        int w = static_cast<int>(getRouteSize(r));
        routeList[r].start = newStart;
        routeList[r].end = newStart + w + 1;
        newStart = routeList[r].end;
    }

    // Route 1 is removed
    routeList[route1] = Route();

}

size_t RoutePlanning::getCustomerInRoute(int route, int customerIdx) {
    return routes[getRouteStart(route) + customerIdx];
}

size_t RoutePlanning::getRouteLoad(size_t route) {
    return routeList[route].load;
}
