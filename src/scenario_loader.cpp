#include <iostream>
#include <fstream>
#include <cstring>
#include "scenario_loader.h"
using namespace std;

//------------------------------------------------------------
// Helper Function to skip blank lines and comments
//------------------------------------------------------------
bool readNextLine(ifstream &file,char line[],int size){
    while(file.getline(line,size)){
        int i=0;
        //Skip spaces and tabs
        while(line[i]==' ' || line[i]=='\t' || line[i]=='\r'){
            i++;
        }
        //Ignore blank lines and comments
        if(line[i]=='\0' || line[i]=='#'){
            continue;
        }
        return true;
    }
    return false;
}
//------------------------------------------------------------
// Helper Function to split a line into tokens
//------------------------------------------------------------
int splitTokens(char line[],char *tokens[],int maxTokens){
    int count=0;
    int i=0;
    while(line[i]!='\0'){
        //Skip white space
        while(line[i]==' ' || line[i]=='\t' || line[i]=='\r'){
            i++;
        }
        if(line[i]=='\0'){
            break;
        }
        if(count>=maxTokens){
            return -1;
        }
        //Store the first character of the current word into the token by reference
        tokens[count]=&line[i];
        count++;

        while(line[i]!='\0' && line[i]!=' ' && line[i]!='\t' && line[i]!='\r'){
            i++;
        }
        //End the current toekn
        if(line[i]!='\0'){
            line[i]='\0';
            i++;
        }
    }
    return count;
}
//------------------------------------------------------------
// Helper Function to convert characters to integers
//------------------------------------------------------------
bool parseInteger(const char *text,int &value){
    if(text==nullptr || text[0]=='\0'){
        return false;
    }
    int i=0;
    int sign=1;
    int result=0;

    if(text[i]=='-'){
        sign=-1;
        i++;
    }
    else if(text[i]=='+'){
        i++;
    }
    if(text[i]=='\0'){
        return false;
    }
    while(text[i]!='\0'){
        if(text[i]<'0' || text[i]>'9'){
            return false;
        }
        //Convert asciis into ints
        int digit=text[i]-'0';
        result=result*10+digit;
        i++;
    }
    value=result*sign;
    return true;
}
//------------------------------------------------------------
// Helper Function to compare two strings
//------------------------------------------------------------
bool sameText(const char *first,const char *second){
    if(first == nullptr || second == nullptr){
        return false;
    }
    return strcmp(first,second)==0;
}
//------------------------------------------------------------
// Helper Function to convert location type
//------------------------------------------------------------
int getLocationType(const char *type){
    if(sameText(type,"JUNCTION")){
        return LOCATION_JUNCTION;
    }
    if(sameText(type,"BUILDING")){
        return LOCATION_BUILDING;
    }
    if(sameText(type,"HOSPITAL")){
        return LOCATION_HOSPITAL;
    }
    if (sameText(type, "DEPOT")) {
        return LOCATION_DEPOT;
    }
    if (sameText(type, "CHARGING")) {
        return LOCATION_CHARGING;
    }
    if (sameText(type, "COMMAND")) {
        return LOCATION_COMMAND;
    }
    return 0;
}
//------------------------------------------------------------
// Helper Function to convert road status
//------------------------------------------------------------
int getRoadStatus(const char *status){
    if(sameText(status,"OPEN")){
        return ROAD_OPEN;
    }
    if(sameText(status,"BLOCKED")){
        return ROAD_BLOCKED;
    }
    return 0;
}
//------------------------------------------------------------
// Helper Function to read a section hader
//------------------------------------------------------------
bool readSectionHeader(ifstream &file,const char *expectedName,int &count){
    char line[512];
    char *tokens[8];

    if(!readNextLine(file,line,512)){
        return false;
    }
    int tokenCount=splitTokens(line,tokens,8);
    if(tokenCount!=2){
        return false;
    }
    if(!sameText(tokens[0],expectedName)){
        return false;
    }
    if(!parseInteger(tokens[1],count) || count<0){
        return false;
    }
    return true;
}

