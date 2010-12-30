//#define __CLVECTOR_FULLAUTO

#include <stdcl.h>
#include <clvector.h>

int main()
{

  int i;
  const int n = 4000;
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
		e = a + b+ 2112.0f * b + sqrt(d) - a*b*c*d;
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
}

