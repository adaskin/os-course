# Makefile see https://docs.kernel.org/kbuild/modules.html
obj-m += mytaskinfo.o 
mytaskinfo-objs += my_module.o my_proc_ops.o 

PWD := $(CURDIR) 

all: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 

clean: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
