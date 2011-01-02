
__kernel void
nbody_kern(
	int n,
	float dt,
	float eps,
	__global float4* pp,
	__global float4* vv,
	__global float4* ppo,
	__local float4* pblock
)
{
	const float4 zero4 = (float4){0.0f,0.0f,0.0f,0.0f};
	const float4 invtwo4 = (float4){0.5f,0.5f,0.5f,0.5f};
	const float4 dt4 = (float4){dt,dt,dt,0.0f};

	int gti = 2*get_global_id(0);

	int ti = get_local_id(0);

	int nt = get_local_size(0);
	int nb = 2*n/nt;

	float4 p0 = ppo[gti+0];
	float4 p1 = ppo[gti+1];

	float4 a0 = zero4;
	float4 a1 = zero4;

	int ib,i;

	// loop over blocks 

	for(ib=0;ib<nb;ib++) {

		prefetch(ppo,64);

		// copy to local memory 

		int gci = ib*nt+ti;

     	pblock[ti] = ppo[gci];

		barrier(CLK_LOCAL_MEM_FENCE);


		// loop within block accumulating acceleration of particles 

		for(i=0;i<nt;i++) {

			float4 p2 = pblock[i];

			float4 dp = p2 - p0;	
			float invr = rsqrt(dp.x*dp.x + dp.y*dp.y + dp.z*dp.z + eps);
//			float f = p2.w * invr*invr*invr;
			a0 += (p2.w * invr*invr*invr)*dp;

			dp = p2 - p1;	
			invr = rsqrt(dp.x*dp.x + dp.y*dp.y + dp.z*dp.z + eps);
//			f = p2.w * invr*invr*invr;
			a1 += (p2.w * invr*invr*invr)*dp;

		}

		barrier(CLK_LOCAL_MEM_FENCE);


	}

	// update position and velocity

	float4 v0 = vv[gti+0];
   p0 += dt4*v0 + invtwo4*dt4*dt4*a0;
   v0 += dt4*a0;
	pp[gti+0] = p0;
	vv[gti+0] = v0;

	float4 v1 = vv[gti+1];
   p1 += dt4*v1 + invtwo4*dt4*dt4*a1;
   v1 += dt4*a1;
	pp[gti+1] = p1;
	vv[gti+1] = v1;



	return;

}



