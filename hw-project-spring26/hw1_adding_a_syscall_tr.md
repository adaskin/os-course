# Ödev 1 — Linux Çekirdeğine Sistem Çağrısı Eklemek

**Son Teslim Tarihi:** Duyurulacak  

## Amaçlar

- Linux çekirdeğine yeni bir sistem çağrısı ekleme adımlarını kavramak.
- Çekirdek ve kullanıcı alanı arasında veri kopyalamayı (`copy_to_user`) öğrenmek.
- Mevcut süreç (process) hakkında bilgi almak için çekirdek veri yapılarına (`task_struct`) erişmek.
- Yeni sistem çağrınızı çağıran bir kullanıcı programı yazmak.

Bu ödev, **Proje 0**'da yaptığınız özel çekirdek derleme işleminin üzerine inşa edilmiştir. Burada çekirdek kaynağını değiştirecek, bir sistem çağrısı ekleyecek, artımlı olarak yeniden derleyecek ve değişikliklerinizi test edeceksiniz.

---

## Arka Plan: Süreç Durumu ve Çekirdek Yapıları

Linux çekirdeğinde her süreç (görev), `<linux/sched.h>` içinde tanımlı `struct task_struct` ile temsil edilir. Bu yapının içinde `__state` adlı bir alan, sürecin o anki durumunu tutar (örneğin çalışıyor, uyuyor, durdurulmuş). Çekirdek, bu durumları insan tarafından okunabilir dizgilere dönüştürmek için bir dizi sağlar:

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

Bir görev için bu dizideki doğru indisi elde etmek için `<linux/sched.h>` içindeki `task_state_index(task)` fonksiyonu kullanılır. Bu fonksiyon, `task_state_array[]` ile kullanılabilecek bir tamsayı döndürür.

Mevcut sürecin (çalışan kodun içinde bulunduğu süreç) göstericisine `current` makrosu ile erişilir.

---

## Sistem Çağrısı: `get_my_state`

Bu ödevde, **mevcut sürecin** durum dizgisini kullanıcı tarafından sağlanan bir tampona kopyalayan **tek bir sistem çağrısı** gerçekleyeceksiniz.

**Prototip** (`include/linux/syscalls.h` içine eklenecek):
```c
asmlinkage long sys_get_my_state(char __user *buf, size_t size);
```

**Davranış**:
- Mevcut sürecin durum indisini al: `int idx = task_state_index(current);`
- İlgili dizgiyi al: `const char *state_str = task_state_array[idx];`
- Dizginin uzunluğunu hesapla (sonlandırıcı null karakter dahil): `len = strlen(state_str) + 1;`
- Eğer `size` bu uzunluktan küçükse `-EINVAL` döndür.
- `copy_to_user(buf, state_str, len)` ile dizgiyi kullanıcı alanına kopyala.
  - Eğer `copy_to_user` başarısız olursa (sıfırdan farklı dönerse) `-EFAULT` döndür.
- Başarı durumunda `0` döndür.

**Önemli Notlar**:
- `__user` ek açıklaması, işaretçinin kullanıcı alanına ait olduğunu belirtir.
- Bellek taşmalarını önlemek için her zaman `size` kontrolü yapın.
- Kullanıcı işaretçisini asla doğrudan yönlendirmeyin (dereference etmeyin); her zaman `copy_to_user()` kullanın.

---

## Adım Adım Yapılacaklar

### 1. Çekirdek Kaynak Kodunu Hazırlayın

Proje 0'dan çekirdek kaynağınız duruyorsa onu kullanabilirsiniz. Yoksa, daha önce derlediğiniz sürümle aynı olan yeni bir kaynak indirin ve açın. Bundan sonraki işlemleri bu dizinde yapacağız.

```bash
cd ~/linux-6.12   # kendi sürümünüzle değiştirin
```

### 2. Sistem Çağrısı Numarası Ekleyin

Sistem çağrıları numaralarla tanımlanır. **Generic** (tüm mimariler için geçerli) sistem çağrı tablosunu kullanacağız. `include/uapi/asm-generic/unistd.h` dosyasını açın ve yeni çağrıların eklendiği bölümü bulun (dosyanın sonlarına doğru). Şu satırları ekleyin:

```c
#define __NR_get_my_state 463   /* kullanılmayan bir numara seçin */
__SYSCALL(__NR_get_my_state, sys_get_my_state)
```

Ardından toplam sistem çağrısı sayısını güncelleyin. Şuna benzer bir satır arayın:
```c
#define __NR_syscalls 463
```
Bu sayıyı bir artırın (örneğin 464 yapın).

*Eğer x86-64 özel tablosunu kullanmak isterseniz `arch/x86/entry/syscalls/syscall_64.tbl` dosyasını da düzenleyebilirsiniz. Ancak generic yöntem daha basittir ve bu ödev için yeterlidir.*

### 3. Sistem Çağrısını Bildirin

`include/linux/syscalls.h` dosyasına prototipi ekleyin. Diğer sistem çağrılarının bildirimlerinin olduğu bölümü bulun (örneğin `asmlinkage long sys_...` satırları) ve şunu ekleyin:

```c
asmlinkage long sys_get_my_state(char __user *buf, size_t size);
```

### 4. Sistem Çağrısını Gerçekleyin

Gerçeklemeyi `kernel/sys.c` dosyasına koyacağız (çeşitli sistem çağrılarının bulunduğu yer). `kernel/sys.c` dosyasını açın ve sonlarına doğru (dosyanın sonundaki `#endif`'den önce) aşağıdaki kodu ekleyin. Gerekli başlık dosyalarının dahil edildiğinden emin olun; `<linux/sched.h>` zaten `sys.c` içinde bulunur, ancak değilse ekleyin.

```c
SYSCALL_DEFINE2(get_my_state, char __user *, buf, size_t, size)
{
    int idx;
    const char *state_str;
    size_t len;

    idx = task_state_index(current);
    state_str = task_state_array[idx];
    len = strlen(state_str) + 1;  /* null karakter dahil */

    if (size < len)
        return -EINVAL;

    if (copy_to_user(buf, state_str, len))
        return -EFAULT;

    return 0;
}
```

**Açıklamalar**:
- `SYSCALL_DEFINE2` makrosu, iki argüman alan asıl sistem çağrısı fonksiyonunu oluşturur.
- `task_state_index(current)` mevcut süreç için indis değerini döndürür.
- `task_state_array` dizisi `fs/proc/array.c` içinde tanımlıdır ve uygun başlık dosyalarıyla bildirilmiştir. `kernel/sys.c` zaten `linux/sched.h` içerdiği için sorun çıkmaması gerekir. Eğer derleme hatası alırsanız, `extern const char * const task_state_array[];` bildirimini ekleyebilirsiniz.

### 5. Çekirdeği Derleyin

Şimdi çekirdeği yeniden derlemeniz gerekiyor. Sadece birkaç dosya değiştirdiğiniz için derleme artımlı olacak ve ilk derlemeden çok daha hızlı bitecektir.

```bash
make -j$(nproc)
```

Proje 0'da olduğu gibi sertifika hataları alırsanız (SYSTEM_TRUSTED_KEYS vb.), bunları devre dışı bırakın:

```bash
scripts/config --disable SYSTEM_TRUSTED_KEYS
scripts/config --disable SYSTEM_REVOCATION_KEYS
make -j$(nproc)
```

### 6. Çekirdeği Yükleyin

Başarılı derlemeden sonra modülleri ve çekirdek görüntüsünü yükleyin:

```bash
sudo make modules_install
sudo make install
sudo update-grub   # otomatik yapılmazsa
```

Ardından sistemi yeniden başlatın ve GRUB menüsünden yeni çekirdeğinizi seçin.

### 7. Yeni Çekirdeği Doğrulayın

Yeniden başlattıktan sonra:

```bash
uname -r
```

Sürümünüzün sizin eklediğiniz yerel sürümü gösterdiğini kontrol edin (örneğin `6.12.0-mylinux`). Sistem çağrınızın varlığını test edeceğiz.

---

## Sistem Çağrısını Test Etme

Sistem çağrınızı `syscall()` fonksiyonu ile çağıran bir C programı yazacaksınız.

### 7.1. Test Programını Yazın

`test_get_my_state.c` adlı bir dosya oluşturun:

```c
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifndef __NR_get_my_state
#define __NR_get_my_state 463   /* atadığınız numara ile aynı olmalı */
#endif

int main()
{
    char buffer[128];
    long rv;

    rv = syscall(__NR_get_my_state, buffer, sizeof(buffer));
    if (rv == 0) {
        printf("Mevcut sürecin durumu: %s\n", buffer);
    } else {
        perror("get_my_state sistem çağrısı başarısız");
    }

    return 0;
}
```

### 7.2. Derleyin ve Çalıştırın

```bash
gcc -o test_get_my_state test_get_my_state.c
./test_get_my_state
```

Çıktı şuna benzer olmalı:
```
Mevcut sürecin durumu: R (running)
```

### 7.3. `strace` ile Test Edin

Programı `strace` altında çalıştırarak gerçek sistem çağrısını görebilirsiniz:

```bash
strace ./test_get_my_state
```

Çıktıda şuna benzer bir satır arayın:
```
get_my_state(0x7ffc12345678, 128) = 0
```

### 7.4. Çekirdek Günlüklerini Kontrol Edin (İsteğe Bağlı)

Sistem çağrınıza bir `printk` eklemediyseniz, çağrının yapıldığını `dmesg` ile göremezsiniz. Ancak merak ederseniz, geçici olarak çağrı içine `printk("get_my_state çağrıldı\n");` ekleyip tekrar derleyebilirsiniz. O zaman `dmesg | tail` ile görebilirsiniz.

---

## Teslim Edilecekler

Tek bir PDF (veya görüntüler) olarak aşağıdakileri gönderin:

1. **`uname -r` çıktısının ekran görüntüsü** – özel çekirdek sürümünüzü gösteriyor.
2. **`include/uapi/asm-generic/unistd.h` dosyasının ilgili kısmının ekran görüntüsü** – `__NR_get_my_state` satırını gösteriyor.
3. **`kernel/sys.c` içindeki gerçeklemenin ekran görüntüsü** (veya nereye koyduysanız).
4. **Test programının çalıştırılmasının ekran görüntüsü** (`./test_get_my_state` çıktısı).
5. **`strace ./test_get_my_state` çıktısının ekran görüntüsü** – sistem çağrısının çağrıldığını gösteriyor.
6. **Kullandığınız `.config` dosyası** (ayrı bir dosya olarak ekleyin veya PDF içine koyun).

Ayrıca test programınızın kaynak kodunu da (düz metin olarak veya PDF içinde) ekleyin.

---

## Sorun Giderme

### “task_state_array” tanımsız hatası

Eğer derleyici `task_state_array`'in tanımlı olmadığını söylerse, fonksiyonunuzun başına şu extern bildirimi ekleyin:

```c
extern const char * const task_state_array[];
```

Alternatif olarak `<linux/sched.h>`'in bu diziyi getirdiğinden emin olun. Çekirdek sürümünüze göre farklılık gösterebilir.

### copy_to_user başarısız oluyor

Kullanıcı tampon işaretçisinin geçerli olduğundan ve `size` değerinin yeterli olduğundan emin olun.

### Sistem çağrı numarası çakışması

Eğer numara zaten kullanılıyorsa, farklı bir numara deneyin. `__NR_syscalls` değerine bakarak mevcut maksimumu görebilirsiniz.

### Çekirdek derleme hataları

Prototipi `syscalls.h`'a doğru eklediğinizden, gerçeklemenin `SYSCALL_DEFINE2` ile yapıldığından ve kodun bir `#ifdef` bloğu içinde kalmadığından emin olun.

---

## Ekstra Zorluk (İsteğe Bağlı)

Erken bitirirseniz sistem çağrınızı genişleterek:

- Mevcut sürecin PID'ini de döndürebilir (ikinci bir işaretçi argümanıyla veya dizgiye ekleyerek).
- Bir PID argümanı alıp o sürecin durumunu döndürebilir (`find_task_by_vpid()` kullanarak).

Ancak bu eklemeler ödev için zorunlu değildir.

---

## Kaynaklar

- [Linux çekirdeği sistem çağrısı ekleme dokümantasyonu (İngilizce)](https://www.kernel.org/doc/html/v4.10/process/adding-syscalls.html)
- [Linux çapraz referans (Bootlin)](https://elixir.bootlin.com/linux/latest/source) – çekirdek kaynağında gezinmek için.
- [man syscall](https://man7.org/linux/man-pages/man2/syscall.2.html)

Başarılar!