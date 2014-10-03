#include <iostream>
#include <stdcl.h>
#define __CLMULTI_ARRAY_FULLAUTO
#include <clarray.h>

using namespace std;


int main(int argc, char** argv)
{
	int jmax = 20;

	Interval JI(1,jmax-1);
	Interval JII(1,jmax-2);

	clarray<double> x(jmax); x(JI) = JI*JI;
	clarray<double> dx(jmax); dx(JI) = JI*x(JI);

	cout << "dx: " << endl;
	for(int i=0; i<jmax; i++)
		cout << " " << dx[i];
	cout << endl;

	clarray<double> dxp2(jmax); dxp2(JI) = 1.0/(dx(JI)*dx(JI));

	clarray<double> dxm2(jmax); dxm2(JI) = 2.0/(dx(JI)*dx(JI));


	for(int loop=0; loop < 10; loop++) {

	dxp2(JII) = 1.0/(x(JII+1)-x(JII))/dx(JII);


	dxm2(JII) = 1.0/(x(JII)-x(JII-1))/dx(JII);

	}

	cout << "dxp2: " << endl;
	for(int i=0; i<jmax; i++)
		cout << " " << dxp2[i];
	cout << endl;

	cout << "dxm2: " << endl;
	for(int i=0; i<jmax; i++)
		cout << " " << dxm2[i];
	cout << endl;

}

