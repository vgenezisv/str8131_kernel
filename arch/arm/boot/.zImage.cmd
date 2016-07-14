cmd_arch/arm/boot/zImage := /opt/codesourcery/arm-2010q1/bin/arm-none-eabi-objcopy -O binary -R .note -R .comment -S  arch/arm/boot/compressed/vmlinux arch/arm/boot/zImage
