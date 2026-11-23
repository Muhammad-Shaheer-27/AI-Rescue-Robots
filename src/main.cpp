#include <iostream>
#include "scenario_loader.h"
#include "routing.h"
#include "event_manager.h"

using namespace std;

// Helper to print pass/fail cleanly for each test
void printTestResult(const char* testName, bool passed) {
    cout << "[" << (passed ? "PASS" : "FAIL") << "] " << testName << endl;
}

void runRoutingTests() {

    cout << "\n====================================\n";
    cout << "       ROUTING MODULE TESTS\n";
    cout << "====================================\n\n";

    // Create a separate environment specifically for routing tests.
    Environment testEnvironment;

    testEnvironment.initialize(5, 6);

    // Add five locations.
    testEnvironment.addLocation(1, "A", LOCATION_COMMAND);
    testEnvironment.addLocation(2, "B", LOCATION_JUNCTION);
    testEnvironment.addLocation(3, "C", LOCATION_JUNCTION);
    testEnvironment.addLocation(4, "D", LOCATION_BUILDING);
    testEnvironment.addLocation(5, "E", LOCATION_HOSPITAL);

    // Add six undirected roads.
    testEnvironment.addRoad(1, 1, 2, 4, ROAD_OPEN);
    testEnvironment.addRoad(2, 1, 3, 2, ROAD_OPEN);
    testEnvironment.addRoad(3, 3, 2, 1, ROAD_OPEN);
    testEnvironment.addRoad(4, 2, 4, 5, ROAD_OPEN);
    testEnvironment.addRoad(5, 3, 4, 8, ROAD_OPEN);
    testEnvironment.addRoad(6, 4, 5, 2, ROAD_OPEN);

    cout << "--- Test Graph ---\n";
    testEnvironment.displayEnvironment();

    int path[10];
    int pathLength;
    int totalCost;

    // ---------------------------------------------------------
    // TEST 1: Shortest path selection
    // ---------------------------------------------------------

    cout << "\n--- [1] Shortest Path Selection ---\n";

    bool found = findShortestPath(
        testEnvironment, 1, 5,
        path, 10, pathLength, totalCost
    );
    
    bool correctPath = (
        found &&
        pathLength == 5 &&
        path[0] == 1 &&
        path[1] == 3 &&
        path[2] == 2 &&
        path[3] == 4 &&
        path[4] == 5 &&
        totalCost == 10
    );

    printTestResult(
        "Find shortest path from 1 to 5 (1-3-2-4-5, cost 10)",
        correctPath
    );

    if (found) {
        cout << "Returned path: ";
        for (int i = 0; i < pathLength; i++) {
            cout << path[i];
            if (i < pathLength - 1) cout << " -> ";
        }
        cout << "\nTotal cost: " << totalCost << "\n";
    }

    // ---------------------------------------------------------
    // TEST 2: Reverse direction (undirected graph)
    // ---------------------------------------------------------

    cout << "\n--- [2] Undirected Road Verification ---\n";

    found = findShortestPath(
        testEnvironment, 5, 1,
        path, 10, pathLength, totalCost
    );
    if (found) {
        cout << "Returned reverse path: ";

        for (int i = 0; i < pathLength; i++) {
            cout << path[i];

            if (i < pathLength - 1) {
                cout << " -> ";
            }
        }

        cout << "\nReturned path length: " << pathLength;
        cout << "\nReturned total cost: " << totalCost << "\n";
    }
    else {
        cout << "Routing function returned false.\n";
    }
    bool reverseCorrect = (
        found &&
        pathLength == 5 &&
        path[0] == 5 &&
        path[1] == 4 &&
        path[2] == 2 &&
        path[3] == 3 &&
        path[4] == 1 &&
        totalCost == 10
    );

    printTestResult(
        "Find reverse path from 5 to 1 (cost 10)",
        reverseCorrect
    );

    // ---------------------------------------------------------
    // TEST 3: Blocked road handling
    // ---------------------------------------------------------

    cout << "\n--- [3] Blocked Road Handling ---\n";

    // Road 3 connects locations 3 and 2.
    // The original shortest path can no longer use it.
    Road* blockedRoad = testEnvironment.getRoad(3);

    if (blockedRoad != nullptr) {
        blockedRoad->status = ROAD_BLOCKED;
    }

    found = findShortestPath(
        testEnvironment, 1, 5,
        path, 10, pathLength, totalCost
    );

    bool blockedCorrect = (
        found &&
        pathLength == 4 &&
        path[0] == 1 &&
        path[1] == 2 &&
        path[2] == 4 &&
        path[3] == 5 &&
        totalCost == 11
    );

    printTestResult(
        "Avoid blocked road 3 and find alternate path (cost 11)",
        blockedCorrect
    );

    // ---------------------------------------------------------
    // TEST 4: Recalculate after multiple road closures
    // ---------------------------------------------------------

    cout << "\n--- [4] Multiple Road Closures ---\n";

    // Road 4 connects 2 to 4.
    // Road 3 is already blocked.
    // The only remaining route to location 5 is through 3 and 4.
    Road* secondBlockedRoad = testEnvironment.getRoad(4);

    if (secondBlockedRoad != nullptr) {
        secondBlockedRoad->status = ROAD_BLOCKED;
    }

    found = findShortestPath(
        testEnvironment, 1, 5,
        path, 10, pathLength, totalCost
    );

    bool multipleBlocksCorrect = (
        found &&
        pathLength == 4 &&
        path[0] == 1 &&
        path[1] == 3 &&
        path[2] == 4 &&
        path[3] == 5 &&
        totalCost == 12
    );

    printTestResult(
        "Find alternate route after two road closures (cost 12)",
        multipleBlocksCorrect
    );

    // ---------------------------------------------------------
    // TEST 5: Unreachable destination
    // ---------------------------------------------------------

    cout << "\n--- [5] Unreachable Destination ---\n";

    // Close the remaining roads connected to location 1.
    testEnvironment.getRoad(1)->status = ROAD_BLOCKED;
    testEnvironment.getRoad(2)->status = ROAD_BLOCKED;

    found = findShortestPath(
        testEnvironment, 1, 5,
        path, 10, pathLength, totalCost
    );

    printTestResult(
        "Reject route when destination is unreachable",
        !found
    );

    // ---------------------------------------------------------
    // TEST 6: Insufficient path array capacity
    // ---------------------------------------------------------

    cout << "\n--- [6] Path Array Capacity ---\n";

    // Restore all roads.
    for (int i = 1; i <= 6; i++) {
        testEnvironment.getRoad(i)->status = ROAD_OPEN;
    }

    // The expected path contains five locations.
    // Capacity of three should be insufficient.
    found = findShortestPath(
        testEnvironment, 1, 5,
        path, 3, pathLength, totalCost
    );

    printTestResult(
        "Reject path when output array capacity is insufficient",
        !found
    );

    // ---------------------------------------------------------
    // TEST 7: Same starting and destination location
    // ---------------------------------------------------------

    cout << "\n--- [7] Same Start and Destination ---\n";

    found = findShortestPath(
        testEnvironment, 3, 3,
        path, 10, pathLength, totalCost
    );

    bool sameLocationCorrect = (
        found &&
        pathLength == 1 &&
        path[0] == 3 &&
        totalCost == 0
    );

    printTestResult(
        "Path from a location to itself has cost 0",
        sameLocationCorrect
    );

    // ---------------------------------------------------------
    // TEST 8: Invalid location IDs
    // ---------------------------------------------------------

    cout << "\n--- [8] Invalid Location IDs ---\n";

    found = findShortestPath(
        testEnvironment, 99, 5,
        path, 10, pathLength, totalCost
    );

    printTestResult(
        "Reject invalid starting location",
        !found
    );

    found = findShortestPath(
        testEnvironment, 1, 99,
        path, 10, pathLength, totalCost
    );

    printTestResult(
        "Reject invalid destination location",
        !found
    );

    cout << "\n====================================\n";
    cout << "       ROUTING TESTS COMPLETE\n";
    cout << "====================================\n";
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
    printTestResult("Raise urgency of request 101 to 5", testList.updateUrgency(101, 5));
    RescueRequest* newTop = testList.getHighestPriorityPending(20);
    printTestResult("Request 101 moved ahead of 104 after urgency update", newTop != nullptr && newTop->id == 101);
    printTestResult("Block urgency update on COMPLETED request 102", !testList.updateUrgency(102, 1));
    printTestResult("Block urgency update on FAILED request 103", !testList.updateUrgency(103, 4));

    // 8. Statistics check
    bool statsCorrect = (testList.getCountByStatus(REQUEST_PENDING) == 2 &&
                         testList.getCountByStatus(REQUEST_COMPLETED) == 1 &&
                         testList.getCountByStatus(REQUEST_FAILED) == 1 &&
                         testList.getAvailablePendingCount(20) == 2);
    printTestResult("Request statistics counts match expected values", statsCorrect);

    testList.displayStatistics();
}

