#include <iostream>

#include "environment.h"
#include "scenario_loader.h"
#include "robot.h"
#include "rescue_request.h"

using namespace std;

int main() {
    Environment environment;
    ScenarioLoader loader;
    RobotList robotList;
    RequestList requestList;

    cout << "\n==================================================" << endl;
    cout << "         FULL SCENARIO INTEGRATION TEST           " << endl;
    cout << "==================================================" << endl;

    // We now pass all structures to be populated directly from the text file
    if (loader.loadEnvironment("data/scenario_provided.txt", environment, robotList, requestList)) {
        
        cout << "\n--- [1] Environment Parameters ---" << endl;
        cout << "Scenario Name: " << loader.getScenarioName() << endl;
        cout << "Maximum Steps: " << loader.getParameters().maxSteps << endl;
        cout << "Speed: " << loader.getParameters().speed << endl;

        cout << "\n--- [2] Loaded Robots ---" << endl;
        // This will prove the loader successfully extracted the robot tokens
        robotList.displayAll();

        cout << "\n--- [3] Loaded Requests (Auto-Sorted) ---" << endl;
        // This will prove the loader extracted the requests AND your priority logic sorted them on arrival
        requestList.displayAll();

    } else {
        cout << "\nFailed to load the scenario file." << endl;
    }

    return 0;
}