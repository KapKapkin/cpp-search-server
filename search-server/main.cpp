// Решите загадку: Сколько чисел от 1 до 1000 содержат как минимум одну цифру 3?
// Напишите ответ здесь: 181
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <map>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

struct Document {
    int id;
    double relevance;
};

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}


class SearchServer {
public:
    vector<Document> FindTopDocuments(const string& raw_query) const {
        Query query = ParseQuery(raw_query);
        vector<Document> all_documents = FindAllDocuments(query);

        sort(all_documents.begin(), all_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                return lhs.relevance > rhs.relevance;
            }
        );
        int end = MAX_RESULT_DOCUMENT_COUNT;
        if (all_documents.size() < 5) {
            end = all_documents.size();
        }
        vector<Document> res(all_documents.begin(), all_documents.begin() + end);
        return res;

    }

    void AddDocument(const int& document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        double tf;
        for (string word : words) {
            tf = static_cast<double>(count(words.begin(), words.end(), word)) / words.size();
            documents_[word].insert({ document_id, tf });

        }
        documents_count_++;

    }

    void SetStopWords(const string& text) {
        for (const string& word : ParseStopWords(text)) {
            stop_words_.insert(word);
        }
    }


private:

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    map<string, map<int, double>> documents_;
    set<string> stop_words_;
    int documents_count_ = 0;

    vector<Document> FindAllDocuments(const Query& query) const {
        vector<Document> res;
        map<int, double> map_id_rel;
        double idf;

        for (const string& word : query.plus_words) {
            if (documents_.count(word)) {
                idf = log(static_cast<double>(documents_count_) / documents_.at(word).size());
                for (const auto& pair_id_tf : documents_.at(word)) {
                    map_id_rel[pair_id_tf.first] += idf * pair_id_tf.second;


                }

            }
        }


        for (const string& word : query.minus_words) {
            if (documents_.count(word))
            {
                for (const auto& id_idf : documents_.at(word)) {
                    map_id_rel[id_idf.first] = -1;
                }
            }
        }

        for (const auto& doc : map_id_rel) {
            if (doc.second != -1) {
                res.push_back({ doc.first, static_cast<double>(doc.second) });
            }

        }


        return res;
    }

    set<string> ParseStopWords(const string& text) {
        set<string> stop_words;
        for (const string& word : SplitIntoWords(text)) {
            stop_words.insert(word);
        }
        return stop_words;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (stop_words_.count(word) == 0) {
                words.push_back(word);
            }
        }
        return words;
    }


    Query ParseQuery(const string& text) const {
        Query res;

        for (const string& word : SplitIntoWordsNoStop(text)) {
            if (word[0] == '-') {
                res.minus_words.insert(word.substr(1));
            }
            else {
                res.plus_words.insert(word);
            }

        }

        return res;
    }
};

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

SearchServer CreateSearchServer() {
    SearchServer server;

    const string stop_words_joined = ReadLine();
    server.SetStopWords(stop_words_joined);
    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; document_id++) {
        server.AddDocument(document_id, ReadLine());
    }

    return server;

}

int main() {
    SearchServer server = CreateSearchServer();

    const string query = ReadLine();
    for (auto document : server.FindTopDocuments(query)) {
        int document_id = document.id;
        double relevance = document.relevance;
        cout << "{ document_id = "s << document_id << ", relevance = "s << relevance << " }"s
            << endl;

    }
}
// Закомитьте изменения и отправьте их в свой репозиторий.

