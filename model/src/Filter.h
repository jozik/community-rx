/*
 * Filter.h
 *
 *  Created on: Apr 20, 2018
 *      Author: nick
 */

#ifndef FILTER_H_
#define FILTER_H_

#include <set>

namespace crx {

template <typename T>
class Filter {

public:
    Filter();
    virtual bool evaluate(T item) = 0;
    virtual ~Filter();
};

template<typename T>
Filter<T>::Filter() {}

template<typename T>
Filter<T>::~Filter() {}


template <typename T>
class ContainsFilter : public Filter<T> {
private:
    std::set<T> filter;

public:
    ContainsFilter();
    void addItem(T to_include);
    virtual bool evaluate(T item);
    virtual ~ContainsFilter();
};


template<typename T>
ContainsFilter<T>::ContainsFilter() : Filter<T>(),  filter() {}

template<typename T>
ContainsFilter<T>::~ContainsFilter() {}

template<typename T>
void ContainsFilter<T>::addItem(T to_include) {
    filter.emplace(to_include);
}

template<typename T>
bool ContainsFilter<T>::evaluate(T item) {
    return filter.find(item) != filter.end();
}


template<typename T>
class AlwaysPassFilter : public Filter<T> {

public:
    AlwaysPassFilter();
    virtual bool evaluate(T item);
    virtual ~AlwaysPassFilter();
};

template<typename T>
AlwaysPassFilter<T>::AlwaysPassFilter() : Filter<T> () {}

template<typename T>
AlwaysPassFilter<T>::~AlwaysPassFilter() {}

template<typename T>
bool AlwaysPassFilter<T>::evaluate(T item) {
    return true;
}

template<typename T>
class NeverPassFilter : public Filter<T> {

public:
    NeverPassFilter();
    virtual bool evaluate(T item);
    virtual ~NeverPassFilter();
};

template<typename T>
NeverPassFilter<T>::NeverPassFilter() : Filter<T>() {}

template<typename T>
NeverPassFilter<T>::~NeverPassFilter() {}

template<typename T>
bool NeverPassFilter<T>::evaluate(T item) {
    return false;
}




} /* namespace crx */

#endif /* FILTER_H_ */
