# Homework 1: Adding a System Call to the Linux Kernel

**Due date:** TBA  

## Objectives

- Understand the steps to add a new system call to the Linux kernel.
- Learn how to copy data between kernel and user space (`copy_to_user`).
- Access kernel data structures (`task_struct`) to retrieve information about the current process.
- Write a user‑space program to invoke your new system call.

This homework builds on **Project 0**, where you compiled a custom kernel. Here you will modify the kernel source, add a system call, recompile (incrementally), and test your changes.

---

## Background: Process State in the Kernel

Every process (task) in Linux is represented by a `struct task_struct` (defined in `<linux/sched.h>`). Among its many fields is `__state`, which holds the current state of the process (e.g., running, sleeping, stopped). The kernel provides a convenient array of human‑readable strings for these states:

```c
static const char * const task_state_array[] = {
    "R (running)",        /* 0x00 */
    "S (sleeping)",       /* 0x01 */
    "D (disk sleep)",     /* 0x02 */
    "T (stopped)",        /* 0x04 */
    "t (tracing stop)",   /* 0x08 */
    "X (dead)",           /* 0x10 */
    "Z (zombie)",         /* 0x20 */
    "P (parked)",         /* 0x40 */
    "I (idle)",           /* 0x80 */
};
```

To get the correct index into this array for a given task, use the inline function `task_state_index(task)` (also in `<linux/sched.h>`). It returns an integer index that you can use with `task_state_array[]`.

For the current task, you can obtain the pointer via the `current` macro.

---

## The System Call: `get_my_state`

You will implement a single system call that copies the state string of the **current** process into a user‑supplied buffer.

**Prototype** (to be added in `include/linux/syscalls.h`):
```c
asmlinkage long sys_get_my_state(char __user *buf, size_t size);
```

**Behaviour**:
- Obtain the state index of the current process: `int idx = task_state_index(current);`
- Retrieve the corresponding string: `const char *state_str = task_state_array[idx];`
- Compute the length of the string (including the null terminator) using `strlen(state_str) + 1`.
- If `size` is smaller than this length, return `-EINVAL`.
- Use `copy_to_user(buf, state_str, len)` to copy the string to user space.
  - If `copy_to_user` fails (returns non‑zero), return `-EFAULT`.
- Otherwise, return `0` on success.

**Important**:
- The `__user` annotation tells the kernel that the pointer belongs to user space.
- Always check the size to avoid buffer overflows.
- Use the kernel’s `copy_to_user()` function – never dereference a user pointer directly.

---

## Step‑by‑Step Instructions

### 1. Prepare Your Kernel Source Tree

If you still have the kernel source from Project 0, you can reuse it. Otherwise, download a fresh copy (the same version you compiled before) and unpack it. We will work in that directory.

```bash
cd ~/linux-6.12   # replace with your version
```

### 2. Add the System Call Number

System calls are identified by numbers. We will use the **generic** syscall table (suitable for all architectures). Edit `include/uapi/asm-generic/unistd.h` and find the section where new syscalls are added (near the end). Add a line for your syscall:

```c
#define __NR_get_my_state 463   /* choose a number that is not already used */
__SYSCALL(__NR_get_my_state, sys_get_my_state)
```

Then update the total number of syscalls. Look for a line like:
```c
#define __NR_syscalls 463
```
Increase it by one (e.g., to 464).

*If you prefer to use the x86‑64 specific table, you may edit `arch/x86/entry/syscalls/syscall_64.tbl` instead. The steps are similar, but the generic method works for all architectures and is simpler for this exercise.*

### 3. Declare the System Call

Add the prototype in `include/linux/syscalls.h`. Find the section where other syscalls are declared (look for `asmlinkage long sys_...`) and add:

```c
asmlinkage long sys_get_my_state(char __user *buf, size_t size);
```

### 4. Implement the System Call

We will put the implementation in `kernel/sys.c` (a common place for miscellaneous syscalls). Open `kernel/sys.c` and add the following code near the end of the file (before the final `#endif`). Make sure to include the necessary headers if they are not already present. The file already includes many headers, but we need `<linux/sched.h>` for `task_state_index` and `current`. It is likely already included. Add the function:

```c
SYSCALL_DEFINE2(get_my_state, char __user *, buf, size_t, size)
{
    int idx;
    const char *state_str;
    size_t len;

    idx = task_state_index(current);
    state_str = task_state_array[idx];
    len = strlen(state_str) + 1;  /* include null terminator */

    if (size < len)
        return -EINVAL;

    if (copy_to_user(buf, state_str, len))
        return -EFAULT;

    return 0;
}
```

