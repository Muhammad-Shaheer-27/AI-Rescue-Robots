// Section A - Header files
#include <iostream>
#include <string>
#include "simulation.h"
#include "scenario_loader.h"
#include "environment.h"
#include "robot.h"
#include "rescue_request.h"
#include "event_manager.h"
#include "logger.h"
#include "statistics.h"

using namespace std;

// Helper to extract the base filename (e.g., "data/scenario_provided.txt" -> "scenario_provided")
static string extractBaseName(const string &filePath) {
    int lastSlash = -1;
    for (int i = 0; i < (int)filePath.length(); i++) {
        if (filePath[i] == '/' || filePath[i] == '\\') {
            lastSlash = i;
        }
    }

    int start = lastSlash + 1;
    int lastDot = -1;
    for (int i = start; i < (int)filePath.length(); i++) {
        if (filePath[i] == '.') {
            lastDot = i;
        }
    }

    int end = (lastDot > start) ? lastDot : (int)filePath.length();
    string baseName = filePath.substr(start, end - start);
    if (baseName.empty()) {
        return "simulation";
    }
    return baseName;
}

// Helper to save logs and statistics into the output/ directory
static void saveOutputs(Logger &logger, Statistics &statistics, const string &baseName) {
    string logFile = "output/" + baseName + "_logs.csv";
    string statsFile = "output/" + baseName + "_statistics.txt";

    cout << "\nSaving simulation outputs..." << endl;

    if (logger.saveToFile(logFile.c_str())) {
        cout << "Simulation log saved to: " << logFile << endl;
    } else {
        cout << "Error: Failed to save simulation log to " << logFile
             << " (make sure the output/ directory exists)." << endl;
    }

    if (statistics.saveStatistics(statsFile.c_str())) {
        cout << "Final statistics saved to: " << statsFile << endl;
    } else {
        cout << "Error: Failed to save final statistics to " << statsFile
             << " (make sure the output/ directory exists)." << endl;
    }
}

int main(int argc, char *argv[]) {
    // Section B - Display the application heading
    cout << "==========================================" << endl;
    cout << "             AI RESCUE ROBOTS             " << endl;
    cout << "   Autonomous Disaster-Response Simulation" << endl;
    cout << "==========================================" << endl;

    // Section C - Obtain the scenario filename
    string filename;
    if (argc > 1) {
        filename = argv[1];
    } else {
        cout << "\nEnter scenario file path (press Enter for data/scenario_provided.txt): ";
        getline(cin, filename);
        if (filename.empty()) {
            filename = "data/scenario_provided.txt";
        }
    }
    cout << "Loading scenario: " << filename << endl;

    // Section D - Create the simulation objects
    Environment environment;
    RobotList robots;
    RequestList requests;
    ScenarioLoader loader;
    EventManager eventManager;
    Logger logger;
    Statistics statistics;
    SimulationManager simulation;

    // Section E - Load and validate the scenario
    if (!loader.loadEnvironment(filename.c_str(), environment, robots, requests)) {
        cout << "\nError: Failed to load scenario file \"" << filename << "\"." << endl;
        cout << "Please verify the file path and scenario format." << endl;
        return 1;
    }
    cout << "\nScenario loaded and validated successfully." << endl;

    // Section F - Display initial scenario information
    const SimulationParameters &params = loader.getParameters();

    cout << "\n========== SCENARIO OVERVIEW ==========" << endl;
    cout << "Scenario Name:          " << loader.getScenarioName() << endl;
    cout << "Maximum Steps:          " << params.maxSteps << endl;
    cout << "Robot Speed:            " << params.speed << endl;
    cout << "Battery Per Cost Unit:  " << params.batteryPerCostUnit << endl;
    cout << "Battery Per Work Step:  " << params.batteryPerWorkStep << endl;
    cout << "Recharge Rate Per Step: " << params.rechargePerStep << endl;
    cout << "---------------------------------------" << endl;
    cout << "Locations:              " << environment.getLocationCount() << endl;
    cout << "Connections:            " << environment.getRoadCount() << endl;
    cout << "Robots:                 " << robots.getCount() << endl;
    cout << "Rescue Requests:        " << requests.getCount() << endl;
    cout << "Scheduled Events:       " << loader.getEventCount() << endl;
    cout << "=======================================" << endl;

    environment.displayEnvironment();
    robots.displayAll();
    requests.displayAll();

    // Section G - Initialize the SimulationManager
    if (!simulation.initialize(environment, robots, requests, loader, eventManager, logger, statistics)) {
        cout << "\nError: Failed to initialize SimulationManager." << endl;
        return 1;
    }

    // Section H - Provide simulation execution options
    string baseName = extractBaseName(filename);
    bool outputsSaved = false;
    bool runningMenu = true;

    while (runningMenu) {
        cout << "\n========== SIMULATION MENU ==========" << endl;
        cout << "1. Run the complete simulation automatically" << endl;
        cout << "2. Advance the simulation by one time step" << endl;
        cout << "3. Display the current simulation status" << endl;
        cout << "4. Display the current environment" << endl;
        cout << "5. Display robot information" << endl;
        cout << "6. Display rescue request information" << endl;
        cout << "7. Exit" << endl;
        cout << "=====================================" << endl;
        cout << "Enter your choice (1-7): ";

        int choice = 0;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "\nInvalid input. Please enter a number between 1 and 7." << endl;
            continue;
        }

        if (choice == 1) {
            if (simulation.isFinished()) {
                cout << "\nThe simulation has already finished." << endl;
            } else {
                simulation.run();
                // Section I - Save the simulation output
                if (simulation.isFinished() && !outputsSaved) {
                    saveOutputs(logger, statistics, baseName);
                    outputsSaved = true;
                }
            }
        } else if (choice == 2) {
            if (simulation.isFinished()) {
                cout << "\nThe simulation has already finished and cannot advance further." << endl;
            } else {
                simulation.step();
                if (simulation.isFinished()) {
                    cout << "\nSimulation reached its termination condition at step "
                         << simulation.getCurrentTime() << "." << endl;
                    statistics.calculate(robots, requests, logger, simulation.getCurrentTime());
                    statistics.displayStatistics();
                    // Section I - Save the simulation output
                    if (!outputsSaved) {
                        saveOutputs(logger, statistics, baseName);
                        outputsSaved = true;
                    }
                }
            }
        } else if (choice == 3) {
            simulation.displaySimulationStatus();
        } else if (choice == 4) {
            environment.displayEnvironment();
        } else if (choice == 5) {
            robots.displayAll();
        } else if (choice == 6) {
            requests.displayAll();
        } else if (choice == 7) {
            runningMenu = false;
        } else {
            cout << "\nInvalid choice. Please enter a number between 1 and 7." << endl;
        }
    }

    // Section J - Clean termination
    cout << "\nExiting AI Rescue Robots Simulation." << endl;
    return 0;
}