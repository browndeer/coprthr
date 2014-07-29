// clete_clmulti_array_example.cpp

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
#define __CLMULTI_ARRAY_FULLAUTO
#include <clmulti_array.h>
#include <CLETE/Interval.h>
#include <clarray.h>

int main()
{
	Setup(0);
	Reset(0);

//   typedef clmulti_array< float, 1 > array1_t;
//   typedef clmulti_array< float, 2 > array2_t;
//   typedef clmulti_array< float, 3 > array3_t;
//   typedef clmulti_array< float, 4 > array4_t;

//   array1_t a(boost::extents[100]);
//   array1_t a2(boost::extents[100]);

	clarray<float> a(10);
	for(int i=0; i<10; i++) 
		a[i] = 1000*i;

//	array2_t b(boost::extents[5][3]);
//	array2_t b2(boost::extents[5][3]);
	clarray<float,2> b(5,3);
	clarray<float,2> b2(5,3);

//	array3_t c(boost::extents[5][3][10]);
//	array3_t c2(boost::extents[5][3][10]);
	clarray<float,3> c(5,3,10);
	clarray<float,3> c2(5,3,10);


//   for(int i = 0; i<100; i++) {
//		a[i] = i;
//		a2[i] = 2*i;
//		for(int j=0; j<30; j++) {
//			b[i][j] = i*j;
//			b2[i][j] = 2*i*j;
//
//			for(int k=0; k<45; k++) {
//				c[i][j][k] = i+j+k;
//				for(int l=0; l<60; l++) d[i][j][k][l] = i*j*k*l;
//			}
//		}
//	}
	for(int i = 0; i<5; i++) {
		for(int j=0; j<3; j++) {
			b[i][j] = 1 + i*3 + j;
		}
	}

	for(int i=0; i<5; i++) 
	for(int j=0; j<3; j++) 
	for(int k=0; k<10; k++) {
		c[i][j][k] = 1 + i*3 + j +100*k;
	}

	float* data_ptr = b.origin();
	for(int i=0; i<15; i++) 
		cout<<" "<<data_ptr[i];
	cout<<"\n";

	Start(0);
//	for(int iter=0;iter<10;iter++) {
//		x = a*b*c*d + sqrt(d) -81.0f + pow(c*d,0.33f);
//		for(int i = 0; i<100; i++) a[i] = cos(x[i][0][0][0]);
//	}

	Interval I(1,5);
	Interval J(1,3);
	Interval K(0,10);
//	b2(I,J) = b(I,J) + b(I-1,J-1);
	b2(I,J) = b(I,J) + b(I-1,J-1) + a(I);

	c2(I,J,K) = c(I,J,K) + c(I-1,J-1,K);
	
	Stop(0);
	double t = GetElapsedTime(0);

//   for(int i=0;i<10;i++) cout<<i<<" "<<x[i][i][i][i]<<endl;
//   for(int i=0;i<10;i++) cout<<i<<" "<<a2[i]<<endl;

	for(int i = 0; i<5; i++) {
		for(int j=0; j<3; j++) {
			cout<<" "<<b[i][j];
		}
		cout<<"\n";
	}

	cout<<"\n";

	for(int i = 0; i<5; i++) {
		for(int j=0; j<3; j++) {
			cout<<" "<<b2[i][j];
		}
		cout<<"\n";
	}

	cout<<"\n";

	int k=2;
	for(int i = 0; i<5; i++) {
		for(int j=0; j<3; j++) {
			cout<<" "<<c[i][j][k];
		}
		cout<<"\n";
	}

	cout<<"\n";

	for(int i = 0; i<5; i++) {
		for(int j=0; j<3; j++) {
			cout<<" "<<c2[i][j][k];
		}
		cout<<"\n";
	}

	cout<<"compute time "<<t<<" (sec)\n";

	cout << c2.size(0) << "\n";
	cout << c2.size(1) << "\n";
	cout << c2.size(2) << "\n";
}