**Explanation**:
- `SYSCALL_DEFINE2` is a macro that creates the actual syscall function with two arguments.
- `task_state_index(current)` returns the index for the current process.
- `task_state_array` is defined in `fs/proc/array.c`, but it is also declared in a header (probably `linux/sched.h` includes it indirectly). If you get a compilation error about `task_state_array` being undeclared, you may need to add an `extern` declaration or include `linux/sched.h` after ensuring it is visible. In practice, `kernel/sys.c` already includes `linux/sched.h`, so it should be fine.

### 5. Compile the Kernel

Now you need to rebuild the kernel. Since you have only changed a few files, the build will be incremental and much faster than the first full compilation.

```bash
make -j$(nproc)
```

If you encounter errors about missing certificates (as in Project 0), disable them again:

```bash
scripts/config --disable SYSTEM_TRUSTED_KEYS
scripts/config --disable SYSTEM_REVOCATION_KEYS
make -j$(nproc)
```

### 6. Install the Kernel

After a successful build, install the modules and the kernel image:

```bash
sudo make modules_install
sudo make install
sudo update-grub   # if not done automatically
```

Then reboot and select your new kernel from the GRUB menu.

### 7. Verify the New Kernel

After booting, run:

```bash
uname -r
```

You should see the version with your local version (e.g., `6.12.0-mylinux`). Also check that your syscall is present (we will test it in the next step).

---

## Testing the System Call

You will write a C program that invokes your new system call using the `syscall()` function.

### 7.1. Write a Test Program

Create a file `test_get_my_state.c`:

```c
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifndef __NR_get_my_state
#define __NR_get_my_state 463   /* must match the number you assigned */
#endif

int main()
{
    char buffer[128];
    long rv;

    rv = syscall(__NR_get_my_state, buffer, sizeof(buffer));
    if (rv == 0) {
        printf("Current process state: %s\n", buffer);
    } else {
        perror("syscall get_my_state failed");
    }

    return 0;
}
```

### 7.2. Compile and Run

```bash
gcc -o test_get_my_state test_get_my_state.c
./test_get_my_state
```

You should see output like:
```
Current process state: R (running)
```

### 7.3. Test with `strace`

Run the program under `strace` to see the actual system call:

```bash
strace ./test_get_my_state
```

Look for a line like:
```
get_my_state(0x7ffc12345678, 128) = 0
```

### 7.4. Check Kernel Logs (Optional)

You haven’t added any `printk` in your syscall, but if you want to see that it was invoked, you could temporarily add a `printk` inside the syscall and then check `dmesg`.

---

## What to Submit

Submit a single PDF (or a set of images) containing:

1. **Screenshot of `uname -r`** showing your custom kernel version.
2. **Screenshot of the relevant part of `include/uapi/asm-generic/unistd.h`** showing the added line for `__NR_get_my_state`.
3. **Screenshot of the implementation in `kernel/sys.c`** (or wherever you placed it).
4. **Screenshot of the test program running** (output of `./test_get_my_state`).
5. **Screenshot of `strace ./test_get_my_state`** showing the syscall invocation.
6. **The `.config` file** you used (attach as a separate file or include its content).

Also, please include the source code of your test program as plain text (or in the PDF).

---

## Troubleshooting

### “task_state_array” undeclared

If the compiler complains that `task_state_array` is not declared, you may need to add an extern declaration at the top of your function. The array is defined in `fs/proc/array.c` and declared in a header. One simple fix is to add:

```c
extern const char * const task_state_array[];
```

before using it. Alternatively, include `<linux/sched.h>` which should bring it in. Check that your kernel version has it properly exposed.

### copy_to_user fails

Ensure that the user buffer pointer is valid and that you have not accidentally passed a kernel pointer. The `__user` annotation helps, but you must also check the size.

### System call number collision

If you get an error about duplicate syscall numbers, pick a different number. You can find the current maximum by looking at the value of `__NR_syscalls` in `unistd.h`.

### Kernel compilation errors after adding syscall

Double-check that you added the prototype in `syscalls.h` correctly and that the implementation uses the `SYSCALL_DEFINE2` macro. Also ensure that you have not placed the code inside an `#ifdef` block that might be disabled.

---

## Additional Challenge (Optional)

If you finish early, try extending your syscall to:

- Also return the PID of the current process (you could return it via a second pointer argument, or combine it into the string).
- Accept a PID argument and return the state of that process (requires finding the task by PID using `find_task_by_vpid()`).

But these are not required for the homework.

---

## Resources

- [Linux kernel documentation on adding syscalls](https://www.kernel.org/doc/html/v4.10/process/adding-syscalls.html)
- [Linux cross reference (Bootlin)](https://elixir.bootlin.com/linux/latest/source) – helpful for browsing kernel source.
- [man syscall](https://man7.org/linux/man-pages/man2/syscall.2.html)

Good luck!