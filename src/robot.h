#ifndef ROBOT_H
#define ROBOT_H

#include "constants.h"

// Helper functions for capability and state names
const char* getCapabilityName(int capability);
const char* getRobotStateName(int state);
int parseCapability(const char* text);

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
    int savedState;

    // Statistics tracking
    int totalMovementCost;
    int totalBatteryConsumed;
    int tasksCompleted;

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
    int getTotalMovementCost() const;
    int getTotalBatteryConsumed() const;
    int getTasksCompleted() const;

    // Setters
    void setCurrentLocation(int loc);
    bool setState(int newState);
    void setAssignedRequestID(int reqId);
    void clearAssignment();

    // Operations
    bool canServe(int requestCategory) const;
    bool isAvailable() const;
    bool hasEnoughBattery(int requiredBattery) const;
    bool consumeBattery(int amount);
    void recharge(int amount);
    bool moveTo(int newLocation, int batteryCost);
    void setOffline();
    void setOnline();
    void completeTask();

    void display() const;
};

// Linked List Node
struct RobotNode {
    Robot robot;
    RobotNode* next;

    RobotNode(const Robot& r, RobotNode* n = nullptr);
};

// Robot Linked List
class RobotList {
private:
    RobotNode* head;
    RobotNode* tail;
    int count;

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

    bool setRobotOffline(int id);
    bool setRobotOnline(int id);
    bool updateBattery(int id, int amount);
    bool rechargeRobot(int id, int amount);

    RobotNode* getHead() const;
    int getCount() const;

    void displayAll() const;
};

#endif
