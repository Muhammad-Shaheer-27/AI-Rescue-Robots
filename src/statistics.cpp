#include "statistics.h"
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

Statistics::Statistics() {
    reset();
}

void Statistics::reset() {
    totalSimulationSteps = 0;
    totalRobots = 0;

    totalRequests = 0;
    completedRequests = 0;
    failedRequests = 0;
    pendingRequests = 0;
    assignedRequests = 0;
    inProgressRequests = 0;

    averageResponseTime = 0.0;
    maxResponseTime = 0;

    robotUtilization = 0.0;
    totalMovementCost = 0;
    totalBatteryConsumed = 0;
    totalBatteryRecharged = 0;
    totalReassignments = 0;
    totalConflicts = 0;
}

void Statistics::calculate( // Aggregates final request counts, robot totals, response times, and utilization for output
    const RobotList& robots,
    const RequestList& requests,
    const Logger& logger,
    int simSteps
) {
    reset();

    totalSimulationSteps = (simSteps > 0) ? simSteps : 0;
    totalRobots = robots.getCount();

    //reuse RequestList counts
    totalRequests = requests.getCount();
    completedRequests = requests.getCountByStatus(REQUEST_COMPLETED);
    failedRequests = requests.getCountByStatus(REQUEST_FAILED);
    pendingRequests = requests.getCountByStatus(REQUEST_PENDING);
    assignedRequests = requests.getCountByStatus(REQUEST_ASSIGNED);
    inProgressRequests = requests.getCountByStatus(REQUEST_IN_PROGRESS);

    // reuse RobotList aggregated stats
    totalMovementCost = robots.getTotalMovementCostAll();
    totalBatteryConsumed = robots.getTotalBatteryConsumedAll();
    totalBatteryRecharged = robots.getTotalBatteryRechargedAll();
    totalReassignments = robots.getTotalReassignmentsAll();

    //now count assignment conflicts from Logger
    totalConflicts = logger.getEventCount(LOG_ASSIGNMENT_CONFLICT);

    // calcul8 response times and work steps from Logger + RequestList
    long long totalResponseTime = 0;
    int completedLogCount = 0;
    int totalWorkSteps = 0;

    LogEntry* logNode = logger.getHead();
    while (logNode != nullptr) {
        if (logNode->eventType == LOG_TASK_COMPLETED && logNode->requestID != -1) {   //if job is completed 
            // Find matching request to get its reportTime
            RequestNode* reqNode = requests.getHead();
            while (reqNode != nullptr) {
                if (reqNode->request.id == logNode->requestID) {
                    int responseTime = logNode->time - reqNode->request.reportTime;    //calculate its completion time - report time
                    if (responseTime < 0) {
                        responseTime = 0;
                    }
                    totalResponseTime += responseTime;
                    completedLogCount++;
                    if (responseTime > maxResponseTime) {
                        maxResponseTime = responseTime;
                    }
                    if (reqNode->request.durationSteps > 0) {
                        totalWorkSteps += reqNode->request.durationSteps;
                    }
                    break;
                }
                reqNode = reqNode->next;
            }
        }
        logNode = logNode->next;
    }

    if (completedLogCount > 0) {
        averageResponseTime = (double)totalResponseTime / completedLogCount;
    }

    //calcul8 robot utilization using movement steps + work steps
    int totalMoveSteps = 0;
    RobotNode* rNode = robots.getHead();
    while (rNode != nullptr) {
        totalMoveSteps += rNode->robot.getTotalStepsMoved();
        rNode = rNode->next;
    }

//utilization = (totalMovementSteps+TotalWorkSteps)/totalrobots+totalSimulationSteps

    int totalUsefulSteps = totalMoveSteps + totalWorkSteps;
    int totalPossibleSteps = totalRobots * totalSimulationSteps;
    if (totalPossibleSteps > 0) {
        robotUtilization = (totalUsefulSteps * 100.0) / totalPossibleSteps; 
        if (robotUtilization > 100.0) {
            robotUtilization = 100.0;
        }
    }
}

