# Nb. the top-level Makefile builds source/compiler for us now...
## SUBDIRS = source/compiler

#
# -DACPI_EXEC_APP is specific to making ACPICA for the fwts method test
#
AM_CPPFLAGS = 						\
	-D_LINUX -DNDEBUG -D_CONSOLE -DACPI_EXEC_APP	\
	-I$(top_srcdir)/src/lib/include			\
	-I$(srcdir)/source/include			\
	-Wall -fno-strict-aliasing

#
#  We rename some of the functions so we can override them in fwts. This
#  is a hack, I feel dirty now.
#
osunixxf_munged.c: $(srcdir)/source/os_specific/service_layers/osunixxf.c
	cat  $^ |							\
	sed 's/^AcpiOsPrintf/__AcpiOsPrintf/' |				\
	sed 's/^AcpiOsReadPort/__AcpiOsReadPort/' |			\
	sed 's/^AcpiOsReadPciConfiguration/__AcpiOsReadPciConfiguration/' | \
	sed 's/^AcpiOsSignalSemaphore/__AcpiOsSignalSemaphore/' |	\
	sed 's/^AcpiOsWaitSemaphore/__AcpiOsWaitSemaphore/' |		\
	sed 's/^AcpiOsCreateSemaphore/__AcpiOsCreateSemaphore/' |	\
	sed 's/^AcpiOsDeleteSemaphore/__AcpiOsDeleteSemaphore/' |	\
	sed 's/^AcpiOsVprintf/__AcpiOsVprintf/' |			\
	sed 's/^AcpiOsSignal/__AcpiOsSignal/' |				\
	sed 's/^AcpiOsSleep/__AcpiOsSleep/' |				\
	sed 's/^AcpiOsExecute/__AcpiOsExecute/'				\
	> $@
#
#  Force maximum loop iterations to be just 128 instead of 0xffff
#  because we really don't want to wait until the sun turns into
#  a lump of coal before we complete a blocked operation
#
dscontrol_munged.c: $(srcdir)/source/components/dispatcher/dscontrol.c
	cat $^ |					\
	sed 's/ACPI_MAX_LOOP_ITERATIONS/0x0080/'	\
	> $@

BUILT_SOURCES = osunixxf_munged.c dscontrol_munged.c

#
#  Source files that are generated on-the fly and need cleaning
#
CLEANFILES = osunixxf_munged.c				\
	dscontrol_munged.c				\
	source/compiler/aslcompiler.output		\
	source/compiler/dtparser.output			\
	source/compiler/dtparser.y.h			\
	source/compiler/prparser.output			\
	source/compiler/prparser.y.h			\
	source/compiler/prparserlex.c			\
	source/compiler/prparserparse.c

pkglib_LTLIBRARIES = libfwtsacpica.la

libfwtsacpica_la_LDFLAGS = -lpthread -version-info 1:0:0
libfwtsacpica_la_CPPFLAGS = $(AM_CPPFLAGS)

