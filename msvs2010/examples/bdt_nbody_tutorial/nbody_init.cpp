/* nbody_init.c */

#include <stdlib.h>

#include <stdcl.h>

#define frand() (rand()/(cl_float)RAND_MAX)

void nbody_init( int n, cl_float4* pos, cl_float4* vel )
{

   int i;

   srand(2112);

   for(i=0;i<n;i++) {
      pos[i].s[0] = frand();
	  pos[i].s[1] = frand();
	  pos[i].s[2] = frand();
	  pos[i].s[3] = frand();
      vel[i].s[0] = 0.0f;
	  vel[i].s[1] = 0.0f;
	  vel[i].s[2] = 0.0f;
	  vel[i].s[3] = 0.0f;
   }

   printf("last mass %e\n",cl_float4_w(pos[n-1]));

   printf("%e %e %e\n",cl_float4_x(pos[0]),cl_float4_y(pos[0]),cl_float4_z(pos[0]));

}

