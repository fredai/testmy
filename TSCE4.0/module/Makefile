CC := gcc
NVCC := nvcc
INCLUDE := -I./ -I../src/_con/ -I../src/clr/ -I../lib/include/ -I./micro_structure/common/ -I./micro_structure/haswell/ 
CFLAGS := -Wall -fPIC
LDFLAGS := -shared
ifeq ($(shell uname -m),x86_64)
    LIBDIRS := -L../lib/lib64/infiniband -L./ -L../lib
    CFLAGS := -Wall -fPIC
else
    LIBDIRS := -L../lib/lib32/infiniband -L./ -L../lib
    CFLAGS := -Wall
endif
CLDFLAGS := -libmad -libumad -lm -lMsrPci -lpthread
CLDFLAGS_GPU_DRIVER :=  -lnvidia-ml
#-Wl,-rpath=../lib

COMMON_OBJS := common.o
MICRO_COMMON_OBJS := ./micro_structure/common/msr.o ./micro_structure/common/pci.o 
MSR_PCI_LIB := libMsrPci.a

OBJS := t_cpu.o t_mem.o t_eth.o t_nfs.o t_disk.o t_ib.o t_staticinfo.o t_cpumicro.o t_rapl.o t_cputemp.o  t_disksize.o  t_memdevice.o  t_nettra.o  t_sysload.o t_iboip.o t_opa.o
OBJS_GPU_DRIVER := t_gpu_base1.o t_gpu_base2.o t_gpu_clock.o t_gpu_power.o t_gpu_ecc.o

TARGET := $(OBJS:%.o=%.so)
TARGET_GPU_DRIVER := $(OBJS_GPU_DRIVER:%.o=%.so)

all: $(MSR_PCI_LIB)  $(TARGET) $(TARGET_GPU_DRIVER) 

$(COMMON_OBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) \
			-c $< -o $@ 
$(TARGET): $(COMMON_OBJS) $(OBJS)
	$(CC) $(LDFLAGS) $(COMMON_OBJS) $(patsubst %.so,%.o,$@) \
		-o $(patsubst t_%,lib%,$@) $(LIBDIRS) $(CLDFLAGS) 
$(TARGET_GPU_DRIVER): $(COMMON_OBJS) $(OBJS_GPU_DRIVER)
	$(CC) $(LDFLAGS) $(COMMON_OBJS) $(patsubst %.so,%.o,$@) \
		-o $(patsubst t_%,lib%,$@) $(LIBDIRS) $(CLDFLAGS_GPU_DRIVER) 
$(OBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) \
			-c $< -o $@ 
$(OBJS_GPU_DRIVER):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) \
			-c $< -o $@ 
$(MSR_PCI_LIB): $(MICRO_COMMON_OBJS)
	ar -crs $@ $(MICRO_COMMON_OBJS) 

$(MICRO_COMMON_OBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) \
			-c $< -o $@ 

clean:
	-rm -f *.so
	-rm -f *.a
	-rm -f *.o
	-rm -f ./micro_structure/common/*.o
	-rm -f ../release/module/*.so
	-rm -f ../release/module/script/*
release:
	cp ./*.so ../release/module/
	cp ./script/* ../release/module/script/
