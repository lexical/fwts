/******************************************************************************
 *
 * Module Name: adisasm - Application-level disassembler routines
 *
 *****************************************************************************/

/******************************************************************************
 *
 * 1. Copyright Notice
 *
 * Some or all of this work - Copyright (c) 1999 - 2014, Intel Corp.
 * All rights reserved.
 *
 * 2. License
 *
 * 2.1. This is your license from Intel Corp. under its intellectual property
 * rights. You may have additional license terms from the party that provided
 * you this software, covering your right to use that party's intellectual
 * property rights.
 *
 * 2.2. Intel grants, free of charge, to any person ("Licensee") obtaining a
 * copy of the source code appearing in this file ("Covered Code") an
 * irrevocable, perpetual, worldwide license under Intel's copyrights in the
 * base code distributed originally by Intel ("Original Intel Code") to copy,
 * make derivatives, distribute, use and display any portion of the Covered
 * Code in any form, with the right to sublicense such rights; and
 *
 * 2.3. Intel grants Licensee a non-exclusive and non-transferable patent
 * license (with the right to sublicense), under only those claims of Intel
 * patents that are infringed by the Original Intel Code, to make, use, sell,
 * offer to sell, and import the Covered Code and derivative works thereof
 * solely to the minimum extent necessary to exercise the above copyright
 * license, and in no event shall the patent license extend to any additions
 * to or modifications of the Original Intel Code. No other license or right
 * is granted directly or by implication, estoppel or otherwise;
 *
 * The above copyright and patent license is granted only if the following
 * conditions are met:
 *
 * 3. Conditions
 *
 * 3.1. Redistribution of Source with Rights to Further Distribute Source.
 * Redistribution of source code of any substantial portion of the Covered
 * Code or modification with rights to further distribute source must include
 * the above Copyright Notice, the above License, this list of Conditions,
 * and the following Disclaimer and Export Compliance provision. In addition,
 * Licensee must cause all Covered Code to which Licensee contributes to
 * contain a file documenting the changes Licensee made to create that Covered
 * Code and the date of any change. Licensee must include in that file the
 * documentation of any changes made by any predecessor Licensee. Licensee
 * must include a prominent statement that the modification is derived,
 * directly or indirectly, from Original Intel Code.
 *
 * 3.2. Redistribution of Source with no Rights to Further Distribute Source.
 * Redistribution of source code of any substantial portion of the Covered
 * Code or modification without rights to further distribute source must
 * include the following Disclaimer and Export Compliance provision in the
 * documentation and/or other materials provided with distribution. In
 * addition, Licensee may not authorize further sublicense of source of any
 * portion of the Covered Code, and must include terms to the effect that the
 * license from Licensee to its licensee is limited to the intellectual
 * property embodied in the software Licensee provides to its licensee, and
 * not to intellectual property embodied in modifications its licensee may
 * make.
 *
 * 3.3. Redistribution of Executable. Redistribution in executable form of any
 * substantial portion of the Covered Code or modification must reproduce the
 * above Copyright Notice, and the following Disclaimer and Export Compliance
 * provision in the documentation and/or other materials provided with the
 * distribution.
 *
 * 3.4. Intel retains all right, title, and interest in and to the Original
 * Intel Code.
 *
 * 3.5. Neither the name Intel nor any other trademark owned or controlled by
 * Intel shall be used in advertising or otherwise to promote the sale, use or
 * other dealings in products derived from or relating to the Covered Code
 * without prior written authorization from Intel.
 *
 * 4. Disclaimer and Export Compliance
 *
 * 4.1. INTEL MAKES NO WARRANTY OF ANY KIND REGARDING ANY SOFTWARE PROVIDED
 * HERE. ANY SOFTWARE ORIGINATING FROM INTEL OR DERIVED FROM INTEL SOFTWARE
 * IS PROVIDED "AS IS," AND INTEL WILL NOT PROVIDE ANY SUPPORT, ASSISTANCE,
 * INSTALLATION, TRAINING OR OTHER SERVICES. INTEL WILL NOT PROVIDE ANY
 * UPDATES, ENHANCEMENTS OR EXTENSIONS. INTEL SPECIFICALLY DISCLAIMS ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY, NONINFRINGEMENT AND FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 * 4.2. IN NO EVENT SHALL INTEL HAVE ANY LIABILITY TO LICENSEE, ITS LICENSEES
 * OR ANY OTHER THIRD PARTY, FOR ANY LOST PROFITS, LOST DATA, LOSS OF USE OR
 * COSTS OF PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, OR FOR ANY INDIRECT,
 * SPECIAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THIS AGREEMENT, UNDER ANY
 * CAUSE OF ACTION OR THEORY OF LIABILITY, AND IRRESPECTIVE OF WHETHER INTEL
 * HAS ADVANCE NOTICE OF THE POSSIBILITY OF SUCH DAMAGES. THESE LIMITATIONS
 * SHALL APPLY NOTWITHSTANDING THE FAILURE OF THE ESSENTIAL PURPOSE OF ANY
 * LIMITED REMEDY.
 *
 * 4.3. Licensee shall not export, either directly or indirectly, any of this
 * software or system incorporating such software without first obtaining any
 * required license or other approval from the U. S. Department of Commerce or
 * any other agency or department of the United States Government. In the
 * event Licensee exports any such software from the United States or
 * re-exports any such software from a foreign destination, Licensee shall
 * ensure that the distribution and export/re-export of the software is in
 * compliance with all laws, regulations, orders, or other restrictions of the
 * U.S. Export Administration Regulations. Licensee agrees that neither it nor
 * any of its subsidiaries will export/re-export any technical data, process,
 * software, or service, directly or indirectly, to any country for which the
 * United States government or any agency thereof requires an export license,
 * other governmental approval, or letter of assurance, without first obtaining
 * such license, approval or letter.
 *
 *****************************************************************************/

