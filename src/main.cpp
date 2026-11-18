#include <iostream>

#include "environment.h"
#include "scenario_loader.h"
#include "robot.h"

using namespace std;

int main() {

    Environment environment;
    ScenarioLoader loader;

    if (loader.loadEnvironment(
            "data/scenario_provided.txt", environment)) {

        cout << "\nScenario Name: "
             << loader.getScenarioName() << endl;

        cout << "\nMaximum Steps: "
             << loader.getParameters().maxSteps << endl;

        environment.displayEnvironment();
    }

    // =========================================================================
    // Robot Module Verification & Testing
    // =========================================================================
    cout << "\n==================================================" << endl;
    cout << "           ROBOT MODULE VERIFICATION              " << endl;
    cout << "==================================================" << endl;

    RobotList robotList;

    // 1. Add robots into the custom linked list
    cout << "\n--- [1] Adding Robots to Linked List ---" << endl;
    robotList.addRobot(1, 13, CAPABILITY_MEDICAL, 100, 100);
    robotList.addRobot(2, 29, CAPABILITY_FIRE, 120, 120);
    robotList.addRobot(3, 48, CAPABILITY_SUPPLY, 150, 120);
    robotList.addRobot(4, 32, CAPABILITY_SEARCH, 90, 70);
    robotList.addRobot(5, 51, CAPABILITY_GENERAL, 110, 110);

    // Test validation: duplicate ID should be rejected
    bool dupAdded = robotList.addRobot(1, 10, CAPABILITY_FIRE, 100, 100);
    cout << "Adding duplicate robot ID 1:   " << (dupAdded ? "FAILED (should reject)" : "PASSED (rejected)") << endl;

    // Test validation: invalid start location (<= 0) should be rejected
    bool badLocAdded = robotList.addRobot(99, -5, CAPABILITY_FIRE, 100, 100);
    cout << "Adding invalid start location: " << (badLocAdded ? "FAILED (should reject)" : "PASSED (rejected)") << endl;

    robotList.displayAll();

    // 2. Capability Matching Tests
    cout << "\n--- [2] Capability Matching Tests ---" << endl;
    Robot* medBot = robotList.findRobot(1);
    Robot* genBot = robotList.findRobot(5);

    if (medBot != nullptr) {
        cout << "Robot 1 (MEDICAL) serving MEDICAL request: " 
             << (medBot->canServe(CATEGORY_MEDICAL) ? "PASSED (Matched)" : "FAILED") << endl;
        cout << "Robot 1 (MEDICAL) serving FIRE request:    " 
             << (medBot->canServe(CATEGORY_FIRE) ? "FAILED (Should not match)" : "PASSED (Rejected)") << endl;
    }

    if (genBot != nullptr) {
        cout << "Robot 5 (GENERAL) serving FIRE request:    " 
             << (genBot->canServe(CATEGORY_FIRE) ? "PASSED (Matched)" : "FAILED") << endl;
        cout << "Robot 5 (GENERAL) serving SEARCH request:  " 
             << (genBot->canServe(CATEGORY_SEARCH) ? "PASSED (Matched)" : "FAILED") << endl;
        cout << "Robot 5 (GENERAL) serving SUPPLY request:  " 
             << (genBot->canServe(CATEGORY_SUPPLY) ? "PASSED (Matched)" : "FAILED") << endl;
    }

    // 3. Movement and Battery Consumption
    cout << "\n--- [3] Movement & Battery Consumption ---" << endl;
    if (medBot != nullptr) {
        cout << "Robot 1 before move: Location = " << medBot->getCurrentLocation() 
             << ", Battery = " << medBot->getCurrentBattery() << endl;

        bool moveSuccess = medBot->moveTo(20, 15); // Move to Location 20 with battery cost 15
        cout << "Move to Loc 20 (cost 15): " << (moveSuccess ? "PASSED (Successful)" : "FAILED") << endl;
        cout << "Robot 1 after move:  Location = " << medBot->getCurrentLocation() 
             << ", Battery = " << medBot->getCurrentBattery() 
             << ", Movement Cost = " << medBot->getTotalMovementCost() << endl;

        // Constraint check: insufficient battery should reject move without changing location or battery
        bool overCostMove = medBot->moveTo(25, 200);
        cout << "Move with cost 200 (insufficient battery): " 
             << (overCostMove ? "FAILED (should reject)" : "PASSED (rejected)") << endl;
    }

    // 4. Recharging Robot
    cout << "\n--- [4] Recharging Robot ---" << endl;
    Robot* searchBot = robotList.findRobot(4);
    if (searchBot != nullptr) {
        cout << "Robot 4 before recharge: Battery = " << searchBot->getCurrentBattery() << endl;
        robotList.rechargeRobot(4, 15);
        cout << "Robot 4 after recharge:  Battery = " << searchBot->getCurrentBattery() << endl;
    }

    // 5. Offline and Online Dynamic Events
    cout << "\n--- [5] State Transitions & Offline Handling ---" << endl;
    Robot* fireBot = robotList.findRobot(2);
    if (fireBot != nullptr) {
        fireBot->setState(ROBOT_TRAVELLING);
        cout << "Robot 2 state before event: " << getRobotStateName(fireBot->getState()) << endl;

        robotList.setRobotOffline(2);
        cout << "Robot 2 state after OFFLINE: " << getRobotStateName(fireBot->getState()) << endl;

        // Constraint check: moving while offline/unavailable should be rejected
        bool offlineMove = fireBot->moveTo(30, 5);
        cout << "Move while OFFLINE: " << (offlineMove ? "FAILED (should reject)" : "PASSED (rejected)") << endl;

        robotList.setRobotOnline(2);
        cout << "Robot 2 state after ONLINE:  " << getRobotStateName(fireBot->getState()) 
             << " (Restored previous state)" << endl;
    }

    // 6. Display Updated Robot List
    cout << "\n--- [6] Final Robot List Status ---" << endl;
    robotList.displayAll();

    return 0;
}
