# embeddedRTPS

This repository contains source code for embeddedRTPS, a portable and open-source C++ implementation of the Real-Time Publish-Subscribe Protocol (RTPS) for embedded system.  RTPS is based on the publish-subscribe mechanism and is at the core of the Data Distribution Service (DDS). DDS is used, among many other applications, in Robot Operating System 2 (ROS2) and is also part of the AUTOSAR Adaptive platform. embeddedRTPS allows to integrate Ethernet-capable microcontrollers into DDS-based systems as first-class participants.

embeddedRTPS is portable, as it only consumes lightweightIP and FreeRTOS APIs, which are available for a large number of embedded systems. embeddedRTPS avoids dynamic memory allocation once endpoints are constructed possible. Please note that embeddedRTPS only implements rudimentary Quality-of-Service (QoS) policies and is far from a complete RTPS implementation.

More information is provided in our ITSC'2019 publication [1], which we kindly ask you to consider citing if you find embeddedRTPS helpful for your work. 

### Features

**Discovery** Simple Participant Discovery Protocol (SPDP) as well as Simple Endpoint Discovery Protocol (SEDP) are implemented.

**Interoperability** We have sucessfully tested interoperability with **FastDDS 2.3.1**

**QoS Policies** Both reliable as well as best-effort endpoints are implemented.

**UDP Multicast** Our implementation supports multicast locators.

**Message Size** Message size is currently limited by lwIP buffer size and are not split up among multiple buffers

### Supported Platforms

We have successfully ported embeddedRTPS on the following platforms:

- Infineon Aurix
- Xilinx UltraScale+ Cortex R5 
- STM32F767ZI

Due to license issues, we can only make source code available for the STM32.

### Examples

The following repository provides code examples for running embeddedRTPS on the STM32F767ZI

- [STM32F767ZI](https://github.com/embedded-software-laboratory/embeddedRTPS-STM32)

#### Runnig on Linux & Unittests

The following repository allow to compile and run embeddedRTPS on Linux for development purposes. This repository also contains unittests for embeddedRTPS.

- [embeddedRTPS-linux](https://github.com/embedded-software-laboratory/embeddedRTPS-Linux)

### Third Party Libraries

embeddedRTPS makes use of the following third party libraries:
- lwIP (Raw Mode)
- FreeRTOS
- [eProsima Micro-CDR](https://github.com/eProsima/Micro-CDR)

### Performance

Round-trip-times (RTT) for different platforms and packet sizes are depicted in the tables below.

**Table 1** 2x Infineon Aurix TC277 running embeddedRTPS. 

**Table 2** STM32F7 running embeddedRTPS connected to an Intel NUC running eProsima FastRTPS 1.8.0.

<img src="https://raw.githubusercontent.com/embedded-software-laboratory/embeddedRTPS/master/media/performance_rtt.png" width="60%">

### Acknowledgment
embeddedRTPS has been developed at **[i11 - Embedded Software, RWTH Aachen University](https://www.embedded.rwth-aachen.de)** in the context of the **[UNICARagil](https://www.unicaragil.de/en/)** project.

*This research is accomplished within the project “UNICARagil” (FKZ EM2ADIS002). We acknowledge the financial support for the project by the Federal Ministry of Education and Research of Germany (BMBF).*


### References

<details><summary>[1] A. Kampmann, A. Wüstenberg, B. Alrifaee and S. Kowalewski, "A Portable Implementation of the Real-Time Publish-Subscribe Protocol for Microcontrollers in Distributed Robotic Applications," 2019 IEEE Intelligent Transportation Systems Conference (ITSC), Auckland, New Zealand, 2019, pp. 443-448.
doi: 10.1109/ITSC.2019.8916835</summary>
<p>

```
@INPROCEEDINGS{8916835, 
author={A. {Kampmann} and A. {Wüstenberg} and B. {Alrifaee} and S. {Kowalewski}}, 
booktitle={2019 IEEE Intelligent Transportation Systems Conference (ITSC)}, 
title={A Portable Implementation of the Real-Time Publish-Subscribe Protocol for Microcontrollers in Distributed Robotic Applications}, 
year={2019}, 
volume={}, 
number={}, 
pages={443-448}, 
keywords={automobiles;intelligent transportation systems;microcontrollers;middleware;operating systems (computers);protocols;robot programming;Robot Operating System 2;microcontrollers;distributed automotive applications;distributed robotic applications;Data Distribution Service;DDS;open-source RTPS implementations;publish-subscribe protocol;AUTOSAR Adaptive platform;Protocols;Automotive engineering;Message systems;Real-time systems;Microcontrollers;Middleware;Operating systems}, 
doi={10.1109/ITSC.2019.8916835}, 
ISSN={null}, 
month={Oct},}
```

</p>
</details>
