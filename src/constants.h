#ifndef CONSTANTS_H
#define CONSTANTS_H

//----------LOCATION TYPES----------
const int LOCATION_JUNCTION=1;
const int LOCATION_BUILDING=2;
const int LOCATION_HOSPITAL=3;
const int LOCATION_DEPOT=4;
const int LOCATION_CHARGING=5;
const int LOCATION_COMMAND=6;

//----------ROBOT CAPABILITIES----------
const int CAPABILITY_MEDICAL=1;
const int CAPABILITY_FIRE=2;
const int CAPABILITY_SUPPLY=3;
const int CAPABILITY_SEARCH=4;
const int CAPABILITY_GENERAL=5;

//----------REQUEST CATEGORIES----------
const int CATEGORY_MEDICAL=1;
const int CATEGORY_FIRE=2;
const int CATEGORY_SUPPLY=3;
const int CATEGORY_SEARCH=4;

//----------ROAD STATUS----------
const int ROAD_OPEN=1;
const int ROAD_BLOCKED=2;

//----------ROBOT STATES----------
const int ROBOT_AVAILABLE=1;
const int ROBOT_TRAVELLING=2;
const int ROBOT_PERFORMING=3;
const int ROBOT_RETURNING=4;
const int ROBOT_RECHARGING=5;
const int ROBOT_UNAVAILABLE=6;

//----------REQUEST STATES----------
const int REQUEST_PENDING=1;
const int REQUEST_ASSIGNED=2;
const int REQUEST_IN_PROGRESS=3;
const int REQUEST_COMPLETED=4;
const int REQUEST_FAILED=5;

//----------EVENT TYPES----------
const int EVENT_BLOCK=1;
const int EVENT_UNBLOCK=2;
const int EVENT_STATION_DOWN=3;
const int EVENT_STATION_UP=4;
const int EVENT_SET_URGENCY=5;
const int EVENT_ROBOT_OFFLINE=6;
const int EVENT_ROBOT_ONLINE=7;

#endif