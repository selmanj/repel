/*
 * infix_ostream_iterator.h
 *
 *  Created on: Nov 9, 2011
 *      Author: joe
 */

#ifndef INFIX_OSTREAM_ITERATOR_H_
#define INFIX_OSTREAM_ITERATOR_H_

#include <ostream>
#include <iterator>

template <class T, class charT=char, class traits=std::char_traits<charT> >
class infix_ostream_iterator : public std::iterator<std::output_iterator_tag, void, void, void, void> {
	std::basic_ostream<charT, traits>* out_stream;
	const charT* delim;
	bool first;

public:
	typedef charT char_type;
	typedef traits traits_type;
	typedef std::basic_ostream<charT, traits> ostream_type;

	infix_ostream_iterator(ostream_type& s) : out_stream(&s), delim(0), first(true) {}
	infix_ostream_iterator(ostream_type& s, const charT* delimiter)
		: out_stream(&s), delim(delimiter), first(true) {}
	infix_ostream_iterator(const infix_ostream_iterator<T, charT, traits>& x)
		: out_stream(x.out_stream), delim(x.delim), first(x.first) {}
	~infix_ostream_iterator() {}

	infix_ostream_iterator<T, charT, traits>& operator= (const T& value) {
		if (delim!=0 && !first) {
			*out_stream << delim;
		}
		*out_stream << value;
		first = false;
		return *this;
	}

	infix_ostream_iterator<T, charT, traits>& operator*() {return *this; }
	infix_ostream_iterator<T, charT, traits>& operator++() {return *this; }
	infix_ostream_iterator<T, charT, traits>& operator++(int) {return *this; }

};


#endif /* INFIX_OSTREAM_ITERATOR_H_ */
