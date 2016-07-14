cmd_arch/arm/nwfpe/extended_cpdo.o := /opt/codesourcery/arm-2010q1/bin/arm-none-eabi-gcc -Wp,-MD,arch/arm/nwfpe/.extended_cpdo.o.d  -nostdinc -isystem /opt/codesourcery/arm-2010q1/bin/../lib/gcc/arm-none-eabi/4.4.1/include -D__KERNEL__ -Iinclude  -include include/linux/autoconf.h -mlittle-endian -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -ffreestanding -O3  -falign-functions=16    -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mabi=apcs-gnu -mno-thumb-interwork -D__LINUX_ARM_ARCH__=4 -march=armv4 -mtune=arm9tdmi  -msoft-float -Uarm -Wdeclaration-after-statement -Wno-pointer-sign -funit-at-a-time    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(extended_cpdo)"  -D"KBUILD_MODNAME=KBUILD_STR(nwfpe)" -c -o arch/arm/nwfpe/extended_cpdo.o arch/arm/nwfpe/extended_cpdo.c

deps_arch/arm/nwfpe/extended_cpdo.o := \
  arch/arm/nwfpe/extended_cpdo.c \
  arch/arm/nwfpe/fpa11.h \
    $(wildcard include/config/fpe/nwfpe/xp.h) \
  include/linux/config.h \
    $(wildcard include/config/h.h) \
  include/linux/thread_info.h \
  include/linux/bitops.h \
  include/asm/types.h \
  include/asm/bitops.h \
    $(wildcard include/config/smp.h) \
  include/linux/compiler.h \
  include/linux/compiler-gcc4.h \
    $(wildcard include/config/forced/inlining.h) \
  include/linux/compiler-gcc.h \
  include/asm/system.h \
    $(wildcard include/config/cpu/sa1100.h) \
    $(wildcard include/config/cpu/sa110.h) \
  include/linux/linkage.h \
  include/asm/linkage.h \
  include/asm/thread_info.h \
  include/asm/fpstate.h \
    $(wildcard include/config/iwmmxt.h) \
  include/asm/ptrace.h \
    $(wildcard include/config/arm/thumb.h) \
  include/asm/domain.h \
  arch/arm/nwfpe/fpsr.h \
  arch/arm/nwfpe/milieu.h \
  arch/arm/nwfpe/ARM-gcc.h \
  arch/arm/nwfpe/softfloat.h \
  arch/arm/nwfpe/fpopcode.h \

arch/arm/nwfpe/extended_cpdo.o: $(deps_arch/arm/nwfpe/extended_cpdo.o)

$(deps_arch/arm/nwfpe/extended_cpdo.o):
