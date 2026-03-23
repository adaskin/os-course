# Ödev 0: Linux Kurulumu, Terminal Temelleri ve İlk Kernel Derlemeniz

**Son Teslim Tarihi:** [TBA]  

## Hedefler

Bu projeyi tamamlayarak şunları yapabileceksiniz:

- Bilgisayarınızda bir Linux sanal makinesi kurmak.
- Dosyalarda gezinmek ve onları yönetmek için gerekli terminal komutlarını öğrenmek.
- `make`'in ne olduğunu ve basit bir Makefile'ın nasıl çalıştığını anlamak.
- Linux çekirdek kaynak kodunu indirmek, yapılandırmak ve kendi özel çekirdeğinizi derlemek.
- Yeni derlediğiniz çekirdekle sistemi başlatmak ve değişiklikleri doğrulamak.

Bu proje dersin geri kalanı için temel oluşturur. Daha önce Linux kullanmış olsanız bile, adımları dikkatlice uygulamak ilerideki çekirdek seviyesindeki ödevlere hazır olmanızı sağlayacaktır.

---

## Bölüm 1: Sanal Makinede Linux Kurulumu

**VirtualBox** (ücretsiz ve açık kaynak) kullanarak bir sanal makine oluşturacağız. Apple Silicon (M1/M2/M3) Mac'iniz varsa, VirtualBox henüz bunu tam olarak desteklemez; onun yerine **UTM** (QEMU tabanlı) kullanın.

### 1.1 VirtualBox veya UTM Kurulumu

