cmd_drivers/i2c/built-in.o :=  /opt/codesourcery/arm-2010q1/bin/arm-none-eabi-ld -EL   -r -o drivers/i2c/built-in.o drivers/i2c/i2c-core.o drivers/i2c/i2c-dev.o drivers/i2c/busses/built-in.o drivers/i2c/chips/built-in.o drivers/i2c/algos/built-in.o