#pragma once
#include <vector>

template <typename It>
class IteratorRange {
public:
    IteratorRange(It begin, It end);
    It begin() const;
    It end() const;
private:
    It _begin;
    It _end;
};

template <typename It>
IteratorRange<It>::IteratorRange(It begin, It end)
    :_begin(begin), _end(end)
{
}

template <typename It>
It IteratorRange<It>::begin() const {
    return _begin;
}

template <typename It>
It IteratorRange<It>::end() const {
    return _end;
}

template <typename It>
class Paginator {
public:
    Paginator(It begin, It end, int page_size);
    const auto& begin() const;
    const auto& end() const;
private:
    std::vector<IteratorRange<It>> _pages;
 };

template <typename It>
Paginator<It>::Paginator(It begin, It end, int page_size) {

    if (page_size == 0) return;
    auto last = begin;
    for (auto i = begin; i != end; advance(i, 1)) {

        if (distance(last, i) == page_size) {
            IteratorRange<It> a(last, i);
            _pages.push_back(a);
            last = i;
        }
        if (distance(i, end) <= 1) {
            IteratorRange<It> a(last, end);
            _pages.push_back(a);
            break;
        }
    }
}

template <typename It>
const auto& Paginator<It>::begin() const {
    return _pages.begin();
}

template <typename It>
const auto& Paginator<It>::end() const {
    return _pages.end();
}

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}
