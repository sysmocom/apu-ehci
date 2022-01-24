apu-ehci - route AMD APU USB ports to EHCI instaed of XHCI
==========================================================

This repository contains the source code to a small C-language program
that can be compiled + executed on a Linux system running the AMD APU
SoC such as those used in the APU2/3/4 boards of PC-Engines.

The tool will re-configure the USB ports of the APU SoC to route
USB Ports 8+9 (those exposed on the physical USB-A connectors of the
PC-Engines boards) from XHCI to EHCI.

Why would one want to do this?  To work around bugs in the XHCI
controller firmware, such as miscalculation of the available isochronous
bandwidth for full-speed devices.  For OHCI/UHCI/EHCI, the operating
system (Linux kernel) is responsible for this, and it does tend to get
it right.   With XHCI, this is implemented in proprietary vendor
firmware, which does tend to get it wrong.

Homepage
--------

The official homepage / repository of the project is
<https://git.sysmocom.de/sysmocom/apu-ehci>

GIT Repository
--------------

You can clone from the official libosmocore.git repository using

	git clone https:///git.sysmocom.de/apu-ehci

Building
--------

As long as you have a C compiler and make installed, all you need to do
is to run `make` to build this.

Running
-------

The following command will enable the USB-to-EHCI routing:

	./apu-ehci enable


You can disable it again with

	./apu-ehci disable

WARNING:: The tool meddles with low-level internals of the APU SoC, so
there is a risk it may crash, lock up or otherwise behave strangely.  If
you want to be safe, it is recommended to first unload the `ehci_pci`
and `xhci_pci` modules before calling the apu-ehci tool.  YMMV.

Further Reading
---------------

Official AMD BIOS and Kernel Developer Guide available from
<https://www.amd.com/system/files/TechDocs/52740_16h_Models_30h-3Fh_BKDG.pdf>
