
#include <iostream>
#include "environment.h"

using namespace std;

int main() {
    Environment environment;

    // Initialize space for 4 locations and 4 roads.
    if (!environment.initialize(4, 4)) {
        cout << "Environment initialization failed!\n";
        return 1;
    }

    // Add locations.
    environment.addLocation(1, "CommandCenter", LOCATION_COMMAND);
    environment.addLocation(2, "Hospital", LOCATION_HOSPITAL);
    environment.addLocation(3, "Residential", LOCATION_BUILDING);
    environment.addLocation(4, "ChargingStation", LOCATION_CHARGING);

    // Add roads.
    environment.addRoad(1, 1, 2, 10, ROAD_OPEN);
    environment.addRoad(2, 2, 3, 15, ROAD_OPEN);
    environment.addRoad(3, 1, 4, 8, ROAD_BLOCKED);
    environment.addRoad(4, 4, 3, 12, ROAD_OPEN);

    // Display environment.
    environment.displayEnvironment();

    return 0;
}
