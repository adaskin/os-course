
# Project 0: Linux Setup, Terminal Basics, and Your First Kernel Compilation
*written with the help of DeepSeek based on previous semester's hw0*

**Due date:** TBA  

## Objectives

By completing this project you will:

- Install a Linux virtual machine on your computer.
- Learn essential terminal commands to navigate and manipulate files.
- Understand what `make` is and how a simple Makefile works.
- Download the Linux kernel source code, configure it, and compile your own custom kernel.
- Boot into your newly compiled kernel and verify the changes.

This project lays the foundation for the rest of the course. Even if you have used Linux before, working through the steps carefully will ensure you are ready for the kernel‑level assignments ahead.

---

## Part 1: Setting Up Linux in a Virtual Machine

We will use **VirtualBox** (free and open‑source) to create a virtual machine. If you have an Apple Silicon (M1/M2/M3) Mac, VirtualBox does not yet fully support it; use **UTM** (based on QEMU) instead.

### 1.1 Install VirtualBox or UTM

- **Windows/Linux/Intel Mac**: Download and install [VirtualBox](https://www.virtualbox.org/wiki/Downloads).
- **Apple Silicon Mac**: Download [UTM](https://mac.getutm.app/) from its website or the Mac App Store.

### 1.2 Download a Linux ISO

Choose a distribution you are comfortable with. **Ubuntu** or **Debian** are good choices because they are well‑documented and have large communities.

- **Ubuntu**: [https://ubuntu.com/download/desktop](https://ubuntu.com/download/desktop)  
- **Debian**: [https://www.debian.org/distrib/](https://www.debian.org/distrib/)

If you are on Apple Silicon, be sure to download the **ARM64** version of the ISO.

### 1.3 Create the Virtual Machine

1. Open VirtualBox/UTM and click **New**.
2. Give your VM a name (e.g., `OS_Lab`).
3. Allocate at least **2 CPU cores** and **4 GB of RAM** (more if your host machine can spare it).
4. Create a virtual hard disk of at least **25 GB** (dynamic allocation is fine).
5. In the VM settings, under **Storage**, attach the downloaded ISO file as a virtual optical disk.
6. Start the VM and follow the on‑screen installation instructions. Choose the graphical installation if you are new to Linux.
7. During installation, create a username and password – you will need them later.

After installation, reboot the VM. You should see a login screen.

### 1.4 Update the System

Open a **terminal** (look for “Terminal” in the applications menu). First, update the package list and upgrade existing packages:

```bash
sudo apt update
sudo apt upgrade -y
```

Then install essential development tools that we will need for compiling the kernel:

```bash
sudo apt install build-essential libncurses-dev bison flex libssl-dev libelf-dev
```

- `build-essential` includes `gcc`, `make`, and other compilation tools.
- `libncurses-dev` is needed for `menuconfig`.
- `bison` and `flex` are parser generators used by the kernel build system.
- `libssl-dev` provides cryptographic libraries needed for kernel features.
- `libelf-dev` handles ELF object files.

Now your Linux VM is ready.

---

## Part 2: Terminal Basics

If you are new to the Linux command line, this section will introduce the most common commands. Practice each command in your VM’s terminal.

### 2.1 Navigation

- `pwd` – **p**rint **w**orking **d**irectory (shows where you are).
- `ls` – list files and directories.
  - `ls -l` – long listing (permissions, size, modification time).
  - `ls -a` – show all files, including hidden ones (those starting with a dot).
- `cd` – **c**hange **d**irectory.
  - `cd /` – go to the root directory.
  - `cd ~` – go to your home directory.
  - `cd ..` – go up one level.

### 2.2 File Operations

- `cp source dest` – copy a file.
  - `cp -r source_dir dest_dir` – copy a directory recursively.
- `mv source dest` – move or rename a file/directory.
- `rm file` – remove a file.
  - `rm -r dir` – remove a directory and its contents (careful!).
- `mkdir dirname` – create a new directory.
- `touch filename` – create an empty file or update its timestamp.
- `cat file` – display the contents of a file.
- `less file` – view a file page by page (press `q` to quit).
- `head -n 5 file` – show the first 5 lines.
- `tail -n 5 file` – show the last 5 lines.

### 2.3 Permissions

- `chmod` – change file permissions.
  - Example: `chmod +x script.sh` makes a file executable.
- `chown` – change file ownership (requires `sudo`).
  - Example: `sudo chown user:group file`.

### 2.4 Getting Help

- `man command` – display the manual for a command (e.g., `man ls`).
- `command --help` – many commands show a brief help message.

### 2.5 Redirection and Pipes

- `>` – redirect output to a file (overwrites).
  - `ls > files.txt` saves the output of `ls` into `files.txt`.
- `>>` – append output to a file.
- `|` – pipe the output of one command to another.
  - `ls -l | less` – view long listing page by page.

### 2.6 Process Management

- `ps` – list running processes.
- `kill PID` – terminate a process with the given process ID.
- `Ctrl+C` – interrupt a running command in the terminal.

### 2.7 Text Editors

You will need to edit text files. Two simple terminal editors are **nano** (easy) and **vim** (powerful but steeper learning curve).

- **nano**: `nano filename` – edit, then `Ctrl+X` to exit, `Y` to save, `Enter` to confirm.
- **vim**: `vim filename` – press `i` to insert, `Esc` to go back to command mode, `:wq` to save and quit.

Choose whichever you prefer. For this project, we will mostly just copy/paste lines, so a simple editor is enough.

---

## Part 3: Introduction to Make and Makefiles

The Linux kernel is a huge project, and it is compiled using **make**. Make is a build automation tool that reads a **Makefile** to determine how to compile and link programs.

### 3.1 What is a Makefile?

A Makefile contains rules of the form:

```
target: dependencies
	commands
```

- **target**: the file to be generated (e.g., an executable).
- **dependencies**: files that must exist before the target can be built.
- **commands**: shell commands that actually build the target (must be preceded by a TAB character).

### 3.2 A Simple Example

Create a file called `hello.c`:

```c
#include <stdio.h>

int main() {
    printf("Hello, world!\n");
    return 0;
}
```

Now create a file named `Makefile` (no extension) in the same directory:

```make
hello: hello.c
	gcc -o hello hello.c
```

Open a terminal in that directory and run:

```bash
make
```

Make will see that `hello` does not exist (or `hello.c` is newer than `hello`), and run the command `gcc -o hello hello.c`. Now you have an executable `hello` that you can run with `./hello`.

You can also define variables and more complex rules. For example:

```make
CC = gcc
CFLAGS = -Wall -g

hello: hello.c
	$(CC) $(CFLAGS) -o hello hello.c

clean:
	rm -f hello
```

Now `make clean` will delete the executable.

### 3.3 How Make Is Used for the Kernel

The kernel source tree contains a top‑level Makefile with thousands of lines. When you type `make` in the kernel directory, it:

- Reads the configuration (from `.config`).
- Decides which files to compile based on that configuration.
- Compiles the kernel image (`vmlinuz`) and loadable modules (`.ko` files).
- Provides targets like `menuconfig`, `modules_install`, and `install`.

Understanding `make` helps you appreciate how the kernel build process is organized.

---

## Part 4: Downloading and Configuring the Linux Kernel

Now we will download the Linux kernel source, configure it, and prepare to compile.

### 4.1 Get the Kernel Source

You can download the official kernel from [kernel.org](https://www.kernel.org/). Choose a stable version, e.g., 6.12.x (or the latest longterm). In your VM, open a terminal and run:

```bash
cd ~
wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.12.tar.xz
tar -xf linux-6.12.tar.xz
cd linux-6.12
```

Replace `6.12` with the actual version you downloaded (your version `$ uname -r`).

### 4.2 Create an Initial Configuration

The kernel build system needs a `.config` file that specifies which features and drivers to include. We will start from the configuration of your currently running kernel. This ensures that all necessary drivers for your VM are enabled.

First, find your current kernel’s configuration. It is usually available at `/proc/config.gz` (if enabled) or in `/boot`. In Ubuntu/Debian, you can do:

```bash
zcat /proc/config.gz > .config
```

If that file does not exist, try:

```bash
cp /boot/config-$(uname -r) .config
```

Now we have a `.config` file that matches your running kernel.

### 4.3 Modify the Local Version

We want to give our custom kernel a distinct name so we can easily identify it later. Edit `.config` and look for the line:

```
CONFIG_LOCALVERSION=""
```

Change it to something like:

```
CONFIG_LOCALVERSION="-yourname"
```

You can use your name, student ID, or any string without spaces. This will appear in `uname -r` output.

To edit the file, you can use `nano .config` and find the line (press `Ctrl+W` in nano to search).

### 4.4 (Optional) Use `menuconfig` for Fine‑Tuning

If you want to explore and change kernel options interactively, run:

```bash
make menuconfig
```

This opens a text‑based menu. You do not need to change anything for now – just exit and save. It will update `.config` if necessary.

---

## Part 5: Compiling and Installing the Kernel

Compiling the kernel takes time (15‑60 minutes depending on your VM’s resources). We will compile in parallel using the `-j` flag.

### 5.1 Compile the Kernel and Modules

In the kernel source directory (`~/linux-6.12`), run:

```bash
make -j$(nproc)
```

- `$(nproc)` returns the number of CPU cores. Using `-j$(nproc)` tells `make` to run that many parallel jobs, speeding up compilation.

If you encounter errors about missing certificates, you can disable them (see Troubleshooting section below).

### 5.2 Install the Modules

After compilation finishes, install the kernel modules:

```bash
sudo make modules_install
```

This copies the compiled modules to `/lib/modules/6.12.0-mylinux/` (the version number will match your kernel with the local version appended).

### 5.3 Install the Kernel Image

Next, install the kernel image itself:

```bash
sudo make install
```

This copies the kernel image (`vmlinuz`) to `/boot`, generates an `initramfs` (initial RAM disk), and updates the bootloader configuration.

If `make install` does not automatically update the bootloader, you may need to run:

```bash
sudo update-grub
```

### 5.4 Reboot and Select Your Kernel

Now reboot the VM:

```bash
sudo reboot
```

When the GRUB menu appears (you may need to hold `Shift` during boot to force it), select **Advanced options for Ubuntu** (or similar) and choose your kernel, which should have the suffix `-mylinux`. If you don’t see the menu, you can edit `/etc/default/grub` to increase the timeout.

After logging in, verify that you are running your custom kernel:

```bash
uname -r
```

You should see something like `6.12.0-mylinux`.

Check the kernel log for your “Linux version” line:

```bash
dmesg | grep "Linux version"
```

---

## 6. Troubleshooting Common Errors

### 6.1 “No rule to make target ‘debian/certs/…’ ” (Certificate errors)

If you see errors about missing certificates during compilation, you can disable the trusted keys. Run:

```bash
scripts/config --disable SYSTEM_TRUSTED_KEYS
scripts/config --disable SYSTEM_REVOCATION_KEYS
```

Then start the compilation again (`make -j$(nproc)`). The changes will be saved in `.config`.

### 6.2 Missing Libraries or Headers

If `make` complains about missing headers or libraries, install them with `apt`. For example:

```bash
sudo apt install libelf-dev libssl-dev
```

Then try again.

### 6.3 Kernel Panic / VFS Error at Boot

If the new kernel fails to boot with a kernel panic (e.g., “VFS: Unable to mount root fs”), you may need to rebuild the initramfs manually:

```bash
sudo update-initramfs -c -k 6.12.0-mylinux   # use your exact kernel version
sudo update-grub
```

Then reboot.

### 6.4 Compilation Takes Too Long

If your VM is slow, consider giving it more CPU cores and RAM. Also, you can use `make -j2` instead of `-j$(nproc)` to avoid overwhelming the system.

---

## 7. Submission Requirements

To receive credit for this project, submit the following items as **one PDF or a set of images** (you can take screenshots in the VM):

1. **Screenshot of `uname -r`** showing your custom kernel version (e.g., `6.12.0-mylinux`).
2. **Screenshot of `dmesg | grep "Linux version"`** showing the kernel boot message with your custom version.
3. **Screenshot of `ls /boot`** showing the installed kernel files (vmlinuz, initrd.img, System.map).
4. **Screenshot of `/etc/fstab`** (just to show you can navigate and view files).
5. **The `.config` file** you used (upload as a separate file or include its content in the PDF).

Submit via the course submission system (e.g., Google Classroom). All group members should submit the same work, but each must perform the steps on their own VM.

---

## Additional Resources

- [Linux Kernel Newbies](https://kernelnewbies.org/) – community for people learning kernel development.
- [Make Tutorial](https://makefiletutorial.com/) – excellent guide to `make`.
- [Linux Command Line Basics](https://ubuntu.com/tutorials/command-line-for-beginners) – from Ubuntu.

