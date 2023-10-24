#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double ALLOWED_ERROR = 1e-6;

template <typename First, typename Second>
ostream& operator << (ostream& out, const pair<First, Second>& container) {
    out << "(";
    out << container.first << ", " << container.second;
    out << ")";
    return out;
}

template <typename Container>
void Print(ostream& out, const Container& container) {
    bool flag = true;
    for (const auto& element : container) {
        if (!flag) {
            out << ", ";
        }
        out << element;
        flag = false;
    }
}

template <typename Term>
ostream& operator << (ostream& out, const vector<Term>& container) {
    out << "[";
    Print(out, container);
    out << "]";
    return out;
}

template <typename Term>
ostream& operator << (ostream& out, const set<Term>& container) {
    out << "{";
    Print(out, container);
    out << "}";
    return out;
}

template <typename Key, typename Value>
ostream& operator << (ostream& out, const map<Key, Value>& container) {
    out << "<";
    Print(out, container);
    out << ">";
    return out;
}

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

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

struct Document {

    Document(int doc_id = 0, double doc_relevance = 0, int doc_rating = 0)
        :id(doc_id), relevance(doc_relevance), rating(doc_rating)
    {
    }

    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:
    inline static constexpr int INVALID_DOCUMENT_ID = -1;


    template <typename Container>
    explicit SearchServer(const Container& stop_words) {
        for (const auto& word : stop_words) {
            if (!CheckTextValid(word)) throw invalid_argument("invalid text"s);
        }
        set<string> res(stop_words.begin(), stop_words.end());
        stop_words_ = res;
    }

    explicit SearchServer(const string& stop_words) {
        SearchServer(SplitIntoWords(stop_words));
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
        if (!CheckIDValid(document_id)) throw invalid_argument("invalid ID"s);
        if (!CheckTextValid(document)) throw invalid_argument("invalid text"s);


        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
        documents_order_.push_back(document_id);

    }


    template <typename Predicate>
    vector<Document> FindTopDocuments(const string& raw_query, Predicate func) const {
        if (!CheckTextValid(raw_query)) throw invalid_argument("invalid text");
        const Query query = ParseQuery(raw_query);

        vector<Document> matched_documents = FindAllDocuments(query, func);
        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                if (abs(lhs.relevance - rhs.relevance) < ALLOWED_ERROR) {
                    return lhs.rating > rhs.rating;
                }
                else {
                    return lhs.relevance > rhs.relevance;
                }
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        return FindTopDocuments(raw_query, [](int document_id, DocumentStatus status, int rating) {return status == DocumentStatus::ACTUAL; });
    }


    size_t GetDocumentCount() const {
        return documents_.size();
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const {
        const Query query = ParseQuery(raw_query);
        if (!CheckTextValid(raw_query)) throw invalid_argument("invalid text"s);
        vector<string> matched_words;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }
        return make_tuple(matched_words, documents_.at(document_id).status);
    }

    int GetDocumentId(int index) const {
        return documents_order_.at(index);
    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;
    vector<int> documents_order_;

    bool CheckIDValid(int document_id) const {
        return !(documents_.count(document_id) == 0 && document_id >= 0);
    }

    bool CheckTextValid(const string& text, bool text_with_minus_words = false) const {
        return none_of(text.begin(), text.end(), [](char c) {return c >= 0 && c <= 31; });
    }

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;

        if (text[0] == '-') {
            if (text.size() == 1) throw invalid_argument("No words after '-'"s);
            if (text[1] == '-') throw invalid_argument("'--' is invalid format for minus word."s);
            is_minus = true;
            text = text.substr(1);
        }
        return { text, is_minus, IsStopWord(text) };
    }

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                }
                else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }

    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }
    template <typename Predicate>
    vector<Document> FindAllDocuments(const Query& query, Predicate filter_key) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                if constexpr (!is_same_v<Predicate, DocumentStatus>) {
                    if (filter_key(document_id, documents_.at(document_id).status, documents_.at(document_id).rating)) {
                        document_to_relevance[document_id] += term_freq * inverse_document_freq;
                    }
                }
                else if (documents_.at(document_id).status == filter_key) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }

            }
        }

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back(
                { document_id, relevance, documents_.at(document_id).rating });
        }
        return matched_documents;
    }
};