/* clete_clmulti_array_example.cpp
 *
 * This is a simple example for Parallella showing the use of CLETE with
 * the clmulti_array<> class.  For more informaton see the COPRTHR Primer in
 * the documentation.
 *
 * THIS FILE ONLY is placed in the public domain by Brown Deer Technology, LLC.
 * in January 2013. No copyright is claimed, and you may use it for any purpose
 * you like. There is ABSOLUTELY NO WARRANTY for any purpose, expressed or
 * implied, and any warranty of any kind is explicitly disclaimed.  This
 * statement applies ONLY TO THE COMPUTER SOURCE CODE IN THIS FILE and does
 * not extend to any other software, source code, documentation, or any other
 * materials in which it may be included, or with which it is co-distributed.
 */

/* DAR */

#include <iostream>
using namespace std;

#include "Timer.h"

///////////////////////////////////////////////////////////////////////////////
// #define __CLMULTI_ARRAY_FULLAUTO_STDACC to enable the CLETE automatic 
// accelerator for pure SIMD operations on clvector data objects.
//
// Set the environmaent variable COPRTHR_LOG_AUTOKERN to see the automatically
// generated OpenCL kernels used to execute the computation on the ACCELERATOR.
//
// With the #define commented out standard expression-templating is used
// to efficiently execute the computation on the CPU.
//////////////////////////////////////////////////////////////////////////////

#include <stdcl.h>
//#define __CLMULTI_ARRAY_FULLAUTO_STDACC
#include <clmulti_array.h>

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

	Start(0);
	for(int iter=0;iter<10;iter++) {
		x = a*b*c*d + sqrt(d) -81.0f + pow(c*d,0.33f);
		for(int i = 0; i<100; i++) a[i] = cos(x[i][0][0][0]);
	}
	Stop(0);
	double t = GetElapsedTime(0);

   for(int i=0;i<10;i++) cout<<i<<" "<<x[i][i][i][i]<<endl;

	cout<<"compute time "<<t<<" (sec)\n";

}