#include "aslcompiler.h"
#include "acparser.h"
#include "amlcode.h"
#include "acdisasm.h"
#include "acdispat.h"
#include "acnamesp.h"
#include "actables.h"
#include "acapps.h"

#include <stdio.h>
#include <time.h>


#define _COMPONENT          ACPI_TOOLS
        ACPI_MODULE_NAME    ("adisasm")

/* Local prototypes */

static void
AdCreateTableHeader (
    char                    *Filename,
    ACPI_TABLE_HEADER       *Table);

/* Stubs for ASL compiler */

#ifndef ACPI_ASL_COMPILER
BOOLEAN
AcpiDsIsResultUsed (
    ACPI_PARSE_OBJECT       *Op,
    ACPI_WALK_STATE         *WalkState)
{
    return TRUE;
}

ACPI_STATUS
AcpiDsMethodError (
    ACPI_STATUS             Status,
    ACPI_WALK_STATE         *WalkState)
{
    return (Status);
}
#endif

ACPI_STATUS
AcpiNsLoadTable (
    UINT32                  TableIndex,
    ACPI_NAMESPACE_NODE     *Node)
{
    return (AE_NOT_IMPLEMENTED);
}

ACPI_STATUS
AcpiDsRestartControlMethod (
    ACPI_WALK_STATE         *WalkState,
    ACPI_OPERAND_OBJECT     *ReturnDesc)
{
    return (AE_OK);
}

void
AcpiDsTerminateControlMethod (
    ACPI_OPERAND_OBJECT     *MethodDesc,
    ACPI_WALK_STATE         *WalkState)
{
    return;
}

ACPI_STATUS
AcpiDsCallControlMethod (
    ACPI_THREAD_STATE       *Thread,
    ACPI_WALK_STATE         *WalkState,
    ACPI_PARSE_OBJECT       *Op)
{
    return (AE_OK);
}

ACPI_STATUS
AcpiDsMethodDataInitArgs (
    ACPI_OPERAND_OBJECT     **Params,
    UINT32                  MaxParamCount,
    ACPI_WALK_STATE         *WalkState)
{
    return (AE_OK);
}


static ACPI_TABLE_DESC      LocalTables[1];
static ACPI_PARSE_OBJECT    *AcpiGbl_ParseOpRoot;


/*******************************************************************************
 *
 * FUNCTION:    AdInitialize
 *
 * PARAMETERS:  None
 *
 * RETURN:      Status
 *
 * DESCRIPTION: ACPICA and local initialization
 *
 ******************************************************************************/

