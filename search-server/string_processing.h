#pragma once
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <string>

#include "paginator.h"
#include "document.h"


template <typename First, typename Second>
std::ostream& operator << (std::ostream& out, const std::pair<First, Second>& container) {
    out << "(";
    out << container.first << ", " << container.second;
    out << ")";
    return out;
}

template <typename Container>
void Print(std::ostream& out, const Container& container) {
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
std::ostream& operator << (std::ostream& out, const std::vector<Term>& container) {
    out << "[";
    Print(out, container);
    out << "]";
    return out;
}

template <typename Term>
std::ostream& operator << (std::ostream& out, const std::set<Term>& container) {
    out << "{";
    Print(out, container);
    out << "}";
    return out;
}

template <typename Key, typename Value>
std::ostream& operator << (std::ostream& out, const std::map<Key, Value>& container) {
    out << "<";
    Print(out, container);
    out << ">";
    return out;
}

template <typename It>
std::ostream& operator << (std::ostream& output, const IteratorRange<It>& pages) {
    for (auto i = pages.begin(); i != pages.end(); i++) {
        output << *i;
    }
    return output;
}

std::vector<std::string> SplitIntoWords(const std::string& text);

std::ostream& operator << (std::ostream& output, const Document& doc);