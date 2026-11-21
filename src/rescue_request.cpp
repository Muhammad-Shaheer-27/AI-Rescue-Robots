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

// Check if moving from currentStatus to newStatus follows the request lifecycle
bool isValidStatusTransition(int currentStatus, int newStatus) {
    if (currentStatus == newStatus) {
        return false;
    }

    if (currentStatus == REQUEST_PENDING) {
        return (newStatus == REQUEST_ASSIGNED || newStatus == REQUEST_FAILED);
    }
    if (currentStatus == REQUEST_ASSIGNED) {
        return (newStatus == REQUEST_IN_PROGRESS ||
                newStatus == REQUEST_PENDING ||
                newStatus == REQUEST_FAILED);
    }
    if (currentStatus == REQUEST_IN_PROGRESS) {
        return (newStatus == REQUEST_COMPLETED ||
                newStatus == REQUEST_PENDING ||
                newStatus == REQUEST_FAILED);
    }

    // COMPLETED and FAILED are final states so they dont change
    return false;
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

bool RescueRequest::isReportedAt(int currentTime) const {
    if (currentTime < 0) {
        return false;
    }
    return reportTime <= currentTime;
}

bool RescueRequest::isAvailableAt(int currentTime) const {
    if (!isReportedAt(currentTime)) {
        return false;
    }
    // Only available if it is pending and has no robot assigned
    return (status == REQUEST_PENDING && assignedRobotId == -1);
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

bool RequestList::isRequestAvailable(int id, int currentTime) {
    RescueRequest* req = findRequest(id);
    if (req == nullptr) {
        return false;
    }
    return req->isAvailableAt(currentTime);
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
    // Block invalid lifecycle jumps
    if (!isValidStatusTransition(req->status, newStatus)) {
        return false;
    }

    req->status = newStatus;

    // clear robot ID if the request goes back to pending
    if (newStatus == REQUEST_PENDING) {
        req->assignedRobotId = -1;
    }
    return true;
}

bool RequestList::isRobotAssigned(int robotId) const {
    if (robotId <= 0) {
        return false;
    }
    RequestNode* current = head;
    while (current != nullptr) {
        int st = current->request.status;
        if (current->request.assignedRobotId == robotId &&
            (st == REQUEST_ASSIGNED || st == REQUEST_IN_PROGRESS)) {
            return true;
        }
        current = current->next;
    }
    return false;
}

bool RequestList::assignRobot(int requestId, int robotId, int currentTime) {
    if (robotId <= 0) {
        return false;
    }

    RescueRequest* req = findRequest(requestId);
    if (req == nullptr) {
        return false;
    }

    //req must be reported pending and not already assigned
    if (!req->isAvailableAt(currentTime)) {
        return false;
    }

    //prevent the same robot from taking two active requests at once
    if (isRobotAssigned(robotId)) {
        return false;
    }

    req->assignedRobotId = robotId;
    req->status = REQUEST_ASSIGNED;
    return true;
}

bool RequestList::clearAssignedRobot(int requestId) {
    RescueRequest* req = findRequest(requestId);
    if (req == nullptr) {
        return false;
    }

    //nothing to clear or request is alr finished
    if (req->assignedRobotId == -1 ||
        req->status == REQUEST_COMPLETED ||
        req->status == REQUEST_FAILED) {
        return false;
    }

    req->assignedRobotId = -1;
    req->status = REQUEST_PENDING;
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

    //do not change urgency of reqs that are already finished
    if (req->status == REQUEST_COMPLETED || req->status == REQUEST_FAILED) {
        return false;
    }

    // Nothing to change if urgency is already the same
    if (req->urgency == newUrgency) {
        return true;
    }

    // Copy the request (keep status and assignedRobotId) remove old node and reinsert
    RescueRequest updatedReq = *req;
    updatedReq.urgency = newUrgency;

    removeRequest(id);
    return addRequestPriority(updatedReq);
}

RescueRequest* RequestList::getHighestPriorityPending(int currentTime) {
    RequestNode* current = head;
    while (current != nullptr) {
        if (current->request.isAvailableAt(currentTime)) {
            return &(current->request);
        }
        current = current->next;
    }
    return nullptr;
}
//filters req list and returns only the no of reqs that can be assigned at current moment
int RequestList::getPendingRequests(int currentTime, RescueRequest* results[], int maxResults) {
    if (results == nullptr || maxResults <= 0) {
        return 0;
    }

    int found = 0;
    RequestNode* current = head;
    while (current != nullptr && found < maxResults) {
        if (current->request.isAvailableAt(currentTime)) {
            results[found] = &(current->request);    //changes req in memory
            found++;
        }
        current = current->next;
    }
    return found;
}

RequestNode* RequestList::getHead() const {
    return head;
}

int RequestList::getCount() const {
    return count;
}

int RequestList::getCountByStatus(int status) const {            
    if (status < REQUEST_PENDING || status > REQUEST_FAILED) {
        return 0;
    }

    int total = 0;
    RequestNode* current = head;
    while (current != nullptr) {
        if (current->request.status == status) {
            total++;
        }
        current = current->next;
    }
    return total;
}

int RequestList::getAvailablePendingCount(int currentTime) const {
    int total = 0;
    RequestNode* current = head;
    while (current != nullptr) {
        if (current->request.isAvailableAt(currentTime)) {      
            total++;              //counts how many reqs are ready to be assigned at curr time
        }
        current = current->next;
    }
    return total;
}

double RequestList::getCompletionRate() const {   //in %
    if (count == 0) {
        return 0.0;
    }
    int completed = getCountByStatus(REQUEST_COMPLETED);
    return (completed * 100.0) / count;
}

void RequestList::displayStatistics() const {
    cout << "\n========== REQUEST STATISTICS ==========\n";
    cout << "Total Requests: " << count << endl;
    cout << "Pending:        " << getCountByStatus(REQUEST_PENDING) << endl;
    cout << "Assigned:       " << getCountByStatus(REQUEST_ASSIGNED) << endl;
    cout << "In Progress:    " << getCountByStatus(REQUEST_IN_PROGRESS) << endl;
    cout << "Completed:      " << getCountByStatus(REQUEST_COMPLETED) << endl;
    cout << "Failed:         " << getCountByStatus(REQUEST_FAILED) << endl;
    cout << "Completion Rate: " << getCompletionRate() << "%" << endl;
    cout << "========================================\n";
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