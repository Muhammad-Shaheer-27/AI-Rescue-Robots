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
    if(from==nullptr || to==nullptr || fromID=toID){
        return false;
    }

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
    first->roadID=id;
    first->next=from->connections;
    //Attach the second node to the first node
    second->neighbourID=fromID;
    second->roadID=id;
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
Road *Environment::getRoad(int id){
    return findRoad(id);
}
int Environment::getLocationCount() const{
    return locationCount;
}
int Environment::getRoadCount() const{
    return roadCount;
}
void Environment::displayEnvironment() const{
    cout<<"\n========== ENVIROMENT ==========\n";
    cout<<"Total Locations: "<<locationCount<<endl;
    cout<<"Total Roads: "<<roadCount<<endl;

    for(int i=0;i<locationCount;i++){
        cout<<"Location ID: "<<locations[i].id<<endl;
        cout<<"Name: "<<locations[i].name<<endl;
        cout<<"Type: "<<locations[i].type<<endl;

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