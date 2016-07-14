cmd_arch/arm/boot/Image := /opt/codesourcery/arm-2010q1/bin/arm-none-eabi-objcopy -O binary -R .note -R .comment -S  vmlinux arch/arm/boot/Image
