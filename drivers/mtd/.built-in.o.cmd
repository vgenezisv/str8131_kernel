cmd_drivers/mtd/built-in.o :=  /opt/codesourcery/arm-2010q1/bin/arm-none-eabi-ld -EL   -r -o drivers/mtd/built-in.o drivers/mtd/mtdcore.o drivers/mtd/mtdpart.o drivers/mtd/mtdchar.o drivers/mtd/mtdblock.o drivers/mtd/mtd_blkdevs.o drivers/mtd/chips/built-in.o drivers/mtd/maps/built-in.o drivers/mtd/devices/built-in.o drivers/mtd/nand/built-in.o drivers/mtd/onenand/built-in.o