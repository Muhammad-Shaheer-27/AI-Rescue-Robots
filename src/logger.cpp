#include "logger.h"
#include <iostream>
#include <fstream>

using namespace std;

const char* getLogEventName(int eventType) {
    switch (eventType) {
        case LOG_ROBOT_ASSIGNED:      return "ROBOT_ASSIGNED";
        case LOG_ROBOT_MOVED:         return "ROBOT_MOVED";
        case LOG_ROBOT_ARRIVED:       return "ROBOT_ARRIVED";
        case LOG_TASK_STARTED:        return "TASK_STARTED";
        case LOG_TASK_COMPLETED:      return "TASK_COMPLETED";
        case LOG_TASK_FAILED:         return "TASK_FAILED";
        case LOG_ROBOT_REASSIGNED:    return "ROBOT_REASSIGNED";
        case LOG_BATTERY_CONSUMED:    return "BATTERY_CONSUMED";
        case LOG_BATTERY_RECHARGED:   return "BATTERY_RECHARGED";
        case LOG_ROAD_BLOCKED:        return "ROAD_BLOCKED";
        case LOG_ROAD_REOPENED:       return "ROAD_REOPENED";
        case LOG_STATION_DOWN:        return "STATION_DOWN";
        case LOG_STATION_UP:          return "STATION_UP";
        case LOG_ROBOT_OFFLINE:       return "ROBOT_OFFLINE";
        case LOG_ROBOT_ONLINE:        return "ROBOT_ONLINE";
        case LOG_URGENCY_CHANGED:     return "URGENCY_CHANGED";
        case LOG_ASSIGNMENT_CONFLICT: return "ASSIGNMENT_CONFLICT";
        default:                      return "UNKNOWN";
    }
}

bool isValidLogEventType(int eventType) {
    return (eventType >= LOG_ROBOT_ASSIGNED && eventType <= LOG_ASSIGNMENT_CONFLICT);
}

LogEntry::LogEntry(int t, int type, int rId, int reqId, int locId, int rdId, int val, LogEntry* n)
    : time(t), eventType(type), robotID(rId), requestID(reqId),
      locationID(locId), roadID(rdId), value(val), next(n) {}

Logger::Logger() : head(nullptr), tail(nullptr), count(0) {}

Logger::~Logger() {
    clear();
}

void Logger::clear() {
    LogEntry* current = head;
    while (current != nullptr) {
        LogEntry* nextNode = current->next;
        delete current;
        current = nextNode;
    }
    head = nullptr;
    tail = nullptr;
    count = 0;
}

bool Logger::addEntry(
    int time,
    int eventType,
    int robotID,
    int requestID,
    int locationID,
    int roadID,
    int value
) {
    if (time < 0 || !isValidLogEventType(eventType)) {
        return false;
    }

    LogEntry* newNode = new LogEntry(time, eventType, robotID, requestID, locationID, roadID, value, nullptr);

    if (head == nullptr) {
        head = newNode;
        tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }

    count++;
    return true;
}

LogEntry* Logger::getHead() const {
    return head;
}

int Logger::getCount() const {
    return count;
}

int Logger::getEventCount(int eventType) const {
    int total = 0;
    LogEntry* current = head;
    while (current != nullptr) {
        if (current->eventType == eventType) {
            total++;
        }
        current = current->next;
    }
    return total;
}

void Logger::displayLogs() const {
    cout << "\n========== SIMULATION LOGS ==========\n";
    cout << "Total Entries: " << count << "\n";

    LogEntry* current = head;
    while (current != nullptr) {
        cout << "[Step " << current->time << "] "
             << getLogEventName(current->eventType)
             << " | Robot: " << current->robotID
             << " | Request: " << current->requestID
             << " | Location: " << current->locationID
             << " | Road: " << current->roadID
             << " | Value: " << current->value << "\n";
        current = current->next;
    }
    cout << "=====================================\n";
}

bool Logger::saveToFile(const char* filename) const {
    if (filename == nullptr) {
        return false;
    }

    ofstream outFile(filename);
    if (!outFile.is_open()) {
        return false;
    }

    outFile << "time,event_type,robot_id,request_id,location_id,road_id,value\n";

    LogEntry* current = head;
    while (current != nullptr) {
        outFile << current->time << ","
                << getLogEventName(current->eventType) << ","
                << current->robotID << ","
                << current->requestID << ","
                << current->locationID << ","
                << current->roadID << ","
                << current->value << "\n";
        current = current->next;
    }

    outFile.close();
    return true;
}