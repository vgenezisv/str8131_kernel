cmd_arch/arm/mm/cache-fa.o := /opt/codesourcery/arm-2010q1/bin/arm-none-eabi-gcc -Wp,-MD,arch/arm/mm/.cache-fa.o.d  -nostdinc -isystem /opt/codesourcery/arm-2010q1/bin/../lib/gcc/arm-none-eabi/4.4.1/include -D__KERNEL__ -Iinclude  -include include/linux/autoconf.h -mlittle-endian -D__ASSEMBLY__ -mabi=apcs-gnu -mno-thumb-interwork -D__LINUX_ARM_ARCH__=4 -march=armv4 -mtune=arm9tdmi -msoft-float    -c -o arch/arm/mm/cache-fa.o arch/arm/mm/cache-fa.S

deps_arch/arm/mm/cache-fa.o := \
  arch/arm/mm/cache-fa.S \
    $(wildcard include/config/cpu/icache/disable.h) \
    $(wildcard include/config/cpu/dcache/disable.h) \
    $(wildcard include/config/cpu/dcache/writethrough.h) \
    $(wildcard include/config/cpu/fa/wb/disable.h) \
    $(wildcard include/config/cpu/fa/btb.h) \
  include/linux/config.h \
    $(wildcard include/config/h.h) \
  include/linux/linkage.h \
  include/asm/linkage.h \
  include/linux/init.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/hotplug.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/memory/hotplug.h) \
  include/linux/compiler.h \
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
  include/asm/page.h \
    $(wildcard include/config/cpu/copy/v3.h) \
    $(wildcard include/config/cpu/copy/v4wt.h) \
    $(wildcard include/config/cpu/copy/v4wb.h) \
    $(wildcard include/config/cpu/fa520.h) \
    $(wildcard include/config/cpu/fa526.h) \
    $(wildcard include/config/cpu/fa626.h) \
    $(wildcard include/config/cpu/copy/fa.h) \
    $(wildcard include/config/cpu/sa1100.h) \
    $(wildcard include/config/cpu/xscale.h) \
    $(wildcard include/config/cpu/copy/v6.h) \
    $(wildcard include/config/aeabi.h) \
  include/asm-generic/page.h \
  arch/arm/mm/proc-macros.S \
  include/asm/asm-offsets.h \
  include/asm/thread_info.h \
  include/asm/fpstate.h \
    $(wildcard include/config/iwmmxt.h) \

arch/arm/mm/cache-fa.o: $(deps_arch/arm/mm/cache-fa.o)

$(deps_arch/arm/mm/cache-fa.o):
