/* Tool to route USB Ports 8+9 to EHCI instead of XHCI on an AMD APU */

/* (C) 2022 by Harald Welte <laforge@gnumonks.org> */

/* To the extent possible under law, the author(s) have dedicated all
 * copyright and related and neighboring rights to this software to the
 * public domain worldwide. This software is distributed without any
 * warranty.
 * You should have received a copy of the CC0 Public Domain Dedication
 * along with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>.  */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/io.h>
#include <sys/types.h>
#include <sys/stat.h>

#define IOPORT_PMINDEX	0xCD6
#define IOPORT_PMDATA	0xCD7

/* Page 938 of "BKDG for AMD Family 16h Models 30-3Fh Processors" */
#define PM_REG_USB_ENABLE	0xEF

static inline void native_cpuid(unsigned int *eax, unsigned int *ebx,
				unsigned int *ecx, unsigned int *edx)
{
	/* ecx is often an input as well as an output. */
	asm volatile("cpuid"
	    : "=a" (*eax),
	      "=b" (*ebx),
	      "=c" (*ecx),
	      "=d" (*edx)
	    : "0" (*eax), "2" (*ecx));
}

static bool compatible_cpu(void)
{
	uint32_t eax, ebx, ecx, edx;
	uint16_t model, family;

	eax = 1;
	native_cpuid(&eax, &ebx, &ecx, &edx);
	family = (eax >> 8) & 0xf;
	model = (eax >> 4) & 0xf;

	if ((family == 6) | (family == 15))
		model |= ((eax >> 16) & 0x0f) << 4;

	if (family == 15)
		family += ((eax >> 20) & 0xff);

	printf("Detected CPU Family: 0x%x, Model: 0x%x\n", family, model);

	if (family != 0x16)
		return false;

	if (model < 0x30 || model > 0x3f)
		return false;

	return true;
}

static uint8_t apu_read_pm_reg(uint8_t reg)
{
	outb(reg, IOPORT_PMINDEX);
	return inb(IOPORT_PMDATA);
}

static void apu_write_pm_reg(uint8_t reg, uint8_t val)
{
	outb(reg, IOPORT_PMINDEX);
	outb(val, IOPORT_PMDATA);
}

static int apu_write_pm_reg_verify(uint8_t reg, uint8_t val)
{
	uint8_t val2;

	apu_write_pm_reg(reg, val);

	val2 = apu_read_pm_reg(reg);
	if (val != val2) {
		fprintf(stderr, "Wrote 0x%02x to register 0x%02x, but verify renders 0x%02x\n", val, reg, val2);
		return -EIO;
	}

	return 0;
}

static int usb_port89_to_ehci(bool to_ehci)
{
	uint8_t reg;

	printf("Routing USB Ports 8+8 to %s\n", to_ehci ? "EHCI2" : "XHCI");
	
	reg = apu_read_pm_reg(PM_REG_USB_ENABLE);
	if (to_ehci)
		reg &= ~0x80;	/* route USB8+USB9 to EHCI */
	else
		reg |= 0x80;
	return apu_write_pm_reg_verify(PM_REG_USB_ENABLE, reg);
}

static int ehci2_enable(bool enable)
{
	uint8_t reg;

	printf("%sabling USB EHCI Controller 2\n", enable ? "En" : "Dis");
	
	reg = apu_read_pm_reg(PM_REG_USB_ENABLE);
	if (enable)
		reg |= 0x20;
	else
		reg &= ~0x20;
	return apu_write_pm_reg_verify(PM_REG_USB_ENABLE, reg);
}

static int rescan_pci(void)
{
	int rc, fd;

	fd = open("/sys/bus/pci/rescan", O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "Error opening /sys/bus/pci/rescan: %s\n", strerror(errno));
		return fd;
	}

	printf("Instructing kernel to re-scan PCI bus\n");

	rc = write(fd, "1\n", 2);
	if (rc != 2) {
		fprintf(stderr, "Error writing to /sys/bus/pci/rescan: %s\n", strerror(errno));
		close(fd);
		return rc;
	}

	close(fd);
	return 0;
}

static void print_usage(void)
{
	printf("Usage: apu-ehci (enable|disable)\n");
}

int main(int argc, char **argv)
{
	int rc;

	rc = ioperm(IOPORT_PMINDEX, 2, 1);
	if (rc < 0) {
		fprintf(stderr, "Unalbe to access I/O ports: %s\n", strerror(errno));
		return rc;
	}

	if (argc < 2) {
		print_usage();
		exit(0);
	}

	if (!compatible_cpu()) {
		fprintf(stderr, "Your CPU is not an AMD APU compatible with this program\n");
		exit(1);
	}

	if (!strcmp(argv[1], "enable")) {
		usb_port89_to_ehci(true);
		ehci2_enable(true);
		rescan_pci();
	} else if (!strcmp(argv[1], "disable")) {
		usb_port89_to_ehci(false);
		ehci2_enable(false);
		rescan_pci();
	} else {
		print_usage();
		exit(2);
	}
}
