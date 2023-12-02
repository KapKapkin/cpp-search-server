#pragma once

#include <iostream>

struct Document {
    Document(int doc_id=0, double doc_relevance=0, int doc_rating=0);
    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,s
};

std::ostream& operator << (std::ostream& output, const Document& doc);