#
#  fwts acpica library sources
#
libfwtsacpica_la_SOURCES =				\
	fwts_acpica.c					\
	osunixxf_munged.c				\
	dscontrol_munged.c				\
	source/os_specific/service_layers/oslibcfs.c	\
	source/components/debugger/dbcmds.c		\
	source/components/debugger/dbdisply.c		\
	source/components/debugger/dbexec.c		\
	source/components/debugger/dbfileio.c		\
	source/components/debugger/dbhistry.c		\
	source/components/debugger/dbinput.c		\
	source/components/debugger/dbstats.c		\
	source/components/debugger/dbutils.c		\
	source/components/debugger/dbxface.c		\
	source/components/debugger/dbmethod.c		\
	source/components/debugger/dbnames.c		\
	source/components/debugger/dbconvert.c		\
	source/components/debugger/dbtest.c		\
	source/components/disassembler/dmbuffer.c	\
	source/components/disassembler/dmnames.c	\
	source/components/disassembler/dmobject.c	\
	source/components/disassembler/dmopcode.c	\
	source/components/disassembler/dmresrc.c	\
	source/components/disassembler/dmresrcl.c	\
	source/components/disassembler/dmresrcs.c	\
	source/components/disassembler/dmutils.c	\
	source/components/disassembler/dmwalk.c		\
	source/components/disassembler/dmresrcl2.c	\
	source/components/disassembler/dmdeferred.c 	\
	source/components/dispatcher/dsfield.c		\
	source/components/dispatcher/dsinit.c		\
	source/components/dispatcher/dsmethod.c		\
	source/components/dispatcher/dsmthdat.c		\
	source/components/dispatcher/dsobject.c		\
	source/components/dispatcher/dsutils.c		\
	source/components/dispatcher/dswexec.c		\
	source/components/dispatcher/dswload.c		\
	source/components/dispatcher/dswscope.c		\
	source/components/dispatcher/dswstate.c		\
	source/components/dispatcher/dsargs.c		\
	source/components/dispatcher/dswload2.c		\
	source/components/dispatcher/dsopcode.c		\
	source/components/events/evevent.c		\
	source/components/events/evgpe.c		\
	source/components/events/evgpeblk.c		\
	source/components/events/evgpeinit.c		\
	source/components/events/evgpeutil.c		\
	source/components/events/evxfgpe.c		\
	source/components/events/evmisc.c		\
	source/components/events/evregion.c		\
	source/components/events/evrgnini.c		\
	source/components/events/evsci.c		\
	source/components/events/evxface.c		\
	source/components/events/evxfevnt.c		\
	source/components/events/evxfregn.c		\
	source/components/events/evglock.c		\
	source/components/events/evhandler.c		\
	source/components/executer/exfield.c		\
	source/components/executer/exfldio.c		\
	source/components/executer/exmisc.c		\
	source/components/executer/exmutex.c		\
	source/components/executer/exnames.c		\
	source/components/executer/exoparg1.c		\
	source/components/executer/exoparg2.c		\
	source/components/executer/exoparg3.c		\
	source/components/executer/exoparg6.c		\
	source/components/executer/exprep.c		\
	source/components/executer/exregion.c		\
	source/components/executer/exresnte.c		\
	source/components/executer/exresolv.c		\
	source/components/executer/exresop.c		\
	source/components/executer/exstore.c		\
	source/components/executer/exstoren.c		\
	source/components/executer/exstorob.c		\
	source/components/executer/exsystem.c		\
	source/components/executer/exutils.c		\
	source/components/executer/exconvrt.c		\
	source/components/executer/excreate.c		\
	source/components/executer/exdump.c		\
	source/components/executer/exdebug.c		\
	source/components/executer/exconfig.c		\
	source/components/hardware/hwacpi.c		\
	source/components/hardware/hwgpe.c		\
	source/components/hardware/hwpci.c		\
	source/components/hardware/hwregs.c		\
	source/components/hardware/hwsleep.c		\
	source/components/hardware/hwvalid.c		\
	source/components/hardware/hwxface.c		\
	source/components/hardware/hwxfsleep.c		\
	source/components/hardware/hwesleep.c		\
	source/components/namespace/nsaccess.c		\
	source/components/namespace/nsalloc.c		\
	source/components/namespace/nsdump.c		\
	source/components/namespace/nsdumpdv.c		\
	source/components/namespace/nseval.c		\
	source/components/namespace/nsinit.c		\
	source/components/namespace/nsload.c		\
	source/components/namespace/nsnames.c		\
	source/components/namespace/nsobject.c		\
	source/components/namespace/nsparse.c		\
	source/components/namespace/nspredef.c		\
	source/components/namespace/nsrepair.c		\
	source/components/namespace/nsrepair2.c		\
	source/components/namespace/nssearch.c		\
	source/components/namespace/nsutils.c		\
	source/components/namespace/nswalk.c		\
	source/components/namespace/nsxfeval.c		\
	source/components/namespace/nsxfname.c		\
	source/components/namespace/nsxfobj.c		\
	source/components/namespace/nsconvert.c		\
	source/components/namespace/nsprepkg.c		\
	source/components/namespace/nsarguments.c	\
	source/components/parser/psargs.c		\
	source/components/parser/psloop.c		\
	source/components/parser/psopcode.c		\
	source/components/parser/psparse.c		\
	source/components/parser/psscope.c		\
	source/components/parser/pstree.c		\
	source/components/parser/psutils.c		\
	source/components/parser/pswalk.c		\
	source/components/parser/psxface.c		\
	source/components/parser/psopinfo.c		\
	source/components/parser/psobject.c		\
	source/components/resources/rsaddr.c		\
	source/components/resources/rscalc.c		\
	source/components/resources/rscreate.c		\
	source/components/resources/rsdump.c		\
	source/components/resources/rsio.c		\
	source/components/resources/rsinfo.c		\
	source/components/resources/rsirq.c		\
	source/components/resources/rslist.c		\
	source/components/resources/rsmemory.c		\
	source/components/resources/rsmisc.c		\
	source/components/resources/rsutils.c		\
	source/components/resources/rsxface.c		\
	source/components/resources/rsserial.c		\
	source/components/resources/rsdumpinfo.c	\
	source/components/tables/tbdata.c		\
	source/components/tables/tbfadt.c		\
	source/components/tables/tbfind.c		\
	source/components/tables/tbinstal.c		\
	source/components/tables/tbutils.c		\
	source/components/tables/tbxface.c		\
	source/components/tables/tbxfroot.c		\
	source/components/tables/tbxfload.c		\
	source/components/tables/tbprint.c		\
	source/components/utilities/utaddress.c		\
	source/components/utilities/utalloc.c		\
	source/components/utilities/utcache.c		\
	source/components/utilities/utcopy.c		\
	source/components/utilities/utdebug.c		\
	source/components/utilities/utdelete.c		\
	source/components/utilities/uteval.c		\
	source/components/utilities/utfileio.c		\
	source/components/utilities/utglobal.c		\
	source/components/utilities/uthex.c		\
	source/components/utilities/utids.c		\
	source/components/utilities/utinit.c		\
	source/components/utilities/utlock.c		\
	source/components/utilities/utmath.c		\
	source/components/utilities/utmisc.c		\
	source/components/utilities/utmutex.c		\
	source/components/utilities/utobject.c		\
	source/components/utilities/utresrc.c		\
	source/components/utilities/utstate.c		\
	source/components/utilities/uttrack.c		\
	source/components/utilities/utosi.c		\
	source/components/utilities/utxferror.c		\
	source/components/utilities/utxface.c		\
	source/components/utilities/utdecode.c		\
	source/components/utilities/utexcep.c		\
	source/components/utilities/utpredef.c		\
	source/components/utilities/utprint.c		\
	source/components/utilities/utstring.c		\
	source/components/utilities/utownerid.c		\
	source/components/utilities/utxfinit.c		\
	source/components/utilities/uterror.c		\
	source/components/utilities/utbuffer.c 		\
	source/components/utilities/utuuid.c		\
	source/common/acgetline.c            		\
	source/common/ahids.c				\
	source/common/cmfsize.c            		\
	source/common/ahuuids.c				\
	source/tools/acpiexec/aehandlers.c

libfwtsacpica_la_LIBADD = \
	-L$(top_builddir)/src/lib/src -lfwts -lrt

