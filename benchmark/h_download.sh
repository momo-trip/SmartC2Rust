#!/bin/bash

# C Programs Download Script
# Downloads various C programs and libraries

set -e  # Exit on any error

######
# echo "Starting download of C programs..."

# # Create temporary directory for downloads
# mkdir -p downloads
# cd downloads

# # SipHash
# echo "Downloading SipHash..."
# git clone https://github.com/veorq/SipHash.git
# mv SipHash ../SipHash

# # sds (Simple Dynamic Strings)
# echo "Downloading sds..."
# git clone https://github.com/antirez/sds.git
# mv sds ../sds

# # mark-sweep garbage collector
# echo "Downloading mark-sweep..."
# git clone https://github.com/munificent/mark-sweep.git
# mv mark-sweep ../mark-sweep

# # which (GNU which-2.21)
# echo "Downloading which-2.21..."
# wget https://ftp.gnu.org/gnu/which/which-2.21.tar.gz
# tar -xzf which-2.21.tar.gz
# mv which-2.21 ../which_2_21
# rm which-2.21.tar.gz

# # Tinyhttpd
# echo "Downloading Tinyhttpd..."
# git clone https://github.com/EZLippi/Tinyhttpd.git
# mv Tinyhttpd ../Tinyhttpd

# # tiny-AES-c
# echo "Downloading tiny-AES-c..."
# git clone https://github.com/kokke/tiny-AES-c.git
# mv tiny-AES-c ../tiny-AES-c

# # time (GNU time-1.9)
# echo "Downloading time-1.9..."
# wget https://ftp.gnu.org/gnu/time/time-1.9.tar.gz
# tar -xzf time-1.9.tar.gz
# mv time-1.9 ../time_1_9
# rm time-1.9.tar.gz

# # c4 (C compiler in 4 functions)
# echo "Downloading c4..."
# git clone https://github.com/rswier/c4.git
# mv c4 ../c4

# # yank
# echo "Downloading yank..."
# git clone https://github.com/mptre/yank.git
# mv yank ../yank

# # su-exec
# echo "Downloading su-exec..."
# git clone https://github.com/ncopa/su-exec.git
# mv su-exec ../su-exec

# # FastestWebsiteEver
# echo "Downloading FastestWebsiteEver..."
# git clone https://github.com/diracdeltas/FastestWebsiteEver.git
# mv FastestWebsiteEver ../FastestWebsiteEver

# # mcrcon
# echo "Downloading mcrcon..."
# git clone https://github.com/Tiiffi/mcrcon.git
# mv mcrcon ../mcrcon

# # zopfli
# echo "Downloading zopfli..."
# git clone https://github.com/google/zopfli.git
# mv zopfli ../zopfli
######


# Programs from crown repository (previous implementations)
echo "Downloading programs from crown repository..."

# Clone the crown repository to get all directories
echo "Cloning crown repository..."
git clone --depth 1 https://github.com/KomaEc/crown.git crown_temp

# Move the required directories from crown/c-code to parent directory
cd crown_temp/c-code

# bst (Binary Search Tree)
if [ -d "bst" ]; then
    echo "Moving bst directory..."
    mv bst ../../bst
else
    echo "Warning: Directory bst not found in crown repository"
fi

# buffer
if [ -d "buffer" ]; then
    echo "Moving buffer directory..."
    mv buffer ../../buffer
else
    echo "Warning: Directory buffer not found in crown repository"
fi

# ht (Hash Table)
if [ -d "ht" ]; then
    echo "Moving ht directory..."
    mv ht ../../ht
else
    echo "Warning: Directory ht not found in crown repository"
fi

# urlparser
if [ -d "urlparser" ]; then
    echo "Moving urlparser directory..."
    mv urlparser ../../urlparser
else
    echo "Warning: Directory urlparser not found in crown repository"
fi

# rgba
if [ -d "rgba" ]; then
    echo "Moving rgba directory..."
    mv rgba ../../rgba
else
    echo "Warning: Directory rgba not found in crown repository"
fi

# quadtree
if [ -d "quadtree" ]; then
    echo "Moving quadtree directory..."
    mv quadtree ../../quadtree
else
    echo "Warning: Directory quadtree not found in crown repository"
fi

# qsort
if [ -d "qsort" ]; then
    echo "Moving qsort directory..."
    mv qsort ../../qsort
else
    echo "Warning: Directory qsort not found in crown repository"
fi

# avl (AVL Tree)
if [ -d "avl" ]; then
    echo "Moving avl directory..."
    mv avl ../../avl
else
    echo "Warning: Directory avl not found in crown repository"
fi

cd ../..

# Clean up temporary crown repository
rm -rf crown_temp