ACPI_STATUS
AdInitialize (
    void)
{
    ACPI_STATUS             Status;


    /* ACPICA subsystem initialization */

    Status = AcpiOsInitialize ();
    if (ACPI_FAILURE (Status))
    {
        return (Status);
    }

    Status = AcpiUtInitGlobals ();
    if (ACPI_FAILURE (Status))
    {
        return (Status);
    }

    Status = AcpiUtMutexInitialize ();
    if (ACPI_FAILURE (Status))
    {
        return (Status);
    }

    Status = AcpiNsRootInitialize ();
    if (ACPI_FAILURE (Status))
    {
        return (Status);
    }

    /* Setup the Table Manager (cheat - there is no RSDT) */

    AcpiGbl_RootTableList.MaxTableCount = 1;
    AcpiGbl_RootTableList.CurrentTableCount = 0;
    AcpiGbl_RootTableList.Tables = LocalTables;

    return (Status);
}


/******************************************************************************
 *
 * FUNCTION:    AdAmlDisassemble
 *
 * PARAMETERS:  Filename            - AML input filename
 *              OutToFile           - TRUE if output should go to a file
 *              Prefix              - Path prefix for output
 *              OutFilename         - where the filename is returned
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Disassemble an entire ACPI table
 *
 *****************************************************************************/

