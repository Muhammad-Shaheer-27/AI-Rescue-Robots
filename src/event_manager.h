#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "constants.h"
#include "environment.h"
#include "robot.h"
#include "rescue_request.h"
#include "scenario_loader.h"

const char* getEventTypeName(int eventType);

class EventManager {
private:
    const ScheduledEvent* events;
    int eventCount;
    int nextEventIndex;

    // Statistics tracking
    int totalEventsProcessed;
    int blockEventsProcessed;
    int unblockEventsProcessed;
    int stationDownEventsProcessed;
    int stationUpEventsProcessed;
    int urgencyEventsProcessed;
    int robotOfflineEventsProcessed;
    int robotOnlineEventsProcessed;

    // Helper to process a single event record
    bool applySingleEvent(const ScheduledEvent& ev, Environment& environment, RobotList& robots, RequestList& requests);

public:
    EventManager();

    // Initialize EventManager with the scenario loader's event array
    void initialize(const ScheduledEvent* eventArray, int totalEvents);

    // Process all events scheduled at or up to currentTime in their original file order
    int processEvents(int currentTime, Environment& environment, RobotList& robots, RequestList& requests);

    // Status and query helpers for the Simulation Manager
    int getTotalEventCount() const;
    int getProcessedEventCount() const;
    int getRemainingEventCount() const;
    bool hasPendingEvents() const;
    int peekNextEventTime() const;
    const ScheduledEvent* peekNextEvent() const;

    // Reset processing state
    void reset();

    // Summary display
    void displayStatistics() const;
};

#endif