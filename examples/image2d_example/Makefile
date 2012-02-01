NAME = image2d_example

OPENCL_INC_PATH = /usr/local/atistream/include
OPENCL_LIB_PATH = /usr/local/atistream/lib/x86_64

STDCL_INC_PATH = /usr/local/browndeer/include
STDCL_LIB_PATH = /usr/local/browndeer/lib

INCS = -I$(OPENCL_INC_PATH) -I$(STDCL_INC_PATH)
LIBS = -L$(OPENCL_LIB_PATH) -lOpenCL -L$(STDCL_LIB_PATH) -lstdcl

CCFLAGS += -O3

OBJS = 

all: $(NAME).x

$(NAME).x: $(NAME).o $(OBJS) ocl.o
	$(CC) $(CCFLAGS) $(INCS) -o $(NAME).x $(NAME).o $(OBJS) ocl.o $(LIBS)

ocl.o: matvecmult_special.cl
	clcc -o ocl.o matvecmult_special.cl

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CCFLAGS) $(INCS) -c $<

clean:
	rm -f *.o *.x


