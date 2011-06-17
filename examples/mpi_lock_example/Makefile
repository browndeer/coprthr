
PROG = mpi_lock_example.x 

OPENCL_INC_PATH = /usr/local/atistream/include
OPENCL_LIB_PATH = /usr/local/atistream/lib/x86_64

STDCL_INC_PATH = /usr/local/browndeer/include
STDCL_LIB_PATH = /usr/local/browndeer/lib

INCS = -I$(OPENCL_INC_PATH) -I$(STDCL_INC_PATH)
LIBS = -L$(OPENCL_LIB_PATH) -lOpenCL -L$(STDCL_LIB_PATH) -lstdcl

CC = mpicc
CCFLAGS += -O3

all: $(PROG)

mpi_lock_example.x: mpi_lock_example.o 
	$(CC) $(CCFLAGS) $(INCS) -o mpi_lock_example.x mpi_lock_example.o $(LIBS)

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CCFLAGS) $(INCS) -c $<

clean:
	rm -f *.o *.x

