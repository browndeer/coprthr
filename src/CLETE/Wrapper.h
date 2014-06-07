
#ifndef _WRAPPER_H
#define _WRAPPER_H

#include "CLETE/Interval.h"

template < class T >
struct Wrapper {
	Wrapper( const T* obj, const Interval& interval ) 
		: p_obj_(obj), interval_(interval) {}
   const T* p_obj_;
	Interval interval_;
};

template < class T >
const T& unwrap( const Wrapper<T>& w ) { return w.p_obj_; }

#endif

