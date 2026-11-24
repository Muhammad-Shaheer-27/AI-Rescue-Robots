#include <iostream>
#include "robot.h"

using namespace std;

// Custom character-by-character string comparison (No strcmp or <cstring> allowed)
bool stringEquals(const char* s1, const char* s2) {
    if (s1 == nullptr || s2 == nullptr) {
        return false;
    }
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] != s2[i]) {
            return false;
        }
        i++;
    }
    return (s1[i] == '\0' && s2[i] == '\0');
}

// Helper Functions
const char* getCapabilityName(int capability) {
    if (capability == CAPABILITY_MEDICAL) return "MEDICAL";
    if (capability == CAPABILITY_FIRE)    return "FIRE";
    if (capability == CAPABILITY_SUPPLY)  return "SUPPLY";
    if (capability == CAPABILITY_SEARCH)  return "SEARCH";
    if (capability == CAPABILITY_GENERAL) return "GENERAL";
    return "UNKNOWN";
}

const char* getRobotStateName(int state) {
    if (state == ROBOT_AVAILABLE)   return "AVAILABLE";
    if (state == ROBOT_TRAVELLING)  return "TRAVELLING";
    if (state == ROBOT_PERFORMING)  return "PERFORMING";
    if (state == ROBOT_RETURNING)   return "RETURNING";
    if (state == ROBOT_RECHARGING)  return "RECHARGING";
    if (state == ROBOT_UNAVAILABLE) return "UNAVAILABLE";
    return "UNKNOWN";
}

int parseCapability(const char* text) {
    if (text == nullptr) return 0;
    if (stringEquals(text, "MEDICAL")) return CAPABILITY_MEDICAL;
    if (stringEquals(text, "FIRE"))    return CAPABILITY_FIRE;
    if (stringEquals(text, "SUPPLY"))  return CAPABILITY_SUPPLY;
    if (stringEquals(text, "SEARCH"))  return CAPABILITY_SEARCH;
    if (stringEquals(text, "GENERAL")) return CAPABILITY_GENERAL;
    return 0;
}

bool isValidRobotState(int state) {
    return (state >= ROBOT_AVAILABLE && state <= ROBOT_UNAVAILABLE);
}

//------------------------------------------------------------
// Robot Implementation
//------------------------------------------------------------
Robot::Robot() {
    id = 0;
    currentLocation = 0;
    startLocation = 0;
    capability = 0;
    maxBattery = 0;
    currentBattery = 0;
    state = ROBOT_AVAILABLE;
    assignedRequestID = -1;
    savedState = ROBOT_AVAILABLE;

    totalMovementCost = 0;
    totalBatteryConsumed = 0;
    totalBatteryRecharged = 0;
    tasksCompleted = 0;
    totalAssignments = 0;
    totalReassignments = 0;
    totalStepsMoved = 0;
}

Robot::Robot(int id, int startLocation, int capability, int maxBattery, int initialBattery) {
    this->id = id;
    this->currentLocation = startLocation;
    this->startLocation = startLocation;
    this->capability = capability;
    this->maxBattery = maxBattery;
    this->currentBattery = initialBattery;
    this->state = ROBOT_AVAILABLE;
    this->assignedRequestID = -1;
    this->savedState = ROBOT_AVAILABLE;

    this->totalMovementCost = 0;
    this->totalBatteryConsumed = 0;
    this->totalBatteryRecharged = 0;
    this->tasksCompleted = 0;
    this->totalAssignments = 0;
    this->totalReassignments = 0;
    this->totalStepsMoved = 0;
}

// Getters
int Robot::getId() const {
    return id;
}

int Robot::getCurrentLocation() const {
    return currentLocation;
}

int Robot::getStartLocation() const {
    return startLocation;
}

int Robot::getCapability() const {
    return capability;
}

int Robot::getMaxBattery() const {
    return maxBattery;
}

int Robot::getCurrentBattery() const {
    return currentBattery;
}

int Robot::getState() const {
    return state;
}

int Robot::getAssignedRequestID() const {
    return assignedRequestID;
}

int Robot::getSavedState() const {
    return savedState;
}

// Statistics Getters
int Robot::getTotalMovementCost() const {
    return totalMovementCost;
}

int Robot::getTotalBatteryConsumed() const {
    return totalBatteryConsumed;
}

