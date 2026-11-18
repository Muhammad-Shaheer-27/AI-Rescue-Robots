#ifndef SCENARIO_LOADER_H
#define SCENARIO_LOADER_H
#include "environment.h"
#include "robot.h"
#include "rescue_request.h"

struct SimulationParameters{
    int maxSteps;
    int speed;
    int batteryPerCostUnit;
    int batteryPerWorkStep;
    int rechargePerStep;
};
class ScenarioLoader{
private:
    char scenarioName[50];
    SimulationParameters parameters;
public:
    ScenarioLoader();

    bool loadEnvironment(const char *filename,Environment &environment, RobotList &robots, RequestList &requests);

    const char *getScenarioName() const;
    const SimulationParameters& getParameters() const;
};

#endif