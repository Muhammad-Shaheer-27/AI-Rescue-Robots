#include <iostream>
#include "simulation.h"
#include "assignment.h"
#include "routing.h"
using namespace std;

//============================================================
//Helper Function:To find the road with the lowest cost
//============================================================
static const Road* findOpenRoadBetween(const Environment &environment,int fromID,int toID){
    const Road *bestRoad=nullptr;
    for(int i=0;i<environment.getRoadCount();i++){
        const Road *road=environment.getRoadAt(i);
        if(road==nullptr || road->status!=ROAD_OPEN){
            continue;
        }
        bool connectsLocations=(road->fromLocation == fromID &&road->toLocation == toID) ||
            (road->fromLocation == toID && road->toLocation == fromID);
        if(connectsLocations){
            if(bestRoad==nullptr || road->cost<bestRoad->cost){
                bestRoad=road;
            }
        }
    }
    return bestRoad;
}
//============================================================
//Helper Function:To find nearest charging station
//============================================================
static bool findNearestChargingStation(const Environment &environment,int startLocation,
int currentBattery,int batteryPerCostUnit,int path[],int pathCapacity,int &stationID,int &routeCost){
    int bestCost=2147483647;
    int bestStationID=-1;
    for (int i = 0; i < environment.getLocationCount(); i++) {
        const Location* location = environment.getLocationAt(i);
        if (location == nullptr) {
            continue;
        }
        // Only consider charging available charging stations
        if (location->type != LOCATION_CHARGING || !environment.isStationAvailable(location->id)) {
            continue;
        }
        int pathLength = 0;
        int totalCost = 0;
        // Find the shortest route to this station.
        bool found = findShortestPath(environment,startLocation,location->id,path,pathCapacity,pathLength,totalCost);
        if (!found) {
            continue;
        }
        // Calculate battery required to reach this station.
        int requiredBattery = totalCost * batteryPerCostUnit;

        if (currentBattery < requiredBattery) {
            continue;
        }
        // Choose the station with the lowest route cost
        // If costs are equal, choose the lower station ID
        if (totalCost < bestCost || (totalCost == bestCost && location->id < bestStationID)) {
            bestCost = totalCost;
            bestStationID = location->id;
        }
    }
    if (bestStationID == -1) {
        return false;
    }
    stationID = bestStationID;
    routeCost = bestCost;

    return true;
}
SimulationManager::SimulationManager(){
    environment=nullptr;
    robot=nullptr;
    requests=nullptr;
    scenario=nullptr;
    manager=nullptr;
    logger=nullptr;
    statistics=nullptr;

    currentTime=0;
    maxSteps=0;

    initialized=false;
    finished=false;

    trackedRequestIDs=nullptr;
    trackedWorkSteps=nullptr;
    trackedCapacity=0;
}
SimulationManager::~SimulationManager(){
    delete[] trackedRequestIDs;
    delete[] trackedWorkSteps;
}
bool SimulationManager::initialize(Environment &env,RobotList &robots,RequestList &reqs,ScenarioLoader &loader,
    EventManager &eventMgr,Logger &log,Statistics &stats){
        if(env.getLocationCount()<=0 || robots.getCount()<=0 || reqs.getCount()<=0){
            return false;
        }
        //Initialize variables
        environment=&env;
        robot=&robots;
        requests=&reqs;
        scenario=&loader;
        manager=&eventMgr;
        logger=&log;
        statistics=&stats;

        //Read simulation parameters
        const SimulationParameters &parameters=loader.getParameters();
        if (parameters.maxSteps <= 0 || parameters.speed <= 0 || parameters.batteryPerCostUnit < 0 ||
        parameters.batteryPerWorkStep < 0 || parameters.rechargePerStep < 0) {
            return false;
        }
        maxSteps=parameters.maxSteps;

        delete[] trackedRequestIDs;
        delete[] trackedWorkSteps;
        trackedCapacity=reqs.getCount();
        trackedRequestIDs=new int[trackedCapacity];
        trackedWorkSteps=new int[trackedCapacity];
        for(int i=0;i<trackedCapacity;i++){
            trackedRequestIDs[i]=-1;
            trackedWorkSteps[i]=0;
        }

        currentTime=0;
        finished=false;
        //Initialize the event manager
        manager->initialize(loader.getEvents(),loader.getEventCount());
        initialized=true;

        cout<<"\nSimulation initialized successfully"<<endl;
        cout<<"Scenario: "<<loader.getScenarioName()<<endl;
        cout<<"Maximum steps: "<<maxSteps<<endl;
        return true;
}
void SimulationManager::processScheduledEvents(){
    if(!initialized){
        return;
    }
    //Events processed in a specific timeframe
    int processed=manager->processEvents(currentTime,*environment,*robot,*requests,*logger);
}
void SimulationManager::assignRequests(){
        if(!initialized){
            return;
        }
        const SimulationParameters &parameters=scenario->getParameters();
        //Assign suitable robots to requests
        int assignedCount=assignPendingRequests(*environment,*robot,*requests,currentTime,
        parameters.batteryPerCostUnit,parameters.batteryPerWorkStep,logger);

        if(assignedCount>0){
            cout<<"\nTime: "<<currentTime<<": "<<assignedCount<<" request(s) assigned"<<endl;
        }
}
void SimulationManager::chargeForPendingRequests(){
    if(!initialized){
        return;
    }
    const SimulationParameters &parameters=scenario->getParameters();
    int locationCount=environment->getLocationCount();
    if(locationCount<=0){
        return;
    }

    int *routePath=new int[locationCount];
    int *stationPath=new int[locationCount];
    RobotNode *currentNode=robot->getHead();
    while(currentNode!=nullptr){
        Robot &currentRobot=currentNode->robot;
        if(currentRobot.getState()==ROBOT_AVAILABLE && !currentRobot.isFullyCharged()){
            bool needsChargeForRequest=false;
            RequestNode *reqNode=requests->getHead();
            while(reqNode!=nullptr){
                RescueRequest &req=reqNode->request;
                if(req.isAvailableAt(currentTime) && currentRobot.canServe(req.category)){
                    int pathLength=0;
                    int routeCost=0;
                    if(findShortestPath(*environment,currentRobot.getCurrentLocation(),req.locationId,
                        routePath,locationCount,pathLength,routeCost)){
                        int requiredBattery=(routeCost * parameters.batteryPerCostUnit) +
                            (req.durationSteps * parameters.batteryPerWorkStep);
                        // Include return to charger battery margin
                        int returnStationID=-1;
                        int returnRouteCost=0;
                        if(parameters.batteryPerCostUnit>0 && findNearestChargingStation(*environment,req.locationId,
                            2147483647,parameters.batteryPerCostUnit,stationPath,locationCount,returnStationID,returnRouteCost)){
                            requiredBattery+=returnRouteCost * parameters.batteryPerCostUnit;
                        }
                        if(!currentRobot.hasEnoughBattery(requiredBattery)){
                            needsChargeForRequest=true;
                            break;
                        }
                    }
                }
                reqNode=reqNode->next;
            }
            if(needsChargeForRequest){
                int nearestStationID=-1;
                int stationRouteCost=0;
                if(findNearestChargingStation(*environment,currentRobot.getCurrentLocation(),
                    currentRobot.getCurrentBattery(),parameters.batteryPerCostUnit,
                    stationPath,locationCount,nearestStationID,stationRouteCost)){
                    if(nearestStationID==currentRobot.getCurrentLocation()){
                        currentRobot.startRecharging(true);
                    }
                    else{
                        currentRobot.setState(ROBOT_RETURNING);
                    }
                }
            }
        }
        currentNode=currentNode->next;
    }
    delete[] routePath;
    delete[] stationPath;
}

