#!/bin/bash
# Download and extract all evaluation packages.
# Each wget is followed by the matching extraction command.

wget https://ftp.gnu.org/gnu/bash/bash-5.3.tar.gz
tar -xzf bash-5.3.tar.gz

wget https://ftp.gnu.org/gnu/bc/bc-1.08.2.tar.gz
tar -xzf bc-1.08.2.tar.gz

wget https://ftp.gnu.org/gnu/bison/bison-3.8.2.tar.gz
tar -xzf bison-3.8.2.tar.gz

wget https://ftp.gnu.org/gnu/emacs/emacs-30.2.tar.gz
tar -xzf emacs-30.2.tar.gz

wget https://ftp.gnu.org/gnu/enscript/enscript-1.6.6.tar.gz
tar -xzf enscript-1.6.6.tar.gz

git clone https://github.com/westes/flex.git

git clone https://github.com/fvwmorg/fvwm3.git

wget https://ftp.gnu.org/gnu/gawk/gawk-5.4.0.tar.gz
tar -xzf gawk-5.4.0.tar.gz

wget https://downloads.sourceforge.net/project/gnuplot/gnuplot/6.0.3/gnuplot-6.0.3.tar.gz
tar -xzf gnuplot-6.0.3.tar.gz

wget https://ftp.gnu.org/gnu/gv/gv-3.7.4.tar.gz
tar -xzf gv-3.7.4.tar.gz

wget https://ftp.gnu.org/gnu/gzip/gzip-1.14.tar.gz
tar -xzf gzip-1.14.tar.gz

wget https://ftp.gnu.org/gnu/m4/m4-1.4.21.tar.gz
tar -xzf m4-1.4.21.tar.gz

wget https://ftp.gnu.org/gnu/rcs/rcs-5.10.1.tar.lz
tar --lzip -xf rcs-5.10.1.tar.lz

wget https://ftp.gnu.org/gnu/groff/groff-1.24.0.tar.gz
tar -xzf groff-1.24.0.tar.gz

wget https://ftp.gnu.org/gnu/dejagnu/dejagnu-1.6.3.tar.gz
tar -xzf dejagnu-1.6.3.tar.gz

wget https://www.cpan.org/src/5.0/perl-5.42.0.tar.gz
tar -xzf perl-5.42.0.tar.gz

wget https://downloads.sourceforge.net/project/zsh/zsh/5.9/zsh-5.9.tar.xz
tar -xJf zsh-5.9.tar.xz

wget https://dianne.skoll.ca/projects/remind/download/remind-06.02.05.tar.gz
tar -xzf remind-06.02.05.tar.gz

wget https://downloads.sourceforge.net/project/mcj/xfig-3.2.9a.tar.xz
tar -xJf xfig-3.2.9a.tar.xz

wget https://github.com/ArtifexSoftware/ghostpdl-downloads/releases/download/gs10050/ghostscript-10.05.0.tar.gz
tar -xzf ghostscript-10.05.0.tar.gz

wget https://ftp.gnu.org/gnu/chess/gnuchess-6.3.0.tar.gz
tar -xzf gnuchess-6.3.0.tar.gz

wget https://ftp.gnu.org/non-gnu/cvs/source/stable/1.11.23/cvs-1.11.23.tar.gz
tar -xzf cvs-1.11.23.tar.gz

wget https://www.python.org/ftp/python/3.13.3/Python-3.13.3.tgz
tar -xzf Python-3.13.3.tgz

wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.19.6.tar.xz
tar -xJf linux-6.19.6.tar.xz

wget https://www.lua.org/ftp/lua-5.5.0.tar.gz
tar -xzf lua-5.5.0.tar.gz