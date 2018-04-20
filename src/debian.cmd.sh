#!/bin/bash

#https://www.debian.org/doc/manuals/debian-reference/    debian官方教程
#https://debian-handbook.info/browse/zh-CN/stable/index.html    debian管理员手册

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
git config --global alias.lg "log --color --graph --pretty=format:'%Cred%h%Creset -%C(yellow)%d%Creset %s %Cgreen(%cr) %C(bold blue)<%an>%Creset' --abbrev-commit"

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
sudo apt install  -y libgmp-dev libmpfr-dev libmpc-dev binutils pkg-config autoconf automake libtool
sudo apt install  -y libsdl1.2-dev libtool-bin libglib2.0-dev libz-dev libpixman-1-dev
git clone http://web.mit.edu/ccutler/www/qemu.git
./configure --disable-kvm --prefix=/opt/qemu --target-list="i386-softmmu x86_64-softmmu"
make
sudo make install

#Linux自字体渲染
sudo apt install dirmngr
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

