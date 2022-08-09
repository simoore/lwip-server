# Using LwIP to Build an Embedded Server

## Building Projects with CMake

For the STM32H7 application, build
```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-none-eabi.cmake -G "Unix Makefiles"
make
```

After the first build, any changes to the 

```bash
cd build
cmake ..
make
```

* Add ARM GCC toolchain bin to the path in ~/.bash_profile
* Variables defined in linker script should be referred to as value types, the convention is char, and then referenced
    to get there memory address. <https://sourceware.org/binutils/docs/ld/Source-Code-Reference.html>
* main in the startup script needs to be called via assembly because the compiler does not allow direct calls to main
    in cpp.

## Using `objdump`

We can view the header of the ELF binary using `arm-none-eabi-objdump.exe -f stm32h7-lwip-server.elf`.

The section headers can be viewed using `arm-none-eabi-objdump.exe -h stm32h7-lwip-server.elf`. Sections that are not
used in the final binary are removed unless no optimization is applied.

View the disassembly of executeable sections (eg. .data) use 
`$ arm-none-eabi-objdump.exe -d stm32h7-lwip-server.elf > app.list`.

Often we are interested in symbols that are not referenced in only executable sections. Use option `-D` to see the 
contents of all sections, `$ arm-none-eabi-objdump.exe -D stm32h7-lwip-server.elf > app.list`.

To add source code side by side with the disassembly use the `-S` option, 
`$arm-none-eabi-objdump -D -S app_binary > app.list`.

Just to note, when I was searching for an initialization issue, I found constructors named like `_ZN11DmaTxBufferC1Ev`
and the constructors to static objects are called via functions created by the compiler that are prefixed with
`_GLOBAL__sub_I`

## Code Snippets

### Execution flow of `ETH_UpdateDescriptor` on startup

```C++
heth->RxDescList.RxBuildDescCnt = ETH_RX_DESC_CNT;
ETH_UpdateDescriptor(heth);
```

`ETH_RX_DESC_CNT` is the number of RX Descriptors allocated in the ethernet peripheral.
At startup, the execution of `ETH_UpdateDescriptor()` will build all descriptors. 
Building descriptors involves the HAL layer asking the application for an allocated buffer that it can attach to 
a decriptor to ready it for reception. The HAL library maintains a tail pointer & counter for built descriptors and
a tail pointer and counter for descriptors that are ready for receiving data.

## References

* [launch.json Exmamples for Cortex Debugging](https://github.com/haneefdm/cortex-debug-samples/blob/master/blink2/.vscode/launch.json)

### Startup Code 

### Newlib syscalls to support printf() an scanf()

* [From Zero to main(): Bootstrapping libc with Newlib](https://interrupt.memfault.com/blog/boostrapping-libc-with-newlib)
* [How to Use printf on STM32](https://shawnhymel.com/1873/how-to-use-printf-on-stm32/)
* [Howto: Porting newlib](https://www.embecosm.com/appnotes/ean9/ean9-howto-newlib-1.0.html)
* <https://github.com/cnoviello/mastering-stm32/blob/master/nucleo-f030R8/system/src/retarget/retarget.c>

### Cache Coherency in STM32

* [DMA is not working on STM32H7 devices](https://community.st.com/s/article/FAQ-DMA-is-not-working-on-STM32H7-devices)

### Linker Script

* [The most thoroughly commented linker script (probably)](https://blog.thea.codes/the-most-thoroughly-commented-linker-script/)

### objdump

* [Global constructor call not in .init_array section](<https://stackoverflow.com/questions/6343348/global-constructor-call-not-in-init-array-section>)