ACPI_STATUS
AdAmlDisassemble (
    BOOLEAN                 OutToFile,
    char                    *Filename,
    char                    *Prefix,
    char                    **OutFilename)
{
    ACPI_STATUS             Status;
    char                    *DisasmFilename = NULL;
    char                    *ExternalFilename;
    ACPI_EXTERNAL_FILE      *ExternalFileList = AcpiGbl_ExternalFileList;
    FILE                    *File = NULL;
    ACPI_TABLE_HEADER       *Table = NULL;
    ACPI_TABLE_HEADER       *ExternalTable;
    ACPI_OWNER_ID           OwnerId;


    /*
     * Input: AML code from either a file or via GetTables (memory or
     * registry)
     */
    if (Filename)
    {
        Status = AcpiDbGetTableFromFile (Filename, &Table);
        if (ACPI_FAILURE (Status))
        {
            return (Status);
        }

        /*
         * External filenames separated by commas
         * Example: iasl -e file1,file2,file3 -d xxx.aml
         */
        while (ExternalFileList)
        {
            ExternalFilename = ExternalFileList->Path;
            if (!ACPI_STRCMP (ExternalFilename, Filename))
            {
                /* Next external file */

                ExternalFileList = ExternalFileList->Next;
                continue;
            }

            Status = AcpiDbGetTableFromFile (ExternalFilename, &ExternalTable);
            if (ACPI_FAILURE (Status))
            {
                return (Status);
            }

            /* Load external table for symbol resolution */

            if (ExternalTable)
            {
                Status = AdParseTable (ExternalTable, &OwnerId, TRUE, TRUE);
                if (ACPI_FAILURE (Status))
                {
                    AcpiOsPrintf ("Could not parse external ACPI tables, %s\n",
                        AcpiFormatException (Status));
                    return (Status);
                }

                /*
                 * Load namespace from names created within control methods
                 * Set owner id of nodes in external table
                 */
                AcpiDmFinishNamespaceLoad (AcpiGbl_ParseOpRoot,
                    AcpiGbl_RootNode, OwnerId);
                AcpiPsDeleteParseTree (AcpiGbl_ParseOpRoot);
            }

            /* Next external file */

            ExternalFileList = ExternalFileList->Next;
        }

        /* Clear external list generated by Scope in external tables */

        if (AcpiGbl_ExternalFileList)
        {
            AcpiDmClearExternalList ();
        }

        /* Load any externals defined in the optional external ref file */

        AcpiDmGetExternalsFromFile ();
    }
    else
    {
        Status = AdGetLocalTables ();
        if (ACPI_FAILURE (Status))
        {
            AcpiOsPrintf ("Could not get ACPI tables, %s\n",
                AcpiFormatException (Status));
            return (Status);
        }

        if (!AcpiGbl_DbOpt_disasm)
        {
            return (AE_OK);
        }

        /* Obtained the local tables, just disassemble the DSDT */

        Status = AcpiGetTable (ACPI_SIG_DSDT, 0, &Table);
        if (ACPI_FAILURE (Status))
        {
            AcpiOsPrintf ("Could not get DSDT, %s\n",
                AcpiFormatException (Status));
            return (Status);
        }

        AcpiOsPrintf ("\nDisassembly of DSDT\n");
        Prefix = AdGenerateFilename ("dsdt", Table->OemTableId);
    }

    /*
     * Output: ASL code. Redirect to a file if requested
     */
    if (OutToFile)
    {
        /* Create/Open a disassembly output file */

        DisasmFilename = FlGenerateFilename (Prefix, FILE_SUFFIX_DISASSEMBLY);
        if (!DisasmFilename)
        {
            fprintf (stderr, "Could not generate output filename\n");
            Status = AE_ERROR;
            goto Cleanup;
        }

        File = fopen (DisasmFilename, "w+");
        if (!File)
        {
            fprintf (stderr, "Could not open output file %s\n", DisasmFilename);
            Status = AE_ERROR;
            ACPI_FREE (DisasmFilename);
            goto Cleanup;
        }

        AcpiOsRedirectOutput (File);
    }

    *OutFilename = DisasmFilename;

    if (!AcpiUtIsAmlTable (Table))
    {
        AdDisassemblerHeader (Filename);
        AcpiOsPrintf (" * ACPI Data Table [%4.4s]\n *\n",
            Table->Signature);
        AcpiOsPrintf (" * Format: [HexOffset DecimalOffset ByteLength]  "
            "FieldName : FieldValue\n */\n\n");

        AcpiDmDumpDataTable (Table);
        fprintf (stderr, "Acpi Data Table [%4.4s] decoded\n",
            Table->Signature);
        fprintf (stderr, "Formatted output:  %s - %u bytes\n",
            DisasmFilename, CmGetFileSize (File));
    }
    else
    {
        /* Always parse the tables, only option is what to display */

        Status = AdParseTable (Table, &OwnerId, TRUE, FALSE);
        if (ACPI_FAILURE (Status))
        {
            AcpiOsPrintf ("Could not parse ACPI tables, %s\n",
                AcpiFormatException (Status));
            goto Cleanup;
        }

        if (AslCompilerdebug)
        {
            AcpiOsPrintf ("/**** Before second load\n");

            NsSetupNamespaceListing (File);
            NsDisplayNamespace ();
            AcpiOsPrintf ("*****/\n");
        }

        /* Load namespace from names created within control methods */

        AcpiDmFinishNamespaceLoad (AcpiGbl_ParseOpRoot,
            AcpiGbl_RootNode, OwnerId);

        /*
         * Cross reference the namespace here, in order to
         * generate External() statements
         */
        AcpiDmCrossReferenceNamespace (AcpiGbl_ParseOpRoot,
            AcpiGbl_RootNode, OwnerId);

        if (AslCompilerdebug)
        {
            AcpiDmDumpTree (AcpiGbl_ParseOpRoot);
        }

        /* Find possible calls to external control methods */

        AcpiDmFindOrphanMethods (AcpiGbl_ParseOpRoot);

        /*
         * If we found any external control methods, we must reparse
         * the entire tree with the new information (namely, the
         * number of arguments per method)
         */
        if (AcpiDmGetExternalMethodCount ())
        {
            fprintf (stderr,
                "\nFound %u external control methods, "
                "reparsing with new information\n",
                AcpiDmGetExternalMethodCount ());

            /* Reparse, rebuild namespace */

            AcpiPsDeleteParseTree (AcpiGbl_ParseOpRoot);
            AcpiGbl_ParseOpRoot = NULL;
            AcpiNsDeleteNamespaceSubtree (AcpiGbl_RootNode);

            AcpiGbl_RootNode                    = NULL;
            AcpiGbl_RootNodeStruct.Name.Integer = ACPI_ROOT_NAME;
            AcpiGbl_RootNodeStruct.DescriptorType = ACPI_DESC_TYPE_NAMED;
            AcpiGbl_RootNodeStruct.Type         = ACPI_TYPE_DEVICE;
            AcpiGbl_RootNodeStruct.Parent       = NULL;
            AcpiGbl_RootNodeStruct.Child        = NULL;
            AcpiGbl_RootNodeStruct.Peer         = NULL;
            AcpiGbl_RootNodeStruct.Object       = NULL;
            AcpiGbl_RootNodeStruct.Flags        = 0;

            Status = AcpiNsRootInitialize ();

            /* New namespace, add the external definitions first */

            AcpiDmAddExternalsToNamespace ();

            /* Parse the table again. No need to reload it, however */

            Status = AdParseTable (Table, NULL, FALSE, FALSE);
            if (ACPI_FAILURE (Status))
            {
                AcpiOsPrintf ("Could not parse ACPI tables, %s\n",
                    AcpiFormatException (Status));
                goto Cleanup;
            }

            /* Cross reference the namespace again */

            AcpiDmFinishNamespaceLoad (AcpiGbl_ParseOpRoot,
                AcpiGbl_RootNode, OwnerId);

            AcpiDmCrossReferenceNamespace (AcpiGbl_ParseOpRoot,
                AcpiGbl_RootNode, OwnerId);

            if (AslCompilerdebug)
            {
                AcpiOsPrintf ("/**** After second load and resource conversion\n");
                NsSetupNamespaceListing (File);
                NsDisplayNamespace ();
                AcpiOsPrintf ("*****/\n");

                AcpiDmDumpTree (AcpiGbl_ParseOpRoot);
            }
        }

        /*
         * Now that the namespace is finalized, we can perform namespace
         * transforms.
         *
         * 1) Convert fixed-offset references to resource descriptors
         *    to symbolic references (Note: modifies namespace)
         */
        AcpiDmConvertResourceIndexes (AcpiGbl_ParseOpRoot, AcpiGbl_RootNode);

        /* Optional displays */

        if (AcpiGbl_DbOpt_disasm)
        {
            /* This is the real disassembly */

            AdDisplayTables (Filename, Table);

            /* Dump hex table if requested (-vt) */

            AcpiDmDumpDataTable (Table);

            fprintf (stderr, "Disassembly completed\n");
            fprintf (stderr, "ASL Output:    %s - %u bytes\n",
                DisasmFilename, CmGetFileSize (File));
        }
    }

Cleanup:

    if (Table && !AcpiUtIsAmlTable (Table))
    {
        ACPI_FREE (Table);
    }

    if (OutToFile && File)
    {
        if (AslCompilerdebug) /* Display final namespace, with transforms */
        {
            NsSetupNamespaceListing (File);
            NsDisplayNamespace ();
        }

        fclose (File);
        AcpiOsRedirectOutput (stdout);
    }

    AcpiPsDeleteParseTree (AcpiGbl_ParseOpRoot);
    AcpiGbl_ParseOpRoot = NULL;
    return (Status);
}


