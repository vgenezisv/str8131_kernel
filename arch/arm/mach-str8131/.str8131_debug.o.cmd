cmd_arch/arm/mach-str8131/str8131_debug.o := /opt/codesourcery/arm-2010q1/bin/arm-none-eabi-gcc -Wp,-MD,arch/arm/mach-str8131/.str8131_debug.o.d  -nostdinc -isystem /opt/codesourcery/arm-2010q1/bin/../lib/gcc/arm-none-eabi/4.4.1/include -D__KERNEL__ -Iinclude  -include include/linux/autoconf.h -mlittle-endian -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -ffreestanding -O3  -falign-functions=16    -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mabi=apcs-gnu -mno-thumb-interwork -D__LINUX_ARM_ARCH__=4 -march=armv4 -mtune=arm9tdmi  -msoft-float -Uarm -Wdeclaration-after-statement -Wno-pointer-sign -funit-at-a-time    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(str8131_debug)"  -D"KBUILD_MODNAME=KBUILD_STR(str8131_debug)" -c -o arch/arm/mach-str8131/str8131_debug.o arch/arm/mach-str8131/str8131_debug.c

deps_arch/arm/mach-str8131/str8131_debug.o := \
  arch/arm/mach-str8131/str8131_debug.c \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
  include/linux/config.h \
    $(wildcard include/config/h.h) \
  include/linux/posix_types.h \
  include/linux/stddef.h \
  include/linux/compiler.h \
  include/linux/compiler-gcc4.h \
    $(wildcard include/config/forced/inlining.h) \
  include/linux/compiler-gcc.h \
  include/asm/posix_types.h \
  include/asm/types.h \
  include/asm/hardware.h \
  include/asm/arch/hardware.h \
  include/asm/arch/param.h \
  include/asm/arch/star_sys_memory_map.h \
    $(wildcard include/config/data/base/addr.h) \
    $(wildcard include/config/addr/base/addr.h) \
    $(wildcard include/config/base/addr.h) \
  include/asm/arch/star_intc.h \
  include/asm/arch/star_timer.h \
  include/asm/arch/star_uart.h \
  include/asm/arch/star_gpio.h \
  include/asm/arch/star_pci_bridge.h \
    $(wildcard include/config/data.h) \
    $(wildcard include/config/data/base.h) \
    $(wildcard include/config/addr.h) \
    $(wildcard include/config/addr/base.h) \
    $(wildcard include/config/data/reg/offset.h) \
    $(wildcard include/config/addr/reg/offset.h) \
  include/asm/arch/star_misc.h \
    $(wildcard include/config/reg.h) \
  include/asm/arch/star_powermgt.h \
    $(wildcard include/config/pllclk/to/cpuclk/ratio.h) \
    $(wildcard include/config/cpuclk/to/hclk/ratio.h) \
    $(wildcard include/config/hclk/to/pclk/ratio.h) \
  include/asm/arch/star_rtc.h \
  include/asm/arch/star_wdtimer.h \
  include/asm/arch/star_smc.h \
  include/asm/arch/star_nic.h \
  include/asm/arch/star_ide.h \
  include/asm/arch/star_sys_memory_map.h \

arch/arm/mach-str8131/str8131_debug.o: $(deps_arch/arm/mach-str8131/str8131_debug.o)

$(deps_arch/arm/mach-str8131/str8131_debug.o):