void SimulationManager::updateRobotMovement() {

    if (!initialized) {
        return;
    }

    const SimulationParameters& parameters = scenario->getParameters();

    int speed = parameters.speed;
    int batteryPerCostUnit = parameters.batteryPerCostUnit;

    int locationCount = environment->getLocationCount();

    if (locationCount <= 0 || speed <= 0) {
        return;
    }

    int* path = new int[locationCount];
    RobotNode* currentNode = robot->getHead();
    while (currentNode != nullptr) {
        Robot& currentRobot = currentNode->robot;
        int currentState = currentRobot.getState();
        // Only travelling and returning robots can move
        if (currentState != ROBOT_TRAVELLING && currentState != ROBOT_RETURNING) {
            currentNode = currentNode->next;
            continue;
        }
        int destinationID = -1;
        int requestID = currentRobot.getAssignedRequestID();

        // CASE 1: Robot is travelling to a rescue request.
        if (currentState == ROBOT_TRAVELLING) {
            RescueRequest* request = requests->findRequest(requestID);
            if (request == nullptr) {
                currentNode = currentNode->next;
                continue;
            }
            destinationID = request->locationId;
        }

        // CASE 2: Robot is returning to a charging station.
        else if (currentState == ROBOT_RETURNING) {
            int nearestStationID = -1;
            int routeCost = 0;
            bool stationFound = findNearestChargingStation(*environment,currentRobot.getCurrentLocation(),
                currentRobot.getCurrentBattery(),batteryPerCostUnit,path,locationCount,nearestStationID,
                routeCost);
            if (!stationFound) {
                currentNode = currentNode->next;
                continue;
            }
            destinationID = nearestStationID;
        }
        int currentLocation = currentRobot.getCurrentLocation();
        // Check whether the robot has already reached its destination.
        if (currentLocation == destinationID) {
            if (currentState == ROBOT_RETURNING) {
                currentRobot.startRecharging(environment->isStationAvailable(currentLocation));
            }
            else {
                logger->addEntry(currentTime,LOG_ROBOT_ARRIVED,currentRobot.getId(),requestID,currentLocation);
            }
            currentNode = currentNode->next;
            continue;
        }
        // Calculate the shortest path to the destination.
        int pathLength = 0;
        int totalCost = 0;
        bool pathFound = findShortestPath(*environment,currentLocation,destinationID,path,locationCount,
            pathLength,totalCost);
        if (!pathFound || pathLength < 2) {
            if (currentState == ROBOT_TRAVELLING && requestID != -1) {
                currentRobot.releaseAssignment();
                requests->clearAssignedRobot(requestID);
                logger->addEntry(currentTime,LOG_ROBOT_REASSIGNED,currentRobot.getId(),requestID,currentLocation);
            }
            currentNode = currentNode->next;
            continue;
        }
        // Move according to the scenario's speed parameter
        for (int move = 0;move < speed && move < pathLength - 1;move++) {
            int fromID = currentRobot.getCurrentLocation();
            int toID = path[move + 1];
            // Find the road connecting these locations.
            const Road* road = findOpenRoadBetween(*environment,fromID,toID);
            if (road == nullptr) {
                break;
            }
            
            int movementCost = road->cost;
            int batteryCost = movementCost * batteryPerCostUnit;
            // Do not move if the robot lacks sufficient battery
            if (!currentRobot.hasEnoughBattery(batteryCost)) {
                break;
            }
            // Move the robot.
            bool moved = currentRobot.moveTo(toID,movementCost,batteryCost);
            if (!moved) {
                break;
            }
            // Record movement
            logger->addEntry(currentTime,LOG_ROBOT_MOVED,currentRobot.getId(),requestID,toID,
                road->id,movementCost);
            // Record battery consumption
            if (batteryCost > 0) {
                logger->addEntry(currentTime,LOG_BATTERY_CONSUMED,currentRobot.getId(),requestID,toID,road->id,batteryCost);
            }
            // Check whether the robot has arrived
            if (toID == destinationID) {
                if (currentState == ROBOT_RETURNING) {
                    currentRobot.startRecharging(environment->isStationAvailable(toID));
                }
                else {
                    logger->addEntry(currentTime,LOG_ROBOT_ARRIVED,currentRobot.getId(),requestID,toID);
                }
                break;
            }
        }
        currentNode = currentNode->next;
    }
    delete[] path;
}
void SimulationManager::updateRobotWork(){
    if(!initialized){
        return;
    }
    const SimulationParameters &parameters=scenario->getParameters();
    
    int batteryPerWorkStep=parameters.batteryPerWorkStep;
    RobotNode *current=robot->getHead();
    while(current!=nullptr){
        Robot &currentRobot=current->robot;

        //Only use travelling and performing robots
        int state=currentRobot.getState();
        if(state!=ROBOT_TRAVELLING && state!=ROBOT_PERFORMING){
            current=current->next;
            continue;
        }
        //Get robot request id
        int requestID=currentRobot.getAssignedRequestID();
        RescueRequest *req=requests->findRequest(requestID);
        if(req==nullptr){
            current=current->next;
            continue;
        }
        if(req->assignedRobotId!=currentRobot.getId()){
            current=current->next;
            continue;
        }
        
        //Start task when robot reached destination
        if(state==ROBOT_TRAVELLING){
            if(currentRobot.getCurrentLocation()!=req->locationId){
                current=current->next;
                continue;
            }
            //Update request status
            if(!requests->updateStatus(requestID,REQUEST_IN_PROGRESS)){
                current=current->next;
                continue;
            }
            //Change robot state
            if(!currentRobot.setState(ROBOT_PERFORMING)){
                current=current->next;
                continue;
            }
            //Record task start
            logger->addEntry(currentTime,LOG_TASK_STARTED,currentRobot.getId(),requestID,
            req->locationId);
            cout<<"\nTime "<<currentTime<<" :Robot "<<currentRobot.getId()<<" started request"<<
            requestID<<endl;
        }
        //Execute one work per step
        if(currentRobot.getState()==ROBOT_PERFORMING){
            //Consume battery for one work per step
            if(!currentRobot.consumeBattery(batteryPerWorkStep)){
                cout<<"\nTime "<<currentTime<<": Robot "<<currentRobot.getId()<<
                "ran out of battery during Request "<<requestID<<endl;
                //Set status to failed
                requests->updateStatus(requestID,REQUEST_FAILED);
                //Record task failure
                logger->addEntry(currentTime,LOG_TASK_FAILED,currentRobot.getId(),requestID,
                req->locationId);

                //Clear the robot assignment 
                currentRobot.clearAssignment();
                currentRobot.setState(ROBOT_AVAILABLE);

                //Clear requests robot reference
                requests->clearAssignedRobot(requestID);
                current=current->next;
                continue;
            }
            //Record battery consumption
            if(batteryPerWorkStep>0){
                logger->addEntry(currentTime,LOG_BATTERY_CONSUMED,currentRobot.getId(),requestID,
                req->locationId,-1,batteryPerWorkStep);
            }
            int trackerIndex=-1;
            for(int i=0;i<trackedCapacity;i++){
                if(trackedRequestIDs[i]==requestID){
                    trackerIndex=i;
                    break;
                }
            }
            if(trackerIndex==-1){
                for(int i=0;i<trackedCapacity;i++){
                    if(trackedRequestIDs[i]==-1){
                        trackedRequestIDs[i]=requestID;
                        trackedWorkSteps[i]=0;
                        trackerIndex=i;
                        break;
                    }
                }
            }
            if(trackerIndex==-1){
                current=current->next;
                continue;
            }
            trackedWorkSteps[trackerIndex]++;
            int completedWorkSteps=trackedWorkSteps[trackerIndex];

            //Complete task if duration is reached
            if(completedWorkSteps>=req->durationSteps){
                bool requestCompleted=requests->updateStatus(requestID,REQUEST_COMPLETED);
                bool robotCompleted=currentRobot.completeTask();
                if(requestCompleted && robotCompleted){
                    logger->addEntry(currentTime,LOG_TASK_COMPLETED,currentRobot.getId(),requestID,
                    req->locationId);
                    cout<<"\nTime "<<currentTime<<": Robot "<<currentRobot.getId()<<" completed Request"
                    <<requestID<<endl;
                }
            }
        }
        current=current->next;
    }
}
void SimulationManager::updateRobotCharging(){
    if(!initialized){
        return;
    }
    const SimulationParameters &parameters=scenario->getParameters();

    int locationCount=environment->getLocationCount();
    if(locationCount<=0){
        return;
    }

    int *path=new int[locationCount];
    RobotNode *currentNode=robot->getHead();
    while(currentNode!=nullptr){
        Robot &currentRobot=currentNode->robot;
        int currentState=currentRobot.getState();
        int currentLocation=currentRobot.getCurrentLocation();

        //Case#1:Robot is already charging
        if(currentState==ROBOT_RECHARGING){
            bool stationAvailable=environment->isStationAvailable(currentLocation);
            int rechargedAmount=currentRobot.stepRecharge(parameters.rechargePerStep,stationAvailable);
            
            if(rechargedAmount>0){
                logger->addEntry(currentTime,LOG_BATTERY_RECHARGED,currentRobot.getId(),-1,currentLocation,
                -1,rechargedAmount);
            }
            currentNode=currentNode->next;
            continue;
        }
        
        //Case#2: Robot is returning to a charging station
        if(currentState==ROBOT_RETURNING){
            const Location *location=environment->getLocation(currentLocation);
            if(location!=nullptr && location->type==LOCATION_CHARGING &&
            environment->isStationAvailable(currentLocation)){
                if(currentRobot.isFullyCharged()){
                    currentRobot.setState(ROBOT_AVAILABLE);
                }
                else{
                    currentRobot.startRecharging(true);
                }
            }
            currentNode=currentNode->next;
            continue;
        }

        //Case#3: Check whether an available robot needs charging
        if(currentState==ROBOT_AVAILABLE){
            int maxBattery=currentRobot.getMaxBattery();
            int currentBattery=currentRobot.getCurrentBattery();

            // Return when the battery is <=25%
            int batteryThreshold=maxBattery/4;
            if(currentBattery<=batteryThreshold){
                int nearestStationID=-1;
                int routeCost=0;
                bool stationFound= findNearestChargingStation(*environment,currentLocation,currentBattery,
                    parameters.batteryPerCostUnit,path,locationCount,nearestStationID,routeCost);
                if(stationFound){
                    //Robot already present at the charging station
                    if(nearestStationID==currentLocation){
                        currentRobot.startRecharging(true);
                    }
                    else{
                        currentRobot.setState(ROBOT_RETURNING);
                    }
                }
            }
        }
        currentNode=currentNode->next;
    }
    delete[] path;
}
bool SimulationManager::shouldTerminate() const{
    if(!initialized){
        return true;
    }
    //Max simulation time reached
    if(currentTime>=maxSteps){
        return true;
    }

    //Count completed and failed requests
    int completedRequests=requests->getCountByStatus(REQUEST_COMPLETED);
    int failedRequests=requests->getCountByStatus(REQUEST_FAILED);

    int totalRequests=requests->getCount();

    //All requests finished
    if(completedRequests+failedRequests==totalRequests && !manager->hasPendingEvents()){
        return true;
    }
    return false;
}
bool SimulationManager::step(){
    if(!initialized){
        return false;
    }
    if(finished){
        return false;
    }
    if(shouldTerminate()){
        finished=true;
        return false;
    }

    cout<<"\n==========Time Step "<<currentTime<<"=========="<<endl;
    //Apply road, station and urgency events
    processScheduledEvents();
    //Recharge robots or start return journey
    updateRobotCharging();
    //Assign pending requests
    assignRequests();
    chargeForPendingRequests();
    //Move robots towards destinatons
    updateRobotMovement();
    //Execcute tasks
    updateRobotWork();

    currentTime++;
    statistics->calculate(*robot,*requests,*logger,currentTime);
    if(shouldTerminate()){
        finished=true;
    }
    return true;
}
bool SimulationManager::run(){
    if(!initialized){
        return false;
    }
    if(finished){
        cout<<"\nSimulation has already finished"<<endl;
        return true;
    }
    cout<<"======================================="<<endl;
    cout<<"          Simulation Started"<<endl;
    cout<<"======================================="<<endl;
    while(!finished){
        if(!step()){
            break;
        }
    }
    cout<<"======================================="<<endl;
    cout<<"          Simulation Finished"<<endl;
    cout<<"======================================="<<endl;
    cout<<"Total Simulation Steps: "<<currentTime<<endl;
    cout<<"Final Statistics: "<<endl;
    statistics->displayStatistics();

    return true;
}
int SimulationManager::getCurrentTime(){
    return currentTime;
}
int SimulationManager::getMaxSteps(){
    return maxSteps;
}
bool SimulationManager::isInitialized(){
    return initialized;
}
bool SimulationManager::isFinished(){
    return finished;
}

