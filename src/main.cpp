#include <iostream>
#include <string>
#include "scenario_loader.h"
#include "routing.h"
#include "event_manager.h"
#include "assignment.h"
#include "logger.h"
#include "simulation.h"

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

    for (int i = 1; i <= 6; i++) {
        testEnvironment.getRoad(i)->status = ROAD_OPEN;
    }

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
    cout << "     EVENT MANAGER & LOGGING TESTS  \n";
    cout << "====================================\n\n";

    // Setup an isolated testing environment
    Environment testEnv;
    testEnv.initialize(4, 4);
    testEnv.addLocation(1, "Depot", LOCATION_DEPOT);
    testEnv.addLocation(2, "Junction_A", LOCATION_JUNCTION);
    testEnv.addLocation(3, "Junction_B", LOCATION_JUNCTION);
    testEnv.addLocation(4, "Charging_Hub", LOCATION_CHARGING);

    printTestResult("Charging station 4 initially available in environment", testEnv.isStationAvailable(4));
    printTestResult("Non-charging location 1 is not a station", !testEnv.isStationAvailable(1));

    testEnv.addRoad(1, 1, 2, 2, ROAD_OPEN);
    testEnv.addRoad(2, 2, 3, 2, ROAD_OPEN);
    testEnv.addRoad(3, 1, 3, 10, ROAD_OPEN);
    testEnv.addRoad(4, 3, 4, 1, ROAD_OPEN);

    RobotList testRobots;
    testRobots.addRobot(1, 1, CAPABILITY_MEDICAL, 100, 100);
    Robot* testBot = testRobots.findRobot(1);
    testBot->setState(ROBOT_TRAVELLING);

    RequestList testRequests;
    testRequests.addRequestPriority(101, 0, 4, CATEGORY_MEDICAL, 2, 5); // urg 2
    testRequests.addRequestPriority(102, 0, 4, CATEGORY_MEDICAL, 4, 5); // urg 4

    int path[10];
    int pathLen = 0;
    int cost = 0;
    bool pathFound = findShortestPath(testEnv, 1, 4, path, 10, pathLen, cost);
    printTestResult("Initial route 1->4 found with cost 5", pathFound && cost == 5);

    // Prepare 8 scheduled events: 7 valid events covering all 7 types + 1 invalid event (to test failure handling)
    ScheduledEvent testEvents[8];
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

    // t=25: Invalid event (BLOCK non-existent Road 999) - should fail and NOT log
    testEvents[7].time = 25;
    testEvents[7].type = EVENT_BLOCK;
    testEvents[7].targetID = 999;
    testEvents[7].value = 0;

    Logger testLogger;
    EventManager em;
    em.initialize(testEvents, 8);

    // 1. Advance to t=4 (no events scheduled)
    int p0 = em.processEvents(4, testEnv, testRobots, testRequests, testLogger);
    printTestResult("No events processed before scheduled time (at t=4)", p0 == 0);

    // 2. Advance to t=5 (BLOCK Road 1)
    int p1 = em.processEvents(5, testEnv, testRobots, testRequests, testLogger);
    printTestResult("1 event processed at t=5 (BLOCK Road 1)", p1 == 1);
    printTestResult("Road 1 status is now BLOCKED in environment", !testEnv.isRoadOpen(1));
    printTestResult("Logger recorded LOG_ROAD_BLOCKED entry", testLogger.getEventCount(LOG_ROAD_BLOCKED) == 1);

    pathFound = findShortestPath(testEnv, 1, 4, path, 10, pathLen, cost);
    bool detourSuccess = (pathFound && pathLen == 3 && path[0] == 1 && path[1] == 3 && path[2] == 4 && cost == 11);
    printTestResult("Subsequent Dijkstra ignores blocked Road 1 & detours via Road 3 (cost 11)", detourSuccess);

    // 3. Process events again at t=5 to verify events are processed strictly once
    int pDup = em.processEvents(5, testEnv, testRobots, testRequests, testLogger);
    printTestResult("Event processed strictly once (re-running t=5 returns 0)", pDup == 0);

    // 4. Advance to t=10 (STATION_DOWN at Location 4)
    int p2 = em.processEvents(10, testEnv, testRobots, testRequests, testLogger);
    printTestResult("1 event processed at t=10 (STATION_DOWN Location 4)", p2 == 1);
    printTestResult("Station 4 availability updated to false in environment", !testEnv.isStationAvailable(4));
    printTestResult("Logger recorded LOG_STATION_DOWN entry", testLogger.getEventCount(LOG_STATION_DOWN) == 1);

    // 5. Advance to t=15 (Two simultaneous events: SET_URGENCY 101->5, ROBOT_OFFLINE 1)
    int p3 = em.processEvents(15, testEnv, testRobots, testRequests, testLogger);
    printTestResult("2 simultaneous events processed at t=15 in file order", p3 == 2);
    RescueRequest* topReq = testRequests.getHighestPriorityPending(15);
    printTestResult("Request 101 urgency updated to 5 & reprioritized to head of queue", topReq != nullptr && topReq->id == 101 && topReq->urgency == 5);
    printTestResult("Robot 1 went OFFLINE (ROBOT_UNAVAILABLE)", testBot->getState() == ROBOT_UNAVAILABLE);
    printTestResult("Logger recorded LOG_URGENCY_CHANGED entry", testLogger.getEventCount(LOG_URGENCY_CHANGED) == 1);
    printTestResult("Logger recorded LOG_ROBOT_OFFLINE entry", testLogger.getEventCount(LOG_ROBOT_OFFLINE) == 1);

    // 6. Advance to t=20 (Three simultaneous events: UNBLOCK 1, STATION_UP 4, ROBOT_ONLINE 1)
    int p4 = em.processEvents(20, testEnv, testRobots, testRequests, testLogger);
    printTestResult("3 simultaneous events processed at t=20 in file order", p4 == 3);
    printTestResult("Road 1 is now UNBLOCKED (ROAD_OPEN in environment)", testEnv.isRoadOpen(1));

    pathFound = findShortestPath(testEnv, 1, 4, path, 10, pathLen, cost);
    bool restoreSuccess = (pathFound && pathLen == 4 && path[1] == 2 && cost == 5);
    printTestResult("Subsequent Dijkstra restores optimal route through Road 1 (cost 5)", restoreSuccess);

    printTestResult("Station 4 availability restored to true in environment", testEnv.isStationAvailable(4));
    printTestResult("Robot 1 restored previous operational state (TRAVELLING)", testBot->getState() == ROBOT_TRAVELLING);
    printTestResult("Logger recorded LOG_ROAD_REOPENED entry", testLogger.getEventCount(LOG_ROAD_REOPENED) == 1);
    printTestResult("Logger recorded LOG_STATION_UP entry", testLogger.getEventCount(LOG_STATION_UP) == 1);
    printTestResult("Logger recorded LOG_ROBOT_ONLINE entry", testLogger.getEventCount(LOG_ROBOT_ONLINE) == 1);

    // 7. Advance to t=25 (Failed event on invalid road 999)
    int preCount = testLogger.getCount();
    int p5 = em.processEvents(25, testEnv, testRobots, testRequests, testLogger);
    printTestResult("Attempted event on invalid road 999 was processed", p5 == 1);
    printTestResult("Failed event was NOT recorded in Logger", testLogger.getCount() == preCount);

    // 8. Verify EventManager statistics consistency:
    // Total attempted = 8, Successful = 7, Failed = 1, Remaining = 0
    printTestResult("Attempted events == 8", em.getAttemptedEventCount() == 8);
    printTestResult("Successful events == 7", em.getSuccessfulEventCount() == 7);
    printTestResult("Failed events == 1", em.getFailedEventCount() == 1);
    printTestResult("Remaining events == 0", em.getRemainingEventCount() == 0);

    // 9. Verify Logger entry values & simultaneous event order
    LogEntry* curLog = testLogger.getHead();
    bool logDetailsOk = true;
    // Entry 1: t=5, LOG_ROAD_BLOCKED, roadID = 1, value = -1
    if (!curLog || curLog->time != 5 || curLog->eventType != LOG_ROAD_BLOCKED || curLog->roadID != 1 || curLog->value != -1) logDetailsOk = false;
    // Entry 2: t=10, LOG_STATION_DOWN, locationID = 4, value = -1
    curLog = curLog ? curLog->next : nullptr;
    if (!curLog || curLog->time != 10 || curLog->eventType != LOG_STATION_DOWN || curLog->locationID != 4 || curLog->value != -1) logDetailsOk = false;
    // Entry 3: t=15, LOG_URGENCY_CHANGED, requestID = 101, value = 5 (Simultaneous 1)
    curLog = curLog ? curLog->next : nullptr;
    if (!curLog || curLog->time != 15 || curLog->eventType != LOG_URGENCY_CHANGED || curLog->requestID != 101 || curLog->value != 5) logDetailsOk = false;
    // Entry 4: t=15, LOG_ROBOT_OFFLINE, robotID = 1, value = -1 (Simultaneous 2 - preserved order!)
    curLog = curLog ? curLog->next : nullptr;
    if (!curLog || curLog->time != 15 || curLog->eventType != LOG_ROBOT_OFFLINE || curLog->robotID != 1 || curLog->value != -1) logDetailsOk = false;
    // Entry 5: t=20, LOG_ROAD_REOPENED, roadID = 1, value = -1
    curLog = curLog ? curLog->next : nullptr;
    if (!curLog || curLog->time != 20 || curLog->eventType != LOG_ROAD_REOPENED || curLog->roadID != 1 || curLog->value != -1) logDetailsOk = false;
    // Entry 6: t=20, LOG_STATION_UP, locationID = 4, value = -1
    curLog = curLog ? curLog->next : nullptr;
    if (!curLog || curLog->time != 20 || curLog->eventType != LOG_STATION_UP || curLog->locationID != 4 || curLog->value != -1) logDetailsOk = false;
    // Entry 7: t=20, LOG_ROBOT_ONLINE, robotID = 1, value = -1
    curLog = curLog ? curLog->next : nullptr;
    if (!curLog || curLog->time != 20 || curLog->eventType != LOG_ROBOT_ONLINE || curLog->robotID != 1 || curLog->value != -1) logDetailsOk = false;

    printTestResult("All Logger entry fields & simultaneous event ordering verified", logDetailsOk);

    em.displayStatistics();
}

