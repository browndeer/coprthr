
#include <iostream>
using namespace std;

#define __CLMULTI_ARRAY_FULLAUTO
#include "clmulti_array.h"

int main()
{

   typedef clmulti_array< float, 1 > array1_t;
   typedef clmulti_array< float, 2 > array2_t;
   typedef clmulti_array< float, 3 > array3_t;
   typedef clmulti_array< float, 4 > array4_t;

   array1_t a(boost::extents[16]);
	array2_t b(boost::extents[16][17]);
	array3_t c(boost::extents[16][17][18]);
	array4_t d(boost::extents[16][17][18][19]);
	array4_t x(boost::extents[16][17][18][19]);

   for(int i = 0; i<16; i++) {
		a[i] = i;
		for(int j=0; j<17; j++) {
			b[i][j] = i*j;
			for(int k=0; k<18; k++) {
				c[i][j][k] = i+j+k;
				for(int l=0; l<19; l++) d[i][j][k][l] = i*j*k*l;
			}
		}
	}

	for(int iter=0;iter<10;iter++) {

		x = a*b*c*d + sqrt(d) -81.0f + pow(c*d,0.33f);

	}

   for(int i=0;i<10;i++) cout<<i<<" "<<x[i][i][i][i]<<endl;

}


