#include <linux/slab.h>    /*for kmalloc()*/
#include <linux/init.h>    /* Needed for the macros */
#include <linux/kernel.h>  /* Needed for pr_info() */
#include <linux/proc_fs.h> /*proc_ops, proc)create, proc_remove, remove_proc_entry...*/
#include <asm/uaccess.h>

#define MYBUF_SIZE 256
#define MYDATA_SIZE 256
struct my_data{
    int size;
    char *buf; /* my data starts here */
};

/**
 * TODO: taskinfoya gore ayarlamaniz gerekiyor
 * @brief
 *
 * @param inode
 * @param file
 * @return int
 */
int my_open(struct inode *inode, struct file *file)
{
    struct my_data *my_data = kmalloc(sizeof(struct my_data) * MYBUF_SIZE, GFP_KERNEL);
    my_data->buf = kmalloc(sizeof(char) * MYBUF_SIZE, GFP_KERNEL);
    my_data->size = MYBUF_SIZE;

    my_data->size = snprintf(my_data->buf, MYBUF_SIZE,  "Hello World\n");

    /* validate access to data
    Not: diger fonksiyonlarda,
    private_datayi farkli adrese point ettirmeyin, 
    yada once my_datayi kfree ile free edin*/
    file->private_data = my_data;

    return 0;
}
/**
 * TODO: taskinfoya gore ayarlamaniz gerekiyor
 *
 * @param inode
 * @param file
 * @return int
 */
int my_release(struct inode *inode, struct file *file)
{
    /*free all memories*/
    struct my_data *my_data = file->private_data;
    kfree(my_data->buf);
    kfree(my_data);

    return 0;
}

/**
 * TODO: taskinfoya gore ayarlamaniz gerekiyor
 * @brief copy data from mydata->buffer to user_buf,
 * file: opened file data
 * usr_buf: usr buffer
 * offset: the cursor position on the mydata->buf from the last call
 * file:
 */
ssize_t my_read(struct file *file, char __user *usr_buf, size_t size, loff_t *offset)
{

    struct my_data *my_data = (struct my_data *)file->private_data;

    int len = min((int)(my_data->size - *offset), (int)size);

    if (len <= 0)
        return 0; /*end of file*/

    if (copy_to_user(usr_buf, my_data->buf + *offset, len))
        return -EFAULT;

    *offset = *offset + len;

    return len; /*the number of bytes copied*/
}

ssize_t my_read_simple(struct file *file, char __user *usr_buf, size_t size, loff_t *offset)
{

    char buf[MYBUF_SIZE] = {'\0'};

    int len = sprintf(buf, "Hello World\n");

    /* copy len byte to userspace usr_buf
     Returns number of bytes that could not be copied.
     On success, this will be zero.
    */
    if (copy_to_user(usr_buf, buf, len))
        return -EFAULT;

    return len; /*the number of bytes copied*/
}

/**
 * @brief TODO: task infoya gore ayarlamaniz gerekiyor
 *
 * @param file
 * @param usr_buf
 * @param size
 * @param offset
 * @return ssize_t
 */
ssize_t my_write(struct file *file, const char __user *usr_buf, size_t size, loff_t *offset)
{
    char *buf = kmalloc(size + 1, GFP_KERNEL);

    /* copies user space usr_buf to kernel buffer */
    if (copy_from_user(buf, usr_buf, size)){
        printk(KERN_INFO "Error copying from user\n");
        return -EFAULT;
    }
    // *offset += size;
    /* yine offseti bazi durumlarda set etmeniz vs gerekebilir,
    user tekrar write yaptiginda buf+*offsete yaziyor */

    buf[size] = '\0';

    printk(KERN_INFO "the value of kernel buf: %s", buf);

    kfree(buf);
    return size;
}
