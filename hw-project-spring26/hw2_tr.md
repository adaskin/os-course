# Ödev 2: Çekirdek Modülü ile İşlem/İş Parçacığı İzleme ve Kullanıcı Alanında Drone Simülasyonu

*Prepared wıth the help of DeepSeek AI*  
**Teslim Tarihi:** Açıklanacak

---

## Amaçlar

- Çalışan bir Linux çekirdeğine dinamik olarak eklenip çıkarılabilen bir çekirdek modülü yazmak.
- Çekirdeğin işlem/iş parçacığı veri yapılarını kullanarak sistemdeki tüm görevler (işlemler ve iş parçacıkları) hakkında bilgi toplamak.
- Bu bilgileri kullanıcı alanına sunmak için `/proc` dosya sistemi altında bir giriş oluşturmak.
- Bir drone teslimat sistemini simüle eden çok iş parçacıklı bir kullanıcı alanı programı yazmak.
- Çekirdek modülünü kullanarak farklı iş yükleri altında iş parçacıklarının durumlarını ve zamanlayıcının davranışını gözlemlemek.

Bu ödev, eşzamanlılığın etkilerini çekirdek perspektifinden gözlemleyebilmenizi amaçlamaktadır.

---

## Ön Bilgiler

### Çekirdek Modülleri
Çekirdek modülü, yeniden başlatma gerektirmeden çalışan çekirdeğe yüklenebilen bir kod parçasıdır. Bu, çekirdek işlevselliğini dinamik olarak genişletmemizi sağlar. Bu ödevde, `/proc` dosya sisteminde bir giriş oluşturan, çalışan görevler (işlemler ve iş parçacıkları) hakkında bilgi okuyan ve bu bilgiyi kullanıcı alanı programlarına sunan bir modül yazacaksınız.

### Çekirdekte İş Parçacıkları
Linux’ta her iş parçacığı (thread) bir `struct task_struct` ile temsil edilir (işlemler için kullanılan yapı). Çekirdek, tek iş parçacıklı bir işlem ile çok iş parçacıklı bir işlem arasında temel bir fark görmez – hepsi “görev”dir. `task_struct` şu alanları içerir:
- `pid` – işlem kimliği (her görev için benzersiz)
- `tgid` – iş parçacığı grubu kimliği (işlemin ana iş parçacığının PID’si)
- `__state` – mevcut durum (çalışıyor, uyuyor, vb.)
- `utime`, `stime` – kullanıcı ve sistem CPU süresi
- `se.vruntime` – zamanlayıcı tarafından kullanılan sanal çalışma süresi (Completely Fair Scheduler için)

### Kullanıcı Alanı İş Parçacıkları
Kullanıcı alanında, tek bir işlem içinde birden çok iş parçacığı oluşturmak için POSIX iş parçacığı kütüphanesini (`pthreads`) kullanırız. Bu tür her iş parçacığı, çekirdek tarafından bir görevle (hafif süreç) desteklenir. Yani `pthread_create()` ile bir iş parçacığı oluşturduğunuzda, çekirdek, ana işlemle aynı bellek alanını ve diğer kaynakları paylaşan yeni bir `task_struct` oluşturur.

### Drone Simülasyonu Örneği
Ders notlarından, birden çok “drone” iş parçacığının paylaşılan bir kuyruktan siparişleri alıp teslim ettiği bir drone teslimat simülasyonu bulunmaktadır. Bu örnek şunları gösterir:
- İş parçacığı oluşturma ve yönetimi.
- Bir kaynağın (sipariş kuyruğu) paylaşımı.
- Mutex ve koşul değişkenleri kullanarak eşitleme.

Bu ödevde, drone simülasyonunu uyarlayarak, drone iş parçacıklarını çekirdek modülünüzle izleyeceksiniz.

---

## Bölüm 1: Basit Çekirdek Modülü

Çekirdek modüllerini yüklemeyi, kaldırmayı ve çekirdek mesajlarını görüntülemeyi öğrenmek için önce basit bir modül yazacaksınız.

### 1.1 Basit Modül Oluşturma

`simple.c` adlı bir dosya oluşturun:

```c
#include <linux/kernel.h>   /* pr_info() için */
#include <linux/module.h>   /* module_init, module_exit, MODULE_LICENSE için */
#include <linux/init.h>     /* __init, __exit için */

static int __init simple_init(void)
{
    pr_info("Basit modül yüklendi\n");
    return 0;
}

static void __exit simple_exit(void)
{
    pr_info("Basit modül kaldırıldı\n");
}

module_init(simple_init);
module_exit(simple_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Çok basit bir çekirdek modülü");
MODULE_AUTHOR("Adınız Soyadınız");
```

