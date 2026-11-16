#include <iostream>

#include "environment.h"
#include "scenario_loader.h"

using namespace std;

int main() {

    Environment environment;
    ScenarioLoader loader;

    if (loader.loadEnvironment(
            "data/scenario_provided.txt", environment)) {

        cout << "\nScenario Name: "
             << loader.getScenarioName() << endl;

        cout << "\nMaximum Steps: "
             << loader.getParameters().maxSteps << endl;

        environment.displayEnvironment();
    }

    return 0;
}