int Robot::getTotalBatteryRecharged() const {
    return totalBatteryRecharged;
}

int Robot::getTasksCompleted() const {
    return tasksCompleted;
}

int Robot::getTotalAssignments() const {
    return totalAssignments;
}

int Robot::getTotalReassignments() const {
    return totalReassignments;
}

int Robot::getTotalStepsMoved() const {
    return totalStepsMoved;
}

// Setters
void Robot::setCurrentLocation(int loc) {
    if (loc > 0) {
        currentLocation = loc;
    }
}

void Robot::setAssignedRequestID(int reqId) {
    assignedRequestID = reqId;
}

void Robot::clearAssignment() {
    assignedRequestID = -1;
}

//------------------------------------------------------------
// State Transition Validation
//------------------------------------------------------------
bool Robot::isValidStateTransition(int fromState, int toState, int savedState) {
    if (fromState == toState) {
        return true; // No-op transition
    }
    if (!isValidRobotState(toState) || !isValidRobotState(fromState)) {
        return false;
    }
    // Any active state can transition to UNAVAILABLE (offline event or critical failure)
    if (toState == ROBOT_UNAVAILABLE) {
        return true;
    }

    switch (fromState) {
        case ROBOT_AVAILABLE:
            // Can start travelling, perform if already at destination, or begin charging
            return (toState == ROBOT_TRAVELLING ||
                    toState == ROBOT_PERFORMING ||
                    toState == ROBOT_RETURNING ||
                    toState == ROBOT_RECHARGING);

        case ROBOT_TRAVELLING:
            // Can arrive at task location, abort to available, return, or reach charging station
            return (toState == ROBOT_PERFORMING ||
                    toState == ROBOT_AVAILABLE ||
                    toState == ROBOT_RETURNING ||
                    toState == ROBOT_RECHARGING);

        case ROBOT_PERFORMING:
            // Finished task (available or returning) or rerouted
            return (toState == ROBOT_AVAILABLE ||
                    toState == ROBOT_RETURNING ||
                    toState == ROBOT_TRAVELLING);

        case ROBOT_RETURNING:
            // Arrived at base, begins charging, or reassigned mid-return
            return (toState == ROBOT_AVAILABLE ||
                    toState == ROBOT_RECHARGING ||
                    toState == ROBOT_TRAVELLING);

        case ROBOT_RECHARGING:
            // Charging complete or preempted
            return (toState == ROBOT_AVAILABLE ||
                    toState == ROBOT_TRAVELLING);

        case ROBOT_UNAVAILABLE:
            // Coming back online: restore previous saved state or available
            return (toState == ROBOT_AVAILABLE || toState == savedState);

        default:
            return false;
    }
}

bool Robot::setState(int newState) {
    if (!isValidStateTransition(state, newState, savedState)) {
        return false;
    }
    state = newState;
    return true;
}

// Operational methods
bool Robot::canServe(int requestCategory) const {
    if (capability == CAPABILITY_GENERAL) {
        return (requestCategory >= CATEGORY_MEDICAL && requestCategory <= CATEGORY_SEARCH);
    }
    return (capability == requestCategory);
}

bool Robot::isAvailable() const {
    return (state == ROBOT_AVAILABLE && assignedRequestID == -1);
}

bool Robot::hasEnoughBattery(int requiredBattery) const {
    return (currentBattery >= requiredBattery);
}

bool Robot::consumeBattery(int amount) {
    if (amount < 0 || amount > currentBattery) {
        return false;
    }
    currentBattery -= amount;
    totalBatteryConsumed += amount;
    return true;
}

int Robot::recharge(int amount) {
    if (amount <= 0) {
        return 0;
    }
    int needed = maxBattery - currentBattery;
    int actual = (amount > needed) ? needed : amount;
    currentBattery += actual;
    totalBatteryRecharged += actual;
    return actual;
}

//------------------------------------------------------------
// Movement & Battery Separation
// (Routing module determines nextLocation; robot executes movement)
//------------------------------------------------------------
bool Robot::moveTo(int newLocation, int movementCost, int batteryCost) {
    // Basic guards
    if (newLocation <= 0 || movementCost < 0 || batteryCost < 0) {
        return false;
    }
    // Robot cannot move if it is offline or currently recharging
    if (state == ROBOT_UNAVAILABLE || state == ROBOT_RECHARGING) {
        return false;
    }
    // Battery resource check
    if (!hasEnoughBattery(batteryCost)) {
        return false;
    }

    currentLocation = newLocation;
    consumeBattery(batteryCost);
    totalMovementCost += movementCost; // Strictly tracked separately from battery
    totalStepsMoved++;

    // If available and now moving, update state to TRAVELLING
    if (state == ROBOT_AVAILABLE) {
        setState(ROBOT_TRAVELLING);
    }
    return true;
}

