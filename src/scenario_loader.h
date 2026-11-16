#ifndef SCENARIO_LOADER_H
#define SCENARIO_LOADER_H
#include "environment.h"

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

    bool loadEnvironment(const char *filename,Environment &environment);

    const char *getScenarioName() const;
    const SimulationParameters& getParameters() const;
};

#endif