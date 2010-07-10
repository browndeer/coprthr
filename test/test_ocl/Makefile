
TESTS = test_arg_int test_block_int test_arg_int4 \
	test_arg_float test_block_float test_arg_float4 \
	test_special \
	test_math_float

#TESTS = test_faf

TOPDIR ?= ../

#INCS = -I$(TOPDIR)/include
INCS = -I/usr/local/browndeer/include
LIBS = -lm -L/usr/local/browndeer/lib -locl
#LIBS = -lm -L/usr/local/atistream/beta4/lib/x86_64 -lOpenCL

SRC_TESTS = $(addsuffix .c,$(TESTS))
CLSRC_TESTS = $(addsuffix .cl,$(TESTS))
PRG_TESTS = $(addsuffix .x,$(TESTS))

#all: $(addsuffix .x,$(TESTS))  
all: $(PRG_TESTS)

#.PHONEY: $(addsuffix .x,$(TESTS))
.PHONEY: $(PRG_TESTS)

.SUFFIXES:
.SUFFIXES: .c .o

#CFLAGS += -g

test.x: test.o
	$(CC) $(CFLAGS) $(INCS) -o test.x test.c $(OBJS) $(LIBS)

test_%.x: ./scripts/gen_c_test_%.pl ./scripts/gen_cl_test_%.pl
	./scripts/gen_c_test_$*.pl > test_$*.c
	./scripts/gen_cl_test_$*.pl > test_$*.cl
	$(CC) $(CFLAGS) $(INCS) -o $@ test_$*.c $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(DEF) $(INCS) -c $<

test: clean $(addsuffix .x,$(TESTS))
	/usr/bin/time ./scripts/run_tests.sh $(PRG_TESTS)

quicktest: clean $(addsuffix .x,$(TESTS))
	/usr/bin/time ./scripts/run_quick_tests.sh $(PRG_TESTS)


clean:
	rm -f *.o *.x
	rm -f $(SRC_TESTS) $(CLSRC_TESTS)