void runEventManagerTests() {
    cout << "\n====================================\n";
    cout << "     EVENT MANAGER UNIT TESTS\n";
    cout << "====================================\n\n";

    // Setup an isolated testing environment
    Environment testEnv;
    testEnv.initialize(4, 4);
    testEnv.addLocation(1, "Depot", LOCATION_DEPOT);
    testEnv.addLocation(2, "Junction_A", LOCATION_JUNCTION);
    testEnv.addLocation(3, "Junction_B", LOCATION_JUNCTION);
    testEnv.addLocation(4, "Charging_Hub", LOCATION_CHARGING);

    // Initial station availability checks
    printTestResult("Charging station 4 initially available in environment", testEnv.isStationAvailable(4));
    printTestResult("Non-charging location 1 is not a station", !testEnv.isStationAvailable(1));

    // Roads:
    // Road 1: 1 <-> 2 (cost 2)
    // Road 2: 2 <-> 3 (cost 2)
    // Road 3: 1 <-> 3 (cost 10)  -- slower backup route
    // Road 4: 3 <-> 4 (cost 1)
    testEnv.addRoad(1, 1, 2, 2, ROAD_OPEN);
    testEnv.addRoad(2, 2, 3, 2, ROAD_OPEN);
    testEnv.addRoad(3, 1, 3, 10, ROAD_OPEN);
    testEnv.addRoad(4, 3, 4, 1, ROAD_OPEN);

    // Setup test robot
    RobotList testRobots;
    testRobots.addRobot(1, 1, CAPABILITY_MEDICAL, 100, 100);
    Robot* testBot = testRobots.findRobot(1);
    testBot->setState(ROBOT_TRAVELLING); // Set travelling to test state restoration

    // Setup test requests (priority ordered)
    RequestList testRequests;
    testRequests.addRequestPriority(101, 0, 4, CATEGORY_MEDICAL, 2, 5); // urg 2
    testRequests.addRequestPriority(102, 0, 4, CATEGORY_MEDICAL, 4, 5); // urg 4 (higher priority)

    RescueRequest* topReq = testRequests.getHighestPriorityPending(0);
    printTestResult("Request 102 initially highest priority (urg 4 vs 2)", topReq != nullptr && topReq->id == 102);

    // Check initial shortest route from 1 to 4:
    // Optimal path: 1 -> 2 -> 3 -> 4 (Cost = 2 + 2 + 1 = 5)
    int path[10];
    int pathLen = 0;
    int cost = 0;
    bool pathFound = findShortestPath(testEnv, 1, 4, path, 10, pathLen, cost);
    printTestResult("Initial route 1->4 found with cost 5", pathFound && cost == 5);

    // Prepare 7 scheduled events covering all 7 event types:
    ScheduledEvent testEvents[7];
    // t=5: BLOCK Road 1
    testEvents[0].time = 5;
    testEvents[0].type = EVENT_BLOCK;
    testEvents[0].targetID = 1;
    testEvents[0].value = 0;

    // t=10: STATION_DOWN on Location 4
    testEvents[1].time = 10;
    testEvents[1].type = EVENT_STATION_DOWN;
    testEvents[1].targetID = 4;
    testEvents[1].value = 0;

    // t=15: Simultaneous event 1: SET_URGENCY Request 101 to 5
    testEvents[2].time = 15;
    testEvents[2].type = EVENT_SET_URGENCY;
    testEvents[2].targetID = 101;
    testEvents[2].value = 5;

    // t=15: Simultaneous event 2: ROBOT_OFFLINE Robot 1
    testEvents[3].time = 15;
    testEvents[3].type = EVENT_ROBOT_OFFLINE;
    testEvents[3].targetID = 1;
    testEvents[3].value = 0;

    // t=20: Simultaneous event 1: UNBLOCK Road 1
    testEvents[4].time = 20;
    testEvents[4].type = EVENT_UNBLOCK;
    testEvents[4].targetID = 1;
    testEvents[4].value = 0;

    // t=20: Simultaneous event 2: STATION_UP Location 4
    testEvents[5].time = 20;
    testEvents[5].type = EVENT_STATION_UP;
    testEvents[5].targetID = 4;
    testEvents[5].value = 0;

    // t=20: Simultaneous event 3: ROBOT_ONLINE Robot 1
    testEvents[6].time = 20;
    testEvents[6].type = EVENT_ROBOT_ONLINE;
    testEvents[6].targetID = 1;
    testEvents[6].value = 0;

    EventManager em;
    em.initialize(testEvents, 7);

    // 1. Advance to t=4 (no events scheduled)
    int p0 = em.processEvents(4, testEnv, testRobots, testRequests);
    printTestResult("No events processed before scheduled time (at t=4)", p0 == 0);

    // 2. Advance to t=5 (BLOCK Road 1)
    int p1 = em.processEvents(5, testEnv, testRobots, testRequests);
    printTestResult("1 event processed at t=5 (BLOCK Road 1)", p1 == 1);
    printTestResult("Road 1 status is now BLOCKED in environment", !testEnv.isRoadOpen(1));

    // Test routing impact: Dijkstra must avoid blocked Road 1 and detour via Road 3 (1 -> 3 -> 4, cost 10 + 1 = 11)
    pathFound = findShortestPath(testEnv, 1, 4, path, 10, pathLen, cost);
    bool detourSuccess = (pathFound && pathLen == 3 && path[0] == 1 && path[1] == 3 && path[2] == 4 && cost == 11);
    printTestResult("Subsequent Dijkstra ignores blocked Road 1 & detours via Road 3 (cost 11)", detourSuccess);

    // 3. Process events again at t=5 to verify events are processed strictly once
    int pDup = em.processEvents(5, testEnv, testRobots, testRequests);
    printTestResult("Event processed strictly once (re-running t=5 returns 0)", pDup == 0);

    // 4. Advance to t=10 (STATION_DOWN at Location 4)
    int p2 = em.processEvents(10, testEnv, testRobots, testRequests);
    printTestResult("1 event processed at t=10 (STATION_DOWN Location 4)", p2 == 1);
    printTestResult("Station 4 availability updated to false in environment", !testEnv.isStationAvailable(4));

    // 5. Advance to t=15 (Two simultaneous events: SET_URGENCY 101->5, ROBOT_OFFLINE 1)
    int p3 = em.processEvents(15, testEnv, testRobots, testRequests);
    printTestResult("2 simultaneous events processed at t=15 in file order", p3 == 2);

    topReq = testRequests.getHighestPriorityPending(15);
    printTestResult("Request 101 urgency updated to 5 & reprioritized to head of queue", topReq != nullptr && topReq->id == 101 && topReq->urgency == 5);
    printTestResult("Robot 1 went OFFLINE (ROBOT_UNAVAILABLE, savedState=TRAVELLING)", testBot->getState() == ROBOT_UNAVAILABLE);

    // 6. Advance to t=20 (Three simultaneous events: UNBLOCK 1, STATION_UP 4, ROBOT_ONLINE 1)
    int p4 = em.processEvents(20, testEnv, testRobots, testRequests);
    printTestResult("3 simultaneous events processed at t=20 in file order", p4 == 3);
    printTestResult("Road 1 is now UNBLOCKED (ROAD_OPEN in environment)", testEnv.isRoadOpen(1));

    // Test routing restoration: optimal path 1->2->3->4 with cost 5 is restored!
    pathFound = findShortestPath(testEnv, 1, 4, path, 10, pathLen, cost);
    bool restoreSuccess = (pathFound && pathLen == 4 && path[1] == 2 && cost == 5);
    printTestResult("Subsequent Dijkstra restores optimal route through Road 1 (cost 5)", restoreSuccess);

    printTestResult("Station 4 availability restored to true in environment", testEnv.isStationAvailable(4));
    printTestResult("Robot 1 restored previous operational state (TRAVELLING)", testBot->getState() == ROBOT_TRAVELLING);

    // 7. Verification of EventManager tracking
    printTestResult("All 7 scheduled events processed", em.getProcessedEventCount() == 7 && em.getRemainingEventCount() == 0);
    printTestResult("No pending events remaining", !em.hasPendingEvents());

    em.displayStatistics();
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
        cout << "Type: " << getEventTypeName(events[i].type) << " (" << events[i].type << ")\n";
        cout << "Target ID: " << events[i].targetID << "\n";
        cout << "Value: " << events[i].value << "\n";
        cout << "-------------------------\n";
    }

    cout << "\n====================================\n";
    cout << "       LOADING TEST COMPLETE\n";
    cout << "====================================\n";

    // Run modular unit tests
    runRequestManagerTests();
    runRoutingTests();
    runEventManagerTests();

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
        bool validTrans = r3->setState(ROBOT_TRAVELLING);
        cout << "Transition AVAILABLE -> TRAVELLING: " << (validTrans ? "ALLOWED (PASS)" : "BLOCKED (FAIL)") << "\n";

        bool badStateTrans = r3->setState(99);
        cout << "Transition to invalid state 99:     " << (!badStateTrans ? "BLOCKED (PASS)" : "ALLOWED (FAIL)") << "\n";

        r3->setState(ROBOT_AVAILABLE);
    }

    // [Test 3] Assignment and Release Lifecycle
    cout << "\n--- [3] Assignment & Release Lifecycle ---\n";
    if (r3) {
        bool assignOk = robots.assignRobot(3, 101);
        cout << "Assign Request 101 to Robot 3:           " << (assignOk ? "SUCCESS (PASS)" : "FAIL") << "\n";
        cout << "Robot 3 State after assignment:          " << getRobotStateName(r3->getState()) << "\n";
        cout << "Robot 3 Active Request ID:               " << r3->getAssignedRequestID() << "\n";

        bool doubleAssign = robots.assignRobot(3, 102);
        cout << "Attempt second assignment (conflict):    " << (!doubleAssign ? "PREVENTED (PASS)" : "FAILED (Allowed)") << "\n";

        bool releaseOk = robots.releaseRobot(3);
        cout << "Release Request 101:                     " << (releaseOk ? "SUCCESS (PASS)" : "FAIL") << "\n";
        cout << "Robot 3 State after release:             " << getRobotStateName(r3->getState()) << "\n";
        cout << "Robot 3 Reassignments counter:           " << r3->getTotalReassignments() << "\n";

        robots.assignRobot(3, 103);
        r3->setState(ROBOT_PERFORMING);
        bool completeOk = robots.completeRobotTask(3);
        cout << "Complete Request 103:                    " << (completeOk ? "SUCCESS (PASS)" : "FAIL") << "\n";
        cout << "Robot 3 Tasks Completed:                 " << r3->getTasksCompleted() << "\n";
        cout << "Robot 3 State after completion:          " << getRobotStateName(r3->getState()) << "\n";
    }

    // [Test 4] Movement: Separate Movement Cost & Battery Consumption
    cout << "\n--- [4] Movement & Resource Constraints ---\n";
    Robot* r4 = robots.findRobot(4); // Start Location 32, max_battery 120, current 120
    if (r4) {
        int initialBat = r4->getCurrentBattery();
        int initialMove = r4->getTotalMovementCost();
        int initialConsumed = r4->getTotalBatteryConsumed();

        int roadCost = 8;
        int batteryCost = 12;
        bool moved = r4->moveTo(39, roadCost, batteryCost);

        cout << "Move to Loc 39 (Road Cost=" << roadCost << ", Battery Cost=" << batteryCost << "): "
             << (moved ? "SUCCESS (PASS)" : "FAIL") << "\n";
        cout << "  New Location:                  " << r4->getCurrentLocation() << "\n";
        cout << "  Total Movement Cost:           " << r4->getTotalMovementCost() << " (Delta: +" << (r4->getTotalMovementCost() - initialMove) << ")\n";
        cout << "  Total Battery Consumed:        " << r4->getTotalBatteryConsumed() << " (Delta: +" << (r4->getTotalBatteryConsumed() - initialConsumed) << ")\n";
        cout << "  Remaining Battery:             " << r4->getCurrentBattery() << "/" << r4->getMaxBattery() << "\n";

        bool overBudgetMove = r4->moveTo(45, 10, r4->getCurrentBattery() + 50);
        cout << "Attempt move with insufficient battery:  " << (!overBudgetMove ? "BLOCKED (PASS)" : "FAILED (Moved)") << "\n";
    }

    // [Test 5] Charging Operations & Station Availability
    cout << "\n--- [5] Charging Operations (Station Availability) ---\n";
    Robot* r8 = robots.findRobot(8); // SEARCH robot, battery 90/90
    if (r8) {
        r8->consumeBattery(40);
        cout << "Robot 8 Battery before charging: " << r8->getCurrentBattery() << "/" << r8->getMaxBattery() << "\n";

        bool startDown = r8->startRecharging(false); // stationAvailable = false
        cout << "Start recharging at OFFLINE station: " << (!startDown ? "REJECTED (PASS)" : "ALLOWED (FAIL)") << "\n";

        bool startUp = r8->startRecharging(true); // stationAvailable = true
        cout << "Start recharging at ONLINE station:  " << (startUp ? "STARTED (PASS)" : "FAIL") << "\n";
        cout << "Robot 8 State while recharging:      " << getRobotStateName(r8->getState()) << "\n";

        bool moveWhileCharging = r8->moveTo(15, 2, 2);
        cout << "Attempt movement while RECHARGING:   " << (!moveWhileCharging ? "BLOCKED (PASS)" : "ALLOWED (FAIL)") << "\n";

        int added = robots.rechargeRobotStep(8, 15, true);
        cout << "Step recharge (+15 units):           Added " << added << " -> Current Battery: " << r8->getCurrentBattery() << "\n";

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

        bool offOk = robots.setRobotOffline(2);
        cout << "Event ROBOT_OFFLINE applied:             " << (offOk ? "OFFLINE (PASS)" : "FAIL") << "\n";
        cout << "Robot 2 state:                           " << getRobotStateName(r2->getState()) << "\n";

        bool moveWhileOffline = r2->moveTo(10, 5, 5);
        cout << "Attempt movement while OFFLINE:          " << (!moveWhileOffline ? "BLOCKED (PASS)" : "ALLOWED (FAIL)") << "\n";

        bool dupOff = robots.setRobotOffline(2);
        cout << "Duplicate ROBOT_OFFLINE safety:          " << (!dupOff ? "IGNORED SAFELY (PASS)" : "FAIL") << "\n";

        bool onOk = robots.setRobotOnline(2);
        cout << "Event ROBOT_ONLINE applied:              " << (onOk ? "ONLINE (PASS)" : "FAIL") << "\n";
        cout << "Robot 2 restored state:                  " << getRobotStateName(r2->getState())
             << (r2->getState() == ROBOT_TRAVELLING ? " (PASS)" : " (FAIL)") << "\n";

        // Edge case: Release assignment while offline
        r2->setState(ROBOT_AVAILABLE);
        bool assign501 = robots.assignRobot(2, 501);
        robots.setRobotOffline(2);
        bool rel501 = robots.releaseRobot(2);
        robots.setRobotOnline(2);
        cout << "Robot 2 online after offline release:    " << getRobotStateName(r2->getState())
             << (assign501 && rel501 && r2->getState() == ROBOT_AVAILABLE ? " (PASS)" : " (FAIL)") << "\n";
    }

    // [Test 7] Robot Fleet Statistics
    cout << "\n--- [7] Fleet & Per-Robot Statistics ---\n";
    robots.displayStatistics();

    // =========================================================================
    // SCENARIO EVENT PROCESSING TEST (Real Scenario Events)
    // =========================================================================
    cout << "\n==========================================================\n";
    cout << "     SCENARIO EVENT PROCESSING (scenario_provided.txt)    \n";
    cout << "==========================================================\n";

    EventManager scenarioEventManager;
    scenarioEventManager.initialize(events, eventCount);

    cout << "\n[Step 1] Advancing to t=20 (First scheduled event in file: BLOCK Road 22)\n";
    int evCount20 = scenarioEventManager.processEvents(20, environment, robots, requests);
    cout << "Events processed at t=20: " << evCount20 << endl;
    printTestResult("Road 22 is now BLOCKED in scenario environment", !environment.isRoadOpen(22));

    cout << "\n[Step 2] Advancing to t=50 (Includes BLOCK 3 and STATION_DOWN 19)\n";
    int evCount50 = scenarioEventManager.processEvents(50, environment, robots, requests);
    cout << "Events processed between t=21 and t=50: " << evCount50 << endl;
    printTestResult("Road 3 is now BLOCKED in scenario environment", !environment.isRoadOpen(3));
    printTestResult("Station 19 is now OFFLINE in scenario environment", !environment.isStationAvailable(19));

    cout << "\n[Step 3] Advancing to end of simulation (max_steps = " << p.maxSteps << ")\n";
    int evCountEnd = scenarioEventManager.processEvents(p.maxSteps, environment, robots, requests);
    cout << "Remaining events processed up to t=" << p.maxSteps << ": " << evCountEnd << endl;
    printTestResult("All scenario events processed", scenarioEventManager.getRemainingEventCount() == 0);

    scenarioEventManager.displayStatistics();

    return 0;
}
