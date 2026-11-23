#include <iostream>
#include "environment.h"
#include <cstring>
using namespace std;

Environment::Environment(){
    locations=nullptr;
    roads=nullptr;

    locationCount=0;
    roadCount=0;

    locationCapacity=0;
    roadCapacity=0;
}
Environment::~Environment(){
    clearAdjacencyLists();

    delete[] locations;
    delete[] roads;
    locations=nullptr;
    roads=nullptr;
}
void Environment::clearAdjacencyLists(){
    if(locations==nullptr){
        return;
    }
    //Delete all linked lists
    for(int i=0;i<locationCount;i++){
        AdjNode *current=locations[i].connections;
        while(current!=nullptr){
            AdjNode *temp=current;
            current=current->next;
            delete temp;
        }
        locations[i].connections=nullptr;
    }
}
bool Environment::initialize(int maxLocations,int maxRoads){
    //Handle invalid parameters
    if(maxLocations<0 || maxRoads<0){
        return false;
    }
    clearAdjacencyLists();
    delete[] locations;
    delete[] roads;

    //Form location and road arrays
    locationCount=0;
    roadCount=0;
    locationCapacity=maxLocations;
    roadCapacity=maxRoads;

    if(maxLocations>0){
        locations=new Location[maxLocations];
        for(int i=0;i<maxLocations;i++){
            locations[i].connections=nullptr;
            locations[i].stationAvailable=false;
        }
    }
    if(maxRoads>0){
        roads=new Road[maxRoads];
    }

    return true;
}
Location *Environment::findLocation(int id){
    for(int i=0;i<locationCount;i++){
        if(locations[i].id==id){
            return &locations[i];
        }
    }
    return nullptr;
}
Road *Environment::findRoad(int id){
    for(int i=0;i<roadCount;i++){
        if(roads[i].id==id){
            return &roads[i];
        }
    }
    return nullptr;
}
bool Environment::addLocation(int id,const char *name,int type){
    if(locations==nullptr || locationCount>=locationCapacity){
        return false;
    }
    if(name==nullptr || strlen(name)>=50){
        return false;
    }
    //Reject negative and non-unique ids
    if(id<=0 || findLocation(id)!=nullptr){
        return false;
    }
    if(type<LOCATION_JUNCTION || type>LOCATION_COMMAND){
        return false;
    }

    locations[locationCount].id=id;
    strcpy(locations[locationCount].name,name);
    locations[locationCount].type=type;
    locations[locationCount].connections=nullptr;
    locations[locationCount].stationAvailable=(type == LOCATION_CHARGING);
    locationCount++;
    
    return true;
}
bool Environment::addRoad(int id,int fromID,int toID,int cost,int status){
    if(roads==nullptr || roadCount>=roadCapacity){
        return false;
    }
    //Reject negative and non-unique ids
    if(id<=0 || findRoad(id)!=nullptr){
        return false;
    }
    //Reject negative costs
    if(cost<=0){
        return false;
    }
    if(status!=ROAD_OPEN && status!=ROAD_BLOCKED){
        return false;
    }

    Location *from=findLocation(fromID);
    Location *to=findLocation(toID);

    if(from==nullptr || to==nullptr || fromID==toID){
        return false;
    }

    int fromIndex=from-locations;
    int toIndex=to-locations;
    int currentRoadIndex=roadCount;

    //Create adjacency node for the first location
    AdjNode *first=new AdjNode;
    if(first==nullptr){
        return false;
    }
    //Create adjacency node for the second location
    AdjNode *second=new AdjNode;
    if(second==nullptr){
        delete first;
        return false;
    }
    //Attach the first node to the second node
    first->neighbourID=toID;
    first->neighbourIndex=toIndex;
    first->roadID=id;
    first->roadIndex=currentRoadIndex;
    first->next=from->connections;
    //Attach the second node to the first node
    second->neighbourID=fromID;
    second->neighbourIndex=fromIndex;
    second->roadID=id;
    second->roadIndex=currentRoadIndex;
    second->next=to->connections;

    //Insert both nodes into their list(head)
    from->connections=first;
    to->connections=second;

    //Store the actual road record
    roads[roadCount].id=id;
    roads[roadCount].fromLocation=fromID;
    roads[roadCount].toLocation=toID;
    roads[roadCount].cost=cost;
    roads[roadCount].status=status;

    roadCount++;

    return true;
}
Location *Environment::getLocation(int id){
    return findLocation(id);
}
const Location *Environment::getLocationAt(int index) const{
    if(index<0 || index>=locationCount){
        return nullptr;
    }
    return &locations[index];
}
Road *Environment::getRoad(int id){
    return findRoad(id);
}
const Road *Environment::getRoadAt(int index) const{
    if(index<0 || index>=roadCount){
        return nullptr;
    }
    return &roads[index];
}
int Environment::getLocationCount() const{
    return locationCount;
}
int Environment::getLocationIndex(int id) const{
    for(int i=0;i<locationCount;i++){
        if(locations[i].id==id){
            return i;
        }
    }
    return -1;
}
int Environment::getRoadCount() const{
    return roadCount;
}

