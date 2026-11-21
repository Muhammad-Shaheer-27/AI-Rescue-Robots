#ifndef ROBOT_H
#define ROBOT_H

#include "constants.h"

// Helper functions for capability and state names (No enums used)
const char* getCapabilityName(int capability);
const char* getRobotStateName(int state);
int parseCapability(const char* text);
bool isValidRobotState(int state);

class Robot {
private:
    int id;
    int currentLocation;
    int startLocation;
    int capability;
    int maxBattery;
    int currentBattery;
    int state;
    int assignedRequestID;
    int savedState; // Preserves state when transitioning offline

    // Statistics tracking (tracked separately)
    int totalMovementCost;
    int totalBatteryConsumed;
    int totalBatteryRecharged;
    int tasksCompleted;
    int totalAssignments;
    int totalReassignments;
    int totalStepsMoved;

public:
    Robot();
    Robot(int id, int startLocation, int capability, int maxBattery, int initialBattery);

    // Getters
    int getId() const;
    int getCurrentLocation() const;
    int getStartLocation() const;
    int getCapability() const;
    int getMaxBattery() const;
    int getCurrentBattery() const;
    int getState() const;
    int getAssignedRequestID() const;
    int getSavedState() const;

    // Statistics Getters
    int getTotalMovementCost() const;
    int getTotalBatteryConsumed() const;
    int getTotalBatteryRecharged() const;
    int getTasksCompleted() const;
    int getTotalAssignments() const;
    int getTotalReassignments() const;
    int getTotalStepsMoved() const;

    // Setters
    void setCurrentLocation(int loc);
    bool setState(int newState);
    void setAssignedRequestID(int reqId);
    void clearAssignment();

    // State transition validation
    static bool isValidStateTransition(int fromState, int toState, int savedState = ROBOT_AVAILABLE);

    // Operational methods
    bool canServe(int requestCategory) const;
    bool isAvailable() const;
    bool hasEnoughBattery(int requiredBattery) const;
    bool consumeBattery(int amount);
    int recharge(int amount);

    // Movement (movementCost and batteryCost tracked strictly separately)
    bool moveTo(int newLocation, int movementCost, int batteryCost);
    bool moveTo(int newLocation, int batteryCost); // Convenience overload

    // Assignment lifecycle
    bool assignRequest(int requestID);
    bool releaseAssignment();
    bool completeTask();

    // Charging operations (Simulation Manager supplies station availability)
    bool startRecharging(bool stationAvailable);
    int stepRecharge(int amount, bool stationAvailable);
    bool stopRecharging();
    bool isFullyCharged() const;

    // Offline / Online handling
    bool setOffline();
    bool setOnline();

    // Display helpers
    void display() const;
    void displayStatistics() const;
};

// Custom Linked List Node
struct RobotNode {
    Robot robot;
    RobotNode* next;

    RobotNode(const Robot& r, RobotNode* n = nullptr);
};

// Custom Robot Linked List (Robot Manager)
class RobotList {
private:
    RobotNode* head;
    RobotNode* tail;
    int count;

    // Prevent shallow copy
    RobotList(const RobotList&);
    RobotList& operator=(const RobotList&);

public:
    RobotList();
    ~RobotList();

    void clear();

    bool addRobot(const Robot& robot);
    bool addRobot(int id, int startLocation, int capability, int maxBattery, int initialBattery);

    Robot* findRobot(int id);
    bool removeRobot(int id);

    // Operational helpers across the list
    bool setRobotOffline(int id);
    bool setRobotOnline(int id);
    bool updateBattery(int id, int amount);
    bool rechargeRobot(int id, int amount);

    // Assignment & Charging delegation
    bool assignRobot(int id, int requestId);
    bool releaseRobot(int id);
    bool completeRobotTask(int id);
    int rechargeRobotStep(int id, int amount, bool stationAvailable);

    // List traversal and query
    RobotNode* getHead() const;
    int getCount() const;
    int getCountByState(int state) const;
    int getCountByCapability(int capability) const;

    // Aggregated statistics across all robots
    int getTotalMovementCostAll() const;
    int getTotalBatteryConsumedAll() const;
    int getTotalBatteryRechargedAll() const;
    int getTotalTasksCompletedAll() const;
    int getTotalAssignmentsAll() const;
    int getTotalReassignmentsAll() const;

    void displayAll() const;
    void displayStatistics() const;
};

#endif
