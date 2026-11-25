#include <iostream>
#include "assignment.h"

using namespace std;

// Find nearest open charger cost
static int findReturnChargerCost(const Environment& environment, int fromLocation, int* pathBuffer, int pathCapacity) {
    int bestCost = -1;
    for (int i = 0; i < environment.getLocationCount(); i++) {
        const Location* loc = environment.getLocationAt(i);
        if (loc == nullptr) {
            continue;
        }
        if (loc->type != LOCATION_CHARGING || !environment.isStationAvailable(loc->id)) {
            continue;
        }
        if (loc->id == fromLocation) {
            return 0;
        }
        int pathLength = 0;
        int totalCost = 0;
        if (findShortestPath(environment, fromLocation, loc->id, pathBuffer, pathCapacity, pathLength, totalCost)) {
            if (bestCost == -1 || totalCost < bestCost) {
                bestCost = totalCost;
            }
        }
    }
    return bestCost;
}

int assignPendingRequests(const Environment& environment,RobotList& robots,RequestList& requests,int currentTime,int batteryPerCostUnit,int batteryPerWorkStep,Logger* logger) {
    int successfulAssignments = 0;

    int locationCount = environment.getLocationCount();
    if (locationCount <= 0) {
        return 0;
    }

    // Allocate path buffer for findShortestPath calls
    int pathCapacity = locationCount;
    int* pathBuffer = new int[pathCapacity];
    if (pathBuffer == nullptr) {
        return 0;
    }

    // Step 1: Traverse requests in priority order
    // RequestList maintains requests sorted by urgency, reportTime, and request ID.
    RequestNode* reqNode = requests.getHead();
    while (reqNode != nullptr) {
        RescueRequest& req = reqNode->request;

        // Advance pointer now in case request state changes
        reqNode = reqNode->next;

        // Only consider requests that have arrived, are pending, and unassigned
        if (!req.isAvailableAt(currentTime)) {
            continue;
        }

        // Reserve battery to reach charger
        int returnCost = 0;
        if (batteryPerCostUnit > 0) {
            returnCost = findReturnChargerCost(environment, req.locationId, pathBuffer, pathCapacity);
            if (returnCost < 0) {
                continue;
            }
        }

        // Step 2 & 4: Find the best suitable robot candidate
        Robot* bestRobot = nullptr;
        int lowestRouteCost = -1;
        int bestRobotID = -1;
        int suitableCandidates = 0;

        RobotNode* rNode = robots.getHead();
        while (rNode != nullptr) {
            Robot& robot = rNode->robot;
            rNode = rNode->next;

            // Condition 1: Robot must be available
            if (!robot.isAvailable()) {
                continue;
            }

            // Condition 2: Capability matching
            if (!robot.canServe(req.category)) {
                continue;
            }

            // Condition 3: Valid route from robot location to request location
            int pathLength = 0;
            int routeCost = 0;
            bool routeFound = findShortestPath(environment,robot.getCurrentLocation(),req.locationId,pathBuffer,pathCapacity,pathLength,routeCost);

            if (!routeFound) {
                continue;
            }

            // Condition 4: Sufficient battery for journey and rescue work
            // B_required = (C_route * B_cost) + (D_work * B_work)
            int requiredBattery = (routeCost * batteryPerCostUnit) + (req.durationSteps * batteryPerWorkStep) + (returnCost * batteryPerCostUnit);
            if (!robot.hasEnoughBattery(requiredBattery)) {
                continue;
            }

            suitableCandidates++;

            // Tie breaking selection rule:
            // 1. Lowest route cost
            // 2. If equal, lower robot ID
            bool isBetter = false;
            if (bestRobot == nullptr) {
                isBetter = true;
            } else if (routeCost < lowestRouteCost) {
                isBetter = true;
            } else if (routeCost == lowestRouteCost && robot.getId() < bestRobotID) {
                isBetter = true;
            }

            if (isBetter) {
                bestRobot = &robot;
                lowestRouteCost = routeCost;
                bestRobotID = robot.getId();
            }
        }

        // Step 5: Assign the best robot if found
        if (bestRobot != nullptr) {
            // Assign in request first
            if (requests.assignRobot(req.id, bestRobot->getId(), currentTime)) {
                // Assign in robot second
                if (robots.assignRobot(bestRobot->getId(), req.id)) {
                    successfulAssignments++;
                    if (logger != nullptr) {
                        logger->addEntry(currentTime, LOG_ROBOT_ASSIGNED, bestRobot->getId(), req.id, bestRobot->getCurrentLocation());
                        if (suitableCandidates > 1) {
                            logger->addEntry(currentTime, LOG_ASSIGNMENT_CONFLICT, bestRobot->getId(), req.id, bestRobot->getCurrentLocation());
                        }
                    }
                } else {
                    // Rollback request assignment if robot assignment fails
                    requests.clearAssignedRobot(req.id);
                }
            }
        }
    }

    delete[] pathBuffer;
    return successfulAssignments;
}