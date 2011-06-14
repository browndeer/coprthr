### Makefile for N-Body program
NAME = nbody
OBJS = nbody_init.o nbody_output.o
OPENCL = /usr/local/atistream
STDCL = /usr/local/browndeer
INCS += -I$(OPENCL)/include -I$(STDCL)/include
LIBS += -L$(OPENCL)/lib/x86_64 -lOpenCL -lpthread -ldl -L$(STDCL)/lib -lstdcl
CFLAGS += -O3

all: $(NAME).x

$(NAME).x: $(NAME).o $(OBJS)
	$(CC) $(CFLAGS) $(INCS) -o $(NAME).x $(NAME).o $(OBJS) $(LIBS)

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) $(INCS) -c $<

clean:
	rm -f *.o *.x 