bool Robot::moveTo(int newLocation, int batteryCost) {
    // Backwards-compatible overload: uses batteryCost as movementCost
    return moveTo(newLocation, batteryCost, batteryCost);
}

//------------------------------------------------------------
// Assignment & Release Lifecycle
//------------------------------------------------------------
bool Robot::assignRequest(int requestID) {
    if (requestID <= 0) {
        return false;
    }
    // Must be available, not offline, not recharging, and without current assignment
    if (!isAvailable() || state == ROBOT_UNAVAILABLE || state == ROBOT_RECHARGING) {
        return false;
    }

    assignedRequestID = requestID;
    totalAssignments++;
    setState(ROBOT_TRAVELLING);
    return true;
}

bool Robot::releaseAssignment() {
    if (assignedRequestID == -1) {
        return false; // No assignment to release
    }
    assignedRequestID = -1;
    totalReassignments++;

    // Reset state to AVAILABLE if it was travelling or performing
    if (state == ROBOT_TRAVELLING || state == ROBOT_PERFORMING) {
        setState(ROBOT_AVAILABLE);
    }
    // If robot is currently offline, fix savedState so it doesn't wake up travelling/performing with no task
    if (state == ROBOT_UNAVAILABLE && (savedState == ROBOT_TRAVELLING || savedState == ROBOT_PERFORMING)) {
        savedState = ROBOT_AVAILABLE;
    }
    return true;
}

bool Robot::completeTask() {
    if (assignedRequestID == -1) {
        return false; // Cannot complete unassigned task
    }
    tasksCompleted++;
    assignedRequestID = -1;
    setState(ROBOT_AVAILABLE);
    return true;
}

//------------------------------------------------------------
// Charging Operations (Controlled by Simulation Manager)
//------------------------------------------------------------
bool Robot::startRecharging(bool stationAvailable) {
    if (!stationAvailable) {
        return false; // Station is offline or non-existent
    }
    if (state == ROBOT_UNAVAILABLE || state == ROBOT_RECHARGING) {
        return false;
    }
    if (isFullyCharged()) {
        return false; // No need to recharge if already at 100%
    }
    return setState(ROBOT_RECHARGING);
}

int Robot::stepRecharge(int amount, bool stationAvailable) {
    if (!stationAvailable) {
        // Station went down during charging: stop recharging and transition to AVAILABLE
        if (state == ROBOT_RECHARGING) {
            setState(ROBOT_AVAILABLE);
        }
        return 0;
    }
    if (state != ROBOT_RECHARGING || amount <= 0) {
        return 0;
    }

    int rechargedAmount = recharge(amount);
    if (isFullyCharged()) {
        setState(ROBOT_AVAILABLE); // Finished charging
    }
    return rechargedAmount;
}

bool Robot::stopRecharging() {
    if (state == ROBOT_RECHARGING) {
        return setState(ROBOT_AVAILABLE);
    }
    return false;
}

bool Robot::isFullyCharged() const {
    return (currentBattery >= maxBattery);
}

//------------------------------------------------------------
// Offline / Online Handling (With State Restoration)
//------------------------------------------------------------
bool Robot::setOffline() {
    if (state == ROBOT_UNAVAILABLE) {
        return false; // Already offline, safe duplicate event handling
    }
    savedState = state;
    state = ROBOT_UNAVAILABLE;
    return true;
}

bool Robot::setOnline() {
    if (state != ROBOT_UNAVAILABLE) {
        return false; // Already online
    }
    if (isValidRobotState(savedState) && savedState != ROBOT_UNAVAILABLE) {
        state = savedState;
    } else {
        state = ROBOT_AVAILABLE;
    }
    savedState = ROBOT_AVAILABLE;
    return true;
}

