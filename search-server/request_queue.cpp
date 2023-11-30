#include "request_queue.h"


RequestQueue::RequestQueue(SearchServer& search_server)
    :server_(search_server)
{}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    return AddFindRequest(raw_query, [status](int document_id, DocumentStatus document_status, int document_rating) {return document_status == status; });
}

int RequestQueue::GetNoResultRequests() const {
    return requests_.front().no_results;
}

void RequestQueue::DeleteLastInQuery() {
    if (requests_.size() >= min_in_day_) {
        if (!requests_.back().status) {
            for (auto i = requests_.begin(); i < requests_.end(); advance(i, 1)) {
                i->no_results--;
            }
        }
        requests_.pop_back();
    }
}