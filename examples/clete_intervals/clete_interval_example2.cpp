// clete_interval_example2.cpp

#include <iostream>
using namespace std;

#include "Timer.h"

///////////////////////////////////////////////////////////////////////////////
// #define __CLMULTI_ARRAY_FULLAUTO to enable CLETE automatic GPU acceleration
// for pure SIMD operations on clvector data objects.
//
// Set the environmaent variable COPRTHR_LOG_AUTOKERN to see the automatically
// generated OpenCL kernels used to execute the computation on GPU.
//
// With the #define commented out standard expression-templating is used
// to efficiently execute the computation on the CPU.
//////////////////////////////////////////////////////////////////////////////

#include <stdcl.h>
//#define __CLMULTI_ARRAY_FULLAUTO
#include <clmulti_array.h>
#include <subset.h>
#include <CLETE/subset_CLETE.h>

int main()
{
	Setup(0);
	Reset(0);

   typedef clmulti_array< float, 1 > array1_t;
   typedef clmulti_array< float, 2 > array2_t;
   typedef clmulti_array< float, 3 > array3_t;
   typedef clmulti_array< float, 4 > array4_t;

   array1_t a(boost::extents[100]);
	array2_t b(boost::extents[100][30]);
	array3_t c(boost::extents[100][30][45]);
	array4_t d(boost::extents[100][30][45][60]);
	array4_t x(boost::extents[100][30][45][60]);

   for(int i = 0; i<100; i++) {
		a[i] = i;
		for(int j=0; j<30; j++) {
			b[i][j] = i*j;
			for(int k=0; k<45; k++) {
				c[i][j][k] = i+j+k;
				for(int l=0; l<60; l++) d[i][j][k][l] = i*j*k*l;
			}
		}
	}

	Interval I(0,100);
	Interval J(0,30);
	Interval K(0,45);
	Interval L(0,60);

	Start(0);
	for(int iter=0;iter<10;iter++) {

		x = a*b*c*d + sqrt(d) -81.0f + pow(c*d,0.33f);
/*
		x(I,J,K,L) = a(I,J,K,L)*b(I,J,K,L)*c(I,J,K,L)*d(I,J,K,L) 
			+ sqrt(d(I,J,K,L) - 81.0f + pow(c(I,J,K,L)*d(I,J,K,L),0.33f);
*/
		for(int i = 0; i<100; i++) a[i] = cos(x[i][0][0][0]);
	}
	Stop(0);
	double t = GetElapsedTime(0);

   for(int i=0;i<10;i++) cout<<i<<" "<<x[i][i][i][i]<<endl;

	cout<<"compute time "<<t<<" (sec)\n";

}


