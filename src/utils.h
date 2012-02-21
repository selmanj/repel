/*
 * utils.h
 *
 *  Created on: Jun 14, 2011
 *      Author: joe
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <set>
#include <stdexcept>

// TODO: just awful, refactor this crap out!

template <class T>
T set_at(const std::set<T> &set, typename std::set<T>::size_type index) {
    if (index >= set.size() || index < 0) {
        throw std::runtime_error("tried to get index out of bounds on set in set_at()");
    }
    typename std::set<T>::const_iterator it = set.begin();
    while (index!=0) {
        it++;
        index--;
    }
    return *it;
}

/**
 * Combine two iterator ranges into a single iterator.
 */

template <typename ForwardIterator1, typename ForwardIterator2, typename T>
class MergedIterator : std::iterator<std::input_iterator_tag, T>{
public:
    MergedIterator(ForwardIterator1 begin1, ForwardIterator1 end1,
            ForwardIterator2 begin2, ForwardIterator2 end2)
        : cur1_(begin1), end1_(end1), cur2_(begin2), end2_(end2) {}
    MergedIterator(const MergedIterator& o)
        : cur1_(o.cur1_), end1_(o.end1_), cur2_(o.cur2_), end2_(o.end2_) {}

    MergedIterator& operator=(const MergedIterator& o) {
        if (this == o) return *this;
        cur1_ = o.cur1_;
        end1_ = o.end1_;
        cur2_ = o.cur2_;
        end2_ = o.end2_;
        return *this;
    }

    template<typename T1, typename T2, typename T3>
    friend bool operator==(const MergedIterator<T1, T2, T3>& a, const MergedIterator<T1, T2, T3>& b);
    template<typename T1, typename T2, typename T3>
    friend bool operator!=(const MergedIterator<T1, T2, T3>& a, const MergedIterator<T1, T2, T3>& b);

    T& operator*() {
        if (cur1_ != end1_) return *cur1_;
        return cur2_;
    }
    T* operator->() {
        if (cur1_ != end1_) return cur1_.operator->();
        return cur2_.operator->();
    }

    MergedIterator<ForwardIterator1, ForwardIterator2, T>& operator++() {
        if (cur1_ != end1_) {
            cur1_++;
        }
        else cur2_++;
        return *this;
    }

    MergedIterator<ForwardIterator1, ForwardIterator2, T> operator++(int) {
        MergedIterator<ForwardIterator1, ForwardIterator2, T> copy = *this;
        operator++();
        return copy;
    }


private:
    ForwardIterator1 cur1_, end1_;
    ForwardIterator2 cur2_, end2_;
};

template<typename T1, typename T2, typename T3>
inline bool operator==(const MergedIterator<T1, T2, T3>& a, const MergedIterator<T1, T2, T3>& b) {
    return a.cur1_ == b.cur1_ && a.end1_ == b.end1_ && a.cur2_ == b.cur2_ && a.end2_ == b.end2_;
}
template<typename T1, typename T2, typename T3>
inline bool operator!=(const MergedIterator<T1, T2, T3>& a, const MergedIterator<T1, T2, T3>& b) {return !operator==(a, b);}

#endif /* UTILS_H_ */