ScenarioLoader::ScenarioLoader(){
    scenarioName[0]='\0';

    parameters.maxSteps=0;
    parameters.speed=0;
    parameters.batteryPerCostUnit=0;
    parameters.batteryPerWorkStep=0;
    parameters.rechargePerStep=0;
}
bool ScenarioLoader::loadEnvironment(const char *filename,Environment &environment, RobotList &robots, RequestList &requests){
    ifstream file(filename);
    if(!file.is_open()){
        cout<<"Error! Could not open scenario file"<<endl;
        return false;
    }

    char line[512];
    char *tokens[8];
    int tokenCount;
    int value;

    //Read scenario name
    if(!readNextLine(file,line,512)){
        cout<<"Error! Empty scenario file"<<endl;
        file.close();
        return false;
    }
    //Split the scenario name into tokens
    tokenCount=splitTokens(line,tokens,8);
    if(tokenCount!=2 || !sameText(tokens[0],"SCENARIO")){
        cout<<"Error! Invalid scenario declaraion"<<endl;
        file.close();
        return false;
    }
    if(strlen(tokens[1]) >= 50){
        cout<<"Error! Scenario file name is too long"<<endl;
        file.close();
        return false;
    }
    //Save the scenario name
    strcpy(scenarioName,tokens[1]);

    //Read parameters
    bool foundMaxSteps=false;
    bool foundSpeed=false;
    bool foundBatteryCost=false;
    bool foundBatteryWork=false;
    bool foundRecharge=false;

    //Read PARAM records until LOCAITONS appears
    while(true){
        if(!readNextLine(file,line,512)){
            cout<<"Error! Missing LOCATIONS section"<<endl;
            file.close();
            return false;
        }
        tokenCount=splitTokens(line,tokens,8);
        if(tokenCount==2 && sameText(tokens[0],"LOCATIONS")){
            break;
        }
        //Check if the paraemter definition format is invalid
        if(tokenCount!=3 || !sameText(tokens[0],"PARAM")){
            cout<<"Error! Invalid PARAM record or section order"<<endl;
            file.close();
            return false;
        }
        //Check if the values defined in the parameter section are invalid
        if(!parseInteger(tokens[2],value)){
            cout<<"Error! Invalid parameter value"<<endl;
            file.close();
            return false;
        }
        //Set max steps
        if(sameText(tokens[1],"max_steps")){
            if (foundMaxSteps || value <= 0) {
                cout << "Error! Invalid or duplicate max_steps."<<endl;
                file.close();
                return false;
            }
            parameters.maxSteps=value;
            foundMaxSteps=true;
        }
        //Set speed
        else if(sameText(tokens[1],"speed")) {
            if(foundSpeed || value<=0){
                cout<<"Error! Invalid or duplicate speed."<<endl;
                file.close();
                return false;
            }
            parameters.speed=value;
            foundSpeed=true;
        }
        //Set battery cost per unit
        else if(sameText(tokens[1],"battery_per_cost_unit")){
            if(foundBatteryCost || value<=0){
                cout<<"Error! Invalid or duplicate battery_per_cost_unit."<<endl;
                file.close();
                return false;
            }
            parameters.batteryPerCostUnit=value;
            foundBatteryCost=true;
        }
        //Set batter per work step
        else if (sameText(tokens[1],"battery_per_work_step")){
            if(foundBatteryWork || value<=0){
                cout<<"Error! Invalid or duplicate battery_per_work_step."<<endl;
                file.close();
                return false;
            }
            parameters.batteryPerWorkStep=value;
            foundBatteryWork=true;
        }
        //Set recharge per step
        else if (sameText(tokens[1],"recharge_per_step")){
            if(foundRecharge || value <= 0){
                cout<<"Error! Invalid or duplicate recharge_per_step."<<endl;
                file.close();
                return false;
            }
            parameters.rechargePerStep=value;
            foundRecharge=true;
        }
        else {
            cout<<"Error! Unknown parameter: "<<tokens[1]<<endl;
            file.close();
            return false;
        }
    }
    //If parameters not found
    if (!foundMaxSteps || !foundSpeed || !foundBatteryCost || !foundBatteryWork || !foundRecharge){
        cout << "Error: One or more parameters are missing."<<endl;
        file.close();
        return false;
    }
    
    //Read location records temporarily
    //The LOCATIOSN header has already been read
    int locationCount;
    if(tokenCount!=2 || !parseInteger(tokens[1],locationCount) || locationCount<0){
        cout<<"Error! Invalid LOCATIONS count"<<endl;
        file.close();
        return false;
    }
    struct TemporaryLocation{
        int id;
        char name[50];
        int type;
    };
    //Create temporary array to validate location data provided
    TemporaryLocation* tempLocations = nullptr;
    if (locationCount > 0) {
        tempLocations = new TemporaryLocation[locationCount];
    }
    for (int i = 0; i < locationCount; i++) {
        if (!readNextLine(file, line, 512)) {
            cout<<"Error! Missing location records"<<endl;
            delete[] tempLocations;
            file.close();
            return false;
        }
        tokenCount=splitTokens(line,tokens,8);
        if (tokenCount != 3) {
            cout<<"Error! Invalid location record"<<endl;
            delete[] tempLocations;
            file.close();
            return false;
        }
        int id;
        //Reject invalid location ID
        if (!parseInteger(tokens[0], id)) {
            cout<<"Error! Invalid location ID"<<endl;
            delete[] tempLocations;
            file.close();
            return false;
        }
        //Reject lengthy location name
        if (strlen(tokens[1]) >= 50) {
            cout<<"Error! Location name is too long"<<endl;
            delete[] tempLocations;
            file.close();
            return false;
        }
        //Reject invalid location types
        int type = getLocationType(tokens[2]);
        if (type == 0) {
            cout<<"Error! Invalid location type"<<endl;
            delete[] tempLocations;
            file.close();
            return false;
        }

        tempLocations[i].id = id;
        strcpy(tempLocations[i].name, tokens[1]);
        tempLocations[i].type = type;
    }

    //ROAD CONNECTION count
    int roadCount;
    if (!readSectionHeader(file, "CONNECTIONS", roadCount)) {
        cout<<"Error! Invalid CONNECTIONS section"<<endl;
        delete[] tempLocations;
        file.close();
        return false;
    }

    //Initialize Environment
    if (!environment.initialize(locationCount, roadCount)) {
        cout<<"Error! Environment initialization failed."<<endl;
        delete[] tempLocations;
        file.close();
        return false;
    }
    // Insert all locations into the Environment.
    for (int i = 0; i < locationCount; i++) {
        if(!environment.addLocation(tempLocations[i].id,tempLocations[i].name,tempLocations[i].type)){
            cout << "Error! Could not insert location "<< tempLocations[i].id << endl;
            delete[] tempLocations;
            file.close();
            return false;
        }
    }
    delete[] tempLocations;

    //Read ROAD CONNECTION records
    for (int i = 0; i < roadCount; i++) {
        //Reject invalid road inputs
        if (!readNextLine(file, line, 512)) {
            cout<<"Error! Missing connection records"<<endl;
            file.close();
            return false;
        }
        tokenCount = splitTokens(line, tokens, 8);
        if (tokenCount != 5) {
            cout<<"Error! Invalid connection record."<<endl;
            file.close();
            return false;
        }
        //Convert road characters into strings
        int id, fromID, toID, cost;
        if (!parseInteger(tokens[0], id) || !parseInteger(tokens[1], fromID) ||
            !parseInteger(tokens[2], toID) || !parseInteger(tokens[3], cost)) {
            cout<<"Error! Invalid numeric value in connection"<<endl;
            file.close();
            return false;
        }
        int status = getRoadStatus(tokens[4]);
        if (status == 0) {
            cout<<"Error! Invalid road status"<<endl;
            file.close();
            return false;
        }
        //Add roads into the environment
        if (!environment.addRoad(id, fromID, toID, cost, status)) {
            cout<<"Error! Could not insert road "<<id<<endl;
            file.close();
            return false;
        }
    }

    //Read ROBOTS section
    int robotCount;
    if (!readSectionHeader(file, "ROBOTS", robotCount)) {
        cout<<"Error! Invalid ROBOTS section"<<endl;
        file.close();
        return false;
    }
    for (int i = 0; i < robotCount; i++) {
        //Reject invalid robot inputs
        if (!readNextLine(file, line, 512)) {
            cout<<"Error! Missing robot records"<<endl;
            file.close();
            return false;
        }
        tokenCount=splitTokens(line, tokens, 8);
        if (tokenCount != 5) {
            cout<<"Error! Invalid robot record"<<endl;
            file.close();
            return false;
        }

        int id, loc, maxBat, initBat;
        if (!parseInteger(tokens[0], id) || !parseInteger(tokens[1], loc) ||
            !parseInteger(tokens[3], maxBat) || !parseInteger(tokens[4], initBat)) {
            cout << "Error! Invalid numeric value in robot record" << endl;
            file.close();
            return false;
        }
        int cap = parseCapability(tokens[2]); 
        
        if (!robots.addRobot(id, loc, cap, maxBat, initBat)) {
            cout << "Error! Could not add robot " << id << endl;
            file.close();
            return false;
        }
    }

    //Read REQUEST Section
    int requestCount;
    if (!readSectionHeader(file, "REQUESTS", requestCount)) {
        cout<<"Error! Invalid REQUESTS section"<<endl;
        file.close();
        return false;
    }
    for (int i = 0;i<requestCount;i++){
        //Reject invalid requests
        if (!readNextLine(file, line, 512)) {
            cout<<"Error! Missing request records."<<endl;
            file.close();
            return false;
        }
        tokenCount=splitTokens(line,tokens,8);
        if (tokenCount != 6) {
            cout << "Error! Invalid request record"<<endl;
            file.close();
            return false;
        }

        int id, time, loc, urg, dur;
        if (!parseInteger(tokens[0], id) || !parseInteger(tokens[1], time) ||
            !parseInteger(tokens[2], loc) || !parseInteger(tokens[4], urg) ||
            !parseInteger(tokens[5], dur)) {
            cout << "Error! Invalid numeric value in request record" << endl;
            file.close();
            return false;
        }
        int cat = parseRequestCategory(tokens[3]);

        if (!requests.addRequestPriority(id, time, loc, cat, urg, dur)) {
            cout << "Error! Could not add request " << id << endl;
            file.close();
            return false;
        }
    }

    //Read EVENTS section
    int eventCount;
    if (!readSectionHeader(file, "EVENTS", eventCount)) {
        cout << "Error! Invalid EVENTS section."<<endl;
        file.close();
        return false;
    }
    for (int i = 0; i < eventCount; i++){
        if (!readNextLine(file, line, 512)) {
            cout << "Error! Missing event records"<<endl;
            file.close();
            return false;
        }
        tokenCount = splitTokens(line, tokens, 8);
        if (tokenCount < 3 || tokenCount > 4) {
            cout << "Error! Invalid event record"<<endl;
            file.close();
            return false;
        }
    }

    //Check END
    if (!readNextLine(file, line, 512)) {
        cout<<"Error! Missing END marker"<<endl;
        file.close();
        return false;
    }
    tokenCount= splitTokens(line,tokens,8);
    if (tokenCount!=1 || !sameText(tokens[0],"END")){
        cout<<"Error! Invalid END marker"<<endl;
        file.close();
        return false;
    }
    file.close();

    cout<<"\nScenario loaded successfully!"<<endl;
    cout<<"Scenario: "<<scenarioName<<endl;
    cout<<"Locations: "<<environment.getLocationCount()<<endl;
    cout<<"Roads: "<<environment.getRoadCount()<<endl;
    return true;
}
const char* ScenarioLoader::getScenarioName() const {
    return scenarioName;
}

const SimulationParameters& ScenarioLoader::getParameters() const {
    return parameters;
}