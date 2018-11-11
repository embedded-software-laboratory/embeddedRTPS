# Preparation on Windows

## Fresh setup without repo
## Compiler
install mingw-w64 [https://mingw-w64.org/doku.php/download/mingw-builds].
There are problems with mingw32, as ntddndis.h is placed in "ddk/" and does not, e.g., define "NDIS_MEDIA_STATE".

Working setup configuration:  
System: i686 (yes, 32bit version)  
Threads: posix

## Prepare necessary folders and files
Create a folder e.g. named LWIP_Windows.
After the following steps, the structure within looks like this:  
- check (optional for unit tests of lwip)
- contrib  
- lwip  
- rtps  
- WpdPack  

Starting point was this [repository](https://github.com/yarrick/lwip-contrib/tree/master/ports/win32).
1. Download lwip-2.1.0.zip and contrib-2.1.0.zip from [here](http://download.savannah.nongnu.org/releases/lwip/).
2. Extract both in the LWIP_Windows folder and name them lwip and contrib, respectively.
3. If you want to run the unit tests for lwip, too, [check](https://github.com/libcheck/check/releases/) (0.12.0 tested) and extracted as well to a folder called check.
3. Download [WinPcap 4.1.2 Developers Pack](https://www.winpcap.org/devel.htm).
4. Extract it in the LWIP_Windows folder (should ne named WpdPack)
5. Clone the [RTPS repository](https://git.rwth-aachen.de/CPM/Project/UNICARagil/Students/Wuestenberg/rtps) into the LWIP_Windows folder and name it rtps.
6. Clone or download and extract the [googletest repository](https://github.com/google/googletest) into LWIP_Windows/rtps/thirdparty/googletest.
Add all files in the repository, not just the ones in the goolgletest subfolder.

Code modifications:
1. Because of a "wrong" pointer cast and non-c90-compliant things like long long, some compiler flags need to be removed. This can be simply done by editing contrib/prots/CMakeCommon.cmake. Just remove/comment out the line 82 set(LWIP_COMPILER_FLAGS ${LWIP_COMPILER_FLAGS_GNU_CLANG}) for GNU within the if CMAKE_C_COMPILER_ID STREQUAL GNU.
2. If core locking has a missing definition put the c++ clause below around the function definitions in lwipopts.h.
```cpp
// Include guard
#ifdef __cplusplus
extern "C"{
#endif

//code

#ifdef __cplusplus
}
#endif
//include guard
```

Now you should be able to either start either the sample project delivered with the lwip port in MS Visual Studio (contrib/port/win32/mscv)
or to open the the RTPS project by opening the rtps folder as a cmake project e.g. in CLion.

Attention:
Make sure you have chosen the correct compiler.

##Configuration of windows
In order to send UDP packets, I had to configure a static IPv4 address on my ethernet port which is equal to the one defined in lwipcfg.h.

# Preparation in the Ecplise-based High-Tec Free Entry Toolchain
## Prepare necessary folders and files
1. Clone this [repository](https://git.rwth-aachen.de/CPM/Project/UNICARagil/Students/Wuestenberg/Aurix_FreeRTOS_lwip).
It contains a High-Tec project which containing FreeRTOS and Lwip.
2. Clone the [RTPS repository](https://git.rwth-aachen.de/CPM/Project/UNICARagil/Students/Wuestenberg/rtps) somewhere outside the above repository.
By doing so, you avoid having a repository within an repository and you don't have (for this toolchain) irrelevant stuff like tests included.

## Condiguration of the IDE
3. NOT WORKING YET: Define an environment variable RTPS_ROOT pointing to the RTPS repository. Maybe you need to add this as a Symbol within the (1.) project properties.
(C/C++ General -> Path and Symbols -> Library Paths -> Add -> Variables..).
4. It is possible that the IDE will complain about missing references e.g. to std::array even though it finds the header and we can jump into it.
This can be resolved by adding another Symbol to C/C++ General -> Path and Symbols -> Language GNU C++.  
Symbol: __cplusplus  
Value: 201103L
5. Another Symbol we need is HIGHTEC_TOOLCHAIN. This is required to trigger the correct #ifdefs. 

# Problem FAQ
## LwIp
-  Assertion **Function called without core lock** failed, even though locks are used:  
There is a problem when using nested "LOCK_TCPIP_CORE()" calls. Lock is granted (each time), the first unlock frees it and other threads can proceed.