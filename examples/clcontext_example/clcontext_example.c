/* clcontext_example.c */

#include <stdio.h>
#include <stdcl.h>

int main()
{
   CLCONTEXT* cp;

	cp = stddev;

   if (cp) {
		printf("\n***** stddev:\n");
		int ndev = clgetndev(cp);

      struct clstat_info stat_info;
		clstat(cp,&stat_info);

      struct cldev_info* dev_info
         = (struct cldev_info*)malloc(ndev*sizeof(struct cldev_info));
		clgetdevinfo(cp,dev_info);

		clfreport_devinfo(stdout,ndev,dev_info);

		if (dev_info) free(dev_info);
	}


	cp = stdcpu;

   if (cp) {
		printf("\n***** stdcpu:\n");
		int ndev = clgetndev(cp);

      struct clstat_info stat_info;
		clstat(cp,&stat_info);

      struct cldev_info* dev_info
         = (struct cldev_info*)malloc(ndev*sizeof(struct cldev_info));
		clgetdevinfo(cp,dev_info);

		clfreport_devinfo(stdout,ndev,dev_info);

		if (dev_info) free(dev_info);
	}


	cp = stdgpu;

   if (cp) {
		printf("\n***** stdgpu:\n");
		int ndev = clgetndev(cp);

      struct clstat_info stat_info;
		clstat(cp,&stat_info);

      struct cldev_info* dev_info
         = (struct cldev_info*)malloc(ndev*sizeof(struct cldev_info));
		clgetdevinfo(cp,dev_info);

		clfreport_devinfo(stdout,ndev,dev_info);

		if (dev_info) free(dev_info);
	}

}