void Robot::display() const {
    cout << "Robot ID: " << id
         << " | Location: " << currentLocation
         << " | Capability: " << getCapabilityName(capability)
         << " | Battery: " << currentBattery << "/" << maxBattery
         << " | State: " << getRobotStateName(state);
    if (assignedRequestID != -1) {
        cout << " | Assigned Request: " << assignedRequestID;
    }
    cout << endl;
}

void Robot::displayStatistics() const {
    cout << "  Robot #" << id << " (" << getCapabilityName(capability) << "):"
         << " Moves=" << totalStepsMoved
         << " | Movement Cost=" << totalMovementCost
         << " | Battery Consumed=" << totalBatteryConsumed
         << " | Battery Recharged=" << totalBatteryRecharged
         << " | Assignments=" << totalAssignments
         << " | Reassignments=" << totalReassignments
         << " | Tasks Done=" << tasksCompleted << endl;
}

//------------------------------------------------------------
// RobotNode Implementation
//------------------------------------------------------------
RobotNode::RobotNode(const Robot& r, RobotNode* n) {
    robot = r;
    next = n;
}

//------------------------------------------------------------
// RobotList Implementation
//------------------------------------------------------------
RobotList::RobotList() {
    head = nullptr;
    tail = nullptr;
    count = 0;
}

RobotList::~RobotList() {
    clear();
}

void RobotList::clear() {
    RobotNode* current = head;
    while (current != nullptr) {
        RobotNode* temp = current;
        current = current->next;
        delete temp;
    }
    head = nullptr;
    tail = nullptr;
    count = 0;
}

bool RobotList::addRobot(const Robot& robot) {
    if (robot.getId() <= 0 || findRobot(robot.getId()) != nullptr) {
        return false;
    }
    if (robot.getStartLocation() <= 0) {
        return false;
    }
    if (robot.getCapability() < CAPABILITY_MEDICAL || robot.getCapability() > CAPABILITY_GENERAL) {
        return false;
    }
    if (robot.getMaxBattery() <= 0 ||
        robot.getCurrentBattery() < 0 ||
        robot.getCurrentBattery() > robot.getMaxBattery()) {
        return false;
    }

    RobotNode* newNode = new RobotNode(robot);
    if (newNode == nullptr) {
        return false;
    }

    if (head == nullptr) {
        head = newNode;
        tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }
    count++;
    return true;
}

bool RobotList::addRobot(int id, int startLocation, int capability, int maxBattery, int initialBattery) {
    Robot robot(id, startLocation, capability, maxBattery, initialBattery);
    return addRobot(robot);
}

Robot* RobotList::findRobot(int id) {
    RobotNode* current = head;
    while (current != nullptr) {
        if (current->robot.getId() == id) {
            return &(current->robot);
        }
        current = current->next;
    }
    return nullptr;
}

bool RobotList::removeRobot(int id) {
    RobotNode* current = head;
    RobotNode* prev = nullptr;

    while (current != nullptr) {
        if (current->robot.getId() == id) {
            if (prev == nullptr) {
                head = current->next;
            } else {
                prev->next = current->next;
            }
            if (current == tail) {
                tail = prev;
            }
            delete current;
            count--;
            return true;
        }
        prev = current;
        current = current->next;
    }
    return false;
}

bool RobotList::setRobotOffline(int id) {
    Robot* robot = findRobot(id);
    if (robot == nullptr) {
        return false;
    }
    return robot->setOffline();
}

bool RobotList::setRobotOnline(int id) {
    Robot* robot = findRobot(id);
    if (robot == nullptr) {
        return false;
    }
    return robot->setOnline();
}

bool RobotList::updateBattery(int id, int amount) {
    Robot* robot = findRobot(id);
    if (robot == nullptr) {
        return false;
    }
    if (amount < 0) {
        return robot->consumeBattery(-amount);
    } else {
        robot->recharge(amount);
        return true;
    }
}

bool RobotList::rechargeRobot(int id, int amount) {
    Robot* robot = findRobot(id);
    if (robot == nullptr) {
        return false;
    }
    return (robot->recharge(amount) > 0);
}

bool RobotList::assignRobot(int id, int requestId) {
    Robot* robot = findRobot(id);
    if (robot == nullptr) {
        return false;
    }
    return robot->assignRequest(requestId);
}

bool RobotList::releaseRobot(int id) {
    Robot* robot = findRobot(id);
    if (robot == nullptr) {
        return false;
    }
    return robot->releaseAssignment();
}

