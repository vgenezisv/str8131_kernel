cmd_fs/super.o := /opt/codesourcery/arm-2010q1/bin/arm-none-eabi-gcc -Wp,-MD,fs/.super.o.d  -nostdinc -isystem /opt/codesourcery/arm-2010q1/bin/../lib/gcc/arm-none-eabi/4.4.1/include -D__KERNEL__ -Iinclude  -include include/linux/autoconf.h -mlittle-endian -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -ffreestanding -O3  -falign-functions=16    -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mabi=apcs-gnu -mno-thumb-interwork -D__LINUX_ARM_ARCH__=4 -march=armv4 -mtune=arm9tdmi  -msoft-float -Uarm -Wdeclaration-after-statement -Wno-pointer-sign -funit-at-a-time    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(super)"  -D"KBUILD_MODNAME=KBUILD_STR(super)" -c -o fs/super.o fs/super.c

deps_fs/super.o := \
  fs/super.c \
  include/linux/config.h \
    $(wildcard include/config/h.h) \
  include/linux/module.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/module/unload.h) \
    $(wildcard include/config/kallsyms.h) \
  include/linux/sched.h \
    $(wildcard include/config/detect/softlockup.h) \
    $(wildcard include/config/split/ptlock/cpus.h) \
    $(wildcard include/config/keys.h) \
    $(wildcard include/config/inotify.h) \
    $(wildcard include/config/schedstats.h) \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/debug/mutexes.h) \
    $(wildcard include/config/bsd/process/acct.h) \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/cpusets.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/preempt.h) \
    $(wildcard include/config/pm.h) \
  include/asm/param.h \
  include/asm/arch/param.h \
  include/linux/capability.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
  include/linux/posix_types.h \
  include/linux/stddef.h \
  include/linux/compiler.h \
  include/linux/compiler-gcc4.h \
    $(wildcard include/config/forced/inlining.h) \
  include/linux/compiler-gcc.h \
  include/asm/posix_types.h \
  include/asm/types.h \
  include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
  include/linux/preempt.h \
    $(wildcard include/config/debug/preempt.h) \
  include/linux/thread_info.h \
  include/linux/bitops.h \
  include/asm/bitops.h \
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
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/spinlock/sleep.h) \
    $(wildcard include/config/printk.h) \
  /opt/codesourcery/arm-2010q1/bin/../lib/gcc/arm-none-eabi/4.4.1/include/stdarg.h \
  include/asm/byteorder.h \
  include/linux/byteorder/little_endian.h \
  include/linux/byteorder/swab.h \
  include/linux/byteorder/generic.h \
  include/asm/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/debug/bugverbose.h) \
  include/asm-generic/bug.h \
  include/linux/stringify.h \
  include/linux/spinlock_types.h \
  include/linux/spinlock_types_up.h \
  include/linux/spinlock_up.h \
  include/linux/spinlock_api_up.h \
  include/asm/atomic.h \
  include/asm-generic/atomic.h \
  include/asm/current.h \
  include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  include/linux/timex.h \
    $(wildcard include/config/time/interpolation.h) \
  include/linux/time.h \
  include/linux/seqlock.h \
  include/asm/timex.h \
  include/asm/arch/timex.h \
  include/linux/jiffies.h \
  include/linux/calc64.h \
  include/asm/div64.h \
  include/linux/rbtree.h \
  include/linux/cpumask.h \
  include/linux/bitmap.h \
  include/linux/string.h \
  include/asm/string.h \
  include/linux/errno.h \
  include/asm/errno.h \
  include/asm-generic/errno.h \
  include/asm-generic/errno-base.h \
  include/linux/nodemask.h \
  include/linux/numa.h \
    $(wildcard include/config/flatmem.h) \
  include/asm/semaphore.h \
  include/linux/wait.h \
  include/linux/list.h \
  include/linux/prefetch.h \
  include/asm/processor.h \
    $(wildcard include/config/mmu.h) \
  include/asm/procinfo.h \
  include/asm/cache.h \
    $(wildcard include/config/arch/str8131.h) \
  include/linux/rwsem.h \
    $(wildcard include/config/rwsem/generic/spinlock.h) \
  include/linux/rwsem-spinlock.h \
  include/asm/locks.h \
  include/asm/page.h \
    $(wildcard include/config/cpu/copy/v3.h) \
    $(wildcard include/config/cpu/copy/v4wt.h) \
    $(wildcard include/config/cpu/copy/v4wb.h) \
    $(wildcard include/config/cpu/fa520.h) \
    $(wildcard include/config/cpu/fa526.h) \
    $(wildcard include/config/cpu/fa626.h) \
    $(wildcard include/config/cpu/copy/fa.h) \
    $(wildcard include/config/cpu/xscale.h) \
    $(wildcard include/config/cpu/copy/v6.h) \
    $(wildcard include/config/aeabi.h) \
  include/asm/glue.h \
    $(wildcard include/config/cpu/arm610.h) \
    $(wildcard include/config/cpu/arm710.h) \
    $(wildcard include/config/cpu/abrt/lv4t.h) \
    $(wildcard include/config/cpu/abrt/ev4.h) \
    $(wildcard include/config/cpu/abrt/ev4t.h) \
    $(wildcard include/config/cpu/abrt/ev5tj.h) \
    $(wildcard include/config/cpu/abrt/ev5t.h) \
    $(wildcard include/config/cpu/abrt/ev6.h) \
  include/asm/memory.h \
    $(wildcard include/config/discontigmem.h) \
  include/asm/arch/memory.h \
    $(wildcard include/config/system/dram/base.h) \
    $(wildcard include/config/str8131/dram/64m.h) \
    $(wildcard include/config/system/dram/size.h) \
    $(wildcard include/config/str8131/dram/32m.h) \
    $(wildcard include/config/str8131/dram/16m.h) \
  include/asm/sizes.h \
  include/asm-generic/page.h \
  include/asm/mmu.h \
  include/asm/cputime.h \
  include/asm-generic/cputime.h \
  include/linux/smp.h \
  include/linux/sem.h \
    $(wildcard include/config/sysvipc.h) \
  include/linux/ipc.h \
  include/asm/ipcbuf.h \
  include/asm/sembuf.h \
  include/linux/signal.h \
  include/asm/signal.h \
  include/asm-generic/signal.h \
  include/asm/sigcontext.h \
  include/asm/siginfo.h \
  include/asm-generic/siginfo.h \
  include/linux/securebits.h \
  include/linux/fs_struct.h \
  include/linux/completion.h \
  include/linux/pid.h \
  include/linux/percpu.h \
  include/linux/slab.h \
    $(wildcard include/config/.h) \
    $(wildcard include/config/slob.h) \
    $(wildcard include/config/debug/slab.h) \
  include/linux/gfp.h \
    $(wildcard include/config/dma/is/dma32.h) \
  include/linux/mmzone.h \
    $(wildcard include/config/force/max/zoneorder.h) \
    $(wildcard include/config/memory/hotplug.h) \
    $(wildcard include/config/flat/node/mem/map.h) \
    $(wildcard include/config/have/memory/present.h) \
    $(wildcard include/config/need/node/memmap/size.h) \
    $(wildcard include/config/need/multiple/nodes.h) \
    $(wildcard include/config/sparsemem.h) \
    $(wildcard include/config/have/arch/early/pfn/to/nid.h) \
    $(wildcard include/config/sparsemem/extreme.h) \
  include/linux/cache.h \
    $(wildcard include/config/x86.h) \
    $(wildcard include/config/sparc64.h) \
    $(wildcard include/config/ia64.h) \
    $(wildcard include/config/parisc.h) \
  include/linux/init.h \
    $(wildcard include/config/hotplug.h) \
  include/linux/memory_hotplug.h \
  include/linux/notifier.h \
  include/linux/topology.h \
    $(wildcard include/config/sched/smt.h) \
  include/asm/topology.h \
  include/asm-generic/topology.h \
  include/linux/kmalloc_sizes.h \
    $(wildcard include/config/large/allocs.h) \
  include/asm/percpu.h \
  include/asm-generic/percpu.h \
  include/linux/seccomp.h \
    $(wildcard include/config/seccomp.h) \
  include/linux/rcupdate.h \
  include/linux/auxvec.h \
  include/asm/auxvec.h \
  include/linux/param.h \
  include/linux/resource.h \
  include/asm/resource.h \
  include/asm-generic/resource.h \
  include/linux/timer.h \
  include/linux/hrtimer.h \
    $(wildcard include/config/no/idle/hz.h) \
  include/linux/ktime.h \
    $(wildcard include/config/ktime/scalar.h) \
  include/linux/aio.h \
  include/linux/workqueue.h \
  include/linux/aio_abi.h \
  include/linux/stat.h \
  include/asm/stat.h \
  include/linux/kmod.h \
    $(wildcard include/config/kmod.h) \
  include/linux/elf.h \
  include/asm/elf.h \
  include/asm/user.h \
  include/linux/kobject.h \
    $(wildcard include/config/net.h) \
  include/linux/sysfs.h \
    $(wildcard include/config/sysfs.h) \
  include/linux/kref.h \
  include/linux/moduleparam.h \
  include/asm/local.h \
  include/asm-generic/local.h \
  include/linux/hardirq.h \
    $(wildcard include/config/preempt/bkl.h) \
    $(wildcard include/config/virt/cpu/accounting.h) \
  include/linux/smp_lock.h \
    $(wildcard include/config/lock/kernel.h) \
  include/asm/hardirq.h \
  include/asm/irq.h \
  include/asm/arch/irqs.h \
    $(wildcard include/config/vic/interrupt.h) \
  include/asm/arch/star_intc.h \
  include/asm/arch/star_sys_memory_map.h \
    $(wildcard include/config/data/base/addr.h) \
    $(wildcard include/config/addr/base/addr.h) \
    $(wildcard include/config/base/addr.h) \
  include/linux/irq_cpustat.h \
  include/asm/module.h \
  include/linux/acct.h \
    $(wildcard include/config/m68k.h) \
    $(wildcard include/config/bsd/process/acct/v3.h) \
  include/linux/blkdev.h \
    $(wildcard include/config/lbd.h) \
  include/linux/major.h \
  include/linux/genhd.h \
    $(wildcard include/config/solaris/x86/partition.h) \
    $(wildcard include/config/bsd/disklabel.h) \
    $(wildcard include/config/unixware/disklabel.h) \
    $(wildcard include/config/minix/subpartition.h) \
  include/linux/device.h \
  include/linux/ioport.h \
  include/linux/klist.h \
  include/linux/pm.h \
  include/linux/fs.h \
    $(wildcard include/config/dnotify.h) \
    $(wildcard include/config/quota.h) \
    $(wildcard include/config/adaptive/readahead.h) \
    $(wildcard include/config/epoll.h) \
    $(wildcard include/config/auditsyscall.h) \
    $(wildcard include/config/fs/xip.h) \
    $(wildcard include/config/migration.h) \
    $(wildcard include/config/security.h) \
  include/linux/limits.h \
  include/linux/ioctl.h \
  include/asm/ioctl.h \
  include/asm-generic/ioctl.h \
  include/linux/kdev_t.h \
  include/linux/dcache.h \
    $(wildcard include/config/profiling.h) \
  include/linux/radix-tree.h \
  include/linux/prio_tree.h \
  include/linux/mutex.h \
  include/linux/quota.h \
  include/linux/dqblk_xfs.h \
  include/linux/dqblk_v1.h \
  include/linux/dqblk_v2.h \
  include/linux/nfs_fs_i.h \
  include/linux/nfs.h \
  include/linux/sunrpc/msg_prot.h \
  include/linux/fcntl.h \
  include/asm/fcntl.h \
  include/asm-generic/fcntl.h \
    $(wildcard include/config/64bit.h) \
  include/linux/err.h \
  include/linux/pagemap.h \
  include/linux/mm.h \
    $(wildcard include/config/sysctl.h) \
    $(wildcard include/config/stack/growsup.h) \
    $(wildcard include/config/highmem.h) \
    $(wildcard include/config/shmem.h) \
    $(wildcard include/config/proc/fs.h) \
    $(wildcard include/config/debug/pagealloc.h) \
  include/asm/pgtable.h \
  include/asm-generic/4level-fixup.h \
  include/asm/proc-fns.h \
    $(wildcard include/config/cpu/32.h) \
    $(wildcard include/config/cpu/arm720t.h) \
    $(wildcard include/config/cpu/arm920t.h) \
    $(wildcard include/config/cpu/arm922t.h) \
    $(wildcard include/config/cpu/arm925t.h) \
    $(wildcard include/config/cpu/arm926t.h) \
    $(wildcard include/config/cpu/arm1020.h) \
    $(wildcard include/config/cpu/arm1020e.h) \
    $(wildcard include/config/cpu/arm1022.h) \
    $(wildcard include/config/cpu/arm1026.h) \
    $(wildcard include/config/cpu/v6.h) \
  include/asm/cpu-single.h \
  include/asm/arch/vmalloc.h \
  include/asm-generic/pgtable.h \
  include/linux/page-flags.h \
    $(wildcard include/config/swap.h) \
  include/linux/highmem.h \
  include/asm/cacheflush.h \
    $(wildcard include/config/cpu/cache/vipt.h) \
    $(wildcard include/config/cpu/cache/vivt.h) \
  include/asm/shmparam.h \
  include/asm/uaccess.h \
  include/linux/backing-dev.h \
  include/linux/mempool.h \
  include/linux/bio.h \
  include/linux/ioprio.h \
  include/asm/io.h \
  include/asm/arch/io.h \
  include/asm/hardware.h \
  include/asm/arch/hardware.h \
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
  include/asm/scatterlist.h \
  include/linux/elevator.h \
  include/linux/quotaops.h \
  include/linux/namei.h \
  include/linux/buffer_head.h \
  include/linux/mount.h \
  include/linux/security.h \
    $(wildcard include/config/security/network.h) \
    $(wildcard include/config/security/network/xfrm.h) \
  include/linux/binfmts.h \
  include/linux/shm.h \
  include/asm/shmbuf.h \
  include/linux/msg.h \
  include/asm/msgbuf.h \
  include/linux/key.h \
  include/linux/syscalls.h \
    $(wildcard include/config/v850.h) \
  include/linux/vfs.h \
  include/linux/statfs.h \
  include/asm/statfs.h \
  include/linux/writeback.h \
  include/linux/idr.h \

fs/super.o: $(deps_fs/super.o)

$(deps_fs/super.o):
