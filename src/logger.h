#ifndef LOGGER_H
#define LOGGER_H

// Log event type constants
const int LOG_ROBOT_ASSIGNED      = 1;
const int LOG_ROBOT_MOVED         = 2;
const int LOG_ROBOT_ARRIVED       = 3;
const int LOG_TASK_STARTED        = 4;
const int LOG_TASK_COMPLETED      = 5;
const int LOG_TASK_FAILED         = 6;
const int LOG_ROBOT_REASSIGNED    = 7;
const int LOG_BATTERY_CONSUMED    = 8;
const int LOG_BATTERY_RECHARGED   = 9;
const int LOG_ROAD_BLOCKED        = 10;
const int LOG_ROAD_REOPENED       = 11;
const int LOG_STATION_DOWN        = 12;
const int LOG_STATION_UP          = 13;
const int LOG_ROBOT_OFFLINE       = 14;
const int LOG_ROBOT_ONLINE        = 15;
const int LOG_URGENCY_CHANGED     = 16;
const int LOG_ASSIGNMENT_CONFLICT = 17;

// Helper functions
const char* getLogEventName(int eventType);
bool isValidLogEventType(int eventType);

// Custom linked list node for a single simulation event
struct LogEntry {
    int time;
    int eventType;
    int robotID;
    int requestID;
    int locationID;
    int roadID;
    int value;

    LogEntry* next;

    LogEntry(int t, int type, int rId, int reqId, int locId, int rdId, int val = 0, LogEntry* n = nullptr);
};

// Custom singly linked list (O(1) append via tail pointer)
class Logger {
private:
    LogEntry* head;
    LogEntry* tail;
    int count;

    // Prevent shallow copy
    Logger(const Logger&);
    Logger& operator=(const Logger&);

public:
    Logger();
    ~Logger();

    bool addEntry(
        int time,
        int eventType,
        int robotID = -1,
        int requestID = -1,
        int locationID = -1,
        int roadID = -1,
        int value = 0
    );

    LogEntry* getHead() const;
    int getCount() const;
    int getEventCount(int eventType) const;

    void displayLogs() const;
    bool saveToFile(const char* filename) const;
    void clear();
};

#endif