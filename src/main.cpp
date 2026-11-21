#include <iostream>
#include "scenario_loader.h"

using namespace std;

// Helper to print pass/fail cleanly for each test
void printTestResult(const char* testName, bool passed) {
    cout << "[" << (passed ? "PASS" : "FAIL") << "] " << testName << endl;
}

void runRequestManagerTests() {
    cout << "\n====================================\n";
    cout << "     REQUEST MANAGER UNIT TESTS\n";
    cout << "====================================\n\n";

    RequestList testList;

    // 1. Add simultaneous requests (same reportTime = 5) and one future request (time = 20)
    testList.addRequestPriority(103, 5, 10, CATEGORY_MEDICAL, 3, 4);
    testList.addRequestPriority(101, 5, 12, CATEGORY_FIRE, 3, 3);    // Same urgency & time as 103, lower ID
    testList.addRequestPriority(102, 5, 14, CATEGORY_SEARCH, 5, 6);  // Higher urgency
    testList.addRequestPriority(104, 20, 15, CATEGORY_SUPPLY, 5, 2); // Future request (time 20)

    // 2. Time-gated availability checks
    printTestResult("Request 102 hidden at time 4", !testList.isRequestAvailable(102, 4));
    printTestResult("Request 102 available at time 5", testList.isRequestAvailable(102, 5));
    printTestResult("Future request 104 hidden at time 10", !testList.isRequestAvailable(104, 10));
    printTestResult("Future request 104 available at time 20", testList.isRequestAvailable(104, 20));

    // 3. Priority retrieval with simultaneous requests at time 5
    RescueRequest* top = testList.getHighestPriorityPending(5);
    printTestResult("Highest priority at time 5 is ID 102 (urgency 5)", top != nullptr && top->id == 102);

    RescueRequest* pending[10];
    int found = testList.getPendingRequests(5, pending, 10);
    // Expected order at time 5: 102 (urg 5), 101 (urg 3, id 101), 103 (urg 3, id 103). 104 is hidden.
    bool orderCorrect = (found == 3 && pending[0]->id == 102 && pending[1]->id == 101 && pending[2]->id == 103);
    printTestResult("Simultaneous requests retrieved in exact priority order", orderCorrect);

    // 4. Robot assignment and duplicate blocking
    printTestResult("Reject assigning robot to future request 104 at time 5", !testList.assignRobot(104, 1, 5));
    printTestResult("Assign robot 1 to request 102 at time 5", testList.assignRobot(102, 1, 5));
    printTestResult("Reject duplicate robot on same request 102", !testList.assignRobot(102, 2, 5));
    printTestResult("Reject same robot 1 on second request 101", !testList.assignRobot(101, 1, 5));

    // 5. Clear robot assignment and reassign
    printTestResult("Clear robot from request 102", testList.clearAssignedRobot(102));
    printTestResult("Request 102 is available again after clear", testList.isRequestAvailable(102, 5));
    printTestResult("Reassign robot 1 to request 102", testList.assignRobot(102, 1, 5));

    // 6. Lifecycle transitions and invalid jump blocking
    printTestResult("Block invalid jump: PENDING -> COMPLETED (ID 101)", !testList.updateStatus(101, REQUEST_COMPLETED));
    printTestResult("Block invalid jump: ASSIGNED -> COMPLETED (ID 102)", !testList.updateStatus(102, REQUEST_COMPLETED));
    printTestResult("Valid transition: ASSIGNED -> IN_PROGRESS (ID 102)", testList.updateStatus(102, REQUEST_IN_PROGRESS));
    printTestResult("Valid transition: IN_PROGRESS -> COMPLETED (ID 102)", testList.updateStatus(102, REQUEST_COMPLETED));
    printTestResult("Block changing finished request: COMPLETED -> PENDING (ID 102)", !testList.updateStatus(102, REQUEST_PENDING));

    // Mark 103 as FAILED to test failed state
    printTestResult("Valid transition: PENDING -> FAILED (ID 103)", testList.updateStatus(103, REQUEST_FAILED));
    printTestResult("Block changing failed request: FAILED -> ASSIGNED (ID 103)", !testList.updateStatus(103, REQUEST_ASSIGNED));

    // 7. Urgency updates and re-sorting
    // Currently at time 20, available pending requests are: 104 (urg 5) and 101 (urg 3, time 5).
    // If we raise 101 to urgency 5, 101 should beat 104 because 101 has an earlier reportTime (5 < 20).
    printTestResult("Raise urgency of request 101 to 5", testList.updateUrgency(101, 5));
    RescueRequest* newTop = testList.getHighestPriorityPending(20);
    printTestResult("Request 101 moved ahead of 104 after urgency update", newTop != nullptr && newTop->id == 101);
    printTestResult("Block urgency update on COMPLETED request 102", !testList.updateUrgency(102, 1));
    printTestResult("Block urgency update on FAILED request 103", !testList.updateUrgency(103, 4));

    // 8. Statistics check
    // Expected: Total 4 | Pending 2 (101, 104) | Completed 1 (102) | Failed 1 (103) | Completion 25%
    bool statsCorrect = (testList.getCountByStatus(REQUEST_PENDING) == 2 &&
                         testList.getCountByStatus(REQUEST_COMPLETED) == 1 &&
                         testList.getCountByStatus(REQUEST_FAILED) == 1 &&
                         testList.getAvailablePendingCount(20) == 2);
    printTestResult("Request statistics counts match expected values", statsCorrect);

    testList.displayStatistics();
}

