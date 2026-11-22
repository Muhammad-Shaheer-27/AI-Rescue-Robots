#include <iostream>
#include <fstream>
#include <cmath>
#include "logger.h"
#include "statistics.h"

using namespace std;

void printTestResult(const char* testName, bool passed) {
    cout << "[" << (passed ? "PASS" : "FAIL") << "] " << testName << endl;
}

bool fileExistsAndNotEmpty(const char* filename) {
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        return false;
    }
    char ch;
    bool hasContent = (bool)(inFile >> ch);
    inFile.close();
    return hasContent;
}

int main() {
    cout << "==============================================\n";
    cout << "   LOGGER & STATISTICS MANAGER UNIT TESTS\n";
    cout << "==============================================\n\n";

    RobotList robots;
    RequestList requests;
    Logger logger;
    Statistics stats;

    // Setup 2 robots and 4 requests
    robots.addRobot(1, 1, CAPABILITY_MEDICAL, 100, 100);
    robots.addRobot(2, 2, CAPABILITY_FIRE, 120, 120);

    requests.addRequestPriority(1, 0, 5, CATEGORY_MEDICAL, 5, 4);
    requests.addRequestPriority(2, 2, 8, CATEGORY_FIRE, 4, 3);
    requests.addRequestPriority(3, 4, 9, CATEGORY_MEDICAL, 3, 5);
    requests.addRequestPriority(4, 6, 10, CATEGORY_FIRE, 2, 2);

    Robot* r1 = robots.findRobot(1);
    Robot* r2 = robots.findRobot(2);

    // Step 0: Multiple events at the same timestep
    robots.assignRobot(1, 1);
    requests.assignRobot(1, 1, 0);
    logger.addEntry(0, LOG_ROBOT_ASSIGNED, 1, 1, 1, -1, 0);
    logger.addEntry(0, LOG_ROAD_BLOCKED, -1, -1, -1, 10, 0);

    // Step 2: Robot 1 moves, arrives, starts Req 1; Robot 2 assigned to Req 2; conflict logged
    r1->moveTo(5, 6, 10);
    logger.addEntry(2, LOG_ROBOT_MOVED, 1, 1, 5, 12, 6);
    logger.addEntry(2, LOG_BATTERY_CONSUMED, 1, 1, 5, -1, 10);
    logger.addEntry(2, LOG_ROBOT_ARRIVED, 1, 1, 5, -1, 0);

    r1->setState(ROBOT_PERFORMING);
    requests.updateStatus(1, REQUEST_IN_PROGRESS);
    logger.addEntry(2, LOG_TASK_STARTED, 1, 1, 5, -1, 0);

    robots.assignRobot(2, 2);
    requests.assignRobot(2, 2, 2);
    logger.addEntry(2, LOG_ROBOT_ASSIGNED, 2, 2, 2, -1, 0);
    logger.addEntry(2, LOG_ASSIGNMENT_CONFLICT, 2, 4, 2, -1, 0);

    // Step 4: Robot 2 moves, road reopens, Robot 2 reassigned
    r2->moveTo(6, 4, 8);
    logger.addEntry(4, LOG_ROBOT_MOVED, 2, 2, 6, 15, 4);
    logger.addEntry(4, LOG_ROAD_REOPENED, -1, -1, -1, 10, 0);

    robots.releaseRobot(2);
    requests.clearAssignedRobot(2);
    logger.addEntry(4, LOG_ROBOT_REASSIGNED, 2, 2, 6, -1, 0);

    robots.assignRobot(2, 2);
    requests.assignRobot(2, 2, 4);
    logger.addEntry(4, LOG_ROBOT_ASSIGNED, 2, 2, 6, -1, 0);

    // Step 6: Robot 1 completes Req 1 (response time = 6 - 0 = 6); Robot 2 arrives & starts Req 2
    robots.completeRobotTask(1);
    requests.updateStatus(1, REQUEST_COMPLETED);
    logger.addEntry(6, LOG_TASK_COMPLETED, 1, 1, 5, -1, 0);

    r2->moveTo(8, 5, 12);
    logger.addEntry(6, LOG_ROBOT_MOVED, 2, 2, 8, 18, 5);
    logger.addEntry(6, LOG_ROBOT_ARRIVED, 2, 2, 8, -1, 0);

    r2->setState(ROBOT_PERFORMING);
    requests.updateStatus(2, REQUEST_IN_PROGRESS);
    logger.addEntry(6, LOG_TASK_STARTED, 2, 2, 8, -1, 0);

    // Step 8: Station down, Req 3 fails, Req 4 urgency raised
    logger.addEntry(8, LOG_STATION_DOWN, -1, -1, 19, -1, 0);
    requests.updateStatus(3, REQUEST_FAILED);
    logger.addEntry(8, LOG_TASK_FAILED, -1, 3, 9, -1, 0);
    requests.updateUrgency(4, 5);
    logger.addEntry(8, LOG_URGENCY_CHANGED, -1, 4, 10, -1, 5);

    // Step 10: Station up, Robot 1 recharges 5 units
    logger.addEntry(10, LOG_STATION_UP, -1, -1, 19, -1, 0);
    r1->startRecharging(true);
    robots.rechargeRobotStep(1, 5, true);
    r1->stopRecharging();
    logger.addEntry(10, LOG_BATTERY_RECHARGED, 1, -1, 19, -1, 5);

    // Step 12: Robot 2 completes Req 2 (response time = 12 - 2 = 10)
    robots.completeRobotTask(2);
    requests.updateStatus(2, REQUEST_COMPLETED);
    logger.addEntry(12, LOG_TASK_COMPLETED, 2, 2, 8, -1, 0);

    // Step 14 & 16: Robot 1 offline then online
    robots.setRobotOffline(1);
    logger.addEntry(14, LOG_ROBOT_OFFLINE, 1, -1, 5, -1, 0);
    robots.setRobotOnline(1);
    logger.addEntry(16, LOG_ROBOT_ONLINE, 1, -1, 5, -1, 0);

    // --- VERIFICATION CHECKS ---

    // 1. Reject invalid entries
    printTestResult("Reject negative timestamp entry", !logger.addEntry(-1, LOG_ROBOT_MOVED));
    printTestResult("Reject invalid event type entry", !logger.addEntry(18, 99));

    // 2. Chronological order check
    bool chronological = true;
    int prevTime = -1;
    LogEntry* cur = logger.getHead();
    while (cur != nullptr) {
        if (cur->time < prevTime) {
            chronological = false;
            break;
        }
        prevTime = cur->time;
        cur = cur->next;
    }
    printTestResult("Chronological event ordering preserved", chronological);

    // 3. Multiple events at the same timestep (Step 0 has 2 events, Step 2 has 6 events)
    int step0Count = 0;
    int step2Count = 0;
    cur = logger.getHead();
    while (cur != nullptr) {
        if (cur->time == 0) step0Count++;
        if (cur->time == 2) step2Count++;
        cur = cur->next;
    }
    printTestResult("Multiple events recorded at same timestep", step0Count == 2 && step2Count == 6);

    // 4. Correct log fields on first two entries
    LogEntry* first = logger.getHead();
    LogEntry* second = (first != nullptr) ? first->next : nullptr;
    bool fieldsCorrect = (first != nullptr && second != nullptr &&
                          first->time == 0 && first->eventType == LOG_ROBOT_ASSIGNED &&
                          first->robotID == 1 && first->requestID == 1 && first->locationID == 1 && first->roadID == -1 &&
                          second->time == 0 && second->eventType == LOG_ROAD_BLOCKED &&
                          second->robotID == -1 && second->roadID == 10);
    printTestResult("Log entry fields stored accurately", fieldsCorrect);

    // Calculate final statistics for a 20-step simulation
    stats.calculate(robots, requests, logger, 20);

    // 5. Request counts: Total=4, Completed=2, Failed=1, Pending=1
    bool reqCountsOk = (stats.getTotalRequests() == 4 &&
                        stats.getCompletedRequests() == 2 &&
                        stats.getFailedRequests() == 1 &&
                        stats.getPendingRequests() == 1);
    printTestResult("Request counts (2 completed, 1 failed, 1 pending)", reqCountsOk);

    // 6. Response times: Req 1 = 6, Req 2 = 10 -> Avg = 8.0, Max = 10
    bool respTimeOk = (fabs(stats.getAverageResponseTime() - 8.0) < 0.01 &&
                       stats.getMaxResponseTime() == 10);
    printTestResult("Average (8.0) and Maximum (10) response times", respTimeOk);

    // 7. Robot aggregation: MoveCost=15, BattConsumed=30, BattRecharged=5, Reassignments=1, Conflicts=1, Util=32.5%
    bool robotAggOk = (stats.getTotalMovementCost() == 15 &&
                       stats.getTotalBatteryConsumed() == 30 &&
                       stats.getTotalBatteryRecharged() == 5 &&
                       stats.getTotalReassignments() == 1 &&
                       stats.getTotalConflicts() == 1 &&
                       fabs(stats.getRobotUtilization() - 32.5) < 0.01);
    printTestResult("Robot statistics and utilization (32.5%) aggregated correctly", robotAggOk);

    // 8. Output file generation
    const char* logFile = "output/sample_simulation_log.csv";
    const char* statsFile = "output/sample_simulation_stats.txt";
    bool logSaved = logger.saveToFile(logFile) && fileExistsAndNotEmpty(logFile);
    bool statsSaved = stats.saveStatistics(statsFile) && fileExistsAndNotEmpty(statsFile);
    printTestResult("Sample log CSV file generated", logSaved);
    printTestResult("Sample statistics TXT file generated", statsSaved);

    logger.displayLogs();
    stats.displayStatistics();

    return 0;
}