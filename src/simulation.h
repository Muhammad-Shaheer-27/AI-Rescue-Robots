#ifndef SIMULATION_H
#define SIMULATION_H
#include "environment.h"
#include "robot.h"
#include "rescue_request.h"
#include "scenario_loader.h"
#include "event_manager.h"
#include "logger.h"
#include "statistics.h"

class SimulationManager{
private:
    //Core modules
    Environment *environment;
    RobotList *robot;
    RequestList *requests;
    ScenarioLoader *scenario;
    EventManager *manager;
    Logger *logger;
    Statistics *statistics;

    //Simulation Time information
    int currentTime;
    int maxSteps;
    //Simulation Status
    bool initialized;
    bool finished;

    int *trackedRequestIDs;
    int *trackedWorkSteps;
    int trackedCapacity;

    //Simulation operations
    void processScheduledEvents();
    void assignRequests();
    void chargeForPendingRequests();

    void updateRobotMovement();
    void updateRobotWork();
    void updateRobotCharging();

    bool shouldTerminate() const;

    SimulationManager(const SimulationManager&);
    SimulationManager &operator=(const SimulationManager&);
public:
    SimulationManager();
    ~SimulationManager();

    //Initialize simulation using the loaded scenario
    bool initialize(Environment &env,RobotList &robots,RequestList &reqs,ScenarioLoader &loader,
    EventManager &eventMgr,Logger &log,Statistics &stats);

    //Process one step
    bool step();
    //Process entire simulation
    bool run();

    int getCurrentTime();
    int getMaxSteps();
    bool isInitialized();
    bool isFinished();

    void displaySimulationStatus() const;

};
#endif