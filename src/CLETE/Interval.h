
#ifndef _INTERVAL_H
#define _INTERVAL_H

#include <vector>

struct Interval {
	Interval( int f, int e, int s = 0) : first(f), end(e), shift(s) {}
   int first, end, shift;
};

Interval operator + ( int s, Interval& ii )
{ return Interval(ii.first,ii.end,ii.shift+s); }

Interval operator + ( Interval& ii, int s )
{ return Interval(ii.first,ii.end,ii.shift+s); }

Interval operator - ( int s, Interval& ii )
{ return Interval(ii.first,ii.end,ii.shift-s); }

Interval operator - ( Interval& ii, int s )
{ return Interval(ii.first,ii.end,ii.shift-s); }

typedef Interval interval2_t[2];

std::vector<Interval> operator , ( Interval& I, Interval& J )
{
	std::vector<Interval> ivec;
	ivec.push_back(I);
	ivec.push_back(J);
	return ivec;
}

std::vector<Interval>& operator , ( std::vector<Interval>& ivec, Interval& J )
{
	ivec.push_back(J);
	return ivec;
}

#endif

