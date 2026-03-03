/**
 * mytaskinfo.c
 * Kernel module mytaskinfo that communicates with /proc file system and creates mytaskinfo file
 * This provides the base logic for hw6 - displaying task information
 * partially based on: https://linux-kernel-labs.github.io/refs/heads/master/labs/device_drivers.html
 * @adaskin, 2022
 */
#include <linux/init.h>    /* Needed for the macros */
#include <linux/kernel.h>  /* Needed for pr_info() */
#include <linux/module.h>  /* Needed by all modules */
#include <linux/proc_fs.h> /*proc_ops, proc)create, proc_remove, remove_proc_entry...*/

ssize_t my_read(struct file *file, char __user *usr_buf, size_t size, loff_t *offset);
ssize_t my_read_simple(struct file *file, char __user *usr_buf, size_t size, loff_t *offset);
int my_open(struct inode *inode, struct file *file);
int my_release(struct inode *inode, struct file *file);
ssize_t my_write(struct file *file, const char __user *usr_buf, size_t size, loff_t *offset);

#define PROCF_NAME "mytaskinfo"

const struct proc_ops my_ops = {
    .proc_open = my_open,
    .proc_read = my_read,
    .proc_write = my_write,
    .proc_release = my_release,
};

/* This function is called when the module is loaded. */
static int __init my_module_init(void)
{
    /* creates the [/proc/procf] entry*/
    proc_create(PROCF_NAME, 0666, NULL, &my_ops);

    printk(KERN_INFO "/proc/%s created\n", PROCF_NAME);

    return 0;
}

/* This function is called when the module is removed. */
static void __exit my_module_exit(void)
{
    /* removes the [/proc/procf] entry*/
    remove_proc_entry(PROCF_NAME, NULL);

    printk(KERN_INFO "/proc/%s removed\n", PROCF_NAME);
}

/* Macros for registering module entry and exit points.
 */
module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("My Task Info Module");
MODULE_AUTHOR("kendi isminiz");
