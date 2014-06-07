
#ifndef _INTERVAL_H
#define _INTERVAL_H

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

#endif

