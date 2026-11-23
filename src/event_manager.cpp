#include <iostream>
#include "event_manager.h"

using namespace std;

// Helper to return string representation of event type
const char* getEventTypeName(int eventType) {
    if (eventType == EVENT_BLOCK)         return "BLOCK";
    if (eventType == EVENT_UNBLOCK)       return "UNBLOCK";
    if (eventType == EVENT_STATION_DOWN)  return "STATION_DOWN";
    if (eventType == EVENT_STATION_UP)    return "STATION_UP";
    if (eventType == EVENT_SET_URGENCY)   return "SET_URGENCY";
    if (eventType == EVENT_ROBOT_OFFLINE) return "ROBOT_OFFLINE";
    if (eventType == EVENT_ROBOT_ONLINE)  return "ROBOT_ONLINE";
    return "UNKNOWN";
}

EventManager::EventManager() {
    events = nullptr;
    eventCount = 0;
    nextEventIndex = 0;

    totalEventsProcessed = 0;
    blockEventsProcessed = 0;
    unblockEventsProcessed = 0;
    stationDownEventsProcessed = 0;
    stationUpEventsProcessed = 0;
    urgencyEventsProcessed = 0;
    robotOfflineEventsProcessed = 0;
    robotOnlineEventsProcessed = 0;
}

void EventManager::initialize(const ScheduledEvent* eventArray, int totalEvents) {
    events = eventArray;
    eventCount = (totalEvents > 0) ? totalEvents : 0;
    nextEventIndex = 0;

    totalEventsProcessed = 0;
    blockEventsProcessed = 0;
    unblockEventsProcessed = 0;
    stationDownEventsProcessed = 0;
    stationUpEventsProcessed = 0;
    urgencyEventsProcessed = 0;
    robotOfflineEventsProcessed = 0;
    robotOnlineEventsProcessed = 0;
}

bool EventManager::applySingleEvent(
    const ScheduledEvent& ev,
    Environment& environment,
    RobotList& robots,
    RequestList& requests,
    Logger& logger
) {
    bool success = false;

    switch (ev.type) {
        case EVENT_BLOCK: {
            success = environment.blockRoad(ev.targetID);
            if (success) {
                blockEventsProcessed++;
                totalEventsProcessed++;
                logger.addEntry(ev.time, LOG_ROAD_BLOCKED, -1, -1, -1, ev.targetID, -1);
            }
            cout << "[Time " << ev.time << "] EVENT BLOCK: Road " << ev.targetID
                 << (success ? " is now BLOCKED." : " FAILED (Road not found).") << endl;
            break;
        }

        case EVENT_UNBLOCK: {
            success = environment.unblockRoad(ev.targetID);
            if (success) {
                unblockEventsProcessed++;
                totalEventsProcessed++;
                logger.addEntry(ev.time, LOG_ROAD_REOPENED, -1, -1, -1, ev.targetID, -1);
            }
            cout << "[Time " << ev.time << "] EVENT UNBLOCK: Road " << ev.targetID
                 << (success ? " is now OPEN." : " FAILED (Road not found).") << endl;
            break;
        }

        case EVENT_STATION_DOWN: {
            success = environment.setStationAvailability(ev.targetID, false);
            if (success) {
                stationDownEventsProcessed++;
                totalEventsProcessed++;
                logger.addEntry(ev.time, LOG_STATION_DOWN, -1, -1, ev.targetID, -1, -1);
            }
            cout << "[Time " << ev.time << "] EVENT STATION_DOWN: Charging Station at Location " << ev.targetID
                 << (success ? " went OFFLINE." : " FAILED (Invalid station location).") << endl;
            break;
        }

        case EVENT_STATION_UP: {
            success = environment.setStationAvailability(ev.targetID, true);
            if (success) {
                stationUpEventsProcessed++;
                totalEventsProcessed++;
                logger.addEntry(ev.time, LOG_STATION_UP, -1, -1, ev.targetID, -1, -1);
            }
            cout << "[Time " << ev.time << "] EVENT STATION_UP: Charging Station at Location " << ev.targetID
                 << (success ? " is now AVAILABLE." : " FAILED (Invalid station location).") << endl;
            break;
        }

        case EVENT_SET_URGENCY: {
            success = requests.updateUrgency(ev.targetID, ev.value);
            if (success) {
                urgencyEventsProcessed++;
                totalEventsProcessed++;
                logger.addEntry(ev.time, LOG_URGENCY_CHANGED, -1, ev.targetID, -1, -1, ev.value);
            }
            cout << "[Time " << ev.time << "] EVENT SET_URGENCY: Request " << ev.targetID
                 << " urgency updated to " << ev.value
                 << (success ? " (Priority order updated)." : " FAILED (Request not found/already completed).") << endl;
            break;
        }

        case EVENT_ROBOT_OFFLINE: {
            success = robots.setRobotOffline(ev.targetID);
            if (success) {
                robotOfflineEventsProcessed++;
                totalEventsProcessed++;
                logger.addEntry(ev.time, LOG_ROBOT_OFFLINE, ev.targetID, -1, -1, -1, -1);
            }
            cout << "[Time " << ev.time << "] EVENT ROBOT_OFFLINE: Robot " << ev.targetID
                 << (success ? " is now OFFLINE (State saved)." : " (Already offline/not found).") << endl;
            break;
        }

        case EVENT_ROBOT_ONLINE: {
            success = robots.setRobotOnline(ev.targetID);
            if (success) {
                robotOnlineEventsProcessed++;
                totalEventsProcessed++;
                logger.addEntry(ev.time, LOG_ROBOT_ONLINE, ev.targetID, -1, -1, -1, -1);
            }
            cout << "[Time " << ev.time << "] EVENT ROBOT_ONLINE: Robot " << ev.targetID
                 << (success ? " is now ONLINE (State restored)." : " (Already online/not found).") << endl;
            break;
        }

        default: {
            cout << "[Time " << ev.time << "] EVENT UNKNOWN: type " << ev.type << endl;
            break;
        }
    }

    return success;
}

