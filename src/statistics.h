#ifndef STATISTICS_H
#define STATISTICS_H

#include "robot.h"
#include "rescue_request.h"
#include "logger.h"

class Statistics {
private:
    int totalSimulationSteps;
    int totalRobots;

    // Request metrics
    int totalRequests;
    int completedRequests;
    int failedRequests;
    int pendingRequests;
    int assignedRequests;
    int inProgressRequests;

    // Response time metrics (report time to completion time)
    double averageResponseTime;
    int maxResponseTime;

    // Robot & resource metrics
    double robotUtilization;
    int totalMovementCost;
    int totalBatteryConsumed;
    int totalBatteryRecharged;
    int totalReassignments;
    int totalConflicts;

public:
    Statistics();

    void reset();    //to reset variables so they dont mix when calc is called multiple times

    void calculate(
        const RobotList& robots,
        const RequestList& requests,
        const Logger& logger,
        int totalSimulationSteps
    );

    // Getters
    int getTotalSimulationSteps() const;
    int getTotalRobots() const;
    int getTotalRequests() const;
    int getCompletedRequests() const;
    int getFailedRequests() const;
    int getPendingRequests() const;
    int getAssignedRequests() const;
    int getInProgressRequests() const;
    double getAverageResponseTime() const;
    int getMaxResponseTime() const;
    double getRobotUtilization() const;
    int getTotalMovementCost() const;
    int getTotalBatteryConsumed() const;
    int getTotalBatteryRecharged() const;
    int getTotalReassignments() const;
    int getTotalConflicts() const;

    void displayStatistics() const;
    bool saveStatistics(const char* filename) const;
};

#endif