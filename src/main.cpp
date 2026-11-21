#include <iostream>
#include "scenario_loader.h"

using namespace std;

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