int Statistics::getTotalSimulationSteps() const { return totalSimulationSteps; }
int Statistics::getTotalRobots() const { return totalRobots; }
int Statistics::getTotalRequests() const { return totalRequests; }
int Statistics::getCompletedRequests() const { return completedRequests; }
int Statistics::getFailedRequests() const { return failedRequests; }
int Statistics::getPendingRequests() const { return pendingRequests; }
int Statistics::getAssignedRequests() const { return assignedRequests; }
int Statistics::getInProgressRequests() const { return inProgressRequests; }
double Statistics::getAverageResponseTime() const { return averageResponseTime; }
int Statistics::getMaxResponseTime() const { return maxResponseTime; }
double Statistics::getRobotUtilization() const { return robotUtilization; }
int Statistics::getTotalMovementCost() const { return totalMovementCost; }
int Statistics::getTotalBatteryConsumed() const { return totalBatteryConsumed; }
int Statistics::getTotalBatteryRecharged() const { return totalBatteryRecharged; }
int Statistics::getTotalReassignments() const { return totalReassignments; }
int Statistics::getTotalConflicts() const { return totalConflicts; }

void Statistics::displayStatistics() const {
    cout << fixed << setprecision(2);
    cout << "\n========== FINAL SIMULATION STATISTICS ==========\n";
    cout << "Total Simulation Steps: " << totalSimulationSteps << "\n";
    cout << "Total Robots:           " << totalRobots << "\n";
    cout << "-------------------------------------------------\n";
    cout << "Total Requests:         " << totalRequests << "\n";
    cout << "  Completed Requests:   " << completedRequests << "\n";
    cout << "  Failed Requests:      " << failedRequests << "\n";
    cout << "  Pending Requests:     " << pendingRequests << "\n";
    cout << "  Assigned Requests:    " << assignedRequests << "\n";
    cout << "  In-Progress Requests: " << inProgressRequests << "\n";
    cout << "-------------------------------------------------\n";
    cout << "Average Response Time:  " << averageResponseTime << " steps\n";
    cout << "Maximum Response Time:  " << maxResponseTime << " steps\n";
    cout << "Robot Utilization:      " << robotUtilization << "%\n";
    cout << "Total Movement Cost:    " << totalMovementCost << "\n";
    cout << "Total Battery Consumed: " << totalBatteryConsumed << "\n";
    cout << "Total Battery Recharged:" << totalBatteryRecharged << "\n";
    cout << "Total Reassignments:    " << totalReassignments << "\n";
    cout << "Total Conflicts:        " << totalConflicts << "\n";
    cout << "=================================================\n";
}

bool Statistics::saveStatistics(const char* filename) const {
    if (filename == nullptr) {
        return false;
    }

    ofstream outFile(filename);
    if (!outFile.is_open()) {
        return false;
    }

    outFile << fixed << setprecision(2);
    outFile << "========== FINAL SIMULATION STATISTICS ==========\n";
    outFile << "Total Simulation Steps: " << totalSimulationSteps << "\n";
    outFile << "Total Robots: " << totalRobots << "\n";
    outFile << "Total Requests: " << totalRequests << "\n";
    outFile << "Completed Requests: " << completedRequests << "\n";
    outFile << "Failed Requests: " << failedRequests << "\n";
    outFile << "Pending Requests: " << pendingRequests << "\n";
    outFile << "Assigned Requests: " << assignedRequests << "\n";
    outFile << "In-Progress Requests: " << inProgressRequests << "\n";
    outFile << "Average Response Time: " << averageResponseTime << "\n";
    outFile << "Maximum Response Time: " << maxResponseTime << "\n";
    outFile << "Robot Utilization (%): " << robotUtilization << "\n";
    outFile << "Total Movement Cost: " << totalMovementCost << "\n";
    outFile << "Total Battery Consumed: " << totalBatteryConsumed << "\n";
    outFile << "Total Battery Recharged: " << totalBatteryRecharged << "\n";
    outFile << "Total Reassignments: " << totalReassignments << "\n";
    outFile << "Total Conflicts: " << totalConflicts << "\n";
    outFile << "=================================================\n";

    outFile.close();
    return true;
}