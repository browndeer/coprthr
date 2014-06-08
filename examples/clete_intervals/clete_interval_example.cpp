// clete_clarray_example.cpp

#include <iostream>
using namespace std;

#include "Timer.h"

///////////////////////////////////////////////////////////////////////////////
// #define __CLVECTOR_FULLAUTO to enable CLETE automatic GPU acceleration
// for pure SIMD operations on clarray data objects.
//
// Set the environmaent variable COPRTHR_LOG_AUTOKERN to see the automatically
// generated OpenCL kernels used to execute the computation on GPU.
//
// With the #define commented out standard expression-templating is used
// to efficiently execute the computation on the CPU.
//////////////////////////////////////////////////////////////////////////////

//#include "interval.h"

#include <stdcl.h>
#include <clarray.h>

#define __CLVECTOR_FULLAUTO
//#define __CLVECTOR_FULLAUTO_STDCPU
//#define __CLVECTOR_FULLAUTO_STDGPU
//#define __CLVECTOR_FULLAUTO_STDACC 
#include <clvector.h>
#include <clarray.h>
#include <CLETE/clarray_CLETE.h>

int main()
{
	Setup(0);
	Reset(0);

	int n = 1048576;

	clvector<float> a,b,c;

	for (int i=0; i<n; ++i) {
		a.push_back(0);
		b.push_back(1.1f*i);
		c.push_back(2.2f*i);
	}

	Interval I(1,n-1);

	Start(0);
	for(int iter=0; iter<10; iter++) {

//		a(I) = b(I-1) + c(I+1);
		a(I) = b(I-1) + c(I+1) + cos(I-2);

	}
	Stop(0);
	double t = GetElapsedTime(0);

//	for (int i=n-10; i<n; ++i) {
	for (int i=0; i<10; ++i) {
		cout << " a(" << i << ") = " << a[i] 
				<< " b(" << i << ") = " << b[i] << endl;
	}

	printf("compute time %f (sec)\n",t);

}

