#ifndef RESCUE_REQUEST_H
#define RESCUE_REQUEST_H
#include "constants.h"

// Helper functions for category and status names
const char* getCategoryName(int category);
const char* getRequestStatusName(int status);
int parseRequestCategory(const char* text);

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
    
    // Getters for the Simulation Manager to loop through pending requests
    RequestNode* getHead() const;
    int getCount() const;

    void displayAll() const;
};

#endif