// Station Availability Operations (Dynamic Events)
bool Environment::setStationAvailability(int locationID, bool available){
    Location *loc = findLocation(locationID);
    if(loc == nullptr || loc->type != LOCATION_CHARGING){
        return false;
    }
    loc->stationAvailable = available;
    return true;
}

bool Environment::isStationAvailable(int locationID) const{
    for(int i=0; i<locationCount; i++){
        if(locations[i].id == locationID){
            return (locations[i].type == LOCATION_CHARGING && locations[i].stationAvailable);
        }
    }
    return false;
}

// Road Status Operations (Dynamic Events)
bool Environment::blockRoad(int roadID){
    Road *road = findRoad(roadID);
    if(road == nullptr){
        return false;
    }
    road->status = ROAD_BLOCKED;
    return true;
}

bool Environment::unblockRoad(int roadID){
    Road *road = findRoad(roadID);
    if(road == nullptr){
        return false;
    }
    road->status = ROAD_OPEN;
    return true;
}

bool Environment::isRoadOpen(int roadID) const{
    for(int i=0; i<roadCount; i++){
        if(roads[i].id == roadID){
            return (roads[i].status == ROAD_OPEN);
        }
    }
    return false;
}

void Environment::displayEnvironment() const{
    cout<<"\n========== ENVIROMENT ==========\n";
    cout<<"Total Locations: "<<locationCount<<endl;
    cout<<"Total Roads: "<<roadCount<<endl;

    for(int i=0;i<locationCount;i++){
        cout<<"Location ID: "<<locations[i].id<<endl;
        cout<<"Name: "<<locations[i].name<<endl;
        cout<<"Type: "<<locations[i].type<<endl;
        if(locations[i].type == LOCATION_CHARGING){
            cout<<"Station Status: "<<(locations[i].stationAvailable ? "AVAILABLE" : "UNAVAILABLE")<<endl;
        }

        cout<<"Connections: ";
        AdjNode *current=locations[i].connections;
        if(current==nullptr){
            cout<<"None";
        }
        while(current!=nullptr){
            cout<<"[Neighbour: "<<current->neighbourID;
            cout<<", Road: "<<current->roadID<<"]";
            current=current->next;
        }
        cout<<endl;
    }

    cout<<"\n========== ROAD RECORDS ==========\n";
    for(int i=0;i<roadCount;i++){
        cout<<"Road ID: "<<roads[i].id<<endl;
        cout<<"From: "<<roads[i].fromLocation<<endl;
        cout<<"To: "<<roads[i].toLocation<<endl;
        cout<<"Cost: "<<roads[i].cost<<endl;

        if(roads[i].status==ROAD_OPEN){
            cout<<"Status: OPEN";
        }
        else{
            cout<<"Status: BLOCKED";
        }
        cout<<endl;
    }
}
