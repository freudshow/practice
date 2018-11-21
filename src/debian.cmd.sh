#!/bin/bash

#https://www.debian.org/doc/manuals/debian-reference/    debian官方教程
#https://debian-handbook.info/browse/zh-CN/stable/index.html    debian管理员手册

#中科大软件源
#Tips: Remember to install package <apt-transport-https>

deb https://mirrors.ustc.edu.cn/debian/ stretch main contrib non-free
deb-src https://mirrors.ustc.edu.cn/debian/ stretch main contrib non-free

deb https://mirrors.ustc.edu.cn/debian/ stretch-updates main contrib non-free
deb-src https://mirrors.ustc.edu.cn/debian/ stretch-updates main contrib non-free

deb https://mirrors.ustc.edu.cn/debian/ stretch-backports main contrib non-free
deb-src https://mirrors.ustc.edu.cn/debian/ stretch-backports main contrib non-free

deb https://mirrors.ustc.edu.cn/debian-security/ stretch/updates main contrib non-free
deb-src https://mirrors.ustc.edu.cn/debian-security/ stretch/updates main contrib non-free

#让apt-get支持https开头的软件源
sudo apt-get install -y apt-transport-https


#普通用户使用sudo命令, 不再需要输入密码:
#把下面的配置写入到 /etc/sudoers, yourname替换为自己的用户名, 下同
yourname	ALL=(ALL) NOPASSWD: NOPASSWD: ALL

#查看debian的版本号
cat /etc/debian_version

#NFS服务器搭建, 以下命令均在root账号下运行,
#否则系统会提示没有权限("/nfsroot", 可替换为自己设置的目录)
sudo apt-get install -y rpcbind nfs-kernel-server nfs-common
echo "/nfsroot    *(rw,sync,no_root_squash)" >> /etc/exports
mkdir -p /nfsroot
chmod 777 -R /nfsroot
/etc/init.d/rpcbind restart
/etc/init.d/nfs-kernel-server restart
mount -t nfs -o nolock localhost:/nfsroot /mnt

sudo apt-get install linux-headers-$(uname -r) dkms caja-open-terminal git vim cscope ctags chromium build-essential -y

#git alias
git config --global user.name "s_baoshan"
git config --global user.email "s_baoshan@163.com"
git config --global alias.co checkout
git config --global alias.br branch
git config --global alias.ci commit
git config --global alias.st status
git config --global core.filemode false
git config --global alias.lg "log --color --graph --pretty=format:'%Cred%h%Creset -%C(yellow)%d%Creset %s %Cgreen(%cr) %C(bold blue)<%an>%Creset' --abbrev-commit"

#把 gw 分支下的libevent/event.h合并到dev_save分支下的libevent/event.h
git checkout dev_save
git checkout --patch gw libevent/event.h
git checkout --patch gw libevent/event.c

#同步远程分支列表
git remote update origin --prune

#virtualbox
echo "deb https://download.virtualbox.org/virtualbox/debian stretch contrib">>/etc/apt/sources.list
wget -q https://www.virtualbox.org/download/oracle_vbox_2016.asc -O- | sudo apt-key add -
wget -q https://www.virtualbox.org/download/oracle_vbox.asc -O- | sudo apt-key add -
sudo apt update
sudo apt-get install -y virtualbox-5.2 libqt5core5a libqt5widgets5 libqt5x11extras5 libssl1.1 libvpx4 libsdl-ttf2.0-0
sudo usermod -a -G vboxsf $(whoami)

#add i386 support
sudo apt install -y firmware-realtek
sudo dpkg --print-architecture
sudo dpkg --add-architecture i386
sudo apt install  -y lib32z1 lib32ncurses5 gcc-multilib

#安装6.828开发环境
sudo apt install  -y libgmp-dev libmpfr-dev libmpc-dev binutils pkg-config autoconf automake libtool zlib1g-dev
sudo apt install  -y libsdl1.2-dev libtool-bin libglib2.0-dev libz-dev libpixman-1-dev
git clone http://web.mit.edu/ccutler/www/qemu.git
./configure --disable-kvm --prefix=/opt/qemu --target-list="i386-softmmu x86_64-softmmu"
make
sudo make install

#apue.3e cannot find -lbsd
sudo apt install -y libbsd-dev

#Linux自字体渲染
sudo apt install -y dirmngr
echo "deb http://ppa.launchpad.net/no1wantdthisname/ppa/ubuntu xenial main" | sudo tee /etc/apt/sources.list.d/infinality.list
echo "deb-src http://ppa.launchpad.net/no1wantdthisname/ppa/ubuntu xenial main" | sudo tee -a /etc/apt/sources.list.d/infinality.list
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys E985B27B
#执行以下命令来升级你的系统并安装 Infinality 包：
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install -y fontconfig-infinality

