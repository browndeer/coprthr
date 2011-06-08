#define __CLVECTOR_FULLAUTO

#include <stdcl.h>
#include <clvector.h>


int main()
{

  int i;
  const int n = 290;


	/* float test */

  clvector<float> a,b,c,d,e;

  for (i = 0; i < n; ++i)
  {
    a.push_back(1.1*i);
    b.push_back(2.2*i);
    c.push_back(3.3*i);
    d.push_back(1.0*i);
    e.push_back(-99.0);
	}

  for(int i = 0; i < 10; i++) {
		e = a + b + 2112.0f * b + sqrt(d) - a*b*c*d;
	}

  for (i = 0; i < n; ++i)
    {
      cout << " a(" << i << ") = " << a[i]
	   << " b(" << i << ") = " << b[i]
	   << " c(" << i << ") = " << c[i]
	   << " d(" << i << ") = " << d[i]
	   << " e(" << i << ") = " << e[i]
	   << endl;
    }

	
	/* cl_float4 test */

  clvector<cl_float4> a4,b4,c4,d4,e4;

  for (i = 0; i < n; ++i)
  {
    a4.push_back((cl_float4){1.1*i,1.1*i,1.1*i,1.1*i});
    b4.push_back((cl_float4){2.2*i,2.2*i,2.2*i,2.2*i});
    c4.push_back((cl_float4){3.3*i,3.3*i,3.3*i,3.3*i});
    d4.push_back((cl_float4){1.0*i,1.0*i,1.0*i,1.0*i});
    e4.push_back((cl_float4){-99.0*i,-99.0*i,-99.0*i,-99.0*i});
	}

  for(int i = 0; i < 10; i++) {
		e4 = a4 + b4 + 2112.0f * b4 + sqrt(d4) - a4*b4*c4*d4;
	}

  for (i = 0; i < n; ++i)
    {
      cout << " a4(" << i << ") = " << a4[i].x
	   << " b4(" << i << ") = " << b4[i].x
	   << " c4(" << i << ") = " << c4[i].x
	   << " d4(" << i << ") = " << d4[i].x
	   << " e4(" << i << ") = " << e4[i].x
	   << endl;
    }

}

