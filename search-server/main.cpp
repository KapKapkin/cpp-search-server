#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>

#include "paginator.h"
#include "document.h"
#include "request_queue.h"
#include "search_server.h"

using namespace std;

template <typename First, typename Second>
std::ostream &operator<<(std::ostream &out, const std::pair<First, Second> &container)
{
    out << "(";
    out << container.first << ", " << container.second;
    out << ")";
    return out;
}

template <typename Container>
void Print(std::ostream &out, const Container &container)
{
    bool flag = true;
    for (const auto &element : container)
    {
        if (!flag)
        {
            out << ", ";
        }

        out << element;
        flag = false;
    }
}

template <typename Term>
std::ostream &operator<<(std::ostream &out, const std::vector<Term> &container)
{
    out << "[";
    Print(out, container);
    out << "]";
    return out;
}

template <typename Term>
std::ostream &operator<<(std::ostream &out, const std::set<Term> &container)
{
    out << "{";
    Print(out, container);
    out << "}";
    return out;
}

template <typename Key, typename Value>
std::ostream &operator<<(std::ostream &out, const std::map<Key, Value> &container)
{
    out << "<";
    Print(out, container);
    out << ">";
    return out;
}

template <typename It>
std::ostream &operator<<(std::ostream &output, const IteratorRange<It> &pages)
{
    for (auto i = pages.begin(); i != pages.end(); i++)
    {
        output << *i;
    }
    return output;
}

int main()
{
    SearchServer search_server("and with"s);

    search_server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, {1, 2, 3});
    search_server.AddDocument(3, "big cat nasty hair"s, DocumentStatus::ACTUAL, {1, 2, 8});
    search_server.AddDocument(4, "big dog cat Vladislav"s, DocumentStatus::ACTUAL, {1, 3, 2});
    search_server.AddDocument(5, "big dog hamster Borya"s, DocumentStatus::ACTUAL, {1, 1, 1});
    const auto search_results = search_server.FindTopDocuments("curly dog"s);
    size_t page_size = 2;

    const auto pages = Paginate(search_results, page_size);
    vector<IteratorRange<vector<Document>::const_iterator>> res(pages.begin(), pages.end());
    for (auto i : res)
    {
        cout << i << endl;
        cout << "Page break"s << endl;
    }
    cout << "ACTUAL by default:"s << endl;
    for (const Document &document : search_server.FindTopDocuments("funny"s))
    {
        cout << document << endl;
    }
    cout << "ACTUAL:"s << endl;
    for (const Document &document : search_server.FindTopDocuments("big dog"s, [](int document_id, DocumentStatus status, int rating)
                                                                   { return status == DocumentStatus::ACTUAL; }))
    {
        cout << document << endl;
    }
    cout << "Even ids:"s << endl;
    for (const Document &document : search_server.FindTopDocuments("big"s, [](int document_id, DocumentStatus status, int rating)
                                                                   { return document_id % 2 == 0; }))
    {
        cout << document << endl;
    }
    return 0;
}