#以太网和wifi同时上网
route
echo "#!/bin/bash">/etc/NetworkManager/dispatcher.d/02myroutes
echo "sudo route del -net default netmask 0.0.0.0 dev enp7s0">>/etc/NetworkManager/dispatcher.d/02myroutes
echo "sudo route add -net 192.168.0.0 netmask 255.255.0.0 gw 192.168.0.1 dev enp7s0">>/etc/NetworkManager/dispatcher.d/02myroutes

#Vim配置
git clone https://github.com/amix/vimrc.git
git clone --depth=1 https://github.com/amix/vimrc.git ~/.vim_runtime
sh ~/.vim_runtime/install_awesome_vimrc.sh

#windows xp sn
MRX3F-47B9T-2487J-KWKMF-RPWBY

#Unable to open serial port /dev/ttyUSB0
sudo echo "KERNEL==\"ttyUSB[0-9]*\", MODE=\"0666\"">/etc/udev/rules.d/70-ttyusb.rules

#delete by inode
ls -il
find ./ -inum
find ./ -inum 277191 -exec rm -i {} \;

#tftp Sever
sudo apt install -y tftpd-hpa tftp
just use Sample configuration
service tftpd-hpa status
service tftpd-hpa stop
service tftpd-hpa start
service tftpd-hpa restart
service tftpd-hpa force-reload
mkdir -p /srv/tftp
sudo chmod 777 /srv/tftp -R

#get tftp files in arm board
tftp tftp-server-ip -g -r remotefile

#install LaTex
sudo apt-get install texlive-full texmaker -y

test tex:
\documentclass{article}
\begin{document}
    Hello, world!
\end{document}

#eclipse  cp  $ProjName
arm-none-linux-gnueabi-strip ${ProjName}&&cp ${ProjName} ../../bin_arm/
.project中<natures>一节,  <nature>org.eclipse.cdt.core.ccnature</nature>定义了项目类型是C++

#install  graphviz
sudo apt install -y graphviz
dot -version  #查看graphviz版本
dot -Tpng sample.dot -o sample.png  #编译成png图
dot -Tsvg sample.dot -o sample.png  #编译成png图

arm-none-linux-gnueabi-strip ${ProjName} ;cp ${ProjName} ../../bin_arm;
echo "#define GL_VERSION \\" > ../../inc/version.h; git log | grep -e 'commit [a-zA-Z0-9]*' | wc -l  >> ../../inc/version.h

#ttyUSB0 permission
sudo chmod 666 /dev/ttyUSB0
sudo gpasswd --add floyd dialout
sudo echo "KERNEL=="ttyUSB0", GROUP="username", MODE="0666"">/etc/udev/rules.d/50-usb-serial.rules
sudo /etc/init.d/udev restart or reboot

#CGAL
# Debian or Linux Mint
sudo apt-get install  -y libcgal-dev  -y# install the CGAL library
sudo apt-get install  -y libcgal-demo  -y# install the CGAL demos


#install chrome
wget https://dl.google.com/linux/direct/google-chrome-stable_current_amd64.deb

#look directory's size
du -sh .



#Ubuntu下配置Common Lisp开发环境
sudo apt-get install emacs -y
#安装 Common Lisp 环境
sudo apt-get install common-lisp-controller -y
#安装 Slime
sudo apt-get install slime -y
#修改 Emacs 配置文件，以支持 Common Lisp
emacs -nw ~/.emacs.d/user.el