bool RobotList::completeRobotTask(int id) {
    Robot* robot = findRobot(id);
    if (robot == nullptr) {
        return false;
    }
    return robot->completeTask();
}

int RobotList::rechargeRobotStep(int id, int amount, bool stationAvailable) {
    Robot* robot = findRobot(id);
    if (robot == nullptr) {
        return 0;
    }
    return robot->stepRecharge(amount, stationAvailable);
}

RobotNode* RobotList::getHead() const {
    return head;
}

int RobotList::getCount() const {
    return count;
}

int RobotList::getCountByState(int state) const {
    int matching = 0;
    RobotNode* current = head;
    while (current != nullptr) {
        if (current->robot.getState() == state) {
            matching++;
        }
        current = current->next;
    }
    return matching;
}

int RobotList::getCountByCapability(int capability) const {
    int matching = 0;
    RobotNode* current = head;
    while (current != nullptr) {
        if (current->robot.getCapability() == capability) {
            matching++;
        }
        current = current->next;
    }
    return matching;
}

int RobotList::getTotalMovementCostAll() const {
    int total = 0;
    RobotNode* current = head;
    while (current != nullptr) {
        total += current->robot.getTotalMovementCost();
        current = current->next;
    }
    return total;
}

int RobotList::getTotalBatteryConsumedAll() const {
    int total = 0;
    RobotNode* current = head;
    while (current != nullptr) {
        total += current->robot.getTotalBatteryConsumed();
        current = current->next;
    }
    return total;
}

int RobotList::getTotalBatteryRechargedAll() const {
    int total = 0;
    RobotNode* current = head;
    while (current != nullptr) {
        total += current->robot.getTotalBatteryRecharged();
        current = current->next;
    }
    return total;
}

int RobotList::getTotalTasksCompletedAll() const {
    int total = 0;
    RobotNode* current = head;
    while (current != nullptr) {
        total += current->robot.getTasksCompleted();
        current = current->next;
    }
    return total;
}

int RobotList::getTotalAssignmentsAll() const {
    int total = 0;
    RobotNode* current = head;
    while (current != nullptr) {
        total += current->robot.getTotalAssignments();
        current = current->next;
    }
    return total;
}

int RobotList::getTotalReassignmentsAll() const {
    int total = 0;
    RobotNode* current = head;
    while (current != nullptr) {
        total += current->robot.getTotalReassignments();
        current = current->next;
    }
    return total;
}

void RobotList::displayAll() const {
    cout << "\n========== ROBOTS LIST ==========\n";
    cout << "Total Robots: " << count << endl;
    RobotNode* current = head;
    while (current != nullptr) {
        current->robot.display();
        current = current->next;
    }
    cout << "=================================\n";
}

void RobotList::displayStatistics() const {
    cout << "\n========== ROBOT FLEET STATISTICS ==========\n";
    cout << "Total Robots in Fleet:     " << count << endl;
    cout << "  Available:               " << getCountByState(ROBOT_AVAILABLE) << endl;
    cout << "  Travelling:              " << getCountByState(ROBOT_TRAVELLING) << endl;
    cout << "  Performing:              " << getCountByState(ROBOT_PERFORMING) << endl;
    cout << "  Returning:               " << getCountByState(ROBOT_RETURNING) << endl;
    cout << "  Recharging:              " << getCountByState(ROBOT_RECHARGING) << endl;
    cout << "  Unavailable (Offline):   " << getCountByState(ROBOT_UNAVAILABLE) << endl;
    cout << "--------------------------------------------\n";
    cout << "Total Tasks Completed:     " << getTotalTasksCompletedAll() << endl;
    cout << "Total Assignments Made:    " << getTotalAssignmentsAll() << endl;
    cout << "Total Reassignments:       " << getTotalReassignmentsAll() << endl;
    cout << "Total Movement Road Cost:  " << getTotalMovementCostAll() << endl;
    cout << "Total Battery Consumed:    " << getTotalBatteryConsumedAll() << endl;
    cout << "Total Battery Recharged:   " << getTotalBatteryRechargedAll() << endl;
    cout << "--------------------------------------------\n";
    cout << "Per-Robot Breakdown:\n";
    RobotNode* current = head;
    while (current != nullptr) {
        current->robot.displayStatistics();
        current = current->next;
    }
    cout << "============================================\n";
}