int main(int argc, char* argv[]) {

    const char* filename = "data/scenario_provided.txt";

    // Allow a filename to be supplied through the command
    if (argc > 1) {
        filename = argv[1];
    }

    Environment environment;
    RobotList robots;
    RequestList requests;
    ScenarioLoader loader;

    cout << "====================================\n";
    cout << "       AI RESCUE ROBOTS TEST\n";
    cout << "====================================\n\n";

    cout << "Loading scenario: " << filename << "\n\n";

    // Load the complete scenario
    if (!loader.loadEnvironment(filename, environment, robots, requests)) {
        cout << "ERROR: Scenario loading failed!\n";
        return 1;
    }

    cout << "Scenario loaded successfully!\n\n";

    // Display scenario information
    cout << "====================================\n";
    cout << "        SCENARIO INFORMATION\n";
    cout << "====================================\n";

    cout << "Scenario Name: " << loader.getScenarioName() << "\n";

    const SimulationParameters& p = loader.getParameters();

    cout << "Maximum Steps: " << p.maxSteps << "\n";
    cout << "Robot Speed: " << p.speed << "\n";
    cout << "Battery Per Cost Unit: " << p.batteryPerCostUnit << "\n";
    cout << "Battery Per Work Step: " << p.batteryPerWorkStep << "\n";
    cout << "Recharge Per Step: " << p.rechargePerStep << "\n";

    // Display environment
    cout << "\n====================================\n";
    cout << "           ENVIRONMENT\n";
    cout << "====================================\n";

    cout << "Total Locations: " << environment.getLocationCount() << "\n";
    cout << "Total Roads: " << environment.getRoadCount() << "\n\n";

    environment.displayEnvironment();

    // Display robots
    cout << "\n====================================\n";
    cout << "              ROBOTS\n";
    cout << "====================================\n";

    cout << "Total Robots: " << robots.getCount() << "\n\n";

    robots.displayAll();

    // Display requests
    cout << "\n====================================\n";
    cout << "             REQUESTS\n";
    cout << "====================================\n";

    cout << "Total Requests: " << requests.getCount() << "\n\n";

    requests.displayAll();

    // Display scheduled events
    cout << "\n====================================\n";
    cout << "        SCHEDULED EVENTS\n";
    cout << "====================================\n";

    const ScheduledEvent* events = loader.getEvents();
    int eventCount = loader.getEventCount();

    cout << "Total Events: " << eventCount << "\n\n";

    for (int i = 0; i < eventCount; i++) {
        cout << "Event " << i + 1 << "\n";
        cout << "Time: " << events[i].time << "\n";
        cout << "Type: " << events[i].type << "\n";
        cout << "Target ID: " << events[i].targetID << "\n";
        cout << "Value: " << events[i].value << "\n";
        cout << "-------------------------\n";
    }

    cout << "\n====================================\n";
    cout << "       LOADING TEST COMPLETE\n";
    cout << "====================================\n";

    runRequestManagerTests();

    // =========================================================================
    // ROBOT MANAGER / ROBOT OPERATIONS TEST SUITE
    // =========================================================================
    cout << "\n==========================================================\n";
    cout << "          ROBOT MANAGER COMPREHENSIVE VERIFICATION        \n";
    cout << "==========================================================\n";

    // [Test 1] Capability Matching
    cout << "\n--- [1] Capability Matching Tests ---\n";
    Robot* r1 = robots.findRobot(1);   // MEDICAL
    Robot* r10 = robots.findRobot(10); // GENERAL
    if (r1 && r10) {
        cout << "Robot 1 (MEDICAL) -> Category MEDICAL: "
             << (r1->canServe(CATEGORY_MEDICAL) ? "MATCHED (PASS)" : "REJECTED (FAIL)") << "\n";
        cout << "Robot 1 (MEDICAL) -> Category FIRE:    "
             << (!r1->canServe(CATEGORY_FIRE) ? "REJECTED (PASS)" : "MATCHED (FAIL)") << "\n";
        cout << "Robot 10 (GENERAL) -> Category FIRE:   "
             << (r10->canServe(CATEGORY_FIRE) ? "MATCHED (PASS)" : "REJECTED (FAIL)") << "\n";
        cout << "Robot 10 (GENERAL) -> Category SEARCH: "
             << (r10->canServe(CATEGORY_SEARCH) ? "MATCHED (PASS)" : "REJECTED (FAIL)") << "\n";
    }

    // [Test 2] State Transitions Enforcement
    cout << "\n--- [2] State Transition Enforcement ---\n";
    Robot* r3 = robots.findRobot(3); // Initially AVAILABLE
    if (r3) {
        cout << "Robot 3 initial state: " << getRobotStateName(r3->getState()) << "\n";
        // Valid: AVAILABLE -> TRAVELLING
        bool validTrans = r3->setState(ROBOT_TRAVELLING);
        cout << "Transition AVAILABLE -> TRAVELLING: " << (validTrans ? "ALLOWED (PASS)" : "BLOCKED (FAIL)") << "\n";

        // Invalid: transition to non-existent state ID
        bool badStateTrans = r3->setState(99);
        cout << "Transition to invalid state 99:     " << (!badStateTrans ? "BLOCKED (PASS)" : "ALLOWED (FAIL)") << "\n";

        // Reset back to AVAILABLE
        r3->setState(ROBOT_AVAILABLE);
    }

    // [Test 3] Assignment and Release Lifecycle
    cout << "\n--- [3] Assignment & Release Lifecycle ---\n";
    if (r3) {
        // Assign Request 101 to Robot 3
        bool assignOk = robots.assignRobot(3, 101);
        cout << "Assign Request 101 to Robot 3:           " << (assignOk ? "SUCCESS (PASS)" : "FAIL") << "\n";
        cout << "Robot 3 State after assignment:          " << getRobotStateName(r3->getState()) << "\n";
        cout << "Robot 3 Active Request ID:               " << r3->getAssignedRequestID() << "\n";

        // Duplicate assignment conflict: cannot assign another request while busy
        bool doubleAssign = robots.assignRobot(3, 102);
        cout << "Attempt second assignment (conflict):    " << (!doubleAssign ? "PREVENTED (PASS)" : "FAILED (Allowed)") << "\n";

        // Release / cancel assignment
        bool releaseOk = robots.releaseRobot(3);
        cout << "Release Request 101:                     " << (releaseOk ? "SUCCESS (PASS)" : "FAIL") << "\n";
        cout << "Robot 3 State after release:             " << getRobotStateName(r3->getState()) << "\n";
        cout << "Robot 3 Reassignments counter:           " << r3->getTotalReassignments() << "\n";

        // Reassign and complete task
        robots.assignRobot(3, 103);
        r3->setState(ROBOT_PERFORMING); // Arrived at site, doing rescue work
        bool completeOk = robots.completeRobotTask(3);
        cout << "Complete Request 103:                    " << (completeOk ? "SUCCESS (PASS)" : "FAIL") << "\n";
        cout << "Robot 3 Tasks Completed:                 " << r3->getTasksCompleted() << "\n";
        cout << "Robot 3 State after completion:          " << getRobotStateName(r3->getState()) << "\n";
    }

    // [Test 4] Movement: Separate Movement Cost & Battery Consumption
    cout << "\n--- [4] Movement & Resource Constraints ---\n";
    Robot* r4 = robots.findRobot(4); // Start Location 32, max_battery 120, current 120
    if (r4) {
        int initialMove = r4->getTotalMovementCost();
        int initialConsumed = r4->getTotalBatteryConsumed();

        // Move to Location 39 with road cost = 8, battery cost = 12 (different to prove separation!)
        int roadCost = 8;
        int batteryCost = 12;
        bool moved = r4->moveTo(39, roadCost, batteryCost);

        cout << "Move to Loc 39 (Road Cost=" << roadCost << ", Battery Cost=" << batteryCost << "): "
             << (moved ? "SUCCESS (PASS)" : "FAIL") << "\n";
        cout << "  New Location:                  " << r4->getCurrentLocation() << "\n";
        cout << "  Total Movement Cost:           " << r4->getTotalMovementCost() << " (Delta: +" << (r4->getTotalMovementCost() - initialMove) << ")\n";
        cout << "  Total Battery Consumed:        " << r4->getTotalBatteryConsumed() << " (Delta: +" << (r4->getTotalBatteryConsumed() - initialConsumed) << ")\n";
        cout << "  Remaining Battery:             " << r4->getCurrentBattery() << "/" << r4->getMaxBattery() << "\n";

        // Insufficient battery test: try moving with battery cost exceeding current battery
        bool overBudgetMove = r4->moveTo(45, 10, r4->getCurrentBattery() + 50);
        cout << "Attempt move with insufficient battery:  " << (!overBudgetMove ? "BLOCKED (PASS)" : "FAILED (Moved)") << "\n";
    }

    // [Test 5] Charging Operations & Station Availability
    cout << "\n--- [5] Charging Operations (Station Availability) ---\n";
    Robot* r8 = robots.findRobot(8); // SEARCH robot, battery 90/90
    if (r8) {
        // Consume some battery first
        r8->consumeBattery(40);
        cout << "Robot 8 Battery before charging: " << r8->getCurrentBattery() << "/" << r8->getMaxBattery() << "\n";

        // Attempt recharge with station DOWN (offline)
        bool startDown = r8->startRecharging(false); // stationAvailable = false
        cout << "Start recharging at OFFLINE station: " << (!startDown ? "REJECTED (PASS)" : "ALLOWED (FAIL)") << "\n";

        // Start recharge with station UP (online)
        bool startUp = r8->startRecharging(true); // stationAvailable = true
        cout << "Start recharging at ONLINE station:  " << (startUp ? "STARTED (PASS)" : "FAIL") << "\n";
        cout << "Robot 8 State while recharging:      " << getRobotStateName(r8->getState()) << "\n";

        // Movement constraint: robot cannot move while recharging
        bool moveWhileCharging = r8->moveTo(15, 2, 2);
        cout << "Attempt movement while RECHARGING:   " << (!moveWhileCharging ? "BLOCKED (PASS)" : "ALLOWED (FAIL)") << "\n";

        // Step charging: recharge 15 units
        int added = robots.rechargeRobotStep(8, 15, true);
        cout << "Step recharge (+15 units):           Added " << added << " -> Current Battery: " << r8->getCurrentBattery() << "\n";

        // Station fails mid-charging: step recharge with station down
        int addedDown = robots.rechargeRobotStep(8, 15, false);
        cout << "Step recharge during station failure: Added " << addedDown << " -> State reset to "
             << getRobotStateName(r8->getState())
             << (addedDown == 0 && r8->getState() == ROBOT_AVAILABLE ? " (PASS)" : " (FAIL)") << "\n";
    }

    // [Test 6] Offline / Online Handling & State Restoration
    cout << "\n--- [6] Offline / Online Handling with State Restoration ---\n";
    Robot* r2 = robots.findRobot(2);
    if (r2) {
        r2->setState(ROBOT_TRAVELLING);
        cout << "Robot 2 operational state before event: " << getRobotStateName(r2->getState()) << "\n";

        // Event: ROBOT_OFFLINE
        bool offOk = robots.setRobotOffline(2);
        cout << "Event ROBOT_OFFLINE applied:             " << (offOk ? "OFFLINE (PASS)" : "FAIL") << "\n";
        cout << "Robot 2 state:                           " << getRobotStateName(r2->getState()) << "\n";

        // Movement constraint: cannot move while offline
        bool moveWhileOffline = r2->moveTo(10, 5, 5);
        cout << "Attempt movement while OFFLINE:          " << (!moveWhileOffline ? "BLOCKED (PASS)" : "ALLOWED (FAIL)") << "\n";

        // Duplicate offline event safety
        bool dupOff = robots.setRobotOffline(2);
        cout << "Duplicate ROBOT_OFFLINE safety:          " << (!dupOff ? "IGNORED SAFELY (PASS)" : "FAIL") << "\n";

        // Event: ROBOT_ONLINE
        bool onOk = robots.setRobotOnline(2);
        cout << "Event ROBOT_ONLINE applied:              " << (onOk ? "ONLINE (PASS)" : "FAIL") << "\n";
        cout << "Robot 2 restored state:                  " << getRobotStateName(r2->getState())
             << (r2->getState() == ROBOT_TRAVELLING ? " (PASS)" : " (FAIL)") << "\n";

        // Edge case: Release assignment while offline
        r2->setState(ROBOT_AVAILABLE); // Reset to AVAILABLE so assignment succeeds
        bool assign501 = robots.assignRobot(2, 501);
        robots.setRobotOffline(2);     // Offline while travelling with 501
        bool rel501 = robots.releaseRobot(2); // Reassigned to another robot while offline
        robots.setRobotOnline(2);      // Comes back online
        cout << "Robot 2 online after offline release:    " << getRobotStateName(r2->getState())
             << (assign501 && rel501 && r2->getState() == ROBOT_AVAILABLE ? " (PASS)" : " (FAIL)") << "\n";
    }

    // [Test 7] Robot Fleet Statistics
    cout << "\n--- [7] Fleet & Per-Robot Statistics ---\n";
    robots.displayStatistics();

    return 0;
}