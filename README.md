# mod-rootkit


A simple proof-of-concept Linux Kernel Rootkit module designed to hide processes, files, and itself from userland visibility for modern kernel versions.  



## Overview

`mod-rootkit` is a Loadable Kernel Module (LKM) that demonstrates basic rootkit techniques in Linux. Once inserted into the kernel, it provides stealth capabilities by intercepting and modifying system behavior to:

- **Process Hiding** – Hide any process whose name matches configured keywords
- **File & Directory Hiding** – Hide files and folders based on configured keywords
- **Network connection hiding** – Hide files and folders based on configured IP addresses or ports
- **Module Hiding** – Hides itself from kernel module listings



##  Getting Started

Requires root access and a kernel with loadable module support.

### Requirements

- Linux system with kernel headers installed
- GCC and make
- Root permissions
- Kernel version - should work on any (see 

Tested on x86_64 Linux only


### Build and Load

```bash
make
sudo insmod mod_rootkit.ko
```

To remove:

```bash
sudo rmmod mod_rootkit
```


## Configuration

Configuration is currently done by modifying `mod_rootkit.c` before compilation:

- `HIDE_PREFIX` – any process or file containing this string will be hidden
- You can change these defaults to match your testing needs


## How It Works

- Hooks the `getdents` and `getdents64` syscalls to filter directory entries
- Filters entries with names matching the hidden prefix
- Erases its own entry from `/proc/modules` and kernel symbol table
- Temporarily disables write-protection to modify kernel memory


## Credits

Check out:

- [Linux Kernel Module Programming Guide](https://tldp.org/LDP/lkmpg/2.6/html/)
- [Practical Rootkits by Greg Hoglund](https://www.amazon.com/Rootkits-Subverting-Windows-Greg-Hoglund/dp/0321294319)


## Disclaimer

This project is provided **strictly for educational and ethical research** purposes.  
Installing or deploying rootkits on unauthorized systems is **illegal and unethical**.

Use this project only in isolated lab environments, virtual machines, or test systems under your full control.



---



* bett tha ldpreload
 * works for new kernel versions
 * credit to hook
* flag to hide md
* exluded params explanation
