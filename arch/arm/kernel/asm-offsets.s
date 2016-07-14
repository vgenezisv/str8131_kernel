	.file	"asm-offsets.c"
	.text
	.align	2
	.p2align 4,,15
	.global	main
	.type	main, %function
main:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
@ 46 "arch/arm/kernel/asm-offsets.c" 1
	
->TSK_ACTIVE_MM #124 offsetof(struct task_struct, active_mm)
@ 0 "" 2
@ 47 "arch/arm/kernel/asm-offsets.c" 1
	
->
@ 0 "" 2
@ 48 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_FLAGS #0 offsetof(struct thread_info, flags)
@ 0 "" 2
@ 49 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_PREEMPT #4 offsetof(struct thread_info, preempt_count)
@ 0 "" 2
@ 50 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_ADDR_LIMIT #8 offsetof(struct thread_info, addr_limit)
@ 0 "" 2
@ 51 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_TASK #12 offsetof(struct thread_info, task)
@ 0 "" 2
@ 52 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_EXEC_DOMAIN #16 offsetof(struct thread_info, exec_domain)
@ 0 "" 2
@ 53 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_CPU #20 offsetof(struct thread_info, cpu)
@ 0 "" 2
@ 54 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_CPU_DOMAIN #24 offsetof(struct thread_info, cpu_domain)
@ 0 "" 2
@ 55 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_CPU_SAVE #28 offsetof(struct thread_info, cpu_context)
@ 0 "" 2
@ 56 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_USED_CP #76 offsetof(struct thread_info, used_cp)
@ 0 "" 2
@ 57 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_TP_VALUE #92 offsetof(struct thread_info, tp_value)
@ 0 "" 2
@ 58 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_FPSTATE #96 offsetof(struct thread_info, fpstate)
@ 0 "" 2
@ 59 "arch/arm/kernel/asm-offsets.c" 1
	
->TI_VFPSTATE #236 offsetof(struct thread_info, vfpstate)
@ 0 "" 2
@ 63 "arch/arm/kernel/asm-offsets.c" 1
	
->
@ 0 "" 2
@ 64 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R0 #0 offsetof(struct pt_regs, ARM_r0)
@ 0 "" 2
@ 65 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R1 #4 offsetof(struct pt_regs, ARM_r1)
@ 0 "" 2
@ 66 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R2 #8 offsetof(struct pt_regs, ARM_r2)
@ 0 "" 2
@ 67 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R3 #12 offsetof(struct pt_regs, ARM_r3)
@ 0 "" 2
@ 68 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R4 #16 offsetof(struct pt_regs, ARM_r4)
@ 0 "" 2
@ 69 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R5 #20 offsetof(struct pt_regs, ARM_r5)
@ 0 "" 2
@ 70 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R6 #24 offsetof(struct pt_regs, ARM_r6)
@ 0 "" 2
@ 71 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R7 #28 offsetof(struct pt_regs, ARM_r7)
@ 0 "" 2
@ 72 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R8 #32 offsetof(struct pt_regs, ARM_r8)
@ 0 "" 2
@ 73 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R9 #36 offsetof(struct pt_regs, ARM_r9)
@ 0 "" 2
@ 74 "arch/arm/kernel/asm-offsets.c" 1
	
->S_R10 #40 offsetof(struct pt_regs, ARM_r10)
@ 0 "" 2
@ 75 "arch/arm/kernel/asm-offsets.c" 1
	
->S_FP #44 offsetof(struct pt_regs, ARM_fp)
@ 0 "" 2
@ 76 "arch/arm/kernel/asm-offsets.c" 1
	
->S_IP #48 offsetof(struct pt_regs, ARM_ip)
@ 0 "" 2
@ 77 "arch/arm/kernel/asm-offsets.c" 1
	
->S_SP #52 offsetof(struct pt_regs, ARM_sp)
@ 0 "" 2
@ 78 "arch/arm/kernel/asm-offsets.c" 1
	
->S_LR #56 offsetof(struct pt_regs, ARM_lr)
@ 0 "" 2
@ 79 "arch/arm/kernel/asm-offsets.c" 1
	
->S_PC #60 offsetof(struct pt_regs, ARM_pc)
@ 0 "" 2
@ 80 "arch/arm/kernel/asm-offsets.c" 1
	
->S_PSR #64 offsetof(struct pt_regs, ARM_cpsr)
@ 0 "" 2
@ 81 "arch/arm/kernel/asm-offsets.c" 1
	
->S_OLD_R0 #68 offsetof(struct pt_regs, ARM_ORIG_r0)
@ 0 "" 2
@ 82 "arch/arm/kernel/asm-offsets.c" 1
	
->S_FRAME_SIZE #72 sizeof(struct pt_regs)
@ 0 "" 2
@ 83 "arch/arm/kernel/asm-offsets.c" 1
	
->
@ 0 "" 2
@ 88 "arch/arm/kernel/asm-offsets.c" 1
	
->VMA_VM_MM #0 offsetof(struct vm_area_struct, vm_mm)
@ 0 "" 2
@ 89 "arch/arm/kernel/asm-offsets.c" 1
	
->VMA_VM_FLAGS #20 offsetof(struct vm_area_struct, vm_flags)
@ 0 "" 2
@ 90 "arch/arm/kernel/asm-offsets.c" 1
	
->
@ 0 "" 2
@ 91 "arch/arm/kernel/asm-offsets.c" 1
	
->VM_EXEC #4 VM_EXEC
@ 0 "" 2
@ 92 "arch/arm/kernel/asm-offsets.c" 1
	
->
@ 0 "" 2
@ 93 "arch/arm/kernel/asm-offsets.c" 1
	
->PAGE_SZ #4096 PAGE_SIZE
@ 0 "" 2
@ 94 "arch/arm/kernel/asm-offsets.c" 1
	
->
@ 0 "" 2
@ 95 "arch/arm/kernel/asm-offsets.c" 1
	
->SYS_ERROR0 #10420224 0x9f0000
@ 0 "" 2
@ 96 "arch/arm/kernel/asm-offsets.c" 1
	
->
@ 0 "" 2
@ 97 "arch/arm/kernel/asm-offsets.c" 1
	
->SIZEOF_MACHINE_DESC #56 sizeof(struct machine_desc)
@ 0 "" 2
	mov	r0, #0
	ldmfd	sp, {fp, sp, pc}
	.size	main, .-main
	.ident	"GCC: (Sourcery G++ Lite 2010q1-188) 4.4.1"
