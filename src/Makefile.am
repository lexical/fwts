# Must build in this order:
#   1. acpica/source/compiler
#   2. lib
#   3. acpica
# ... because the libs in this bundled acpica/ depend on lib/ (libfwts.so),
# but libfwts.so depends on libraries produced by acpica/source/compiler.
SUBDIRS = acpica/source/compiler lib acpica

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/lib/include \
	-I$(top_srcdir)/src/acpica/source/include \
	-I$(top_srcdir)/src/acpica/source/compiler \
	-I$(top_srcdir)/efi_runtime \
	-pthread `pkg-config --cflags glib-2.0 gio-2.0` \
	-Wall -Werror -Wextra

bin_PROGRAMS = fwts

fwts_CPPFLAGS = $(AM_CPPFLAGS) -DACPI_DEBUG_OUTPUT

#
#  fwts main + tests
#
fwts_SOURCES = main.c 				\
	acpi/ac_adapter/ac_adapter.c 		\
	acpi/acpidump/acpidump.c 		\
	acpi/acpiinfo/acpiinfo.c 		\
	acpi/acpitables/acpitables.c 		\
	acpi/apicinstance/apicinstance.c 	\
	acpi/battery/battery.c 			\
	acpi/brightness/brightness-helper.c	\
	acpi/brightness/brightness.c 		\
	acpi/brightness/autobrightness.c 	\
	acpi/checksum/checksum.c 		\
	acpi/crsdump/crsdump.c			\
	acpi/crsdump/prsdump.c			\
	acpi/cstates/cstates.c 			\
	acpi/dmar/dmar.c 			\
	acpi/fadt/fadt.c 			\
	acpi/fan/fan.c 				\
	acpi/gpedump/gpedump.c			\
	acpi/lid/lid.c 				\
	acpi/mcfg/mcfg.c 			\
	acpi/method/method.c 			\
	acpi/osilinux/osilinux.c 		\
	acpi/pcc/pcc.c 				\
	acpi/powerbutton/powerbutton.c 		\
	acpi/plddump/plddump.c			\
	acpi/s3/s3.c 				\
	acpi/s3power/s3power.c 			\
	acpi/s4/s4.c 				\
	acpi/syntaxcheck/syntaxcheck.c 		\
	acpi/wakealarm/wakealarm.c 		\
	acpi/wmi/wmi.c 				\
	apic/apicedge/apicedge.c 		\
	bios/bios_info/bios_info.c 		\
	bios/bios32/bios32.c 			\
	bios/ebda_region/ebda_region.c 		\
	bios/ebdadump/ebdadump.c 		\
	bios/hdaaudio/hdaaudio.c 		\
	bios/memmapdump/memmapdump.c 		\
	bios/mtrr/mtrr.c 			\
	bios/multiproc/mpcheck.c 		\
	bios/multiproc/mpdump.c 		\
	bios/os2gap/os2gap.c 			\
	bios/pciirq/pciirq.c 			\
	bios/pnp/pnp.c 				\
	bios/romdump/romdump.c 			\
	cmos/cmosdump/cmosdump.c 		\
	cpu/virt/virt.c 			\
	cpu/virt/virt_svm.c 			\
	cpu/virt/virt_vmx.c			\
	cpu/maxfreq/maxfreq.c 			\
	cpu/cpufreq/cpufreq.c 			\
	cpu/nx/nx.c 				\
	cpu/msr/msr.c 				\
	cpu/microcode/microcode.c 		\
	dmi/dmicheck/dmicheck.c 		\
	hotkey/hotkey/hotkey.c 			\
	hpet/hpet_check/hpet_check.c 		\
	kernel/klog/klog.c 			\
	kernel/oops/oops.c 			\
	kernel/version/version.c 		\
	pci/aspm/aspm.c 			\
	pci/crs/crs.c 				\
	pci/maxreadreq/maxreadreq.c 		\
	uefi/csm/csm.c 				\
	uefi/uefidump/uefidump.c 		\
	uefi/uefirttime/uefirttime.c		\
	uefi/uefirtvariable/uefirtvariable.c	\
	uefi/uefirtmisc/uefirtmisc.c		\
	uefi/securebootcert/securebootcert.c	\
	uefi/uefivarinfo/uefivarinfo.c		\
	uefi/uefibootpath/uefibootpath.c

fwts_LDFLAGS = -lm `pkg-config --libs glib-2.0 gio-2.0`

fwts_LDADD = \
	-L$(top_builddir)/src/acpica -lfwtsacpica \
	-Llib/src -lfwts

man_MANS = ../doc/fwts.1 ../doc/fwts-collect.1 ../doc/fwts-frontend-text.1
