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
    return 0;
}