int EventManager::processEvents(
    int currentTime,
    Environment& environment,
    RobotList& robots,
    RequestList& requests,
    Logger& logger
) {
    if (events == nullptr || nextEventIndex >= eventCount) {
        return 0;
    }

    int countThisStep = 0;

    // Process all events scheduled at or before currentTime in their exact input file order
    while (nextEventIndex < eventCount && events[nextEventIndex].time <= currentTime) {
        const ScheduledEvent& ev = events[nextEventIndex];
        applySingleEvent(ev, environment, robots, requests, logger);
        nextEventIndex++; // Strictly advances: event cannot be processed more than once
        countThisStep++;
    }

    return countThisStep;
}

int EventManager::processEvents(
    int currentTime,
    Environment& environment,
    RobotList& robots,
    RequestList& requests
) {
    Logger dummyLogger;
    return processEvents(currentTime, environment, robots, requests, dummyLogger);
}

int EventManager::getTotalEventCount() const {
    return eventCount;
}

int EventManager::getAttemptedEventCount() const {
    return nextEventIndex;
}

int EventManager::getSuccessfulEventCount() const {
    return totalEventsProcessed;
}

int EventManager::getFailedEventCount() const {
    return (nextEventIndex - totalEventsProcessed);
}

int EventManager::getProcessedEventCount() const {
    return totalEventsProcessed;
}

int EventManager::getRemainingEventCount() const {
    return (eventCount - nextEventIndex);
}

bool EventManager::hasPendingEvents() const {
    return (nextEventIndex < eventCount);
}

int EventManager::peekNextEventTime() const {
    if (nextEventIndex < eventCount && events != nullptr) {
        return events[nextEventIndex].time;
    }
    return -1; // No more events
}

const ScheduledEvent* EventManager::peekNextEvent() const {
    if (nextEventIndex < eventCount && events != nullptr) {
        return &events[nextEventIndex];
    }
    return nullptr;
}

void EventManager::reset() {
    nextEventIndex = 0;
    totalEventsProcessed = 0;
    blockEventsProcessed = 0;
    unblockEventsProcessed = 0;
    stationDownEventsProcessed = 0;
    stationUpEventsProcessed = 0;
    urgencyEventsProcessed = 0;
    robotOfflineEventsProcessed = 0;
    robotOnlineEventsProcessed = 0;
}

void EventManager::displayStatistics() const {
    cout << "\n========== EVENT MANAGER STATISTICS ==========\n";
    cout << "Total Scheduled Events:       " << eventCount << endl;
    cout << "Total Events Attempted:       " << nextEventIndex << endl;
    cout << "Total Events Succeeded:       " << totalEventsProcessed << endl;
    cout << "Total Events Failed:          " << (nextEventIndex - totalEventsProcessed) << endl;
    cout << "Remaining Pending Events:     " << getRemainingEventCount() << endl;
    cout << "----------------------------------------------\n";
    cout << "  Road Block Events:          " << blockEventsProcessed << endl;
    cout << "  Road Unblock Events:        " << unblockEventsProcessed << endl;
    cout << "  Station Down Events:        " << stationDownEventsProcessed << endl;
    cout << "  Station Up Events:          " << stationUpEventsProcessed << endl;
    cout << "  Urgency Update Events:      " << urgencyEventsProcessed << endl;
    cout << "  Robot Offline Events:       " << robotOfflineEventsProcessed << endl;
    cout << "  Robot Online Events:        " << robotOnlineEventsProcessed << endl;
    cout << "==============================================\n";
}