void runAssignmentTests() {
    cout << "\n====================================\n";
    cout << "     ASSIGNMENT MODULE UNIT TESTS   \n";
    cout << "====================================\n\n";

    // Setup an environment for assignment tests:
    // Locations: 1 (Base), 2 (Midpoint), 3 (NearSite), 4 (IncidentSite), 5 (Island)
    // Roads:
    // Road 1: 1 <-> 2 (cost 2)
    // Road 2: 2 <-> 4 (cost 3)   [Total route 1->4 = 5]
    // Road 3: 3 <-> 4 (cost 1)   [Total route 3->4 = 1]
    // Location 5 has no roads (disconnected)
    Environment env;
    env.initialize(5, 3);
    env.addLocation(1, "Base", LOCATION_DEPOT);
    env.addLocation(2, "Midpoint", LOCATION_JUNCTION);
    env.addLocation(3, "NearSite", LOCATION_JUNCTION);
    env.addLocation(4, "IncidentSite", LOCATION_BUILDING);
    env.addLocation(5, "Island", LOCATION_BUILDING);

    env.addRoad(1, 1, 2, 2, ROAD_OPEN);
    env.addRoad(2, 2, 4, 3, ROAD_OPEN);
    env.addRoad(3, 3, 4, 1, ROAD_OPEN);

    int batteryPerCostUnit = 1;
    int batteryPerWorkStep = 2;

    // Test 1: One request and one suitable robot — assignment succeeds.
    {
        RobotList robs;
        robs.addRobot(1, 1, CAPABILITY_MEDICAL, 100, 100);
        RequestList reqs;
        reqs.addRequestPriority(101, 0, 4, CATEGORY_MEDICAL, 3, 5); // Req battery: 5*1 + 5*2 = 15

        int count = assignPendingRequests(env, robs, reqs, 0, batteryPerCostUnit, batteryPerWorkStep);
        printTestResult("Test 1: One request & one suitable robot -> assignment succeeds",
                        count == 1 && robs.findRobot(1)->getAssignedRequestID() == 101);
    }

    // Test 2: Two requests and one robot — the robot is not assigned twice.
    {
        RobotList robs;
        robs.addRobot(1, 1, CAPABILITY_MEDICAL, 100, 100);
        RequestList reqs;
        reqs.addRequestPriority(101, 0, 4, CATEGORY_MEDICAL, 4, 5);
        reqs.addRequestPriority(102, 0, 4, CATEGORY_MEDICAL, 2, 5);

        int count = assignPendingRequests(env, robs, reqs, 0, batteryPerCostUnit, batteryPerWorkStep);
        printTestResult("Test 2: Two requests & one robot -> robot is assigned only once",
                        count == 1 && reqs.findRequest(101)->assignedRobotId == 1 && reqs.findRequest(102)->assignedRobotId == -1);
    }

    // Test 3: Robot with an incompatible capability — request remains pending.
    {
        RobotList robs;
        robs.addRobot(1, 1, CAPABILITY_FIRE, 100, 100); // FIRE robot
        RequestList reqs;
        reqs.addRequestPriority(101, 0, 4, CATEGORY_MEDICAL, 3, 5); // MEDICAL request

        int count = assignPendingRequests(env, robs, reqs, 0, batteryPerCostUnit, batteryPerWorkStep);
        printTestResult("Test 3: Incompatible capability -> request remains pending",
                        count == 0 && reqs.findRequest(101)->assignedRobotId == -1);
    }

    // Test 4: Robot with insufficient battery — request remains pending.
    {
        // Required battery for 1->4 is route 5*1 + work 5*2 = 15
        RobotList robs;
        robs.addRobot(1, 1, CAPABILITY_MEDICAL, 100, 10); // Only 10 battery
        RequestList reqs;
        reqs.addRequestPriority(101, 0, 4, CATEGORY_MEDICAL, 3, 5);

        int count = assignPendingRequests(env, robs, reqs, 0, batteryPerCostUnit, batteryPerWorkStep);
        printTestResult("Test 4: Insufficient battery -> request remains pending",
                        count == 0 && reqs.findRequest(101)->assignedRobotId == -1);
    }

    // Test 5: Robot with no available route — request remains pending.
    {
        RobotList robs;
        robs.addRobot(1, 1, CAPABILITY_MEDICAL, 100, 100);
        RequestList reqs;
        reqs.addRequestPriority(101, 0, 5, CATEGORY_MEDICAL, 3, 5); // Location 5 is disconnected!

        int count = assignPendingRequests(env, robs, reqs, 0, batteryPerCostUnit, batteryPerWorkStep);
        printTestResult("Test 5: No available route -> request remains pending",
                        count == 0 && reqs.findRequest(101)->assignedRobotId == -1);
    }

    // Test 6: Two suitable robots with different route costs — cheaper route is selected.
    {
        RobotList robs;
        robs.addRobot(1, 1, CAPABILITY_MEDICAL, 100, 100); // Loc 1->4 cost = 5
        robs.addRobot(2, 3, CAPABILITY_MEDICAL, 100, 100); // Loc 3->4 cost = 1 (Cheaper!)
        RequestList reqs;
        reqs.addRequestPriority(101, 0, 4, CATEGORY_MEDICAL, 3, 5);

        int count = assignPendingRequests(env, robs, reqs, 0, batteryPerCostUnit, batteryPerWorkStep);
        printTestResult("Test 6: Different route costs -> cheaper robot 2 selected",
                        count == 1 && robs.findRobot(2)->getAssignedRequestID() == 101);
    }

    // Test 7: Two suitable robots with equal route costs — lower robot ID is selected.
    {
        RobotList robs;
        robs.addRobot(5, 1, CAPABILITY_MEDICAL, 100, 100); // Loc 1, cost 5
        robs.addRobot(2, 1, CAPABILITY_MEDICAL, 100, 100); // Loc 1, cost 5, lower ID 2!
        RequestList reqs;
        reqs.addRequestPriority(101, 0, 4, CATEGORY_MEDICAL, 3, 5);

        int count = assignPendingRequests(env, robs, reqs, 0, batteryPerCostUnit, batteryPerWorkStep);
        printTestResult("Test 7: Equal route costs -> lower robot ID 2 selected",
                        count == 1 && robs.findRobot(2)->getAssignedRequestID() == 101);
    }

    // Test 8: Multiple pending requests — higher-priority request is considered first.
    {
        RobotList robs;
        robs.addRobot(1, 1, CAPABILITY_MEDICAL, 100, 100); // Only 1 robot available
        RequestList reqs;
        reqs.addRequestPriority(101, 0, 4, CATEGORY_MEDICAL, 2, 5); // Urgency 2
        reqs.addRequestPriority(102, 0, 4, CATEGORY_MEDICAL, 5, 5); // Urgency 5 (Higher priority!)

        int count = assignPendingRequests(env, robs, reqs, 0, batteryPerCostUnit, batteryPerWorkStep);
        printTestResult("Test 8: Priority ordering -> higher-urgency request 102 assigned",
                        count == 1 && reqs.findRequest(102)->assignedRobotId == 1 && reqs.findRequest(101)->assignedRobotId == -1);
    }

    // Test 9: Request reported in the future — it is not assigned.
    {
        RobotList robs;
        robs.addRobot(1, 1, CAPABILITY_MEDICAL, 100, 100);
        RequestList reqs;
        reqs.addRequestPriority(101, 10, 4, CATEGORY_MEDICAL, 5, 5); // Report time 10

        int count = assignPendingRequests(env, robs, reqs, 5, batteryPerCostUnit, batteryPerWorkStep); // currentTime 5
        printTestResult("Test 9: Future request (reported at t=10, current t=5) is not assigned",
                        count == 0 && reqs.findRequest(101)->assignedRobotId == -1);
    }

    // Test 10: General-capability robot — it can serve any valid request category.
    {
        RobotList robs;
        robs.addRobot(1, 1, CAPABILITY_GENERAL, 100, 100); // GENERAL robot
        RequestList reqs;
        reqs.addRequestPriority(101, 0, 4, CATEGORY_FIRE, 4, 3); // FIRE request

        int count = assignPendingRequests(env, robs, reqs, 0, batteryPerCostUnit, batteryPerWorkStep);
        printTestResult("Test 10: General capability robot successfully assigned to FIRE request",
                        count == 1 && robs.findRobot(1)->getAssignedRequestID() == 101);
    }
}
void runRobotManagerTests() {

    cout << "\n==========================================================\n";
    cout << "          ROBOT MANAGER COMPREHENSIVE VERIFICATION        \n";
    cout << "==========================================================\n";

    // Independent robots: these tests do not depend on any scenario.
    RobotList testRobots;

    testRobots.addRobot(1, 1, CAPABILITY_MEDICAL, 100, 100);
    testRobots.addRobot(2, 2, CAPABILITY_GENERAL, 100, 100);
    testRobots.addRobot(3, 3, CAPABILITY_SEARCH, 100, 100);
    testRobots.addRobot(4, 32, CAPABILITY_MEDICAL, 120, 120);
    testRobots.addRobot(8, 8, CAPABILITY_SEARCH, 90, 90);

    // ---------------------------------------------------------
    // [1] Capability Matching Tests
    // ---------------------------------------------------------
    cout << "\n--- [1] Capability Matching Tests ---\n";

    Robot* r1 = testRobots.findRobot(1);
    Robot* r2 = testRobots.findRobot(2);

    if (r1 && r2) {
        printTestResult(
            "MEDICAL robot accepts MEDICAL request",
            r1->canServe(CATEGORY_MEDICAL)
        );

        printTestResult(
            "MEDICAL robot rejects FIRE request",
            !r1->canServe(CATEGORY_FIRE)
        );

        printTestResult(
            "GENERAL robot accepts FIRE request",
            r2->canServe(CATEGORY_FIRE)
        );

        printTestResult(
            "GENERAL robot accepts SEARCH request",
            r2->canServe(CATEGORY_SEARCH)
        );
    }

    // ---------------------------------------------------------
    // [2] State Transition Enforcement
    // ---------------------------------------------------------
    cout << "\n--- [2] State Transition Enforcement ---\n";

    Robot* r3 = testRobots.findRobot(3);

    if (r3) {
        cout << "Robot 3 initial state: "
             << getRobotStateName(r3->getState()) << "\n";

        bool validTrans = r3->setState(ROBOT_TRAVELLING);

        printTestResult(
            "AVAILABLE -> TRAVELLING is allowed",
            validTrans
        );

        bool badStateTrans = r3->setState(99);

        printTestResult(
            "Transition to invalid state 99 is rejected",
            !badStateTrans
        );

        r3->setState(ROBOT_AVAILABLE);
    }

    // ---------------------------------------------------------
    // [3] Assignment & Release Lifecycle
    // ---------------------------------------------------------
    cout << "\n--- [3] Assignment & Release Lifecycle ---\n";

    if (r3) {
        bool assignOk = testRobots.assignRobot(3, 101);

        printTestResult(
            "Assign Request 101 to Robot 3",
            assignOk
        );

        bool doubleAssign = testRobots.assignRobot(3, 102);

        printTestResult(
            "Prevent second assignment to Robot 3",
            !doubleAssign
        );

        bool releaseOk = testRobots.releaseRobot(3);

        printTestResult(
            "Release Robot 3 from Request 101",
            releaseOk
        );

        bool assignAgain = testRobots.assignRobot(3, 103);

        if (assignAgain) {
            r3->setState(ROBOT_PERFORMING);
        }

        bool completeOk = testRobots.completeRobotTask(3);

        printTestResult(
            "Complete Robot 3's assigned task",
            assignAgain && completeOk
        );

        cout << "Robot 3 Tasks Completed: "
             << r3->getTasksCompleted() << "\n";
    }

    // ---------------------------------------------------------
    // [4] Movement & Resource Constraints
    // ---------------------------------------------------------
    cout << "\n--- [4] Movement & Resource Constraints ---\n";

    Robot* r4 = testRobots.findRobot(4);

    if (r4) {
        int initialMove = r4->getTotalMovementCost();
        int initialConsumed = r4->getTotalBatteryConsumed();

        int roadCost = 8;
        int batteryCost = 12;

        bool moved = r4->moveTo(39, roadCost, batteryCost);

        printTestResult(
            "Robot can move when it has sufficient battery",
            moved
        );

        cout << "New Location: " << r4->getCurrentLocation() << "\n";
        cout << "Movement Cost Added: "
             << r4->getTotalMovementCost() - initialMove << "\n";
        cout << "Battery Consumed Added: "
             << r4->getTotalBatteryConsumed() - initialConsumed << "\n";

        bool overBudgetMove = r4->moveTo(
            45, 10, r4->getCurrentBattery() + 50
        );

        printTestResult(
            "Reject movement when battery is insufficient",
            !overBudgetMove
        );
    }

    // ---------------------------------------------------------
    // [5] Charging Operations
    // ---------------------------------------------------------
    cout << "\n--- [5] Charging Operations & Station Availability ---\n";

    Robot* r8 = testRobots.findRobot(8);

    if (r8) {
        r8->consumeBattery(40);

        cout << "Battery before charging: "
             << r8->getCurrentBattery() << "/"
             << r8->getMaxBattery() << "\n";

        bool startDown = r8->startRecharging(false);

        printTestResult(
            "Reject charging at an offline station",
            !startDown
        );

        bool startUp = r8->startRecharging(true);

        printTestResult(
            "Allow charging at an online station",
            startUp
        );

        bool moveWhileCharging = r8->moveTo(15, 2, 2);

        printTestResult(
            "Prevent movement while recharging",
            !moveWhileCharging
        );

        int added = testRobots.rechargeRobotStep(8, 15, true);

        cout << "Battery added in recharge step: "
             << added << "\n";

        int addedDown = testRobots.rechargeRobotStep(8, 15, false);

        printTestResult(
            "Stop charging when station becomes unavailable",
            addedDown == 0 &&
            r8->getState() == ROBOT_AVAILABLE
        );
    }

    // ---------------------------------------------------------
    // [6] Offline / Online Handling
    // ---------------------------------------------------------
    cout << "\n--- [6] Offline / Online Handling ---\n";

    Robot* offlineRobot = testRobots.findRobot(1);

    if (offlineRobot) {
        offlineRobot->setState(ROBOT_TRAVELLING);

        bool offOk = testRobots.setRobotOffline(1);

        printTestResult(
            "Robot can be taken offline",
            offOk && offlineRobot->getState() == ROBOT_UNAVAILABLE
        );

        bool moveWhileOffline = offlineRobot->moveTo(10, 5, 5);

        printTestResult(
            "Prevent movement while robot is offline",
            !moveWhileOffline
        );

        bool duplicateOffline = testRobots.setRobotOffline(1);

        printTestResult(
            "Handle duplicate offline event safely",
            !duplicateOffline
        );

        bool onOk = testRobots.setRobotOnline(1);

        printTestResult(
            "Robot can be brought online",
            onOk && offlineRobot->getState() == ROBOT_TRAVELLING
        );
    }

    // ---------------------------------------------------------
    // [7] Fleet & Per-Robot Statistics
    // ---------------------------------------------------------
    cout << "\n--- [7] Fleet & Per-Robot Statistics ---\n";

    testRobots.displayStatistics();

    cout << "\n==========================================================\n";
    cout << "          ROBOT MANAGER TESTS COMPLETE                   \n";
    cout << "==========================================================\n";
}
int main(int argc, char* argv[]) {

    const char* filename = "data/scenario_simultaneous_events.txt";

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

    if (!loader.loadEnvironment(filename, environment, robots, requests)) {
        cout << "ERROR: Scenario loading failed!\n";
        return 1;
    }

    cout << "Scenario loaded successfully!\n\n";

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

    cout << "\n====================================\n";
    cout << "           ENVIRONMENT\n";
    cout << "====================================\n";

    cout << "Total Locations: " << environment.getLocationCount() << "\n";
    cout << "Total Roads: " << environment.getRoadCount() << "\n\n";

    environment.displayEnvironment();

    cout << "\n====================================\n";
    cout << "              ROBOTS\n";
    cout << "====================================\n";

    cout << "Total Robots: " << robots.getCount() << "\n\n";

    robots.displayAll();

    cout << "\n====================================\n";
    cout << "             REQUESTS\n";
    cout << "====================================\n";

    cout << "Total Requests: " << requests.getCount() << "\n\n";

    requests.displayAll();

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
    runAssignmentTests();

    // =========================================================================
    // ROBOT MANAGER / ROBOT OPERATIONS TEST SUITE
    // =========================================================================
    cout << "\n==========================================================\n";
    cout << "          ROBOT MANAGER COMPREHENSIVE VERIFICATION        \n";
    cout << "==========================================================\n";
    runRobotManagerTests();
    // =========================================================================
    // SCENARIO EVENT PROCESSING & LOGGING (scenario_provided.txt)
    // =========================================================================
    cout << "\n==========================================================\n";
    cout << "     SCENARIO EVENT PROCESSING (scenario_provided.txt)    \n";
    cout << "==========================================================\n";

Logger scenarioLogger;
EventManager scenarioEventManager;

scenarioEventManager.initialize(events, eventCount);

cout << "\nProcessing scheduled events from the loaded scenario...\n";

int totalProcessed = 0;

// Process events at each simulation timestep.
// This preserves their scheduled times and file order.
for (int time = 0; time <= p.maxSteps; time++) {

    int processed = scenarioEventManager.processEvents(
        time,
        environment,
        robots,
        requests,
        scenarioLogger
    );

    if (processed > 0) {
        cout << "Time " << time << ": "
             << processed << " event(s) processed\n";

        totalProcessed += processed;
    }
}

cout << "\n--- EVENT PROCESSING RESULTS ---\n";

cout << "Events defined in scenario: "
     << eventCount << "\n";

cout << "Events attempted: "
     << scenarioEventManager.getAttemptedEventCount() << "\n";

cout << "Events successfully applied: "
     << scenarioEventManager.getSuccessfulEventCount() << "\n";

cout << "Events failed: "
     << scenarioEventManager.getFailedEventCount() << "\n";

cout << "Events remaining: "
     << scenarioEventManager.getRemainingEventCount() << "\n";

printTestResult(
    "All scheduled events were attempted",
    scenarioEventManager.getRemainingEventCount() == 0
);

printTestResult(
    "Processed event count matches attempted event count",
    totalProcessed == scenarioEventManager.getAttemptedEventCount()
);

printTestResult(
    "Logger entries match successfully applied events",
    scenarioLogger.getCount() ==
    scenarioEventManager.getSuccessfulEventCount()
);


    // =========================================================
    // COMPLETE SIMULATION TEST
    // =========================================================

    cout << "\n==========================================================\n";
    cout << "              COMPLETE SIMULATION TEST                  \n";
    cout << "==========================================================\n";

    // Create fresh objects for the simulation.
    // This prevents previous unit tests from affecting it.

    Environment simulationEnvironment;
    RobotList simulationRobots;
    RequestList simulationRequests;
    ScenarioLoader simulationLoader;

    Logger simulationLogger;
    EventManager simulationEventManager;
    Statistics simulationStatistics;
    SimulationManager simulation;

    cout << "\nLoading a fresh scenario for simulation...\n";

    if (!simulationLoader.loadEnvironment(
            filename,
            simulationEnvironment,
            simulationRobots,
            simulationRequests)) {

        cout << "ERROR: Could not load scenario for simulation!\n";
        return 1;
    }

    cout << "Scenario loaded successfully!\n";

    // Initialize the SimulationManager.
    bool initialized = simulation.initialize(
        simulationEnvironment,
        simulationRobots,
        simulationRequests,
        simulationLoader,
        simulationEventManager,
        simulationLogger,
        simulationStatistics
    );

    if (!initialized) {
        cout << "ERROR: Simulation initialization failed!\n";
        return 1;
    }

    printTestResult(
        "Simulation initialized successfully",
        simulation.isInitialized()
    );

    // Display initial simulation status.
    cout << "\n--- INITIAL SIMULATION STATUS ---\n";
    simulation.displaySimulationStatus();

    // Run the complete simulation.
    cout << "\n--- RUNNING COMPLETE SIMULATION ---\n";

    bool simulationResult = simulation.run();
// Save output files using the scenario filename as the base name.
string outputBase = filename;

size_t slashPosition = outputBase.find_last_of("/\\");
if (slashPosition != string::npos) {
    outputBase = outputBase.substr(slashPosition + 1);
}

size_t extensionPosition = outputBase.find_last_of(".");
if (extensionPosition != string::npos) {
    outputBase = outputBase.substr(0, extensionPosition);
}

string logFilename = outputBase + "_logs.csv";
string statisticsFilename = outputBase + "_statistics.txt";

bool logSaved = simulationLogger.saveToFile(logFilename.c_str());
bool statisticsSaved =
    simulationStatistics.saveStatistics(statisticsFilename.c_str());

printTestResult("Simulation log saved to file", logSaved);
printTestResult("Simulation statistics saved to file", statisticsSaved);

if (logSaved) {
    cout << "Log file: " << logFilename << "\n";
}

if (statisticsSaved) {
    cout << "Statistics file: " << statisticsFilename << "\n";
}
    printTestResult(
        "Simulation run completed successfully",
        simulationResult && simulation.isFinished()
    );

    // Display final simulation status.
    cout << "\n--- FINAL SIMULATION STATUS ---\n";
    simulation.displaySimulationStatus();

    cout << "\n--- FINAL ROBOT STATISTICS ---\n";
    simulationRobots.displayStatistics();

    cout << "\n--- FINAL REQUEST STATISTICS ---\n";
    simulationRequests.displayStatistics();

    cout << "\n==========================================================\n";
    cout << "              SIMULATION TEST COMPLETE                  \n";
    cout << "==========================================================\n";

    return 0;
}