### 1.2 Makefile Oluşturma

Aynı dizinde aşağıdaki içeriğe sahip bir `Makefile` oluşturun:

```make
obj-m += simple.o

PWD := $(CURDIR)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

### 1.3 Modülü Derleme ve Yükleme

- `make` komutunu çalıştırarak modülü derleyin. `simple.ko` dosyası oluşmalıdır.
- `sudo insmod simple.ko` ile modülü yükleyin.
- `lsmod | grep simple` ile yüklendiğini doğrulayın.
- `dmesg | tail` ile çekirdek günlüğünün son satırlarını görüntüleyin. “Basit modül yüklendi” mesajını görmelisiniz.
- `sudo rmmod simple` ile modülü kaldırın ve `dmesg` ile kaldırma mesajını kontrol edin.

**Ekran görüntüsü:** Modülü yükledikten sonra `lsmod` çıktısını ve `dmesg`’den ilgili satırları gösteren bir ekran görüntüsü alın.

---

## Bölüm 2: İş Parçacığı Bilgi Modülü

Şimdi basit modülü, sistemdeki tüm iş parçacıklarını PID, TGID, durum ve CPU süreleriyle birlikte listeleyen bir `/proc` girişi oluşturacak şekilde genişleteceksiniz.

### 2.1 /proc Girişi Ekleme

Önceki modülü değiştirerek (veya yeni bir modül oluşturarak) aşağıdaki özellikleri ekleyin:
- `for_each_process(task)` kullanarak tüm görevler üzerinde dönen bir fonksiyon.
- Her iş parçacığı için bir satır oluşturan bir dize inşa etme.
- Dosya okunduğunda kullanıcı alanına veri aktarmak için `copy_to_user()` kullanımı.
- `cat` ve diğer araçların çalışması için kısmi okumaları (`*off` parametresi) doğru şekilde ele alma.

Aşağıdaki şablonu başlangıç noktası olarak kullanabilirsiniz.

### 2.2 Şablon: `thread_monitor.c`

```c
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>   /* for_each_process için */
#include <linux/uaccess.h>
#include <linux/init.h>

#define PROC_NAME "thread_monitor"

static const char * const task_state_array[] = {
    "R (running)",   /* 0x00 */
    "S (sleeping)",  /* 0x01 */
    "D (disk sleep)",/* 0x02 */
    "T (stopped)",   /* 0x04 */
    "t (tracing stop)",/* 0x08 */
    "X (dead)",      /* 0x10 */
    "Z (zombie)",    /* 0x20 */
    "P (parked)",    /* 0x40 */
    "I (idle)",      /* 0x80 */
};

static ssize_t thread_monitor_read(struct file *file, char __user *buf,
                                   size_t len, loff_t *off)
{
    char *kbuf;
    size_t kbuf_size = PAGE_SIZE;
    size_t written = 0;
    struct task_struct *task;

    kbuf = kmalloc(kbuf_size, GFP_KERNEL);
    if (!kbuf)
        return -ENOMEM;

    rcu_read_lock();

    for_each_process(task) {
        int state_idx = task_state_index(task);
        const char *state_str = task_state_array[state_idx];
        unsigned long long utime = task->utime;
        unsigned long long stime = task->stime;

        written += snprintf(kbuf + written, kbuf_size - written,
                            "PID: %d, TGID: %d, Durum: %s, utime: %llu, stime: %llu\n",
                            task->pid, task->tgid, state_str, utime, stime);

        if (written >= kbuf_size - 128)
            break;
    }

    rcu_read_unlock();

    /* Kısmi okumaları işle */
    if (*off >= written) {
        kfree(kbuf);
        return 0;
    }
    if (len > written - *off)
        len = written - *off;

    if (copy_to_user(buf, kbuf + *off, len)) {
        kfree(kbuf);
        return -EFAULT;
    }
    *off += len;
    kfree(kbuf);
    return len;
}

static const struct proc_ops thread_monitor_ops = {
    .proc_read = thread_monitor_read,
};

static int __init thread_monitor_init(void)
{
    proc_create(PROC_NAME, 0444, NULL, &thread_monitor_ops);
    pr_info("İş parçacığı izleme modülü yüklendi\n");
    return 0;
}

