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

#endif /* UTILS_H_ */
