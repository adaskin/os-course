## **HW0: Downloading-compiling-building linux kernel**

Due date Sept 31, 2024

[Önceden Bilinmesi Gerekenler](#heading=)

[Öğrenme Hedefleri](#heading=)

[Kaynaklar](#heading=)

[Kernel indirme](#heading=)

[indirilen Kernel konfigürasyonunun yapılması](#heading=)

[.config dosyasi olusturma](#.config-dosyasi-olusturma)

[.config dosyasinda yapilacak degisiklikler](#.config-dosyasinda-yapilacak-degisiklikler)

[Kernel derleme ve yükleme adımları](#heading=)

[Debian (Ubuntu vb)](#heading=)

[Archlinux icin](#heading=)

[make’le ilgili açıklamalar](#heading=)

[Bazı Kernel Compile/Deploy Hataları](#heading=)

[Teslim](#teslim)

# **Önceden Bilinmesi Gerekenler**

* Linux terminal

# **Öğrenme Hedefleri**

* Compiling Linux kernel

# **Kaynaklar**

Internette bir suru kernel compile etmeyle ilgili kaynak bulabilirsiniz mesela; 

* [How to Compile a Linux Kernel](https://www.linux.com/topic/desktop/how-compile-linux-kernel-0/)   
* [Kernel/BuildYourOwnKernel \- Ubuntu Wiki](https://wiki.ubuntu.com/Kernel/BuildYourOwnKernel) 

# **Kernel indirme**

* Önce kendi sisteminize bi zarar vermemek için, [VMware](https://www.vmware.com/) yada [VirtualBox](https://www.virtualbox.org/wiki/Downloads) yuklemenizi tavsiye ederim

  * macOS M1 ve M2 ler icin virtualbox Apple Silicon versiyonu calisiyor. Ancak M3ler icin qemu kullanmanizi tavsiye ederim.

    * utm(qemu) indirmek icin [https://mac.getutm.app/](https://mac.getutm.app/)

    * Yine Apple chipler icin icin indireceginiz linux kernel versiyonunun **ARM64** olmasina dikkat edin.

* sonra indirdiğiniz .iso dosyasını kullanarak linuxu (debian,ubuntu, archlinux, kali, fedora, opensuse, ne olursa artik) virtualbox ile yeni bir makine tanımlayarak yukleyin

  * Eğer Linux-terminal bilginiz iyi ise, GUI olmadan ve minimal bir şekilde yükleyebilirsiniz (çalıştırması daha hızlı olur).

* kernel compile etmek icin gerekli libraryleri yukleyin [minimal requirements](https://www.kernel.org/doc/html/next/process/changes.html)

* VirtualBox’ta kurduğunuz linuxu calistirarak once [**https://www.kernel.org/**](https://www.kernel.org/)   dan veya

```git clone git://kernel.ubuntu.com/ubuntu/ubuntu-\<release codename\>.git ```


  kullanarak linux kernel indirin(isterseniz sisteminizde kullandiginiz kernel ile ayni versiyon olabilir).

  * Sisteminizde hali hazırda çalışan kernel versiyonunu gormek icin

``uname -r ``


* Aşağıdaki komutlada indirebilirsiniz, o zaman /usr/src/ klasörüne indiriyor.

```sudo apt-get source linux-source```

* indirme işlemini terminal yada browser üzerinden yapabilirsiniz Eger terminalde deb-src not in source.list benzeri bir hata alirsaniz, /etc/apt/sources.list dosyasinda deb-srclari aktif etmeniz gerekiyor. Bunu grafiksel (software-properties-gtk) veya terminal uzerinden editorle acarak yada asagidaki sekilde yapabilirsiniz:

```bash
sudo sed -Ei 's/^\# deb-src /deb-src /' /etc/apt/sources.list
sudo apt-get update
```

* Terminalde **wget, curl**, vsde kullanabilirsiniz.

| wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.11.tar.xz |
| :---- |


## **indirilen Kernel konfigürasyonunun yapılması**

* tar dosyasını bir klasöre çıkarın

| tar xvf linux-indirdiginizversion |
| :---- |

* indirdiginizversiyon 6.1.0-25-amd64 benzeri bir sey olmasi gerekiyor.

* sonra bu klasöre terminalden cd ile icerisine giriniz (**path ve directory isimlerinde boşluk, Türkçe karakter, noktalama işaretleri vs olmasın-hata alirsiniz**),

  cd linux-indirdiginizversiyon

## **.config dosyasi olusturma** {#.config-dosyasi-olusturma}

[https://www.kernel.org/doc/Documentation/admin-guide/README.rst](https://www.kernel.org/doc/Documentation/admin-guide/README.rst) 

**Burada farkli secenekler kullanabilirsiniz:**

* kendi mevcut kerneldaki config dosyasini kopyalamak ve bunun uzerinden gitmek icin, once mevcut sistemde kendi config dosyanizi bulun. Sisteme bagli olarak config dosyasi farkli yerlerde olabilir (ubuntuda bootun icerisinde)

| /proc/config.gz/boot/config/boot/config-$(uname \-r) |
| :---- |


  Kopyalama yapmak icin

| zcat /proc/config.gz \> .config |
| :---- |

  Eger config uzantisi gz degilse normal cat komutu kullanabilirsiniz

* Sistemdeki mevcut config filedan kendi oluşturacağınız kernelın config file’ını düzenlemek için

| yes "" | make oldconfig |
| :---- |

* Yada sistemdekini aynen kopyalamak için (yukaridakinin aynisi)

| make olddefconfig |
| :---- |

* minimal bir kernel olusturmak icin**(baslatmasi biraz problem olabilir)**

| make tinyconfig  |
| :---- |

## 

## **.config dosyasinda yapilacak degisiklikler** {#.config-dosyasinda-yapilacak-degisiklikler}

.config içerisinde aşağıdaki şekilde versiyon ismini değiştirin( kernel sistemde eklendiğinde bu isimle eklenecek)

* Edit CONFIG\_LOCALVERSION="..."

| CONFIG\_LOCALVERSION="-kendiisminiz" |
| :---- |

  * grup ismi vsde verebilirsiniz. Bosluk vb karakterler kullanmayiniz.

  * Not: ilerde compile esnasında “deb..cert.perm” bulunamadi seklinde sertifikalarla ilgili hata verirse .config dosyasından mesajda gecen debian.. cert.permi silerek sadece bos birakin (**veya /usr/src/linux-.. icerisindeki debian directoryi kopyalayin**)

| CONFIG\_SYSTEM\_TRUSTED\_KEYS \= ""SYSTEM\_REVOCATION\_KEYS \= "" |
| :---- |

Yada terminal üzerinden

| scripts/config \--disable SYSTEM\_TRUSTED\_KEYSscripts/config \--disable SYSTEM\_REVOCATION\_KEYS |
| :---- |

# **Kernel derleme ve yükleme adımları**

Aşağıda sadece ubuntu/debian ve arch linux icin verildi; diger linuxler icin benzer adimlari web de bulabilirsiniz.

## **Debian (Ubuntu vb)**

| \# Compile the kernel$ make \-j4 \# Compile and install modules \# eger modullerden cok fazlasini aktif ettiyseniz, \# olusacak kernel size vboxta tanimli makine icin cok buyuk olabilir\# make menuconfig ile yuklenecek modulleri belirleyebilirsinizmake \-j4 modules\_install\# kernelin /boot ve grub menuye yuklenmesimake install\# gerekli olabilirupdate-initramfs \-c \-k versionupdate-grub  |
| :---- |

## 

## **Archlinux icin**

| \# Change this if you'd like. It has no relation\# to the suffix set in the kernel config.SUFFIX="-kendiisminiz"\# Compile the kernelmake \-j4\# Compile and install modulesmake \-j4 modules\_install\# Install kernel imagecp arch/x86\_64/boot/bzImage /boot/vmlinuz-linux$SUFFIX\# Create preset and build initramfssed s/linux/linux$SUFFIX/g \\    \</etc/mkinitcpio.d/linux.preset \\    \>/etc/mkinitcpio.d/linux$SUFFIX.presetmkinitcpio \-p linux$SUFFIX\# Update bootloader entries with new kernels.grub-mkconfig \-o /boot/grub/grub.cfg |
| :---- |

Eğer hata almadıysanız kernel başarıyla eklendi (sudo update-grub vermiş olduğu outa bakarak güncel listeyi görebilirsiniz).

Sistemi tekrar baslatin( reboot) ve grub menüden oluşturduğunuz kerneli seçin (ESCye basarak, grub da “advance options” tan kendiisminiz suffixli versiyonu seçin).

# **make’le ilgili açıklamalar**

* make yazdığınız kernel’ı verilen config dosyasındaki parametreleri kullanarak derlemesini ve linklemesini yapıyor. Bu derleme sonucunda size **vmlinuz** isminde bir tek dosya oluşturuyor.

* make modules:Zaten make çalıştırılınca ayrıca çalıştırılması gerekmiyor. Temelde modüllerin derlemesini yapıyor. Bu modüller kernel config oluşturulurken (make menuconfig, oldconfig vs ile config dosyası oluşturulabiliyor) M olarak cevaplanmış dosyalar olmaktadır. Yine Y olarak cevapladıklarınız vmlinuz dosyasının bir parçası olmaktadırlar.

* make modules\_install:Derlenen kernel modüllerini (derlenmediyse derlemesini yapıyor) /lib/modules/versiyona yüklüyor (yani kernelin modül directorysine).

* make install oluşturulan kernel image vmlinuz dosyasini /boot’a kopyaliyor.

Not: Şuanki debianda kernel’i boot menüye otomatik olarak ekleme yapıyor ancak eğer yapmazsa

| update-initramfs \-c \-k kendiversiyonunuzupdate-grub |
| :---- |

## **Bazı Kernel Compile/Deploy Hataları**

Kernel derlenirken bazen makinede yüklü librarylerle, kernel icin gerekli libraryler arasında uyumsuzluk olabilir. Bu sebeple derleme esnasında farkli hatalar alabilirsiniz( özellikle son kernel sürümlerinde bunlardan bazilari raporlanmis bazilari raporlanmamış bugda olabilir)

Hataları indirgemek için aşağıdaki adımları takip etmeniz mevcut makine kütüphanelerini uyumlu hale getirmeniz açısından önem arz etmektedir.

| sudo apt updatesudo apt upgrade |
| :---- |

* sonra su siteye giderek elinizdeki kernel icin gereken minimal requirementslari kontrol edin ve makinedeki uyusmayan libraryleri guncelleyin [https://www.kernel.org/doc/html/latest/process/changes.html](https://www.kernel.org/doc/html/latest/process/changes.html#)

  * burada **latest** yerine, indirdiginiz kernel versiyonuna gore degistirerek o siteye girebilirsiniz.

* sertifikalarla ilgili hata verirse (“deb..cert.perm” bulanamadi seklinde) .config dosyasindan mesajda gecen debian.. cert.permi silerek sadece bos birakin .... \=" ". Terminalden 

| scripts/config \--disable SYSTEM\_TRUSTED\_KEYS \\scripts/config \--disable SYSTEM\_REVOCATION\_KEYS |
| :---- |


* $ makeden sonra library eksikliklerinden gelen hatalari

| sudo apt install "ilgili library" |
| :---- |

* ile yükleyerek giderebilirsiniz (sonra tekrar make’den devam edin).

* Hatayi internetten aratarak hangi library eksik olduğunu bulabilirsiniz.

* make cok zaman alabilir, ancak make clean yapmadiginiz muddetce sonraki make daha hizli olacaktir.

* Eger yuklediginizde baslatirken kernel panic VFS file system hatasi aliyorsaniz asagidakini deneyin

| update-initramfs \-c \-k kendiversiyonunuzupdate-grub |
| :---- |

Kendiversiyonunuz, yuklediginiz linux versiyonun tam ismi **6.2.0-34-generic-kendiisminiz** gibi

* Diğer raporlanan hataları buraya eklemeye devam edeceğim

# **Teslim** {#teslim}

Once kendiveriyonunuzda sistemi baslatarak asagidaki komutlarin ekran goruntulerini resim olarak yukleyiniz.

| uname \-r |
| :---- |

| sudo dmesg | grep "Linux" |
| :---- |

| sudo update-grub |
| :---- |

| sudo ls /boot |
| :---- |

| sudo cat /etc/fstab |
| :---- |

**Ve olustudugunuz source dosyasi icerisindeki .config dosyasini yukleyiniz.** 

* **Tum grup uyeleri teslim etsin\!**