static void __exit thread_monitor_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    pr_info("İş parçacığı izleme modülü kaldırıldı\n");
}

module_init(thread_monitor_init);
module_exit(thread_monitor_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("/proc/thread_monitor ile iş parçacığı izleme");
MODULE_AUTHOR("Adınız Soyadınız");
```

### 2.3 Makefile’ı Güncelleme

Modül adını `thread_monitor` olarak belirlediyseniz, Makefile’da `obj-m += thread_monitor.o` satırını kullanın.

### 2.4 Modülü Test Etme

- `make` ile derleyin.
- `sudo insmod thread_monitor.ko` ile yükleyin.
- `cat /proc/thread_monitor | head -20` ile çıktıyı kontrol edin. Tüm iş parçacıklarının (çekirdek iş parçacıkları dahil) listelendiğini görmelisiniz.

**Ekran görüntüsü:** `cat /proc/thread_monitor` çıktısının ilk 20 satırını gösteren bir ekran görüntüsü alın.

---

## Bölüm 3: Drone Simülasyonu ve İş Parçacığı İzleme

Şimdi, birden çok drone iş parçacığı kullanan bir drone teslimat sistemini simüle eden bir kullanıcı alanı programı yazacaksınız. Program:

- Belirli sayıda drone iş parçacığı oluşturacak (örneğin 4).
- Her drone iş parçacığı sürekli olarak paylaşılan bir kuyruktan sipariş alacak, “teslimat” yapacak (uyku ile simüle edilecek) ve tekrarlayacak.
- Ayrı bir sipariş üretici iş parçacığı, kuyruğa rastgele aralıklarla yeni siparişler ekleyecek.
- Ana iş parçacığı, belirli aralıklarla (örneğin her 2 saniyede bir) `/proc/thread_monitor` dosyasını okuyacak ve drone iş parçacıkları ile üretici iş parçacığının bilgilerini yazdıracak (TGID’ye göre filtreleyebilir veya tümünü yazdırabilirsiniz).

**Ders notlarındaki drone teslimat örneğini uyarlayacaksınız** (bkz. `4b-multi-thread-programming-examples.md`).

### 3.1 Uygulama Taslağı

`drone_sim.c` adlı bir C dosyası oluşturun ve aşağıdaki bileşenleri ekleyin:

- Paylaşılan veri yapıları: bir kuyruk (bağlı liste veya C++ kullanıyorsanız `std::queue`) ve ilişkili mutex/koşul değişkeni. POSIX iş parçacıklarını kullanın.
- Drone işlevi: sonsuz döngü, sipariş al, teslimatı simüle et, tekrarla.
- Sipariş üretici işlevi: rastgele aralıklarla kuyruğa sipariş ekle.
- Ana fonksiyon: iş parçacıklarını oluştur, ardından periyodik olarak izle.

**Önemli:** Yarış koşullarından kaçınmak için uygun eşitleme mekanizmalarını kullanın (`pthread_mutex_t`, `pthread_cond_t`).

### 3.2 İzleme

İzleme kısmı şunları yapmalıdır:

- `/proc/thread_monitor` dosyasını okumak için açın.
- İçeriğini (veya ilk birkaç satırı) okuyun ve yazdırın.
- İsteğe bağlı olarak, sadece programınıza ait iş parçacıklarını göstermek için filtreleme yapın (ana iş parçacığının PID’sini `getpid()` ile alabilir, TGID ile karşılaştırabilirsiniz). Bu, çıktıyı daha sade hale getirir.

**Örnek izleme döngüsü (`main()` içinde):**

```c
while (1) {
    sleep(2);
    FILE *fp = fopen("/proc/thread_monitor", "r");
    if (!fp) {
        perror("fopen");
        break;
    }
    char line[256];
    printf("\n--- İş parçacığı anlık görüntüsü ---\n");
    while (fgets(line, sizeof(line), fp)) {
        // İsteğe bağlı: TGID'ye göre filtrele
        printf("%s", line);
    }
    fclose(fp);
}
```

### 3.3 Derleme ve Çalıştırma

- Derleme: `gcc -pthread -o drone_sim drone_sim.c`
- Çekirdek modülünü daha önce yüklemediyseniz yükleyin: `sudo insmod thread_monitor.ko`
- Programı çalıştırın: `./drone_sim`. Periyodik olarak tüm iş parçacıklarının (drone iş parçacıkları dahil) anlık görüntülerini görmelisiniz.

**Ekran görüntüsü:** Drone simülasyonunun çalışması sırasında, en az iki izleme anlık görüntüsünü gösteren bir ekran görüntüsü alın. Drone iş parçacıklarına karşılık gelen satırları vurgulayın.

---

## Bölüm 4: Analiz ve Rapor

Aşağıdaki soruları kısa bir rapor halinde (en fazla 2 sayfa) cevaplayın ve ekran görüntülerini ekleyin:

1. 4 drone ile simülasyonu çalıştırdığınızda kaç iş parçacığı oluşturulur? Bunlar `/proc/thread_monitor` çıktısında nasıl görünür (PID, TGID, durum)?
2. Bir iş parçacığının `pid` ve `tgid` değerleri arasındaki fark nedir? Aynı işlemin tüm iş parçacıkları için hangisi aynıdır?
3. Simülasyon çalışırken drone iş parçacıklarının durumları nasıl değişir? Sipariş beklerken ve teslimat yaparken durumları ne olur? Kod üzerinden açıklayın.
4. Drone simülasyonunda `pthreads` (çekirdek iş parçacıkları) kullanıldı. Eğer bunun yerine kullanıcı seviyesinde iş parçacıkları (örneğin GNU Portable Threads gibi bir kütüphane) kullanılsaydı, `/proc/thread_monitor` çıktısı nasıl farklı olurdu? (Uygulamanız gerekmez, teorik olarak düşünün.)
5. Bir çekirdek modülünü yüklemek ve kaldırmak için hangi adımlar izlenir? Her adımda hangi çekirdek fonksiyonları çağrılır?

---

## Teslim

Aşağıdaki dosyaları tek bir sıkıştırılmış arşiv (`.zip` veya `.tar.gz`) olarak teslim edin:

1. **Kaynak kodlar**:
   - `simple.c` ve `Makefile` (veya `thread_monitor.c` ve `Makefile`).
   - `drone_sim.c`.
2. **Ekran görüntüleri** (resim dosyaları veya PDF):
   - Modülü yükledikten sonra `lsmod` çıktısı.
   - Modül yükleme/kaldırma mesajlarını gösteren `dmesg` çıktısı.
   - `cat /proc/thread_monitor` çıktısının ilk 20 satırı.
   - `./drone_sim` çalıştırılması sırasında en az iki izleme anlık görüntüsü.
3. **Rapor** (PDF) – analiz sorularının cevaplarını içeren.

**Not:** Tüm grup üyeleri bireysel olarak teslim etmelidir (veya ders politikasına göre), ancak işbirliği yapabilirsiniz. Grup üyelerinizi arşivdeki `README.txt` dosyasında belirtin.

---

## Değerlendirme Kriterleri

| Bileşen | Puan |
|---------|------|
| **Bölüm 1** (basit modül derlenir, yüklenir, kaldırılır) | 15 |
| **Bölüm 2** (iş parçacığı izleme modülü çalışır, doğru çıktı verir) | 30 |
| **Bölüm 3** (drone simülasyonu derlenir, çalışır, izleme yapar) | 35 |
| **Rapor** (cevaplar, analiz, ekran görüntüleri) | 20 |
| **Toplam** | 100 |

**Kesintiler**:
- Derleme uyarıları içeren kod: -5
- Eksik veya hatalı ekran görüntüleri: her eksik için -5
- İntihal veya izinsiz kod paylaşımı: 0 puan (ders politikasına göre)

---

## İpuçları ve Kaynaklar

- Sağlanan çekirdek modülü şablonunu kullanın; `copy_to_user` ve `*off` işleme mantığını iyi anlayın.
- Görev listesinde dolaşırken, liste değişimini engellemek için `rcu_read_lock()` ve `rcu_read_unlock()` kullanın.
- Drone simülasyonu dikkatli eşitleme gerektirir; kuyruk için mutex, yeni siparişler geldiğinde drone’ları uyandırmak için koşul değişkeni kullanın.
- Sonsuz döngüden kaçınmak için sipariş sayısını sınırlayabilir veya programı manuel olarak sonlandırana kadar çalıştırabilirsiniz (Ctrl+C).
- Yalnızca kendi iş parçacıklarınızı görmek için, ana iş parçacığının TGID’sini alıp (bu, ana işlemin PID’sidir) `/proc/thread_monitor`’dan okuduğunuz her satırda TGID ile karşılaştırarak filtreleme yapabilirsiniz.

Kolay gelsin!