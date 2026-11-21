#ifndef RESCUE_REQUEST_H
#define RESCUE_REQUEST_H
#include "constants.h"

// Helper functions for category and status names
const char* getCategoryName(int category);
const char* getRequestStatusName(int status);
int parseRequestCategory(const char* text);
bool isValidStatusTransition(int currentStatus, int newStatus);

// Structure to hold individual rescue request data
struct RescueRequest {
    int id;
    int reportTime;
    int locationId;
    int category;      // Uses CATEGORY_* from constants.h
    int urgency;       // 1 to 5
    int durationSteps; 
    int status;        // Uses REQUEST_* from constants.h
    int assignedRobotId;

    // Constructors
    RescueRequest();
    RescueRequest(int r_id, int time, int loc, int cat, int urg, int dur);

    // Check if request has arrived and is ready for assignment
    bool isReportedAt(int currentTime) const;
    bool isAvailableAt(int currentTime) const;
    
    // Helper to print request details easily for testing
    void display() const;
};

// Node for our custom linked list (No STL)
struct RequestNode {
    RescueRequest request;
    RequestNode* next;

    RequestNode(const RescueRequest& req, RequestNode* n = nullptr);
};

// Priority ordered Linked List to manage all requests
class RequestList {
private:
    RequestNode* head;
    int count;

    // Prevent shallow copying
    RequestList(const RequestList&);
    RequestList& operator=(const RequestList&);

public:
    RequestList();
    ~RequestList();

    // Memory management
    void clear();

    // Core functionality
    bool addRequestPriority(const RescueRequest& req);     //func overloading
    bool addRequestPriority(int id, int time, int loc, int cat, int urg, int dur);
    
    RescueRequest* findRequest(int id);
    bool removeRequest(int id);
    bool updateStatus(int id, int newStatus);
    bool updateUrgency(int id, int newUrgency);

    bool isRequestAvailable(int id, int currentTime);

    // Robot assignment management
    bool isRobotAssigned(int robotId) const;
    bool assignRobot(int requestId, int robotId, int currentTime);
    bool clearAssignedRobot(int requestId);

    //retrieve available pending requests in priority order
    RescueRequest* getHighestPriorityPending(int currentTime);
    int getPendingRequests(int currentTime, RescueRequest* results[], int maxResults);
    
    //getters for the Simulation Manager to loop through pending requests
    RequestNode* getHead() const;
    int getCount() const;

    // Statistics
    int getCountByStatus(int status) const;
    int getAvailablePendingCount(int currentTime) const;
    double getCompletionRate() const;
    void displayStatistics() const;

    void displayAll() const;
};

#endif