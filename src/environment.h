#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H
#include "constants.h"

struct AdjNode{
    int neighbourID;
    int neighbourIndex;

    int roadID;
    int roadIndex;

    AdjNode *next;
};
struct Location{
    int id;
    char name[50];
    int type;
    AdjNode *connections;
    bool stationAvailable;
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

    const Location *getLocationAt(int index) const;
    const Road *getRoadAt(int index) const;

    int getLocationCount() const;
    int getRoadCount() const;
    int getLocationIndex(int id) const;

    // Station Availability Operations (Dynamic Events)
    bool setStationAvailability(int locationID, bool available);
    bool isStationAvailable(int locationID) const;

    // Road Status Operations (Dynamic Events)
    bool blockRoad(int roadID);
    bool unblockRoad(int roadID);
    bool isRoadOpen(int roadID) const;

    void displayEnvironment() const;
};
#endif
