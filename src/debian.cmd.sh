#!/bin/bash

#https://www.debian.org/doc/manuals/debian-reference/    debian官方教程
#https://debian-handbook.info/browse/zh-CN/stable/index.html    debian管理员手册

#让apt-get支持https开头的软件源
sudo apt-get install apt-transport-https


#普通用户使用sudo命令, 不再需要输入密码:
#把下面的配置写入到 /etc/sudoers, yourname替换为自己的用户名, 下同
yourname	ALL=(ALL) NOPASSWD: NOPASSWD: ALL

#查看debian的版本号
cat /etc/debian_version

#NFS服务器搭建, 以下命令均在root账号下运行,
#否则系统会提示没有权限("/nfsroot", 可替换为自己设置的目录)
apt-get install rpcbind nfs-kernel-server nfs-common -y
echo "/nfsroot    *(rw,sync,no_root_squash)" >> /etc/exports
mkdir -p /nfsroot
chmod 777 -R /nfsroot
/etc/init.d/rpcbind restart
/etc/init.d/nfs-kernel-server restart
mount -t nfs -o nolock server:/nfsroot /mnt

sudo apt-get install linux-headers-$(uname -r) dkms caja-open-terminal git vim cscope ctags chromium build-essential -y

#git alias
git config --global alias.co checkout
git config --global alias.br branch
git config --global alias.ci commit
git config --global alias.st status
git config --global alias.lg "log --color --graph --pretty=format:'%Cred%h%Creset -%C(yellow)%d%Creset %s %Cgreen(%cr) %C(bold blue)<%an>%Creset' --abbrev-commit"

#virtualbox
echo "deb https://download.virtualbox.org/virtualbox/debian stretch contrib">>/etc/apt/sources.list
wget -q https://www.virtualbox.org/download/oracle_vbox_2016.asc -O- | sudo apt-key add -
wget -q https://www.virtualbox.org/download/oracle_vbox.asc -O- | sudo apt-key add -
apt update
sudo apt-get install virtualbox-5.2
sudo usermod -a -G vboxsf yourname

#add i386 support
dpkg --print-architecture
dpkg --add-architecture i386
apt install lib32z1 lib32ncurses5 gcc-multilib firmware-realtek

#安装6.828开发环境
apt install libgmp-dev libmpfr-dev libmpc-dev binutils pkg-config autoconf automake libtool
apt install libsdl1.2-dev libtool-bin libglib2.0-dev libz-dev libpixman-1-dev
git clone http://web.mit.edu/ccutler/www/qemu.git
./configure --disable-kvm --prefix=/opt/qemu --target-list="i386-softmmu x86_64-softmmu"
make
sudo make install

