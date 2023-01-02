# Using LwIP to Build an Embedded Server

## Features

* TCP echo server on port 7.

## Building Projects with CMake

To build the STM32H7 application and the unit tests:

```bash
mkdir build-stm32h7-debug
mkdir build-tests
cd build-stm32h7-debug
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-none-eabi.cmake -G "Unix Makefiles"
make
cd ../build-tests
cmake .. -DCMAKE_BUILD_TYPE=Debug -G "Unix Makefiles"
make
```

The build type and toolchain are cached after the first build. Any changes to the cmake project only requires the
following commands to build:

```bash
cd build-stm32h7-debug
cmake ..
make
```

## Notes

* Add ARM GCC toolchain bin to the path in ~/.bash_profile
* Variables defined in linker script should be referred to as value types, the convention is char, and then referenced
    to get there memory address. <https://sourceware.org/binutils/docs/ld/Source-Code-Reference.html>
* `main` in the startup script needs to be called via assembly because the compiler does not allow direct calls to main
    in cpp.
* IP Address 192.168.x.x have been assigned for private networks and is used in this project.
* Due to many small incompatibilies between MSYS2 and the windows environment, I should learn powershell for automation.

## Network Troubleshooting

* `arp -a` lists all devices found on the network your machine is connected to.
* `ipconfig /all` on windows will show the IP address and MAC address associated with a network interface along with 
    some other details.

## LwIP Debugging

* When debugging LwIP, `ethernet_input` displays when a packet, the source and destination MAC address, and the payload
    type. Payload types (https://en.wikipedia.org/wiki/EtherType) are for example 0x800 for IPv4 and ARP for 0x806.
* MAC address ff:ff:ff:ff:ff:ff is a broadcast.
* Don't use cacheing for the ethernet DMA buffers. You can use the address of LwIP's heap from lwipopts.h in the 
    configuration of the MPU.

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

## LwIP Port

Architecture specific includes must be placed in a folder called `arch` on the include path when compiling LwIP. These
can include:
* `bpstruct.h`: Statements to be appended to the start of a packed struct definition. We do not requrie this when
    using GCC to compile. An example is apply the `#pragma pack(1)` precompiler statement when using the IAR compiler.
* `epstruct.h`: Statements to be appended to the end of a packed struct definition.
* `perf.h`: Is used for profiling sections of LwIP. PERF_START & PERF_STOP. We won't use this by setting `LWIP_PERF` 
    to 0 in `lwipopts.h`.
* `sys_arch.h`: Needs to be provided to define things like mutexes, semaphores, and threads if using `NO_SYS 0`, that 
    is using an operating system. For bare metal, this isn't required.
* `cc.h`: Contains settings related to machine architecture and compiler in use.

## TODO: 

* Determine the HSE, CSI, HSI values.
* Unit test DMA buffer classes, I'm sure at least the TX one has a bug in it.
* Move stack and functions and application data to DTCMEM & ITCMEM
* Investigate industrial protocols: EtherNet/IP - other possibilites: ControlNet, DeviceNet, Modbus, Profibus, 
    EtherCAT and CC-Link
* Make sure MPU region size and the LWIP RAM size are the same.
* Allow _write to always use DMA by servicing the debug uart in the _read while loop.
* Figure out where the 8Mhz HSE clock comes from on the board schematic (does it come from the STLINK processor?)
* Re-write more optimal ethernet driver
* Get a working MQTT client.

## References

* [launch.json Exmamples for Cortex Debugging](https://github.com/haneefdm/cortex-debug-samples/blob/master/blink2/.vscode/launch.json)

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

### LwIP Porting, Debugging

* [Porting For Bare Metal](https://lwip.fandom.com/wiki/Porting_For_Bare_Metal)
* [Enabling debug output in LWIP](https://community.nxp.com/t5/LPC-Microcontrollers-Knowledge/Enabling-debug-output/ta-p/1128854)

### MQTT

* [MQTT: The Standard for IoT Messaging](https://mqtt.org/)