/******************************************************************************
 *
 * FUNCTION:    AdDisassemblerHeader
 *
 * PARAMETERS:  Filename            - Input file for the table
 *
 * RETURN:      None
 *
 * DESCRIPTION: Create the disassembler header, including ACPICA signon with
 *              current time and date.
 *
 *****************************************************************************/

void
AdDisassemblerHeader (
    char                    *Filename)
{
    time_t                  Timer;

    time (&Timer);

    /* Header and input table info */

    AcpiOsPrintf ("/*\n");
    AcpiOsPrintf (ACPI_COMMON_HEADER ("AML Disassembler", " * "));

    AcpiOsPrintf (" * Disassembly of %s, %s", Filename, ctime (&Timer));
    AcpiOsPrintf (" *\n");
}


/******************************************************************************
 *
 * FUNCTION:    AdCreateTableHeader
 *
 * PARAMETERS:  Filename            - Input file for the table
 *              Table               - Pointer to the raw table
 *
 * RETURN:      None
 *
 * DESCRIPTION: Create the ASL table header, including ACPICA signon with
 *              current time and date.
 *
 *****************************************************************************/

static void
AdCreateTableHeader (
    char                    *Filename,
    ACPI_TABLE_HEADER       *Table)
{
    char                    *NewFilename;
    UINT8                   Checksum;


    /*
     * Print file header and dump original table header
     */
    AdDisassemblerHeader (Filename);

    AcpiOsPrintf (" * Original Table Header:\n");
    AcpiOsPrintf (" *     Signature        \"%4.4s\"\n",    Table->Signature);
    AcpiOsPrintf (" *     Length           0x%8.8X (%u)\n", Table->Length, Table->Length);

    /* Print and validate the revision */

    AcpiOsPrintf (" *     Revision         0x%2.2X",      Table->Revision);

    switch (Table->Revision)
    {
    case 0:

        AcpiOsPrintf (" **** Invalid Revision");
        break;

    case 1:

        /* Revision of DSDT controls the ACPI integer width */

        if (ACPI_COMPARE_NAME (Table->Signature, ACPI_SIG_DSDT))
        {
            AcpiOsPrintf (" **** 32-bit table (V1), no 64-bit math support");
        }
        break;

    default:

        break;
    }
    AcpiOsPrintf ("\n");

    /* Print and validate the table checksum */

    AcpiOsPrintf (" *     Checksum         0x%2.2X",        Table->Checksum);

    Checksum = AcpiTbChecksum (ACPI_CAST_PTR (UINT8, Table), Table->Length);
    if (Checksum)
    {
        AcpiOsPrintf (" **** Incorrect checksum, should be 0x%2.2X",
            (UINT8) (Table->Checksum - Checksum));
    }
    AcpiOsPrintf ("\n");

    AcpiOsPrintf (" *     OEM ID           \"%.6s\"\n",     Table->OemId);
    AcpiOsPrintf (" *     OEM Table ID     \"%.8s\"\n",     Table->OemTableId);
    AcpiOsPrintf (" *     OEM Revision     0x%8.8X (%u)\n", Table->OemRevision, Table->OemRevision);
    AcpiOsPrintf (" *     Compiler ID      \"%.4s\"\n",     Table->AslCompilerId);
    AcpiOsPrintf (" *     Compiler Version 0x%8.8X (%u)\n", Table->AslCompilerRevision, Table->AslCompilerRevision);
    AcpiOsPrintf (" */\n");

    /* Create AML output filename based on input filename */

    if (Filename)
    {
        NewFilename = FlGenerateFilename (Filename, "aml");
    }
    else
    {
        NewFilename = UtStringCacheCalloc (9);
        if (NewFilename)
        {
            strncat (NewFilename, Table->Signature, 4);
            strcat (NewFilename, ".aml");
        }
    }

    if (!NewFilename)
    {
        AcpiOsPrintf (" **** Could not generate AML output filename\n");
        return;
    }

    /* Open the ASL definition block */

    AcpiOsPrintf (
        "DefinitionBlock (\"%s\", \"%4.4s\", %hu, \"%.6s\", \"%.8s\", 0x%8.8X)\n",
        NewFilename, Table->Signature, Table->Revision,
        Table->OemId, Table->OemTableId, Table->OemRevision);
}


