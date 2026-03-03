HW —Linux Kernel Module Introduction
===========================

## Kaynaklar
https://sysprog21.github.io/lkmpg/

https://linux-kernel-labs.github.io/refs/heads/master/labs/kernel_modules.htm

## Giriş

Dersten hatırlarsanız linux kernel'ın modullerden oluştuğunu ve yeni moduller ekleyip kaldırabildiğimizden bahsetmiştik. Böylelikle çalışan bir kerneli yeniden compile etmeden içerisine eklemeler yapabiliyoruz.

Linuxte çalışma zamanındaki system info edinmek için, system call alternatifi ([System call Alternatifleri](https://www.kernel.org/doc/html/latest/process/adding-syscalls.html?highlight=system+call#:~:text=patches.rst.-,System%20Call%20Alternatives,%C2%B6,-The%20first%20thing) ) olarak `sysfs` yada `/proc `filesystem kullanılabilir.

Bunu 2.kısma bırakırken bu kısımda sadece kernela modül yükleme adımlarını ele alacağız.

### Linux kernel modul yükleme ve kaldırma

#### Kernel header dosyalarını yükleme
Linuxte kerneldaki headerları kendi C programınızda include edebilmek ve C programlarınızı kernel dosyalarıyla derleyebilmek için öncelikle makinenizde linuxun header dosyalarının yüklü olduğundan emin olun.
```bash
apt-cache search linux-headers-`uname -r`
```
Return değeri olarak `linux-headers-5.15.0-52-generic - Linux kernel headers for version 5.15.0 on 64 bit x86 SMP` benzeri bir çıktı almanız gerekiyor.

Eğer almazsanız header dosyalarını yüklemek için

```bash
sudo apt install linux-headers-`uname -r`
```


#### Kernel modul ekleme

Öncelikle kernel modüllerini manage edebilmek için `kmod` paketini yükleyin:
```bash
sudo apt install kmod
```
`kmod`u farklı opsiyonlarla çalıştırabilirsiniz (mesela `kmod list`). Bu paketi yükleyince kullanabileceğiniz komutlar şöyle:

```bash
lsmod    
rmmod      
insmod    
modinfo    
modprobe    
depmod       
```
Bunlarla yeniden başlatma vs yapmadan kernela modül yükleyip kaldırabiliyoruz.

Bu işlemin nasıl yapıldığını göstermek için, öncelikle basit bir modül oluşturup bunu kernela ekleyeceğiz.


### Linux kernel modül oluşturma
Her bir modülde başlama ve çıkış anlarında nelerin (hangi fonksiyonların yada statementların) çalışacağını belirtmemiz gerekiyor. Herhangi bir module implement ederken [module.h](https://elixir.bootlin.com/linux/v6.1-rc2/source/include/linux/module.h) de belirtilen aşağıdaki fonksiyonları implement etmemiz gerekiyor:
```C
int init_module(void);
void cleanup_module(void);
```
Bunlardan `init_module` insmod yapıldığında `cleanup`da modül kaldırıldığında çalışmaktadır.



#### Simple kernel module
```C
/**
 * hello-1.c - The simplest kernel module.
 * taken from https://sysprog21.github.io/lkmpg/
 *
 */

#include <linux/kernel.h> /* Needed for pr_info() */
#include <linux/module.h> /* Needed by all modules */

int init_module(void)
{

    pr_info("Hello world 1.\n"); /*= printk(KERN_INFO "Hello world 1.\n")*/
    
    /* A non 0 return means init_module failed; module can't be loaded. */
    return 0;
}

void cleanup_module(void)
{
    pr_info("Goodbye world 1.\n");
}
MODULE_LICENSE("GPL");
```

Eski kernellarda yukarıdaki yöntem geçerliyken 2.5 den sonra init ve cleanup fonksiyonlarına istediğiniz ismi verererek modul_init ve module_exite bunları argüman olarak verebilirsiniz (**tercih edilen yöntem**):

```C
/**
 * hello-2.c - Demonstrating the module_init() and module_exit() macros.
 * This is preferred over using init_module() and cleanup_module().
 * taken from https://sysprog21.github.io/lkmpg/
 */

#include <linux/init.h> /* Needed for the macros */
#include <linux/kernel.h> /* Needed for pr_info() */
#include <linux/module.h> /* Needed by all modules */

static int __init hello_2_init(void)
{
    pr_info("Hello, world 2\n");
    return 0;
}

static void __exit hello_2_exit(void)
{
    pr_info("Goodbye, world 2\n");
}

module_init(hello_2_init);
module_exit(hello_2_exit);
MODULE_DESCRIPTION("My kernel module");
MODULE_AUTHOR("Me");
MODULE_LICENSE("GPL");
```

#### Compile Etme
Bir tane Makefile oluşturun:
```bash
#Makefile
obj-m += hello-2.o

PWD := $(CURDIR)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
  ```

Yukarıda `make`lerden önce tablar kullanılması gerekiyor (space varsa silerek tab yapın).
Terminalden hello-2.c ve Makefile'ın olduğu directoryde `make` yazarak modülü compile edin. Sonuçta hello-1.**ko** oluşması gerekiyor.

Eğer birden fazla `.c` file varsa; mesela `helloinit.c` ve `helloexit.c` o zaman `hello-objs = helloinit.o helloexit.o` eklememiz gerekiyor:

  ```bash
#Makefile
obj-m += hello.o
hello-objs := helloinit.o helloexit.o

PWD := $(CURDIR)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
  ```

### Modülü yükleme
Hata almadıysanız eğer, makefileda eger `obj-m += hello.o` kullandiysaniz bu bize `hello.ko` seklinde bir kernel modulu olusturuyor. Yuklemek icin
```bash
sudo insmod hello.ko
```
Yine yüklü modülü listeleyerek görmek için:

```bash
sudo lsmod | grep hello
```
Modülün başlarken print ettiği kernel info mesajı görmek için:
```bash
sudo dmesg
```
Yine modülü kaldırmak için:

```bash
sudo rmmod hello
```

## Yapilacaklar
- init ve exit fonksiyonlarini iki ayri c dosyasina alarak **kendi isminiz(veya grup isminizde)** bir kernel modulu olusturunuz.

## Teslim
1. `isminizinit.c`, `isminizexit.c`, `isminiz.ko` ve `Makefile`
2. Aşağıdaki çalıştırmanın terminal görüntüsünü
```bash
sudo insmod isminiz.ko
sudo rmmod isminiz.ko
```
3. Aşağıdaki outputu gönderiniz.
```bash
sudo dmesg | tail -10 > out.txt
```

Tum grup uyelerinin teslim etmesi gerekiyor. Grup uylerinizi `grup.txt` dosyasinda belirtiniz.
