#ifndef ROUTING_H
#define ROUTING_H
#include "environment.h"

bool findShortestPath(const Environment & environment,int startID,int destinationID,int path[],
int pathCapacity,int &pathLength,int &totalCost
);

#endif