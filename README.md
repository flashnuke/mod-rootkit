```
███╗   ███╗ ██████╗ ██████╗       ██████╗  ██████╗  ██████╗ ████████╗██╗  ██╗██╗████████╗
████╗ ████║██╔═══██╗██╔══██╗      ██╔══██╗██╔═══██╗██╔═══██╗╚══██╔══╝██║ ██╔╝██║╚══██╔══╝
██╔████╔██║██║   ██║██║  ██║█████╗██████╔╝██║   ██║██║   ██║   ██║   █████╔╝ ██║   ██║   
██║╚██╔╝██║██║   ██║██║  ██║╚════╝██╔══██╗██║   ██║██║   ██║   ██║   ██╔═██╗ ██║   ██║   
██║ ╚═╝ ██║╚██████╔╝██████╔╝      ██║  ██║╚██████╔╝╚██████╔╝   ██║   ██║  ██╗██║   ██║   
╚═╝     ╚═╝ ╚═════╝ ╚═════╝       ╚═╝  ╚═╝ ╚═════╝  ╚═════╝    ╚═╝   ╚═╝  ╚═╝╚═╝   ╚═╝   
                                                                                                                                               
```


A simple proof-of-concept Linux Kernel Rootkit module designed to hide processes, files, and itself from userland visibility for modern kernel versions.  

This module operates at the kernel level, allowing it to intercept system calls directly, enabling a higher degree of stealth compared to user-space techniques (i.e `LD_PRELOAD`).

Unlike traditional rootkits that rely on direct syscall table hooking or exported symbols like kallsyms_lookup_name(), this implementation leverages kprobes and ftrace (credit goes to [xcellerator](https://github.com/xcellerator/linux_kernel_hacking) for this method) — for greater stealth and compatibility. It avoids deprecated or removed interfaces, ensuring operability on recent kernel versions (e.g., 5.7+ where kallsyms_lookup_name() is no longer exported).

## Overview

`mod-rootkit` is a Loadable Kernel Module (LKM) that demonstrates basic rootkit techniques in Linux. Once inserted into the kernel, it provides stealth capabilities by intercepting and modifying system behavior to:

- **Process Hiding** – Hide any process based on configured keywords (that appear in the cmdline)
- **File & Directory Hiding** – Hide files and folders based on configured keywords
- **Network connection hiding** – Hide files and folders based on configured IP addresses or ports
- **Module Hiding** – Hides itself from kernel module listings

Demo - before and after loading the module:

<img width="446" alt="image" src="https://github.com/user-attachments/assets/391053e8-06c7-47aa-9301-043b5590e619" />


## Requirements

- Linux system with kernel headers installed
- GCC, make and kernel headers: `sudo apt update && sudo apt install -y build-essential linux-headers-$(uname -r) make gcc`
- Root permissions
- Kernel version - should work on any

Tested on x86_64 Linux only


# Usage
```bash
git clone https://github.com/flashnuke/mod-rootkit.git
cd mod-rootkit
make STRING_EXCLUDES="SOME_FILENAME1,SOME_FILENAME2" NET_EXCLUDES="127.0.0.1,2222" HIDE_MODULE=0
sudo make install
make clean
```

## Breakdown:

### Building the Module
To compile the module:
```bash
make STRING_EXCLUDES="SOME_FILENAME1,SOME_FILENAME2" NET_EXCLUDES="127.0.0.1,2222" HIDE_MODULE=0
```

| Parameter        | Required? | Description                                                                 |
|------------------|-----------|-----------------------------------------------------------------------------|
| `STRING_EXCLUDES`| No        | Comma-separated list of strings to hide from `getdents` (e.g., file/process names) |
| `NET_EXCLUDES`   | No        | Comma-separated list of IPs/ports to ignore or hide connections                   |
| `HIDE_MODULE`    | No        | Set to `1` to auto-hide the module after load (hides from `lsmod`, etc.)  |
| `MODULE_NAME`    | No        | Default is `mod_rootkit`, override to change the module name                |

Notes
* When hiding a process - make sure its cmdline contains a substr that is then passed via `STRING_EXCLUDES`, ie: running "`./HIDEME.sh`" and then passing `make ... STRING_EXCLUDES=HIDEME ...`
* For every str inside the `*_EXCLUDES` params, it's enough for a partial match in order for an entry to be hidden (doesn't have to be a full match, i.e: `STRING_EXCLUDES=ABC` would hide entry `...ABCDE...`)
* Setting `HIDE_MODULE=1` hides the module, use with caution as it's not trivial to remove it afterwards
* `NET_EXCLUDES` example - `NET_EXCLUDES=127.0.0.1,12345` would exclude all connections to/from `127.0.0.1` and all connections to/from port `12345`

### Loading / removing the module manually
To load:
```bash
sudo insmod mod_rootkit.ko
```

To remove:
```bash
sudo rmmod mod_rootkit
```

### Install the module (Auto-load on boot)
```bash
sudo make install
```
This will register the module, create a boot-time autoload config and load the module immediately with `modprobe`

### Uninstall the module
```bash
sudo make uninstall
```
This will attempt to unload the module from the running kernel and remove it from auto-load on boot

### Clean build artifacts
```bash
make clean
```
This will remove all temporary build files

## Disclaimer

This project is provided **strictly for educational and ethical research** purposes.  
Installing or deploying rootkits on unauthorized systems is **illegal and unethical**.

Use this project only in isolated lab environments, virtual machines, or test systems under your full control.


