


# Compile flags for the module
DEBFLAGS = -O2 -DTZM_DEBUG


EXTRA_CFLAGS = $(DEBFLAGS)
EXTRA_CFLAGS += -I$(LDDINC)

ifneq ($(KERNELRELEASE),)
# For the SECOND call of Makefile (KERNELRELEASE is defined in this case)
# This call will be done by the kernel build system

# Define all modules that should be compiled. All other actions will be
# done by kernel build system.
translate-objs := trans.o 
# Define all kernel modules that should be generated All other actions 
# will be done by kernel build system.
obj-m := trans.o   

else
# For the FIRST call of Makefile  out of the kernel build env.

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) LDDINC=$(PWD)/../include modules
endif

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions modules.order Module.symvers 

# EOF
