# BIL 301: Homework — System Call to Change Task State
## Kaynaklar
https://www.kernel.org/doc/html/v4.10/process/adding-syscalls.html

https://linux-test-project.readthedocs.io/en/latest/developers/test_case_tutorial.html#find-an-untested-system-call


## Bilinmesi Gerekenler
- Kernel compiling
- Kernel'a system call ekleme
- C programdan system call çağırma

## Öğrenme hedefleri
System callların kullanılarak kernel spaceden user space'e veya tam tersi data kopyalanması. Kernel data structurelarinin kavranmasi. Kerneldaki bilgilerin degistirilerek kernel protection ve security mekanizmasinin bozulabileceginin gosterilmesi.

## Giriş
Linux kernelde, process listesi kernelda [`<linux/sched.h>`](https://elixir.bootlin.com/linux/latest/source/include/linux/sched.h) headerda tanimli [`task_struct`](https://elixir.bootlin.com/linux/v6.11.1/source/include/linux/sched.h#L756)  isminde data structure ile yapilmaktadir. Bu data structure içerisinde bir çok bilgiyi barındırmasına rağmen `__state`i kullanacagiz:

```C
struct task_struct{
    ...
    unsigned int  __state;
    pid_t pid;
    ...
}
```

Bu data structure uzerinde ileri gitmek icin kullanabileceginiz loop yapisi macro ile tanimlanmistir: `for_each_process()` macro kullanarak sistemdeki mevcut taskler üzerinde iterasyon oluşturabilirsiniz:

```C
struct task_struct *task;
for_each_process(task) {
/* on each iteration task points to the next task:
task->anymember */
}
```

Burada herhangi bir `task` icin [`task->__state`](https://elixir.bootlin.com/linux/latest/source/include/linux/sched.h#L96) aşağıdaki şekilde tanımlanmıştır:
```C
/* Used in tsk->__state: */
#define TASK_RUNNING			0x00000000
#define TASK_INTERRUPTIBLE		0x00000001
#define TASK_UNINTERRUPTIBLE		0x00000002
#define __TASK_STOPPED			0x00000004
#define __TASK_TRACED			0x00000008
/* Used in tsk->exit_state: */
#define EXIT_DEAD			0x00000010
#define EXIT_ZOMBIE			0x00000020
#define EXIT_TRACE			(EXIT_ZOMBIE | EXIT_DEAD)
/* Used in tsk->__state again: */
#define TASK_PARKED			0x00000040
#define TASK_DEAD			0x00000080
#define TASK_WAKEKILL			0x00000100
#define TASK_WAKING			0x00000200
#define TASK_NOLOAD			0x00000400
#define TASK_NEW			0x00000800
#define TASK_RTLOCK_WAIT		0x00001000
#define TASK_FREEZABLE			0x00002000
#define __TASK_FREEZABLE_UNSAFE	       (0x00004000 * IS_ENABLED(CONFIG_LOCKDEP))
#define TASK_FROZEN			0x00008000
#define TASK_STATE_MAX			0x00010000
#define TASK_ANY			(TASK_STATE_MAX-1)
/*
 * DO NOT ADD ANY NEW USERS !
 */
#define TASK_FREEZABLE_UNSAFE		(TASK_FREEZABLE | __TASK_FREEZABLE_UNSAFE)
/* Convenience macros for the sake of set_current_state: */
#define TASK_KILLABLE			(TASK_WAKEKILL | TASK_UNINTERRUPTIBLE)
#define TASK_STOPPED			(TASK_WAKEKILL | __TASK_STOPPED)
#define TASK_TRACED			__TASK_TRACED
#define TASK_IDLE			(TASK_UNINTERRUPTIBLE | TASK_NOLOAD)
/* Convenience macros for the sake of wake_up(): */
#define TASK_NORMAL			(TASK_INTERRUPTIBLE | TASK_UNINTERRUPTIBLE)
/* get_task_state(): */
#define TASK_REPORT			(TASK_RUNNING | TASK_INTERRUPTIBLE | \
					 TASK_UNINTERRUPTIBLE | __TASK_STOPPED | \
					 __TASK_TRACED | EXIT_DEAD | EXIT_ZOMBIE | \
					 TASK_PARKED)
#define task_is_running(task)		(READ_ONCE((task)->__state) == TASK_RUNNING)
#define task_is_traced(task)		((READ_ONCE(task->jobctl) & JOBCTL_TRACED) != 0)
#define task_is_stopped(task)		((READ_ONCE(task->jobctl) & JOBCTL_STOPPED) != 0)
#define task_is_stopped_or_traced(task)	((READ_ONCE(task->jobctl) & (JOBCTL_STOPPED | JOBCTL_TRACED)) != 0)
```

Buradaki bilgileri okumak için asagidaki sekilde data structure tanimlamasi kullanabilirsiniz (https://elixir.bootlin.com/linux/latest/source/fs/proc/array.c#L126 ):

```C
static const char * const task_state_array[] = {
    /* states in TASK_REPORT: */
    "R (running)",        /* 0x00 */
    "S (sleeping)",        /* 0x01 */
    "D (disk sleep)",    /* 0x02 */
    "T (stopped)",        /* 0x04 */
    "t (tracing stop)",    /* 0x08 */
    "X (dead)",        /* 0x10 */
    "Z (zombie)",        /* 0x20 */
    "P (parked)",        /* 0x40 */
    /* states beyond TASK_REPORT: */
    "I (idle)",        /* 0x80 */
};
```

Sonra `sched.h` te tanımlı [`task_state_index()`](https://elixir.bootlin.com/linux/latest/source/include/linux/sched.h#L1612) fonksiyonunu kullanarak arraydeki string karşılığını bulabilirsiniz:

```C
  task_state_array[task_state_index(task)];
```

Yine current processin `state`ini [`get_current_state()`](https://elixir.bootlin.com/linux/latest/source/include/linux/sched.h#L296) macroyu kullanarak okuyabilirsiniz:

```C
#define get_current_state()	READ_ONCE(current->__state)
```

## Sisteme state bilgisi icin eklenecek system callar

Yukaridaki bilgileri user programa sunabilmek icin asagidaki system callarini tanimlayiniz.

1.
  ```C
  /**
  pid ile verilen processin statetine,
  state ile verilen hexadecimal value'yu atamaktadir.  
  Eger pid 0 ise current processin statetini
  set_current_state(state); ile degistirmekte
  ve schedule(); cagirmaktadir.
  Eng: It assigns the hex value state of the process given by pid.
  If pid is 0,
  then it changes the current process state  with
    set_current_state(state);  
  and calls schedule(); */
  int set_proc_state(pid_t pid, int state);
  ```

  - Not: [`set_current_state(state_value)`](https://elixir.bootlin.com/linux/v6.11.1/source/include/linux/sched.h#L187) makrosu mevcut processin durumunu degistirmektedir.

2.
```C
/**pid processin state bilgisini return etmektedir.
pid 0 ise cagiran processin state bilgisini return etmektedir. */
int get_proc_state(pid_t pid);
```

3.
  ```C
  /**pid processin state bilgisini user space'e kopyalamaktadir.
  pid 0 ise cagiran processin durumunu userspace'e kopyalamaktadir.
  Burada userspacete buf addresine copy_to_user() kullanarak
  uygun veriyi kopyalamaniz gerekiyor. */
  int get_proc_state_string(pid_t pid, void *buf, size_t size);
  ```

  Bu system call sadece asagidaki stringlerden birisini buf kismina yazmaktadir:
  Yani buf kismina string yazilmakta ve bu string asagidaki bilgilerden birisi olmaktadir:
  ```C
  static const char * const task_state_array[] = {
      /* states in TASK_REPORT: */
      "R (running)",        /* 0x00 */
      "S (sleeping)",        /* 0x01 */
      "D (disk sleep)",    /* 0x02 */
      "T (stopped)",        /* 0x04 */
      "t (tracing stop)",    /* 0x08 */
      "X (dead)",        /* 0x10 */
      "Z (zombie)",        /* 0x20 */
      "P (parked)",        /* 0x40 */
      /* states beyond TASK_REPORT: */
      "I (idle)",        /* 0x80 */
  };
  ```

  `sched.h` da tanımlı `task_state_index()` fonksiyonunu kullanarak arraydeki string karşılığını bulunuz:
  task_state_array[task_state_index(task)];
  Sonra bu stringi [`copy_to_user(void *touser, void *fromkernel, unsigned long nbytes)`](https://elixir.bootlin.com/linux/latest/source/include/linux/uaccess.h#L201) kullanarak yapiniz:

  - `touser`
    - Destination address, in user space.
  - `fromkernel`
    - Source address, in kernel space.
  - `nbytes`
    - Number of bytes to copy.

## System callari kernela eklemek
Tanimlayacagimiz system call ismi `name` olsun:

Kernela system call eklerken [`../include/linux/syscalls.h`](https://elixir.bootlin.com/linux/latest/source/include/linux/syscalls.h) dosyasi mevcut syscallarin listesini tutmaktadir. Buradaki prototiplere baktiginizda su sekilde isimler goreceksiniz:

```C
asmlinkage long sys_name(...);
```
Prototipten ayri definition (tanimlamasi) bir `.c` dosyasina konmasi ve bunun kernel compile edilirken makefileda icerilmesi gerekmektedir. Yine definitionda `sys_name(...){...}` seklinde acik bir tanimlama yerine kernelda tanimli `SYSCALL_DEFINEn(name, ...){}` macrosu kullanilmalidir.

```C
SYSCALL_DEFINEn(name, arg1 type, arg1 name, arg2 type, arg2 name ...)
{
  ...
}
```

Yukarida `SYSCALL_DEFINEn()` deki `n`  arguman sayisi olmakta: Mesela `close(0)` seklinde cagrilan bir system call'u icin `n` sayisi `1`dir. Tanimlamasida su sekilde yazilabilir `SYSCALL_DEFINE1(close, int, fd)`.   

`#if`lerin vbnin icerisinde bir blokta olmamasina dikkat ederek system call tanimlamanizi [`kernel/sys.c`](https://elixir.bootlin.com/linux/latest/source/kernel/sys.c) icerisinde dosyanin en sonuna ekleyiniz.

Tanimlamalari dogru yaptiginizdan emin olmak icin asagidaki verilen `msg`i yazdiran system callunu ekleyiniz.

```C
SYSCALL_DEFINE1(kendi_isminiz, char *, msg)
{
  printk("Syscall entry............!\n");
  char buf[256];
  long copied = strncpy_from_user(buf, msg, sizeof(buf));

  /*not: copy_from_user ile strncpy_from_user benzerler:
  userspace den kernel space’e data kopyalaniyor…
  Tam tersini copy_to_user ile yapabiliriz
  */
  
  if (copied < 0 || copied == sizeof(buf))
    return -EFAULT;
  printk(KERN_INFO "kendiisminiz syscall called with \"%.256s\"\n", buf);
  return 0;
}

```

### System call'a numara vermek ve user processlerin erisimine acmak
Yukaridaki tanimlamalardan sonra, her systemcallu architecture'a spesifik veya generik veya compatible olarak numara almaktadir. (Numaralarla system callar cagrilmakta, dersteki assembly kod orneginde gormustuk)

#### Generic numara vermek
Once [`include/uapi/asm-generic/unistd.h`](https://elixir.bootlin.com/linux/latest/source/include/uapi/asm-generic/unistd.h) dosyasini aciniz. Ve asagidakine benzer olarak tum system callariniz icin numara ekleyiniz:
```C
#define __NR_hello 463
__SYSCALL(__NR_hello, sys_hello)
```
Sonra [`__NR_syscalls`](https://elixir.bootlin.com/linux/latest/source/include/uapi/asm-generic/unistd.h#L845) degerinide guncelleyin.

#### Architecture spesifik numara vermek
Generic olunca tum architecturelar icin ayni system calllari kullanilmis oluyor. Farkli mimariler icin ozel callar da tanimlayabilirsiniz.
- x86-64 icin
[`arch/x86/entry/syscalls/syscall_64.tbl`](https://elixir.bootlin.com/linux/latest/source/arch/x86/entry/syscalls/syscall_64.tbl)
- x86 icin
[`arch/x86/entry/syscalls/syscall_32.tbl`](https://elixir.bootlin.com/linux/latest/source/arch/x86/entry/syscalls/syscall_32.tbl)


### Farkli olarak `sys.c` yerine ayri bir `.c` dosyasi kullanmak

  -  Not isterseniz `asmlinkage long sys_name(..)` iceren ayri bir `.c` dosyasida kullanabilirsiniz. O durumda once `obj-y := mysyscall.o` iceren bir tane `Makefile` olusturmaniz, sonra linux directoryde bulunan ana `Makefile` icerisinde `core-y := ..` kisminin sonuna `kendi_directoryniz/`ide eklemeniz gerekiyor.

## Testler: User Space de Syscallarin Kullanimi
1. Ornek olarak `kendi_isminiz` isminde olusturdugumuz syscallu bir C programinda kullanabiliriz:

  ```C
  #include <unistd.h>
  #include <string.h>
  #include <sys/syscall.h>
  #include <stdio.h>
  #include <errno.h>

  #define __NR_kendi_isminiz 463

  int main(int argc, char *argv[]) {
    long rv;
    /*yani syscall(335,arg)*/
    rv = syscall(__NR_kendi_isminiz, "hello my system call\n");
    if(rv < 0) {
        perror("syscall failed");
    }
    else {
        printf("syscall ran successfully, check dmesg output\n");
    }
    return 0;
  }
  ```

2. `get_proc_state ve _string test:`
komut satirindan pidi arguman olarak istenilen processing statetini hexadecimal (`get_proc_state`)  ve string (`get_proc_state_string`) kullanarak bulup **hexadecimal ve string** olarak yazdiran bir C programi yaziniz. Arguman bossa current process icin calistiriniz.
Ornek calisma ve output

  ```bash
  ./test_get_state pid
  pid: ..., state_hex:...., state_string:....
  ```

3. `set_proc_state` test kullanarak verilen bir processin durumunu hexadecimal olarak verilen state'e degistiren bir program yaziniz.

  ```bash
  ./test_set_state pid 20

  #status degisikligini teyit etme
  cat /proc/pid/status
  ```

## Teslim
1. Kernel compile ettikten sonra (1.odevdekine benzer olarak) asagidakilerin ekran goruntusu(isterseniz bir pdfede koyabilirsiniz):

  ```bash
  uname -r
  sudo dmesg | grep "Linux"
  sudo update-grub
  sudo ls /boot
  sudo cat /etc/fstab
  ```

2. tüm c dosyalarınızı ve degisiklik yaptiginiz sys.c, syscall.h vb dosyalar

3. Testlerin calisma goruntuleri:

  ```bash
$ ./test_kendi_isminiz
...
$ sudo dmesg | tail -10
$ strace ./test_kendi_isminiz
  ```
  ___

  ```bash
$ ./test_get_state ornekid
....
$ cat /proc/ornekid/status
$ sudo dmesg | tail -10
$ strace ./test_get_state ornekid
  ```
  ___

  ```bash
$ ./test_set_state ornekid 01
...
$ cat /proc/ornekid/status
$ sudo dmesg | tail -10
$ strace ./test_set_state ornekid 01
  ```
  ___

## Değerlendirme
**Testleri olmayan (yani calismayan, kernel compile edilmemis) sisteme eklenememis veya hatali eklenmis system callari degerlendirmeye tabi degildir.**
1. 50 kendi_isminiz system call eklenmesi ve ornek calistirilmasi
2. 15 get_proc_state (testleri calisan)
3. 15 get_proc_state_string (testleri calisan)
3. 20 set_proc_state (testleri calisan)
-  her bir hata -10 puan, küçük hatalar -5 puan
- -10 puan warnings ve kodlama standartları.
