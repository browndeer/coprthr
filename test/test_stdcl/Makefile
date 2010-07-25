
TESTS = arg_int arg_int4 arg_float arg_float4 block_int block_float special \
	ndev_float4

CLSRC = test_arg_int.cl test_arg_int4.cl \
	test_arg_float.cl test_arg_float4.cl \
	test_block_int.cl test_block_float.cl test_special.cl

TOPDIR ?= ../

INCS = -I/usr/local/browndeer/include
LIBS = -lm -L/usr/local/browndeer/lib -lstdcl

SRC_TESTS = $(addprefix test_stdcl_,$(addsuffix .c,$(TESTS)))
#CLSRC = $(addprefix test_,$(addsuffix .cl,$(TESTS)))
PRG_TESTS = $(addprefix test_stdcl_,$(addsuffix .x,$(TESTS)))

all: getndev.x $(PRG_TESTS)

#.PHONEY: $(PRG_TESTS)

.SUFFIXES:
.SUFFIXES: .c .o

CFLAGS += -g

test.x: test.o
	$(CC) $(CFLAGS) $(INCS) -o test.x test.c $(OBJS) $(LIBS)

test_%.cl: ./scripts/gen_cl_test_%.pl
	./scripts/gen_cl_test_$*.pl > test_$*.cl

#test_stdcl_%.x: ./scripts/gen_c_test_stdcl_%.pl ./scripts/gen_cl_test_%.pl
#	./scripts/gen_c_test_stdcl_$*.pl > test_stdcl_$*.c
#	./scripts/gen_cl_test_$*.pl > test_$*.cl
#	$(CC) $(CFLAGS) $(INCS) -o $@ test_stdcl_$*.c $(OBJS) $(LIBS)

test_stdcl_%.x: ./scripts/gen_c_test_stdcl_%.pl 
	./scripts/gen_c_test_stdcl_$*.pl > test_stdcl_$*.c
	$(CC) $(CFLAGS) $(INCS) -o $@ test_stdcl_$*.c $(OBJS) $(LIBS)

getndev.x: ./src/getndev.c
	$(CC) $(CFLAGS) $(INCS) -o $@ ./src/getndev.c $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(DEF) $(INCS) -c $<

test: clean getndev.x $(CLSRC) $(PRG_TESTS)
	/usr/bin/time ./scripts/run_tests.sh $(PRG_TESTS)

quicktest: clean getndev.x $(CLSRC) $(PRG_TESTS)
	/usr/bin/time ./scripts/run_quick_tests.sh $(PRG_TESTS)

looptest: clean getndev.x $(CLSRC) $(PRG_TESTS)
	/usr/bin/time ./scripts/run_loop_tests.sh $(PRG_TESTS)


clean:
	rm -f *.o getndev.x
	rm -f $(SRC_TESTS) $(CLSRC) $(PRG_TESTS)
