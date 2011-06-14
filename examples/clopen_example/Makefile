
PROG = clopen_example1.x clopen_example2.x

### add this example if clld is available
PROG += clopen_example3.x

OPENCL_INC_PATH = /usr/local/atistream/include
OPENCL_LIB_PATH = /usr/local/atistream/lib/x86_64

STDCL_INC_PATH = /usr/local/browndeer/include
STDCL_LIB_PATH = /usr/local/browndeer/lib

INCS = -I$(OPENCL_INC_PATH) -I$(STDCL_INC_PATH)
LIBS = -L$(OPENCL_LIB_PATH) -lOpenCL -L$(STDCL_LIB_PATH) -lstdcl

CCFLAGS += -O3

all: $(PROG)

clopen_example1.x: clopen_example1.o 
	$(CC) $(CCFLAGS) $(INCS) -o clopen_example1.x clopen_example1.o $(LIBS)

clopen_example2.x: clopen_example2.o 
	$(CC) $(CCFLAGS) $(INCS) -o clopen_example2.x clopen_example2.o $(LIBS)

clopen_example3.x: clopen_example3.o out_clld.o
	$(CC) $(CCFLAGS) $(INCS) -o clopen_example3.x clopen_example3.o $(LIBS) \
		out_clld.o

out_clld.o: outerprod_three.cl
	clld --cl-source outerprod_three.cl

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CCFLAGS) $(INCS) -c $<

clean:
	rm -f *.o *.x