/******************************************************************************
 *
 * FUNCTION:    AdDisplayTables
 *
 * PARAMETERS:  Filename            - Input file for the table
 *              Table               - Pointer to the raw table
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Display (disassemble) loaded tables and dump raw tables
 *
 *****************************************************************************/

ACPI_STATUS
AdDisplayTables (
    char                    *Filename,
    ACPI_TABLE_HEADER       *Table)
{


    if (!AcpiGbl_ParseOpRoot)
    {
        return (AE_NOT_EXIST);
    }

    if (!AcpiGbl_DbOpt_verbose)
    {
        AdCreateTableHeader (Filename, Table);
    }

    AcpiDmDisassemble (NULL, AcpiGbl_ParseOpRoot, ACPI_UINT32_MAX);

    if (AcpiGbl_DbOpt_verbose)
    {
        AcpiOsPrintf ("\n\nTable Header:\n");
        AcpiUtDebugDumpBuffer ((UINT8 *) Table, sizeof (ACPI_TABLE_HEADER),
            DB_BYTE_DISPLAY, ACPI_UINT32_MAX);

        AcpiOsPrintf ("Table Body (Length 0x%X)\n", Table->Length);
        AcpiUtDebugDumpBuffer (((UINT8 *) Table + sizeof (ACPI_TABLE_HEADER)),
            Table->Length, DB_BYTE_DISPLAY, ACPI_UINT32_MAX);
    }

    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AdGetLocalTables
 *
 * PARAMETERS:  None
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Get the ACPI tables from either memory or a file
 *
 *****************************************************************************/

ACPI_STATUS
AdGetLocalTables (
    void)
{
    ACPI_STATUS             Status;
    ACPI_TABLE_HEADER       TableHeader;
    ACPI_TABLE_HEADER       *NewTable;
    UINT32                  TableIndex;


    /* Get the DSDT via table override */

    ACPI_MOVE_32_TO_32 (TableHeader.Signature, ACPI_SIG_DSDT);
    AcpiOsTableOverride (&TableHeader, &NewTable);
    if (!NewTable)
    {
        fprintf (stderr, "Could not obtain DSDT\n");
        return (AE_NO_ACPI_TABLES);
    }

    AdWriteTable (NewTable, NewTable->Length,
        ACPI_SIG_DSDT, NewTable->OemTableId);

    /* Store DSDT in the Table Manager */

    Status = AcpiTbStoreTable (0, NewTable, NewTable->Length,
                0, &TableIndex);
    if (ACPI_FAILURE (Status))
    {
        fprintf (stderr, "Could not store DSDT\n");
        return (AE_NO_ACPI_TABLES);
    }

    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AdParseTable
 *
 * PARAMETERS:  Table               - Pointer to the raw table
 *              OwnerId             - Returned OwnerId of the table
 *              LoadTable           - If add table to the global table list
 *              External            - If this is an external table
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Parse the DSDT.
 *
 *****************************************************************************/

ACPI_STATUS
AdParseTable (
    ACPI_TABLE_HEADER       *Table,
    ACPI_OWNER_ID           *OwnerId,
    BOOLEAN                 LoadTable,
    BOOLEAN                 External)
{
    ACPI_STATUS             Status = AE_OK;
    ACPI_WALK_STATE         *WalkState;
    UINT8                   *AmlStart;
    UINT32                  AmlLength;
    UINT32                  TableIndex;


    if (!Table)
    {
        return (AE_NOT_EXIST);
    }

    /* Pass 1:  Parse everything except control method bodies */

    fprintf (stderr, "Pass 1 parse of [%4.4s]\n", (char *) Table->Signature);

    AmlLength = Table->Length - sizeof (ACPI_TABLE_HEADER);
    AmlStart = ((UINT8 *) Table + sizeof (ACPI_TABLE_HEADER));

    /* Create the root object */

    AcpiGbl_ParseOpRoot = AcpiPsCreateScopeOp ();
    if (!AcpiGbl_ParseOpRoot)
    {
        return (AE_NO_MEMORY);
    }

    /* Create and initialize a new walk state */

    WalkState = AcpiDsCreateWalkState (0,
                        AcpiGbl_ParseOpRoot, NULL, NULL);
    if (!WalkState)
    {
        return (AE_NO_MEMORY);
    }

    Status = AcpiDsInitAmlWalk (WalkState, AcpiGbl_ParseOpRoot,
                NULL, AmlStart, AmlLength, NULL, ACPI_IMODE_LOAD_PASS1);
    if (ACPI_FAILURE (Status))
    {
        return (Status);
    }

    WalkState->ParseFlags &= ~ACPI_PARSE_DELETE_TREE;
    WalkState->ParseFlags |= ACPI_PARSE_DISASSEMBLE;

    Status = AcpiPsParseAml (WalkState);
    if (ACPI_FAILURE (Status))
    {
        return (Status);
    }

    /* If LoadTable is FALSE, we are parsing the last loaded table */

    TableIndex = AcpiGbl_RootTableList.CurrentTableCount - 1;

    /* Pass 2 */

    if (LoadTable)
    {
        Status = AcpiTbStoreTable ((ACPI_PHYSICAL_ADDRESS) Table, Table,
                    Table->Length, ACPI_TABLE_ORIGIN_INTERNAL_VIRTUAL,
                    &TableIndex);
        if (ACPI_FAILURE (Status))
        {
            return (Status);
        }
        Status = AcpiTbAllocateOwnerId (TableIndex);
        if (ACPI_FAILURE (Status))
        {
            return (Status);
        }
        if (OwnerId)
        {
            Status = AcpiTbGetOwnerId (TableIndex, OwnerId);
            if (ACPI_FAILURE (Status))
            {
                return (Status);
            }
        }
    }

    fprintf (stderr, "Pass 2 parse of [%4.4s]\n", (char *) Table->Signature);

    Status = AcpiNsOneCompleteParse (ACPI_IMODE_LOAD_PASS2, TableIndex, NULL);
    if (ACPI_FAILURE (Status))
    {
        return (Status);
    }

    /* No need to parse control methods of external table */

    if (External)
    {
        return (AE_OK);
    }

    /*
     * Pass 3: Parse control methods and link their parse trees
     * into the main parse tree
     */
    fprintf (stderr,
        "Parsing Deferred Opcodes (Methods/Buffers/Packages/Regions)\n");
    Status = AcpiDmParseDeferredOps (AcpiGbl_ParseOpRoot);
    fprintf (stderr, "\n");

    /* Process Resource Templates */

    AcpiDmFindResources (AcpiGbl_ParseOpRoot);

    fprintf (stderr, "Parsing completed\n");
    return (AE_OK);
}
