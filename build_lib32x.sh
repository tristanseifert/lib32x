./gcc-sh/bin/sh-elf-gcc -c -O3 -o SH2/lib32x/lib32x_c.o SH2/lib32x/lib32x.c
./gcc-sh/bin/sh-elf-as --small -o SH2/lib32x/lib32x_asm.o SH2/lib32x/lib32x.s
./gcc-sh/bin/sh-elf-ld -T SH2/32x.ld -relax -small -r SH2/lib32x/lib32x_asm.o SH2/lib32x/lib32x_c.o -o SH2/lib32x.o

rm SH2/lib32x/lib32x_c.o
rm SH2/lib32x/lib32x_asm.o