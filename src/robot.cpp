#include <iostream>
#include "robot.h"

using namespace std;

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
    if (capability == CAPABILITY_FIRE) return "FIRE";
    if (capability == CAPABILITY_SUPPLY) return "SUPPLY";
    if (capability == CAPABILITY_SEARCH) return "SEARCH";
    if (capability == CAPABILITY_GENERAL) return "GENERAL";
    return "UNKNOWN";
}

const char* getRobotStateName(int state) {
    if (state == ROBOT_AVAILABLE) return "AVAILABLE";
    if (state == ROBOT_TRAVELLING) return "TRAVELLING";
    if (state == ROBOT_PERFORMING) return "PERFORMING";
    if (state == ROBOT_RETURNING) return "RETURNING";
    if (state == ROBOT_RECHARGING) return "RECHARGING";
    if (state == ROBOT_UNAVAILABLE) return "UNAVAILABLE";
    return "UNKNOWN";
}

int parseCapability(const char* text) {
    if (text == nullptr) return 0;
    if (stringEquals(text, "MEDICAL")) return CAPABILITY_MEDICAL;
    if (stringEquals(text, "FIRE")) return CAPABILITY_FIRE;
    if (stringEquals(text, "SUPPLY")) return CAPABILITY_SUPPLY;
    if (stringEquals(text, "SEARCH")) return CAPABILITY_SEARCH;
    if (stringEquals(text, "GENERAL")) return CAPABILITY_GENERAL;
    return 0;
}

// Robot Implementation
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
    tasksCompleted = 0;
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
    this->tasksCompleted = 0;
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

int Robot::getTotalMovementCost() const {
    return totalMovementCost;
}

int Robot::getTotalBatteryConsumed() const {
    return totalBatteryConsumed;
}

int Robot::getTasksCompleted() const {
    return tasksCompleted;
}

// Setters
void Robot::setCurrentLocation(int loc) {
    currentLocation = loc;
}

bool Robot::setState(int newState) {
    if (newState < ROBOT_AVAILABLE || newState > ROBOT_UNAVAILABLE) {
        return false;
    }
    state = newState;
    return true;
}

void Robot::setAssignedRequestID(int reqId) {
    assignedRequestID = reqId;
}

void Robot::clearAssignment() {
    assignedRequestID = -1;
}

// Operational methods
bool Robot::canServe(int requestCategory) const {
    // A GENERAL robot can serve any valid category
    if (capability == CAPABILITY_GENERAL) {
        return (requestCategory >= CATEGORY_MEDICAL && requestCategory <= CATEGORY_SEARCH);
    }
    return (capability == requestCategory);
}

bool Robot::isAvailable() const {
    return (state == ROBOT_AVAILABLE);
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

void Robot::recharge(int amount) {
    if (amount <= 0) {
        return;
    }
    currentBattery += amount;
    if (currentBattery > maxBattery) {
        currentBattery = maxBattery;
    }
}

bool Robot::moveTo(int newLocation, int batteryCost) {
    if (newLocation <= 0 || batteryCost < 0 || state == ROBOT_UNAVAILABLE || !hasEnoughBattery(batteryCost)) {
        return false;
    }
    currentLocation = newLocation;
    consumeBattery(batteryCost);
    totalMovementCost += batteryCost;
    return true;
}

void Robot::setOffline() {
    if (state != ROBOT_UNAVAILABLE) {
        savedState = state;
    }
    state = ROBOT_UNAVAILABLE;
}

void Robot::setOnline() {
    if (state == ROBOT_UNAVAILABLE) {
        if (savedState >= ROBOT_AVAILABLE && savedState < ROBOT_UNAVAILABLE) {
            state = savedState;
        } else {
            state = ROBOT_AVAILABLE;
        }
    }
}

void Robot::completeTask() {
    tasksCompleted++;
    assignedRequestID = -1;
    state = ROBOT_AVAILABLE;
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

// RobotNode
RobotNode::RobotNode(const Robot& r, RobotNode* n) {
    robot = r;
    next = n;
}

// RobotList
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
    // Reject invalid IDs or duplicate IDs
    if (robot.getId() <= 0 || findRobot(robot.getId()) != nullptr) {
        return false;
    }
    // Reject invalid start location
    if (robot.getStartLocation() <= 0) {
        return false;
    }
    // Reject invalid capability
    if (robot.getCapability() < CAPABILITY_MEDICAL || robot.getCapability() > CAPABILITY_GENERAL) {
        return false;
    }
    // Reject invalid battery limits
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
    robot->setOffline();
    return true;
}

bool RobotList::setRobotOnline(int id) {
    Robot* robot = findRobot(id);
    if (robot == nullptr) {
        return false;
    }
    robot->setOnline();
    return true;
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
    robot->recharge(amount);
    return true;
}

RobotNode* RobotList::getHead() const {
    return head;
}

int RobotList::getCount() const {
    return count;
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
