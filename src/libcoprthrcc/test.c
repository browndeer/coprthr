
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coprthr_cc.h"
#include "coprthr.h"

char src[] = \
	"__kernel void\n" \
	"my_kern( float* a, float* b, float* c) {\n" \
	"	int gtid = get_global_id(0);\n" \
	"	c[gtid] = a[gtid]+b[gtid];\n" \
	"}\n";

int main()
{
	int i;

	printf("\ncheck version:\n\n");
	char* log = 0;
	coprthr_cc(0,0,"--version",&log);
	printf("%s\n",log);
	free(log);

	printf("\ncheck targets:\n\n");
	log = 0;
	coprthr_cc(0,0,"--targets",&log);
	printf("%s\n",log);
	free(log);


	printf("src |%s|\n",src);
	printf("src_sz %ld\n",sizeof(src));

	coprthr_program_t prg1 = coprthr_cc(src,sizeof(src),"-mtarget=x86_64",0);
	printf("prg=%p\n",prg1);
	coprthr_cc_write_bin("./bin_x86_64.o",prg1,0);

	coprthr_program_t prg2 = coprthr_cc(src,sizeof(src),"-mtarget=i386",0);
	printf("prg=%p\n",prg2);
	coprthr_cc_write_bin("./bin_i386.o",prg2,0);

	coprthr_program_t prg1_ld = coprthr_cc_read_bin("./bin_x86_64.o",0);
	printf("prg=%p\n",prg1_ld);

	coprthr_program_t prg2_ld = coprthr_cc_read_bin("./bin_x86_64.o",0);
	printf("prg=%p\n",prg2_ld);

	coprthr_dev_t dev = coprthr_getdev(0,0);
	coprthr_devlink(dev,prg1,0);

	coprthr_kernel_t krn = coprthr_sym(prg1,"my_kern");
	printf("krn=%p\n",krn);

}