- **Windows/Linux/Intel Mac**: [VirtualBox](https://www.virtualbox.org/wiki/Downloads) indirip kurun.
- **Apple Silicon Mac**: [UTM](https://mac.getutm.app/)'i web sitesinden veya Mac App Store'dan indirin.

### 1.2 Linux ISO İndirme

Size kolay gelecek bir dağıtım seçin. **Ubuntu** veya **Debian** iyi seçeneklerdir çünkü belgeleri boldur ve geniş toplulukları vardır.

- **Ubuntu**: [https://ubuntu.com/download/desktop](https://ubuntu.com/download/desktop)
- **Debian**: [https://www.debian.org/distrib/](https://www.debian.org/distrib/)

Apple Silicon kullanıyorsanız, ISO'nun **ARM64** sürümünü indirdiğinizden emin olun.

### 1.3 Sanal Makine Oluşturma

1. VirtualBox/UTM'yi açın ve **Yeni**'ye tıklayın.
2. Sanal makineye bir isim verin (örneğin `OS_Lab`).
3. En az **2 CPU çekirdeği** ve **4 GB RAM** ayırın (ana makineniz izin veriyorsa daha fazlasını verebilirsiniz).
4. En az **25 GB** boyutunda bir sanal sabit disk oluşturun (dinamik boyutlandırma sorun değildir).
5. Sanal makine ayarlarında, **Depolama** altında, indirdiğiniz ISO dosyasını sanal bir optik disk olarak ekleyin.
6. Sanal makineyi başlatın ve ekrandaki kurulum talimatlarını izleyin. Linux'ta yeniyseniz grafiksel kurulumu seçin.
7. Kurulum sırasında bir kullanıcı adı ve şifre oluşturun – bunları daha sonra kullanacaksınız.

Kurulumdan sonra sanal makineyi yeniden başlatın. Bir giriş ekranı görmelisiniz.

### 1.4 Sistemi Güncelleme

Bir **terminal** açın (uygulama menüsünde "Terminal" veya "Uçbirim" arayın). İlk olarak paket listesini güncelleyin ve mevcut paketleri yükseltin:

```bash
sudo apt update
sudo apt upgrade -y
```

Ardından çekirdek derlemek için ihtiyaç duyacağımız geliştirme araçlarını yükleyin:

```bash
sudo apt install build-essential libncurses-dev bison flex libssl-dev libelf-dev
```

- `build-essential`: `gcc`, `make` ve diğer derleme araçlarını içerir.
- `libncurses-dev`: `menuconfig` için gereklidir.
- `bison` ve `flex`: Çekirdek derleme sisteminin kullandığı ayrıştırıcı oluşturuculardır.
- `libssl-dev`: Çekirdek özellikleri için gereken kriptografik kütüphaneleri sağlar.
- `libelf-dev`: ELF nesne dosyalarını işlemek içindir.

Artık Linux VM'iniz hazır.

---

## Bölüm 2: Terminal Temelleri

Linux komut satırında yeniyseniz, bu bölüm en yaygın komutları tanıtacaktır. VM'inizdeki terminalde her komutu deneyin.

### 2.1 Dizinler Arasında Gezinme

- `pwd` – bulunduğunuz dizini gösterir (**p**rint **w**orking **d**irectory).
- `ls` – dosya ve dizinleri listeler.
  - `ls -l` – ayrıntılı liste (izinler, boyut, değiştirilme zamanı).
  - `ls -a` – gizli dosyalar dahil tüm dosyaları gösterir (nokta ile başlayanlar).
- `cd` – dizin değiştir (**c**hange **d**irectory).
  - `cd /` – kök dizine git.
  - `cd ~` – ev dizinine git.
  - `cd ..` – bir üst dizine çık.

### 2.2 Dosya İşlemleri

- `cp kaynak hedef` – dosya kopyalar.
  - `cp -r kaynak_dizin hedef_dizin` – bir dizini ve içindekileri kopyalar.
- `mv kaynak hedef` – dosyayı/dizini taşır veya yeniden adlandırır.
- `rm dosya` – dosya siler.
  - `rm -r dizin` – bir dizini ve içindekileri siler (dikkatli olun!).
- `mkdir dizin_adi` – yeni bir dizin oluşturur.
- `touch dosya_adi` – boş bir dosya oluşturur veya zaman damgasını günceller.
- `cat dosya` – dosyanın içeriğini görüntüler.
- `less dosya` – dosyayı sayfa sayfa görüntüler (çıkmak için `q` tuşlayın).
- `head -n 5 dosya` – ilk 5 satırı gösterir.
- `tail -n 5 dosya` – son 5 satırı gösterir.

### 2.3 İzinler

- `chmod` – dosya izinlerini değiştirir.
  - Örnek: `chmod +x betik.sh` dosyayı çalıştırılabilir yapar.
- `chown` – dosya sahipliğini değiştirir (`sudo` gerektirir).
  - Örnek: `sudo chown kullanıcı:grup dosya`.

### 2.4 Yardım Alma

- `man komut` – bir komutun kılavuzunu görüntüler (örn. `man ls`).
- `komut --help` – birçok komut kısa bir yardım mesajı gösterir.

### 2.5 Yönlendirme ve Borular (Pipes)

- `>` – çıktıyı bir dosyaya yönlendirir (üzerine yazar).
  - `ls > dosyalar.txt` komutunun çıktısını `dosyalar.txt` dosyasına kaydeder.
- `>>` – çıktıyı bir dosyaya ekler.
- `|` – bir komutun çıktısını başka bir komuta girdi olarak verir.
  - `ls -l | less` – ayrıntılı listeyi sayfa sayfa görüntüler.

### 2.6 Süreç Yönetimi

- `ps` – çalışan süreçleri listeler.
- `kill PID` – belirtilen işlem kimliğine sahip süreci sonlandırır.
- `Ctrl+C` – terminalde çalışan komutu keser.

### 2.7 Metin Düzenleyiciler

Metin dosyalarını düzenlemeniz gerekecek. İki basit terminal düzenleyicisi **nano** (kolay) ve **vim** (güçlü ama öğrenmesi biraz daha zordur).

- **nano**: `nano dosya_adi` – düzenleyin, ardından çıkmak için `Ctrl+X`, kaydetmek için `Y`, onaylamak için `Enter`.
- **vim**: `vim dosya_adi` – ekleme moduna geçmek için `i`, komut moduna dönmek için `Esc`, kaydedip çıkmak için `:wq`.

Hangisi size kolay gelirse onu kullanın. Bu proje için çoğunlukla sadece satırları kopyalayıp yapıştıracağız, bu nedenle basit bir düzenleyici yeterlidir.

---

## Bölüm 3: Make ve Makefile'a Giriş

Linux çekirdeği çok büyük bir projedir ve **make** kullanılarak derlenir. Make, programların nasıl derlenip bağlanacağını belirleyen bir **Makefile** okuyan bir yapı otomasyon aracıdır.

### 3.1 Makefile Nedir?

Bir Makefile şu biçimde kurallar içerir:

```
hedef: bağımlılıklar
	komutlar
```

- **hedef**: oluşturulacak dosya (örneğin bir çalıştırılabilir).
- **bağımlılıklar**: hedef oluşturulmadan önce var olması gereken dosyalar.
- **komutlar**: hedefi gerçekten oluşturan kabuk komutları (başında TAB karakteri olmalıdır).

### 3.2 Basit Bir Örnek

`hello.c` adında bir dosya oluşturun:

```c
#include <stdio.h>

int main() {
    printf("Merhaba dünya!\n");
    return 0;
}
```

Şimdi aynı dizinde `Makefile` (uzantısız) adında bir dosya oluşturun:

```make
hello: hello.c
	gcc -o hello hello.c
```

O dizinde bir terminal açın ve şunu çalıştırın:

```bash
make
```

Make, `hello` dosyasının olmadığını (veya `hello.c`'nin `hello`'dan daha yeni olduğunu) görecek ve `gcc -o hello hello.c` komutunu çalıştıracaktır. Artık `./hello` ile çalıştırabileceğiniz bir `hello` çalıştırılabiliri vardır.

Değişkenler ve daha karmaşık kurallar da tanımlayabilirsiniz. Örneğin:

```make
CC = gcc
CFLAGS = -Wall -g

hello: hello.c
	$(CC) $(CFLAGS) -o hello hello.c

clean:
	rm -f hello
```

Şimdi `make clean` çalıştırılabilir dosyayı silecektir.

### 3.3 Make Çekirdek İçin Nasıl Kullanılır?

Çekirdek kaynak ağacı, binlerce satırlık bir üst düzey Makefile içerir. Çekirdek dizininde `make` yazdığınızda:

- Yapılandırmayı okur (`.config` dosyasından).
- Bu yapılandırmaya göre hangi dosyaların derleneceğine karar verir.
- Çekirdek görüntüsünü (`vmlinuz`) ve yüklenebilir modülleri (`.ko` dosyaları) derler.
- `menuconfig`, `modules_install` ve `install` gibi hedefler sağlar.

`make`'i anlamak, çekirdek derleme sürecinin nasıl organize edildiğini kavramanıza yardımcı olur.

---

## Bölüm 4: Linux Çekirdeğini İndirme ve Yapılandırma

Şimdi Linux çekirdek kaynağını indirecek, yapılandıracak ve derlemeye hazırlanacağız.

### 4.1 Çekirdek Kaynağını Edinin

Resmi çekirdeği [kernel.org](https://www.kernel.org/) adresinden indirebilirsiniz. Kararlı bir sürüm seçin, örneğin 6.12.x (veya en son uzun süreli destek sürümü). VM'inizde bir terminal açın ve şunları çalıştırın:

```bash
cd ~
wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.12.tar.xz
tar -xf linux-6.12.tar.xz
cd linux-6.12
```

`6.12`'yi indirdiğiniz gerçek sürümle değiştirin.

### 4.2 Başlangıç Yapılandırması Oluşturma

Çekirdek derleme sisteminin, hangi özelliklerin ve sürücülerin dahil edileceğini belirten bir `.config` dosyasına ihtiyacı vardır. Mevcut çalışan çekirdeğinizin yapılandırmasından başlayacağız. Bu, VM'iniz için gerekli tüm sürücülerin etkinleştirilmesini sağlar.

İlk olarak, mevcut çekirdeğinizin yapılandırma dosyasını bulun. Genellikle `/proc/config.gz` adresinde bulunur (eğer etkinleştirilmişse) veya `/boot` dizininde. Ubuntu/Debian'da şunu yapabilirsiniz:

```bash
zcat /proc/config.gz > .config
```

Eğer bu dosya yoksa, şunu deneyin:

```bash
cp /boot/config-$(uname -r) .config
```

Artık çalışan çekirdeğinizle eşleşen bir `.config` dosyanız var.

### 4.3 Yerel Sürümü (Local Version) Değiştirme

Özel çekirdeğimize farklı bir isim vermek istiyoruz, böylece daha sonra kolayca tanımlayabiliriz. `.config` dosyasını düzenleyin ve şu satırı bulun:

```
CONFIG_LOCALVERSION=""
```

Bunu şu şekilde değiştirin:

```
CONFIG_LOCALVERSION="-kendiadiniz"
```

**Kendi adınızı, öğrenci numaranızı veya boşluksuz herhangi bir metni kullanabilirsiniz.** Bu, `uname -r` çıktısında görünecektir.

Dosyayı düzenlemek için `nano .config` kullanabilir ve satırı bulabilirsiniz (nano'da arama yapmak için `Ctrl+W` tuşlayın).

### 4.4 (İsteğe Bağlı) İnce Ayar İçin `menuconfig` Kullanımı

Kernel seçeneklerini keşfetmek ve etkileşimli olarak değiştirmek isterseniz, şunu çalıştırın:

```bash
make menuconfig
```

Bu, metin tabanlı bir menü açar. Şimdilik herhangi bir şeyi değiştirmenize gerek yok – sadece çıkın ve kaydedin. Gerekirse `.config` dosyasını güncelleyecektir.

---

## Bölüm 5: Çekirdeği Derleme ve Kurma

Çekirdeği derlemek zaman alır (VM'inizin kaynaklarına bağlı olarak 15-60 dakika). Paralel olarak derlemek için `-j` bayrağını kullanacağız.

### 5.1 Çekirdeği ve Modülleri Derleme

Çekirdek kaynak dizininde (`~/linux-6.12`), şunu çalıştırın:

```bash
make -j$(nproc)
```

- `$(nproc)` CPU çekirdeği sayısını döndürür. `-j$(nproc)` kullanmak, `make`'e bu kadar sayıda paralel iş çalıştırmasını söyler, bu da derlemeyi hızlandırır.

Eksik sertifikalarla ilgili hatalar alırsanız, bunları devre dışı bırakabilirsiniz (aşağıdaki Sorun Giderme bölümüne bakın).

### 5.2 Modülleri Kurma

Derleme tamamlandıktan sonra, çekirdek modüllerini kurun:

```bash
sudo make modules_install
```

Bu, derlenmiş modülleri `/lib/modules/6.12.0-benimlinux/` dizinine kopyalar (sürüm numarası, yerel sürümünüzle birlikte çekirdeğinizle eşleşecektir).

### 5.3 Çekirdek Görüntüsünü Kurma

Ardından, çekirdek görüntüsünün kendisini kurun:

```bash
sudo make install
```

Bu, çekirdek görüntüsünü (`vmlinuz`) `/boot` dizinine kopyalar, bir `initramfs` (ilk RAM disk) oluşturur ve önyükleyici yapılandırmasını günceller.

Eğer `make install` önyükleyiciyi otomatik olarak güncellemezse, şunu çalıştırmanız gerekebilir:

```bash
sudo update-grub
```

### 5.4 Yeniden Başlatma ve Çekirdeğinizi Seçme

Şimdi VM'i yeniden başlatın:

```bash
sudo reboot
```

GRUB menüsü göründüğünde (görmek için önyükleme sırasında `Shift` tuşunu basılı tutmanız gerekebilir), **Advanced options for Ubuntu** (veya benzeri) seçeneğini seçin ve ardından `-benimlinux` sonekine sahip çekirdeğinizi seçin. Menüyü görmüyorsanız, zaman aşımını artırmak için `/etc/default/grub` dosyasını düzenleyebilirsiniz.

Giriş yaptıktan sonra, özel çekirdeğinizde çalıştığınızı doğrulayın:

```bash
uname -r
```

`6.12.0-benimlinux` gibi bir şey görmelisiniz.

Çekirdek günlüğünde "Linux version" satırınızı kontrol edin:

```bash
dmesg | grep "Linux version"
```

---

## 6. Sık Karşılaşılan Hatalar ve Çözümleri

### 6.1 “No rule to make target ‘debian/certs/…’ ” (Sertifika hataları)

Derleme sırasında eksik sertifikalarla ilgili hatalar görürseniz, güvenilen anahtarları devre dışı bırakabilirsiniz. Şunu çalıştırın:

```bash
scripts/config --disable SYSTEM_TRUSTED_KEYS
scripts/config --disable SYSTEM_REVOCATION_KEYS
```

Ardından derlemeyi yeniden başlatın (`make -j$(nproc)`). Değişiklikler `.config` dosyasına kaydedilecektir.

### 6.2 Eksik Kütüphaneler veya Başlık Dosyaları

`make` eksik başlık dosyaları veya kütüphanelerden şikayet ederse, bunları `apt` ile kurun. Örneğin:

```bash
sudo apt install libelf-dev libssl-dev
```

Ardından tekrar deneyin.

### 6.3 Önyüklemede Çekirdek Panigi / VFS Hatası

Yeni çekirdek önyükleme yapmazsa ve bir çekirdek paniği alırsanız (örneğin “VFS: Unable to mount root fs”), initramfs'i manuel olarak yeniden oluşturmanız gerekebilir:

```bash
sudo update-initramfs -c -k 6.12.0-benimlinux   # tam çekirdek sürümünüzü kullanın
sudo update-grub
```

Ardından yeniden başlatın.

### 6.4 Derleme Çok Uzun Sürüyor

VM'iniz yavaşsa, ona daha fazla CPU çekirdeği ve RAM vermeyi düşünün. Ayrıca, sistemi aşırı yüklememek için `-j$(nproc)` yerine `make -j2` kullanabilirsiniz.

---

## 7. Teslim Gereksinimleri

Bu projeden not almak için aşağıdaki öğeleri **tek bir PDF veya resim seti** olarak gönderin (VM'de ekran görüntüsü alabilirsiniz):

1. **`uname -r` ekran görüntüsü** – özel çekirdek sürümünüzü gösteren (örneğin `6.12.0-benimlinux`).
2. **`dmesg | grep "Linux version"` ekran görüntüsü** – özel sürümünüzle çekirdek önyükleme mesajını gösteren.
3. **`ls /boot` ekran görüntüsü** – kurulu çekirdek dosyalarını gösteren (vmlinuz, initrd.img, System.map).
4. **`/etc/fstab` ekran görüntüsü** (sadece dosyalarda gezinebildiğinizi ve görüntüleyebildiğinizi göstermek için).
5. Kullandığınız **`.config` dosyası** (ayrı bir dosya olarak yükleyin veya içeriğini PDF'e ekleyin).

Teslimatı dersin teslim sistemi üzerinden yapın (örneğin Google Classroom). Tüm grup üyeleri aynı çalışmayı teslim etmelidir, ancak herkes adımları kendi VM'inde uygulamalıdır.

---

## Ek Kaynaklar

- [Linux Kernel Newbies](https://kernelnewbies.org/) – çekirdek geliştirmeyi öğrenen insanlar için topluluk.
- [Make Tutorial](https://makefiletutorial.com/) – `make` için mükemmel bir rehber.
- [Linux Command Line Basics](https://ubuntu.com/tutorials/command-line-for-beginners) – Ubuntu'dan.

