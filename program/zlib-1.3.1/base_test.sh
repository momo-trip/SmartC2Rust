#!/bin/bash
# base_test.sh for zlib-1.3.1 (testing minigzip)

set -u
BIN=./minigzip
failed=0

# Test 1: Round-trip compression/decompression of a small text file (pipe mode)
echo "hello world" > /tmp/t1.txt
$BIN < /tmp/t1.txt > /tmp/t1.gz
$BIN -d < /tmp/t1.gz > /tmp/t1.out
cmp -s /tmp/t1.txt /tmp/t1.out || { echo "test1 failed"; failed=1; }

# Test 2: Round-trip compression/decompression of a larger text file (README, a few KB)
$BIN < README > /tmp/t2.gz
$BIN -d < /tmp/t2.gz > /tmp/t2.out
cmp -s README /tmp/t2.out || { echo "test2 failed"; failed=1; }

# Test 3: Round-trip compression/decompression of binary data
head -c 65536 /dev/urandom > /tmp/t3.bin
$BIN < /tmp/t3.bin > /tmp/t3.gz
$BIN -d < /tmp/t3.gz > /tmp/t3.out
cmp -s /tmp/t3.bin /tmp/t3.out || { echo "test3 failed"; failed=1; }

# Test 4: Compression with maximum compression level (-9)
$BIN -9 < README > /tmp/t4.gz
$BIN -d < /tmp/t4.gz > /tmp/t4.out
cmp -s README /tmp/t4.out || { echo "test4 failed"; failed=1; }

# Test 5: Compression with fastest compression level (-1)
$BIN -1 < README > /tmp/t5.gz
$BIN -d < /tmp/t5.gz > /tmp/t5.out
cmp -s README /tmp/t5.out || { echo "test5 failed"; failed=1; }

# Test 6: File mode (minigzip file -> file.gz, then restore with -d)
cp README /tmp/t6.txt
$BIN /tmp/t6.txt           # creates /tmp/t6.txt.gz and removes the original
$BIN -d /tmp/t6.txt.gz     # restores /tmp/t6.txt
cmp -s README /tmp/t6.txt || { echo "test6 failed"; failed=1; }

# Test 7: Gzip compatibility (verify that the output can be decompressed by the system gzip)
echo "compat check" > /tmp/t7.txt
$BIN < /tmp/t7.txt > /tmp/t7.gz
gzip -dc /tmp/t7.gz > /tmp/t7.out
cmp -s /tmp/t7.txt /tmp/t7.out || { echo "test7 failed"; failed=1; }

# Test 8: Empty input file
: > /tmp/t8.txt
$BIN < /tmp/t8.txt > /tmp/t8.gz
$BIN -d < /tmp/t8.gz > /tmp/t8.out
cmp -s /tmp/t8.txt /tmp/t8.out || { echo "test8 failed"; failed=1; }

exit $failed