#include<iostream>
#include<climits>
#include "routing.h"

bool findShortestPath(const Environment &environment,int startID,int destinationID,int path[],
int pathCapacity,int &pathLength,int &totalCost){
    //Initialize
    pathLength=0;
    totalCost=0;
    if(path==nullptr || pathCapacity<=0){
        return false;
    }
    int locationCount=environment.getLocationCount();
    if(locationCount<=0){
        return false;
    }
    //Convert locationIDs into array indices
    int startIndex=environment.getLocationIndex(startID);
    int destinationIndex=environment.getLocationIndex(destinationID);
    if(startIndex==-1 || destinationIndex==-1){
        return false;
    }
    //Arrays for dijkstra
    int *distance=new int [locationCount];
    int *parent=new int [locationCount];
    bool *visited=new bool [locationCount];
    for(int i=0;i<locationCount;i++){
        visited[i]=false;
        distance[i]=INT_MAX;
        parent[i]=-1;
    }
    //Start of dijkstra algorith
    distance[startIndex]=0;
    for(int i=0;i<locationCount;i++){
        int currentIndex=-1;
        int minimumDistance=INT_MAX;
        //Find the unvisited location with the smallest distance
        for(int j=0;j<locationCount;j++){
            if(!visited[j] && distance[j]<minimumDistance){
                minimumDistance=distance[j];
                currentIndex=j;
            }
        }
        if(currentIndex==-1){
            break;
        }
        visited[currentIndex]=true;
        //Break if we reach the destination
        if(currentIndex==destinationIndex){
            break;
        }
        //Get current location
        const Location *currentLocation=environment.getLocationAt(currentIndex);
        if(currentLocation==nullptr){
            return false;
        }
        //Traverse the linked list
        const AdjNode *current=currentLocation->connections;
        while(current!=nullptr){
            int neighbourIndex=current->neighbourIndex;
            const Road *road=environment.getRoadAt(current->roadIndex);
            //Ignore invalid locations and roads
            if(neighbourIndex<0 || neighbourIndex>=locationCount || road==nullptr){
                current=current->next;
                continue;
            }
            //Ignore blocked roads
            if(road->status!=ROAD_OPEN){
                current=current->next;
                continue;
            }
            if(!visited[neighbourIndex]){
                if(distance[currentIndex]<=INT_MAX-road->cost){
                    int newDistance=distance[currentIndex]+road->cost;
                    //Relaxation
                    if(newDistance<distance[neighbourIndex]){
                        distance[neighbourIndex]=newDistance;
                        parent[neighbourIndex]=currentIndex;
                    }
                }
            }
            current=current->next;
        }
    }
    //Check if the destination is unreachable
    if(distance[destinationIndex]==INT_MAX){
        delete[] distance;
        delete[] visited;
        delete[] parent;

        return false;
    }
    //Count the number of locations in the path
    int currentIndex=destinationIndex;
    int count=0;
    bool reachedStart=false;
    while(currentIndex!=-1 && count<locationCount){
        count++;
        if(currentIndex==startIndex){
            reachedStart=true;
            break;
        }
        currentIndex=parent[currentIndex];
    }
    if(!reachedStart || count>pathCapacity){
        delete[] distance;
        delete[] visited;
        delete[] parent;

        return false;
    }
    //Reconstruct the path in reverse
    currentIndex=destinationIndex;
    for(int i=0;i<count;i++){
        const Location *location=environment.getLocationAt(currentIndex);
        if (location == nullptr) {
            delete[] distance;
            delete[] visited;
            delete[] parent;

            return false;
        }
        path[i]=location->id;
        currentIndex=parent[currentIndex];
    }
    //Reverse the path to get from start to destination
    for(int i=0;i<count/2;i++){
        int temp=path[i];
        path[i]=path[count-i-1];
        path[count-i-1]=temp;
    }
    //Set outputs
    pathLength=count;
    totalCost=distance[destinationIndex];

    delete[] distance;
    delete[] visited;
    delete[] parent;
    return true;
}