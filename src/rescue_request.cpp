#include <iostream>
#include "rescue_request.h"

using namespace std;

// Helper function to compare strings
bool sameString(const char* s1, const char* s2) {
    if (s1 == nullptr || s2 == nullptr) {
        return false;
    }
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] != s2[i]) {
            return false;
        }
        i++;
    }
    return (s1[i] == '\0' && s2[i] == '\0');
}

// Helper Functions
const char* getCategoryName(int category) {
    if (category == CATEGORY_MEDICAL) return "MEDICAL";
    if (category == CATEGORY_FIRE) return "FIRE";
    if (category == CATEGORY_SUPPLY) return "SUPPLY";
    if (category == CATEGORY_SEARCH) return "SEARCH";
    return "UNKNOWN";
}

const char* getRequestStatusName(int status) {
    if (status == REQUEST_PENDING) return "PENDING";
    if (status == REQUEST_ASSIGNED) return "ASSIGNED";
    if (status == REQUEST_IN_PROGRESS) return "IN_PROGRESS";
    if (status == REQUEST_COMPLETED) return "COMPLETED";
    if (status == REQUEST_FAILED) return "FAILED";
    return "UNKNOWN";
}

int parseRequestCategory(const char* text) {
    if (text == nullptr) return 0;
    if (sameString(text, "MEDICAL")) return CATEGORY_MEDICAL;
    if (sameString(text, "FIRE")) return CATEGORY_FIRE;
    if (sameString(text, "SUPPLY")) return CATEGORY_SUPPLY;
    if (sameString(text, "SEARCH")) return CATEGORY_SEARCH;
    return 0;
}

// Helper function to check if Request A has higher priority than Request B
// Rules: 1. Higher urgency, 2. Earlier report time, 3. Lower request ID
bool hasHigherPriority(const RescueRequest& a, const RescueRequest& b) {
    if (a.urgency != b.urgency) {
        return a.urgency > b.urgency;
    }
    if (a.reportTime != b.reportTime) {
        return a.reportTime < b.reportTime;
    }
    return a.id < b.id;
}

// RescueRequest Implementation
RescueRequest::RescueRequest() {   //default constructor
    id = 0;
    reportTime = 0;
    locationId = 0;
    category = 0;
    urgency = 0;
    durationSteps = 0;
    status = REQUEST_PENDING;
    assignedRobotId = -1;
}

RescueRequest::RescueRequest(int r_id, int time, int loc, int cat, int urg, int dur) {
    id = r_id;
    reportTime = time;
    locationId = loc;
    category = cat;
    urgency = urg;
    durationSteps = dur;
    status = REQUEST_PENDING;
    assignedRobotId = -1;
}

void RescueRequest::display() const {
    cout << "Request ID: " << id
         << " | Time: " << reportTime
         << " | Location: " << locationId
         << " | Category: " << getCategoryName(category)
         << " | Urgency: " << urgency
         << " | Duration: " << durationSteps
         << " | Status: " << getRequestStatusName(status);
    if (assignedRobotId != -1) {
        cout << " | Assigned Robot: " << assignedRobotId;
    }
    cout << endl;
}

// RequestNode
RequestNode::RequestNode(const RescueRequest& req, RequestNode* n) {
    request = req;
    next = n;
}

// RequestList
RequestList::RequestList() {  //default constructor
    head = nullptr;
    count = 0;
}

RequestList::~RequestList() {
    clear();
}

void RequestList::clear() {
    RequestNode* current = head;
    while (current != nullptr) {
        RequestNode* temp = current;
        current = current->next;
        delete temp;
    }
    head = nullptr;
    count = 0;
}

bool RequestList::addRequestPriority(const RescueRequest& req) {
    // Reject invalid IDs or duplicate IDs
    if (req.id <= 0 || findRequest(req.id) != nullptr) {
        return false;
    }
    // Reject invalid report time or location
    if (req.reportTime < 0 || req.locationId <= 0) {
        return false;
    }
    // Reject invalid category
    if (req.category < CATEGORY_MEDICAL || req.category > CATEGORY_SEARCH) {
        return false;
    }
    // Reject invalid urgency or duration
    if (req.urgency < 1 || req.urgency > 5 || req.durationSteps <= 0) {
        return false;
    }

    RequestNode* newNode = new RequestNode(req);
    if (newNode == nullptr) {
        return false;    //if the insertion fails due to full memory
    }

    // Insert at head if list is empty or new request has higher priority than head
    if (head == nullptr || hasHigherPriority(req, head->request)) {
        newNode->next = head;
        head = newNode;
        count++;
        return true;
    }

    // Traverse list to find the right priority spot
    RequestNode* current = head;
    while (current->next != nullptr && !hasHigherPriority(req, current->next->request)) {
        current = current->next;
    }

    newNode->next = current->next;
    current->next = newNode;
    count++;
    return true;
}

bool RequestList::addRequestPriority(int id, int time, int loc, int cat, int urg, int dur) {
    RescueRequest req(id, time, loc, cat, urg, dur);
    return addRequestPriority(req);
}

RescueRequest* RequestList::findRequest(int id) {
    RequestNode* current = head;
    while (current != nullptr) {
        if (current->request.id == id) {
            return &(current->request);
        }
        current = current->next;
    }
    return nullptr;
}

bool RequestList::removeRequest(int id) {
    RequestNode* current = head;
    RequestNode* prev = nullptr;

    while (current != nullptr) {
        if (current->request.id == id) {
            if (prev == nullptr) {
                head = current->next;
            } else {
                prev->next = current->next;
            }
            delete current;
            count--;
            return true;
        }
        prev = current;
        current = current->next;
    }
    return false;
}

bool RequestList::updateStatus(int id, int newStatus) {
    if (newStatus < REQUEST_PENDING || newStatus > REQUEST_FAILED) {
        return false;
    }
    RescueRequest* req = findRequest(id);
    if (req == nullptr) {
        return false;
    }
    req->status = newStatus;
    return true;
}

bool RequestList::updateUrgency(int id, int newUrgency) {
    if (newUrgency < 1 || newUrgency > 5) {
        return false;
    }
    RescueRequest* req = findRequest(id);
    if (req == nullptr) {
        return false;
    }

    // Copy the request, update urgency, remove old node, and re-insert to keep sorted order
    RescueRequest updatedReq = *req;
    updatedReq.urgency = newUrgency;

    removeRequest(id);
    return addRequestPriority(updatedReq);
}

RequestNode* RequestList::getHead() const {
    return head;
}

int RequestList::getCount() const {
    return count;
}

void RequestList::displayAll() const {
    cout << "\n========== REQUESTS LIST ==========\n";
    cout << "Total Requests: " << count << endl;
    RequestNode* current = head;
    while (current != nullptr) {
        current->request.display();
        current = current->next;
    }
    cout << "===================================\n";
}