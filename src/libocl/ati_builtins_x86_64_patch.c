#include <math.h>

int __convert_int_f32( float x ) { return( (int)x ); }

float __convert_float_i32( int i ) { return( (float)i ); }

double __lgamma_r_pf64i32 ( double x, int* ps )
{
return( lgamma_r(x,ps) ); 
}
