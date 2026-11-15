#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H
#include "constants.h"

struct AdjNode{
    int neighbourID;
    int roadID;
    AdjNode *next;
};
struct Location{
    int id;
    char name[50];
    int type;
    AdjNode *connections;
};
struct Road{
    int id;
    int fromLocation;
    int toLocation;
    int cost;
    int status;
};
class Environment{
private:
    Location *locations;
    Road *roads;

    int locationCount;
    int roadCount;

    int locationCapacity;
    int roadCapacity;

    Location *findLocation(int id);
    Road *findRoad(int id);

    void clearAdjacencyLists();
    

    //To prevent shallow copy
    Environment(const Environment&);
    Environment &operator=(const Environment&);
public:
    Environment();
    ~Environment();

    bool initialize(int maxLocations,int maxRoads);

    bool addLocation(int id,const char *name,int type);
    bool addRoad(int id,int fromID,int toID,int cost,int status);

    Location *getLocation(int id);
    Road *getRoad(int id);

    int getLocationCount() const;
    int getRoadCount() const;
    void displayEnvironment() const;
};
#endif