void SimulationManager::displaySimulationStatus() const {
    if (!initialized) {
        cout << "\nSimulation has not been initialized.\n";
        return;
    }
    cout<<"\n========== SIMULATION STATUS ==========\n";
    cout<<"Scenario: "<<scenario->getScenarioName()<<endl;
    cout<<"Current time: "<<currentTime<<endl;
    cout<<"Maximum steps: "<<maxSteps<<endl;
    cout << "Simulation state: ";
    if (finished) {
        cout << "Finished";
    }
    else {
        cout << "Running";
    }
    cout<< "\n\nRobot Status:\n";

    cout<< "Available: "<< robot->getCountByState(ROBOT_AVAILABLE) << endl;

    cout<< "Travelling: "<< robot->getCountByState(ROBOT_TRAVELLING) << endl;

    cout<< "Performing: "<< robot->getCountByState(ROBOT_PERFORMING) << endl;

    cout<< "Returning: "<< robot->getCountByState(ROBOT_RETURNING) << endl;

    cout<< "Recharging: "<< robot->getCountByState(ROBOT_RECHARGING) << endl;

    cout<< "Unavailable: "<< robot->getCountByState(ROBOT_UNAVAILABLE) << endl;

    cout<< "\nRequest Status:\n";

    cout<< "Pending: "<< requests->getCountByStatus(REQUEST_PENDING) << endl;

    cout<< "Assigned: "<< requests->getCountByStatus(REQUEST_ASSIGNED) << endl;

    cout<< "In Progress: "<< requests->getCountByStatus(REQUEST_IN_PROGRESS) << endl;

    cout<< "Completed: "<< requests->getCountByStatus(REQUEST_COMPLETED) << endl;

    cout<< "Failed: "<< requests->getCountByStatus(REQUEST_FAILED) << endl;

    cout<< "=======================================\n";
}