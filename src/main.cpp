#include <iostream>
#include "scenario_loader.h"

using namespace std;

int main(int argc, char* argv[]) {

    const char* filename = "data/scenario_provided.txt";

    // Allow a filename to be supplied through the command
    if (argc > 1) {
        filename = argv[1];
    }

    Environment environment;
    RobotList robots;
    RequestList requests;
    ScenarioLoader loader;

    cout << "====================================\n";
    cout << "       AI RESCUE ROBOTS TEST\n";
    cout << "====================================\n\n";

    cout << "Loading scenario: " << filename << "\n\n";

    // Load the complete scenario
    if (!loader.loadEnvironment(filename, environment, robots, requests)) {
        cout << "ERROR: Scenario loading failed!\n";
        return 1;
    }

    cout << "Scenario loaded successfully!\n\n";

    // Display scenario information
    cout << "====================================\n";
    cout << "        SCENARIO INFORMATION\n";
    cout << "====================================\n";

    cout << "Scenario Name: " << loader.getScenarioName() << "\n";

    const SimulationParameters& p = loader.getParameters();

    cout << "Maximum Steps: " << p.maxSteps << "\n";
    cout << "Robot Speed: " << p.speed << "\n";
    cout << "Battery Per Cost Unit: " << p.batteryPerCostUnit << "\n";
    cout << "Battery Per Work Step: " << p.batteryPerWorkStep << "\n";
    cout << "Recharge Per Step: " << p.rechargePerStep << "\n";

    // Display environment
    cout << "\n====================================\n";
    cout << "           ENVIRONMENT\n";
    cout << "====================================\n";

    cout << "Total Locations: " << environment.getLocationCount() << "\n";
    cout << "Total Roads: " << environment.getRoadCount() << "\n\n";

    environment.displayEnvironment();

    // Display robots
    cout << "\n====================================\n";
    cout << "              ROBOTS\n";
    cout << "====================================\n";

    cout << "Total Robots: " << robots.getCount() << "\n\n";

    robots.displayAll();

    // Display requests
    cout << "\n====================================\n";
    cout << "             REQUESTS\n";
    cout << "====================================\n";

    cout << "Total Requests: " << requests.getCount() << "\n\n";

    requests.displayAll();

    // Display scheduled events
    cout << "\n====================================\n";
    cout << "        SCHEDULED EVENTS\n";
    cout << "====================================\n";

    const ScheduledEvent* events = loader.getEvents();
    int eventCount = loader.getEventCount();

    cout << "Total Events: " << eventCount << "\n\n";

    for (int i = 0; i < eventCount; i++) {
        cout << "Event " << i + 1 << "\n";
        cout << "Time: " << events[i].time << "\n";
        cout << "Type: " << events[i].type << "\n";
        cout << "Target ID: " << events[i].targetID << "\n";
        cout << "Value: " << events[i].value << "\n";
        cout << "-------------------------\n";
    }

    cout << "\n====================================\n";
    cout << "       LOADING TEST COMPLETE\n";
    cout << "====================================\n";

    return 0;
}