(setq inferior-lisp-program "/usr/bin/sbcl")
    (add-to-list 'load-path "/usr/local/bin/slime/")
    (require 'slime)
    (slime-setup)
(slime-setup '(slime-fancy))
'
#验证开发环境
emacs 或 emacs -nw
输入 Alt + X，输入 slime，回车

#mount ftpfs
sudo apt install curlftpfs -y
mkdir -p ~/ftpfs
curlftpfs ftp://root:1@192.168.0.4 /home/floyd/ftpfs/

#pppoe-server
sudo apt-get install pppoe pppoeconf
sudo echo "\"user\" * \"123\" *">/etc/pap-secrets
sudo echo "\"user\" * \"123\" *">/etc/chap-secrets
sudo echo "login">/etc/ppp/pppoe-server-options
sudo echo "lcp-echo-interval 30">>/etc/ppp/pppoe-server-options
sudo echo "lcp-echo-failure 4">>/etc/ppp/pppoe-server-options
sudo echo "ms-dns  202.118.224.101">>/etc/ppp/pppoe-server-options
modprobe pppoe
sudo pppoe-server -I enp9s0 -L 192.168.13.1 -R 192.168.13.100 -N 333
sudo pppoe-server -I eno1 -L 192.168.13.1 -R 192.168.13.100 -N 333

#git clone 时显示Filename too long的解决办法
git config --global core.longpaths true

#debian中查找已安装软件及卸载软件
dpkg -l | grep -i name
apt-get remove name

#make update.sh
./tmake -t 2 -l ZheJiang --cmd "cj event set 3106 06 01 {1,4320,5,1,1320,1760};cj event enable 3106 1 1;cj dev set f101 0 0;cp /nand/event/property/3106/* /nor/init/;ifconfig eth0 192.168.0.4 netmask 255.255.255.0 up;echo \"ifconfig eth0 192.168.0.4 netmask 255.255.255.0 up\">/nor/rc.d/ip.sh" -v

./tmake -t 2 -l ShanDong -v --cmd "cj ip 192.168.1.9:8889 192.168.1.9:8889;cj para set 4510 gw \"192.168.1.177\";cj para set 4510 mask \"255.255.255.255\""

#cat /etc/fstab , UUID can be got with cmd: 'ls /dev/disk/by-uuid/'
# /etc/fstab: static file system information.
#
# Use 'blkid' to print the universally unique identifier for a
# device; this may be used with UUID= as a more robust way to name devices
# that works even if disks are added and removed. See fstab(5).
#
# <file system> <mount point>   <type>  <options>       <dump>  <pass>
# / was on /dev/sda3 during installation
UUID=790013c3-2d51-432b-b1e7-5929503eb15d /               ext4    errors=remount-ro 0       1
# /home was on /dev/sda5 during installation
UUID=21206b36-e29b-4758-8982-c6206e46e442 /home           ext4    defaults        0       2
# swap was on /dev/sda6 during installation
UUID=314d0826-7f4a-45aa-bc9b-047afddb34e0 none            swap    sw              0       0
/dev/sr0        /media/cdrom0   udf,iso9660 user,noauto     0       0
UUID=000C03AB00086F92 /win/c          ntfs    rw        0       2
UUID=B8745094745056EA /win/d          ntfs    rw        0       2

#如何在快速定位Debian系统软件的安装路径信息
$ dpkg -l | grep openjdk
ii  openjdk-8-jdk:amd64                           8u181-b13-1~deb9u1                          amd64        OpenJDK Development Kit (JDK)
ii  openjdk-8-jdk-headless:amd64                  8u181-b13-1~deb9u1                          amd64        OpenJDK Development Kit (JDK) (headless)
ii  openjdk-8-jre:amd64                           8u181-b13-1~deb9u1                          amd64        OpenJDK Java runtime, using Hotspot JIT
ii  openjdk-8-jre-headless:amd64                  8u181-b13-1~deb9u1                          amd64        OpenJDK Java runtime, using Hotspot JIT (headless)

#知道了包的准确名字, 就可以定位了
$ dpkg -L openjdk-8-jdk
/.
/usr
/usr/lib
/usr/lib/jvm
/usr/lib/jvm/java-8-openjdk-amd64
/usr/lib/jvm/java-8-openjdk-amd64/bin
/usr/lib/jvm/java-8-openjdk-amd64/bin/appletviewer
/usr/lib/jvm/java-8-openjdk-amd64/bin/jconsole
/usr/lib/jvm/java-8-openjdk-amd64/include
/usr/lib/jvm/java-8-openjdk-amd64/include/jawt.h
/usr/lib/jvm/java-8-openjdk-amd64/include/linux
/usr/lib/jvm/java-8-openjdk-amd64/include/linux/jawt_md.h
/usr/lib/jvm/java-8-openjdk-amd64/lib
/usr/lib/jvm/java-8-openjdk-amd64/lib/jconsole.jar
/usr/lib/jvm/java-8-openjdk-amd64/man
/usr/lib/jvm/java-8-openjdk-amd64/man/ja_JP.UTF-8
/usr/lib/jvm/java-8-openjdk-amd64/man/ja_JP.UTF-8/man1
/usr/lib/jvm/java-8-openjdk-amd64/man/ja_JP.UTF-8/man1/appletviewer.1.gz
/usr/lib/jvm/java-8-openjdk-amd64/man/ja_JP.UTF-8/man1/jconsole.1.gz
/usr/lib/jvm/java-8-openjdk-amd64/man/man1
/usr/lib/jvm/java-8-openjdk-amd64/man/man1/appletviewer.1.gz
/usr/lib/jvm/java-8-openjdk-amd64/man/man1/jconsole.1.gz
/usr/share
/usr/share/doc
/usr/share/doc/openjdk-8-jre-headless
/usr/share/doc/openjdk-8-jre-headless/test-amd64
/usr/share/doc/openjdk-8-jre-headless/test-amd64/check-hotspot-hotspot.log.gz
/usr/share/doc/openjdk-8-jre-headless/test-amd64/check-langtools-hotspot.log.gz
/usr/share/doc/openjdk-8-jre-headless/test-amd64/failed_tests-hotspot.tar.gz
/usr/share/doc/openjdk-8-jre-headless/test-amd64/jtreg-summary-hotspot.log.gz
/usr/share/doc/openjdk-8-jdk
