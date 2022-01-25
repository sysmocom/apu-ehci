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

Which should render something like this in your dmesg:

	usb 2-1: USB disconnect, device number 2
	pci 0000:00:16.0: [1022:7808] type 00 class 0x0c0320
	pci 0000:00:16.0: reg 0x10: [mem 0x00000000-0x000000ff]
	pci 0000:00:16.0: supports D1 D2
	pci 0000:00:16.0: PME# supported from D0 D1 D2 D3hot D3cold
	pci 0000:00:16.0: BAR 0: assigned [mem 0xe0000000-0xe00000ff]
	pci 0000:00:16.0: enabling device (0000 -> 0002)
	pci 0000:00:16.0: PME# does not work under D3, disabling it
	ehci-pci 0000:00:16.0: EHCI Host Controller
	ehci-pci 0000:00:16.0: new USB bus registered, assigned bus number 4
	ehci-pci 0000:00:16.0: applying AMD SB700/SB800/Hudson-2/3 EHCI dummy qh workaround
	ehci-pci 0000:00:16.0: debug port 2
	ehci-pci 0000:00:16.0: irq 55, io mem 0xe0000000
	ehci-pci 0000:00:16.0: USB 2.0 started, EHCI 1.00
	usb usb4: New USB device found, idVendor=1d6b, idProduct=0002, bcdDevice= 5.10
	usb usb4: New USB device strings: Mfr=3, Product=2, SerialNumber=1
	usb usb4: Product: EHCI Host Controller
	usb usb4: Manufacturer: Linux 5.10.0-10-amd64 ehci_hcd
	usb usb4: SerialNumber: 0000:00:16.0
	hub 4-0:1.0: USB hub found
	hub 4-0:1.0: 2 ports detected
	usb 4-1: new high-speed USB device number 2 using ehci-pci
	usb 4-1: New USB device found, idVendor=0438, idProduct=7900, bcdDevice= 0.18
	usb 4-1: New USB device strings: Mfr=0, Product=0, SerialNumber=0
	hub 4-1:1.0: USB hub found
	hub 4-1:1.0: 2 ports detected

Furthermore, the USB device[s] attahced to port 8/9 should now re-enumerate on the new bus:

	usb 4-1.1: new full-speed USB device number 3 using ehci-pci
	usb 4-1.1: New USB device found, idVendor=1d50, idProduct=6145, bcdDevice= 0.03
	usb 4-1.1: New USB device strings: Mfr=2, Product=3, SerialNumber=1
	usb 4-1.1: Product: icE1usb
	usb 4-1.1: Manufacturer: osmocom
	usb 4-1.1: SerialNumber: dc697407e7881531


You can theoretically disable it again with the following command (not working here):

	./apu-ehci disable

WARNING:: The tool meddles with low-level internals of the APU SoC, so
there is a risk it may crash, lock up or otherwise behave strangely.  If
you want to be safe, it is recommended to first unload the `ehci_pci`
and `xhci_pci` modules before calling the apu-ehci tool.  YMMV.

Further Reading
---------------

Official AMD BIOS and Kernel Developer Guide available from
<https://www.amd.com/system/files/TechDocs/52740_16h_Models_30h-3Fh_BKDG.pdf>
