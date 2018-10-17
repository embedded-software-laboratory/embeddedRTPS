
# Preparation on Windows

## Fresh setup without repo
##Compiler
install mingw-w64 [https://mingw-w64.org/doku.php/download/mingw-builds].
There are problems with mingw32, as ntddndis.h is placed in "ddk/" and does not, e.g., define "NDIS_MEDIA_STATE".

Working setup configuration:  
System: i686 (yes, 32bit version)  
Threads: posix

# Prepare necessary folders and files
Create a folder e.g. named LWIP_Windows.
After the following steps, the structure within looks like this:  
- check (optional for unit tests of lwip)
- contrib  
- lwip  
- rtps  
- WpdPack  

Starting point was this repository [https://github.com/yarrick/lwip-contrib/tree/master/ports/win32]
1. Download lwip-2.1.0.zip and contrib-2.1.0.zip from [http://download.savannah.nongnu.org/releases/lwip/]
2. Extract both in the LWIP_Windows folder and name them lwip and contrib, respectively.
3. If you want to run the unit tests for lwip, too, check (0.12.0 tested) should be downloaded [https://github.com/libcheck/check/releases/] and extracted as well to a folder called check.
3. Download WinPcap 4.1.2 Developers Pack [https://www.winpcap.org/devel.htm]
4. Extract it in the LWIP_Windows folder (should ne named WpdPack)
5. Clone the rtps repository into the LWIP_Windows folder and name it rtps.
6. Clone or download and extract the googletest repository [https://github.com/google/googletest] into LWIP_Windows/rtps/thirdparty/googletest.
Add all files in the repository, not just the ones in the goolgletest subfolder.
7. Because of a "wrong" pointer cast and non-c90-compliant things like long long, some compiler flags need to be removed. This can be simply done by editing contrib/prots/CMakeCommon.cmake. Just remove/comment out the line 82 set(LWIP_COMPILER_FLAGS ${LWIP_COMPILER_FLAGS_GNU_CLANG}) for GNU within the if CMAKE_C_COMPILER_ID STREQUAL GNU.


Now you should be able to either start the sample project in MS Visual Studio (contrib/port/win32/mscv) or open the rtps folder as a cmake project.

Attention:
Make sure you have chosen the correct compiler.
