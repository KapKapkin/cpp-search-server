#pragma once

#include <vector>
#include <string>
#include <deque>

#include "document.h"
#include "search_server.h"


class RequestQueue {
public:
    explicit RequestQueue(SearchServer& search_server);

    // сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status = DocumentStatus::ACTUAL);

    int GetNoResultRequests() const;

private:
    struct QueryResult {
        bool status;
        int no_results;
    };
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    SearchServer& server_;

    void DeleteLastInQuery();
};


template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
    DeleteLastInQuery();
    std::vector<Document> result = server_.FindTopDocuments(raw_query, document_predicate);
    bool status = requests_.empty();

    if (requests_.empty()) {

        requests_.push_front({ !status, (int)status });
    }
    else {
        int cur_no_results = requests_.front().no_results;
        requests_.push_front({ status, !result.empty() ? cur_no_results : cur_no_results + 1 });
    }
    return result;
}
