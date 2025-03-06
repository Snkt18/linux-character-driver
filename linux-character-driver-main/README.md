# Linux Kernel Module Development for Character Devices (C, Make, Linux Kernel)

Developed a Linux Kernel Module for character devices, enabling communication between user space and kernel.
Implemented device registration and management, utilizing kernel structures to handle I/O operations.

## Commands to Build and Load the Module

Run the following command to build and load the module:
```bash
make && sudo insmod mychardev.ko
```

## Testing the Module

To test the module, run:
```bash
head -c29 /dev/mychardev-1
```

It should show output: `Hello from the kernel world!`
