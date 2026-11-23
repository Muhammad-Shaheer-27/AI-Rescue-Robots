#ifndef ASSIGNMENT_H
#define ASSIGNMENT_H

#include "constants.h"
#include "environment.h"
#include "robot.h"
#include "rescue_request.h"
#include "routing.h"

// Assigns pending rescue requests to suitable robots based on priority,
// capability, shortest route cost, and battery constraints.
// Returns the total number of successful assignments made.
int assignPendingRequests(const Environment& environment,RobotList& robots,RequestList& requests,int currentTime,int batteryPerCostUnit,int batteryPerWorkStep);

#endif
