cmd_drivers/scsi/scsi_sysctl.o := /opt/codesourcery/arm-2010q1/bin/arm-none-eabi-gcc -Wp,-MD,drivers/scsi/.scsi_sysctl.o.d  -nostdinc -isystem /opt/codesourcery/arm-2010q1/bin/../lib/gcc/arm-none-eabi/4.4.1/include -D__KERNEL__ -Iinclude  -include include/linux/autoconf.h -mlittle-endian -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -ffreestanding -O3  -falign-functions=16    -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mabi=apcs-gnu -mno-thumb-interwork -D__LINUX_ARM_ARCH__=4 -march=armv4 -mtune=arm9tdmi  -msoft-float -Uarm -Wdeclaration-after-statement -Wno-pointer-sign -funit-at-a-time    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(scsi_sysctl)"  -D"KBUILD_MODNAME=KBUILD_STR(scsi_mod)" -c -o drivers/scsi/scsi_sysctl.o drivers/scsi/scsi_sysctl.c

deps_drivers/scsi/scsi_sysctl.o := \
  drivers/scsi/scsi_sysctl.c \
  include/linux/errno.h \
  include/asm/errno.h \
  include/asm-generic/errno.h \
  include/asm-generic/errno-base.h \
  include/linux/init.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/hotplug.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/memory/hotplug.h) \
  include/linux/config.h \
    $(wildcard include/config/h.h) \
  include/linux/compiler.h \
  include/linux/compiler-gcc4.h \
    $(wildcard include/config/forced/inlining.h) \
  include/linux/compiler-gcc.h \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/spinlock/sleep.h) \
    $(wildcard include/config/printk.h) \
  /opt/codesourcery/arm-2010q1/bin/../lib/gcc/arm-none-eabi/4.4.1/include/stdarg.h \
  include/linux/linkage.h \
  include/asm/linkage.h \
  include/linux/stddef.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
  include/linux/posix_types.h \
  include/asm/posix_types.h \
  include/asm/types.h \
  include/linux/bitops.h \
  include/asm/bitops.h \
    $(wildcard include/config/smp.h) \
  include/asm/system.h \
    $(wildcard include/config/cpu/sa1100.h) \
    $(wildcard include/config/cpu/sa110.h) \
  include/asm/byteorder.h \
  include/linux/byteorder/little_endian.h \
  include/linux/byteorder/swab.h \
  include/linux/byteorder/generic.h \
  include/asm/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/debug/bugverbose.h) \
  include/asm-generic/bug.h \
  include/linux/sysctl.h \
  include/linux/list.h \
  include/linux/prefetch.h \
  include/asm/processor.h \
    $(wildcard include/config/mmu.h) \
  include/asm/ptrace.h \
    $(wildcard include/config/arm/thumb.h) \
  include/asm/procinfo.h \
  include/asm/cache.h \
    $(wildcard include/config/arch/str8131.h) \
  drivers/scsi/scsi_logging.h \
    $(wildcard include/config/scsi/logging.h) \

drivers/scsi/scsi_sysctl.o: $(deps_drivers/scsi/scsi_sysctl.o)

$(deps_drivers/scsi/scsi_sysctl.o):
