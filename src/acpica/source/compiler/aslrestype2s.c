/******************************************************************************
 *
 * Module Name: aslrestype2s - Serial Large resource descriptors
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
#include "aslcompiler.y.h"
#include "amlcode.h"

#define _COMPONENT          ACPI_COMPILER
        ACPI_MODULE_NAME    ("aslrestype2s")


static UINT16
RsGetBufferDataLength (
    ACPI_PARSE_OBJECT       *InitializerOp);

static UINT16
RsGetInterruptDataLength (
    ACPI_PARSE_OBJECT       *InitializerOp);

static BOOLEAN
RsGetVendorData (
    ACPI_PARSE_OBJECT       *InitializerOp,
    UINT8                   *VendorData,
    ACPI_SIZE               DescriptorOffset);

/*
 * This module contains descriptors for serial buses and GPIO:
 *
 * GpioInt
 * GpioIo
 * I2cSerialBus
 * SpiSerialBus
 * UartSerialBus
 */


/*******************************************************************************
 *
 * FUNCTION:    RsGetBufferDataLength
 *
 * PARAMETERS:  InitializerOp       - Current parse op, start of the resource
 *                                    descriptor
 *
 * RETURN:      Length of the data buffer
 *
 * DESCRIPTION: Get the length of a RawDataBuffer, used for vendor data.
 *
 ******************************************************************************/

static UINT16
RsGetBufferDataLength (
    ACPI_PARSE_OBJECT       *InitializerOp)
{
    UINT16                  ExtraDataSize = 0;
    ACPI_PARSE_OBJECT       *DataList;


    /* Find the byte-initializer list */

    while (InitializerOp)
    {
        if (InitializerOp->Asl.ParseOpcode == PARSEOP_DATABUFFER)
        {
            /* First child is the optional length (ignore it here) */

            DataList = InitializerOp->Asl.Child;
            DataList = ASL_GET_PEER_NODE (DataList);

            /* Count the data items (each one is a byte of data) */

            while (DataList)
            {
                ExtraDataSize++;
                DataList = ASL_GET_PEER_NODE (DataList);
            }

            return (ExtraDataSize);
        }

        InitializerOp = ASL_GET_PEER_NODE (InitializerOp);
    }

    return (ExtraDataSize);
}


/*******************************************************************************
 *
 * FUNCTION:    RsGetInterruptDataLength
 *
 * PARAMETERS:  InitializerOp       - Current parse op, start of the resource
 *                                    descriptor
 *
 * RETURN:      Length of the interrupt data list
 *
 * DESCRIPTION: Get the length of a list of interrupt DWORDs for the GPIO
 *              descriptors.
 *
 ******************************************************************************/

static UINT16
RsGetInterruptDataLength (
    ACPI_PARSE_OBJECT       *InitializerOp)
{
    UINT16                  InterruptLength;
    UINT32                  i;


    /* Count the interrupt numbers */

    InterruptLength = 0;
    for (i = 0; InitializerOp; i++)
    {
        InitializerOp = ASL_GET_PEER_NODE (InitializerOp);

        /* Interrupt list starts at offset 10 (Gpio descriptors) */

        if (i >= 10)
        {
            InterruptLength += 2;
        }
    }

    return (InterruptLength);
}


/*******************************************************************************
 *
 * FUNCTION:    RsGetVendorData
 *
 * PARAMETERS:  InitializerOp       - Current parse op, start of the resource
 *                                    descriptor.
 *              VendorData          - Where the vendor data is returned
 *              DescriptorOffset    - Where vendor data begins in descriptor
 *
 * RETURN:      TRUE if valid vendor data was returned, FALSE otherwise.
 *
 * DESCRIPTION: Extract the vendor data and construct a vendor data buffer.
 *
 ******************************************************************************/

static BOOLEAN
RsGetVendorData (
    ACPI_PARSE_OBJECT       *InitializerOp,
    UINT8                   *VendorData,
    ACPI_SIZE               DescriptorOffset)
{
    ACPI_PARSE_OBJECT       *BufferOp;
    UINT32                  SpecifiedLength = ACPI_UINT32_MAX;
    UINT16                  ActualLength = 0;


    /* Vendor Data field is always optional */

    if (InitializerOp->Asl.ParseOpcode == PARSEOP_DEFAULT_ARG)
    {
        return (FALSE);
    }

    BufferOp = InitializerOp->Asl.Child;
    if (!BufferOp)
    {
        AslError (ASL_ERROR, ASL_MSG_SYNTAX, InitializerOp, "");
        return (FALSE);
    }

    /* First child is the optional buffer length (WORD) */

    if (BufferOp->Asl.ParseOpcode != PARSEOP_DEFAULT_ARG)
    {
        SpecifiedLength = (UINT16) BufferOp->Asl.Value.Integer;
    }

    /* Insert field tag _VEN */

    RsCreateByteField (InitializerOp, ACPI_RESTAG_VENDORDATA,
        (UINT16) DescriptorOffset);

    /* Walk the list of buffer initializers (each is one byte) */

    BufferOp = RsCompleteNodeAndGetNext (BufferOp);
    if (BufferOp->Asl.ParseOpcode != PARSEOP_DEFAULT_ARG)
    {
        while (BufferOp)
        {
            *VendorData = (UINT8) BufferOp->Asl.Value.Integer;
            VendorData++;
            ActualLength++;
            BufferOp = RsCompleteNodeAndGetNext (BufferOp);
        }
    }

    /* Length validation. Buffer cannot be of zero length */

    if ((SpecifiedLength == 0) ||
        ((SpecifiedLength == ACPI_UINT32_MAX) && (ActualLength == 0)))
    {
        AslError (ASL_ERROR, ASL_MSG_BUFFER_LENGTH, InitializerOp, NULL);
        return (FALSE);
    }

    if (SpecifiedLength != ACPI_UINT32_MAX)
    {
        /* ActualLength > SpecifiedLength -> error */

        if (ActualLength > SpecifiedLength)
        {
            AslError (ASL_ERROR, ASL_MSG_LIST_LENGTH_LONG, InitializerOp, NULL);
            return (FALSE);
        }

        /* ActualLength < SpecifiedLength -> remark */

        else if (ActualLength < SpecifiedLength)
        {
            AslError (ASL_REMARK, ASL_MSG_LIST_LENGTH_SHORT, InitializerOp, NULL);
            return (FALSE);
        }
    }

    return (TRUE);
}


/*******************************************************************************
 *
 * FUNCTION:    RsDoGpioIntDescriptor
 *
 * PARAMETERS:  Op                  - Parent resource descriptor parse node
 *              CurrentByteOffset   - Offset into the resource template AML
 *                                    buffer (to track references to the desc)
 *
 * RETURN:      Completed resource node
 *
 * DESCRIPTION: Construct a long "GpioInt" descriptor
 *
 ******************************************************************************/

ASL_RESOURCE_NODE *
RsDoGpioIntDescriptor (
    ACPI_PARSE_OBJECT       *Op,
    UINT32                  CurrentByteOffset)
{
    AML_RESOURCE            *Descriptor;
    ACPI_PARSE_OBJECT       *InitializerOp;
    ASL_RESOURCE_NODE       *Rnode;
    char                    *ResourceSource = NULL;
    UINT8                   *VendorData = NULL;
    UINT16                  *InterruptList = NULL;
    UINT16                  ResSourceLength;
    UINT16                  VendorLength;
    UINT16                  InterruptLength;
    UINT16                  DescriptorSize;
    UINT32                  i;


    InitializerOp = Op->Asl.Child;

    /*
     * Calculate lengths for fields that have variable length:
     * 1) Resource Source string
     * 2) Vendor Data buffer
     * 3) PIN (interrupt) list
     */
    ResSourceLength = RsGetStringDataLength (InitializerOp);
    VendorLength = RsGetBufferDataLength (InitializerOp);
    InterruptLength = RsGetInterruptDataLength (InitializerOp);

    DescriptorSize = ACPI_AML_SIZE_LARGE (AML_RESOURCE_GPIO) +
        ResSourceLength + VendorLength + InterruptLength;

    /* Allocate the local resource node and initialize */

    Rnode = RsAllocateResourceNode (DescriptorSize + sizeof (AML_RESOURCE_LARGE_HEADER));

    Descriptor = Rnode->Buffer;
    Descriptor->Gpio.ResourceLength  = DescriptorSize;
    Descriptor->Gpio.DescriptorType  = ACPI_RESOURCE_NAME_GPIO;
    Descriptor->Gpio.RevisionId      = AML_RESOURCE_GPIO_REVISION;
    Descriptor->Gpio.ConnectionType  = AML_RESOURCE_GPIO_TYPE_INT;

    /* Build pointers to optional areas */

    InterruptList = ACPI_ADD_PTR (UINT16, Descriptor, sizeof (AML_RESOURCE_GPIO));
    ResourceSource = ACPI_ADD_PTR (char, InterruptList, InterruptLength);
    VendorData = ACPI_ADD_PTR (UINT8, ResourceSource, ResSourceLength);

    /* Setup offsets within the descriptor */

    Descriptor->Gpio.PinTableOffset = (UINT16)
        ACPI_PTR_DIFF (InterruptList, Descriptor);

    Descriptor->Gpio.ResSourceOffset = (UINT16)
        ACPI_PTR_DIFF (ResourceSource, Descriptor);

    DbgPrint (ASL_DEBUG_OUTPUT,
        "%16s - Actual: %.2X, Base: %.2X, ResLen: %.2X, VendLen: %.2X, IntLen: %.2X\n",
        "GpioInt", Descriptor->Gpio.ResourceLength, (UINT16) sizeof (AML_RESOURCE_GPIO),
        ResSourceLength, VendorLength, InterruptLength);

    /* Process all child initialization nodes */

    for (i = 0; InitializerOp; i++)
    {
        switch (i)
        {
        case 0: /* Interrupt Mode - edge/level [Flag] (_MOD) */

            RsSetFlagBits16 (&Descriptor->Gpio.IntFlags, InitializerOp, 0, 0);
            RsCreateBitField (InitializerOp, ACPI_RESTAG_MODE,
                CurrentByteOffset + ASL_RESDESC_OFFSET (Gpio.IntFlags), 0);
            break;

        case 1: /* Interrupt Polarity - Active high/low [Flags] (_POL) */

            RsSetFlagBits16 (&Descriptor->Gpio.IntFlags, InitializerOp, 1, 0);
            RsCreateMultiBitField (InitializerOp, ACPI_RESTAG_POLARITY,
                CurrentByteOffset + ASL_RESDESC_OFFSET (Gpio.IntFlags), 1, 2);
            break;

        case 2: /* Share Type - Default: exclusive (0) [Flags] (_SHR) */

            RsSetFlagBits16 (&Descriptor->Gpio.IntFlags, InitializerOp, 3, 0);
            RsCreateMultiBitField (InitializerOp, ACPI_RESTAG_INTERRUPTSHARE,
                CurrentByteOffset + ASL_RESDESC_OFFSET (Gpio.IntFlags), 3, 2);
            break;

        case 3: /* Pin Config [BYTE] (_PPI) */

            Descriptor->Gpio.PinConfig = (UINT8) InitializerOp->Asl.Value.Integer;
            RsCreateByteField (InitializerOp, ACPI_RESTAG_PINCONFIG,
                CurrentByteOffset + ASL_RESDESC_OFFSET (Gpio.PinConfig));
            break;

        case 4: /* Debounce Timeout [WORD] (_DBT) */

            Descriptor->Gpio.DebounceTimeout = (UINT16) InitializerOp->Asl.Value.Integer;
            RsCreateWordField (InitializerOp, ACPI_RESTAG_DEBOUNCETIME,
                CurrentByteOffset + ASL_RESDESC_OFFSET (Gpio.DebounceTimeout));
            break;

        case 5: /* ResSource [Optional Field - STRING] */

            if (ResSourceLength)
            {
                /* Copy string to the descriptor */

                strcpy (ResourceSource,
                    InitializerOp->Asl.Value.String);
            }
            break;

        case 6: /* Resource Index */

            if (InitializerOp->Asl.ParseOpcode != PARSEOP_DEFAULT_ARG)
            {
                Descriptor->Gpio.ResSourceIndex = (UINT8) InitializerOp->Asl.Value.Integer;
            }
            break;

        case 7: /* Resource Usage (consumer/producer) */

            RsSetFlagBits16 (&Descriptor->Gpio.Flags, InitializerOp, 0, 1);
            break;

        case 8: /* Resource Tag (Descriptor Name) */

            UtAttachNamepathToOwner (Op, InitializerOp);
            break;

        case 9: /* Vendor Data (Optional - Buffer of BYTEs) (_VEN) */

            /*
             * Always set the VendorOffset even if there is no Vendor Data.
             * This field is required in order to calculate the length
             * of the ResourceSource at runtime.
             */
            Descriptor->Gpio.VendorOffset = (UINT16)
                ACPI_PTR_DIFF (VendorData, Descriptor);

            if (RsGetVendorData (InitializerOp, VendorData,
                    (CurrentByteOffset +  Descriptor->Gpio.VendorOffset)))
            {
                Descriptor->Gpio.VendorLength = VendorLength;
            }
            break;

        default:
            /*
             * PINs come through here, repeatedly. Each PIN must be a DWORD.
             * NOTE: there is no "length" field for this, so from ACPI spec:
             *  The number of pins in the table can be calculated from:
             *  PinCount = (Resource Source Name Offset - Pin Table Offset) / 2
             *  (implies resource source must immediately follow the pin list.)
             *  Name: _PIN
             */
            *InterruptList = (UINT16) InitializerOp->Asl.Value.Integer;
            InterruptList++;

            /* Case 10: First interrupt number in list */

            if (i == 10)
            {
                if (InitializerOp->Asl.ParseOpcode == PARSEOP_DEFAULT_ARG)
                {
                    /* Must be at least one interrupt */

                    AslError (ASL_ERROR, ASL_MSG_EX_INTERRUPT_LIST_MIN,
                        InitializerOp, NULL);
                }

                /* Check now for duplicates in list */

                RsCheckListForDuplicates (InitializerOp);

                /* Create a named field at the start of the list */

                RsCreateDwordField (InitializerOp, ACPI_RESTAG_PIN,
                    CurrentByteOffset + Descriptor->Gpio.PinTableOffset);
            }
            break;
        }

        InitializerOp = RsCompleteNodeAndGetNext (InitializerOp);
    }

    return (Rnode);
}


/*******************************************************************************
 *
 * FUNCTION:    RsDoGpioIoDescriptor
 *
 * PARAMETERS:  Op                  - Parent resource descriptor parse node
 *              CurrentByteOffset   - Offset into the resource template AML
 *                                    buffer (to track references to the desc)
 *
 * RETURN:      Completed resource node
 *
 * DESCRIPTION: Construct a long "GpioIo" descriptor
 *
 ******************************************************************************/

ASL_RESOURCE_NODE *
RsDoGpioIoDescriptor (
    ACPI_PARSE_OBJECT       *Op,
    UINT32                  CurrentByteOffset)
{
    AML_RESOURCE            *Descriptor;
    ACPI_PARSE_OBJECT       *InitializerOp;
    ASL_RESOURCE_NODE       *Rnode;
    char                    *ResourceSource = NULL;
    UINT8                   *VendorData = NULL;
    UINT16                  *InterruptList = NULL;
    UINT16                  ResSourceLength;
    UINT16                  VendorLength;
    UINT16                  InterruptLength;
    UINT16                  DescriptorSize;
    UINT32                  i;


    InitializerOp = Op->Asl.Child;

    /*
     * Calculate lengths for fields that have variable length:
     * 1) Resource Source string
     * 2) Vendor Data buffer
     * 3) PIN (interrupt) list
     */
    ResSourceLength = RsGetStringDataLength (InitializerOp);
    VendorLength = RsGetBufferDataLength (InitializerOp);
    InterruptLength = RsGetInterruptDataLength (InitializerOp);

    DescriptorSize = ACPI_AML_SIZE_LARGE (AML_RESOURCE_GPIO) +
        ResSourceLength + VendorLength + InterruptLength;

    /* Allocate the local resource node and initialize */

    Rnode = RsAllocateResourceNode (DescriptorSize + sizeof (AML_RESOURCE_LARGE_HEADER));

    Descriptor = Rnode->Buffer;
    Descriptor->Gpio.ResourceLength  = DescriptorSize;
    Descriptor->Gpio.DescriptorType  = ACPI_RESOURCE_NAME_GPIO;
    Descriptor->Gpio.RevisionId      = AML_RESOURCE_GPIO_REVISION;
    Descriptor->Gpio.ConnectionType  = AML_RESOURCE_GPIO_TYPE_IO;

    /* Build pointers to optional areas */

    InterruptList = ACPI_ADD_PTR (UINT16, Descriptor, sizeof (AML_RESOURCE_GPIO));
    ResourceSource = ACPI_ADD_PTR (char, InterruptList, InterruptLength);
    VendorData = ACPI_ADD_PTR (UINT8, ResourceSource, ResSourceLength);

    /* Setup offsets within the descriptor */

    Descriptor->Gpio.PinTableOffset = (UINT16)
        ACPI_PTR_DIFF (InterruptList, Descriptor);

    Descriptor->Gpio.ResSourceOffset = (UINT16)
        ACPI_PTR_DIFF (ResourceSource, Descriptor);

    DbgPrint (ASL_DEBUG_OUTPUT,
        "%16s - Actual: %.2X, Base: %.2X, ResLen: %.2X, VendLen: %.2X, IntLen: %.2X\n",
        "GpioIo", Descriptor->Gpio.ResourceLength, (UINT16) sizeof (AML_RESOURCE_GPIO),
        ResSourceLength, VendorLength, InterruptLength);

    /* Process all child initialization nodes */

    for (i = 0; InitializerOp; i++)
    {
        switch (i)
        {
        case 0: /* Share Type [Flags] (_SHR) */

            RsSetFlagBits16 (&Descriptor->Gpio.IntFlags, InitializerOp, 3, 0);
            RsCreateBitField (InitializerOp, ACPI_RESTAG_INTERRUPTSHARE,
                CurrentByteOffset + ASL_RESDESC_OFFSET (Gpio.IntFlags), 3);
            break;

        case 1: /* Pin Config [BYTE] (_PPI) */

            Descriptor->Gpio.PinConfig = (UINT8) InitializerOp->Asl.Value.Integer;
            RsCreateByteField (InitializerOp, ACPI_RESTAG_PINCONFIG,
                CurrentByteOffset + ASL_RESDESC_OFFSET (Gpio.PinConfig));
            break;

        case 2: /* Debounce Timeout [WORD] (_DBT) */

            Descriptor->Gpio.DebounceTimeout = (UINT16) InitializerOp->Asl.Value.Integer;
            RsCreateWordField (InitializerOp, ACPI_RESTAG_DEBOUNCETIME,
                CurrentByteOffset + ASL_RESDESC_OFFSET (Gpio.DebounceTimeout));
            break;

        case 3: /* Drive Strength [WORD] (_DRS) */

            Descriptor->Gpio.DriveStrength = (UINT16) InitializerOp->Asl.Value.Integer;
            RsCreateWordField (InitializerOp, ACPI_RESTAG_DRIVESTRENGTH,
                CurrentByteOffset + ASL_RESDESC_OFFSET (Gpio.DriveStrength));
            break;

        case 4: /* I/O Restriction [Flag] (_IOR) */

            RsSetFlagBits16 (&Descriptor->Gpio.IntFlags, InitializerOp, 0, 0);
            RsCreateMultiBitField (InitializerOp, ACPI_RESTAG_IORESTRICTION,
                CurrentByteOffset + ASL_RESDESC_OFFSET (Gpio.IntFlags), 0, 2);
            break;

        case 5: /* ResSource [Optional Field - STRING] */

            if (ResSourceLength)
            {
                /* Copy string to the descriptor */

                strcpy (ResourceSource,
                    InitializerOp->Asl.Value.String);
            }
            break;

        case 6: /* Resource Index */

            if (InitializerOp->Asl.ParseOpcode != PARSEOP_DEFAULT_ARG)
            {
                Descriptor->Gpio.ResSourceIndex = (UINT8) InitializerOp->Asl.Value.Integer;
            }
            break;

        case 7: /* Resource Usage (consumer/producer) */

            RsSetFlagBits16 (&Descriptor->Gpio.Flags, InitializerOp, 0, 1);
            break;

        case 8: /* Resource Tag (Descriptor Name) */

            UtAttachNamepathToOwner (Op, InitializerOp);
            break;

        case 9: /* Vendor Data (Optional - Buffer of BYTEs) (_VEN) */
            /*
             * Always set the VendorOffset even if there is no Vendor Data.
             * This field is required in order to calculate the length
             * of the ResourceSource at runtime.
             */
            Descriptor->Gpio.VendorOffset = (UINT16)
                ACPI_PTR_DIFF (VendorData, Descriptor);

            if (RsGetVendorData (InitializerOp, VendorData,
                    (CurrentByteOffset + Descriptor->Gpio.VendorOffset)))
            {
                Descriptor->Gpio.VendorLength = VendorLength;
            }
            break;

        default:
            /*
             * PINs come through here, repeatedly. Each PIN must be a DWORD.
             * NOTE: there is no "length" field for this, so from ACPI spec:
             *  The number of pins in the table can be calculated from:
             *  PinCount = (Resource Source Name Offset - Pin Table Offset) / 2
             *  (implies resource source must immediately follow the pin list.)
             *  Name: _PIN
             */
            *InterruptList = (UINT16) InitializerOp->Asl.Value.Integer;
            InterruptList++;

            /* Case 10: First interrupt number in list */

            if (i == 10)
            {
                if (InitializerOp->Asl.ParseOpcode == PARSEOP_DEFAULT_ARG)
                {
                    /* Must be at least one interrupt */

                    AslError (ASL_ERROR, ASL_MSG_EX_INTERRUPT_LIST_MIN,
                        InitializerOp, NULL);
                }

                /* Check now for duplicates in list */

                RsCheckListForDuplicates (InitializerOp);

                /* Create a named field at the start of the list */

                RsCreateDwordField (InitializerOp, ACPI_RESTAG_PIN,
                    CurrentByteOffset + Descriptor->Gpio.PinTableOffset);
            }
            break;
        }

        InitializerOp = RsCompleteNodeAndGetNext (InitializerOp);
    }

    return (Rnode);
}


/*******************************************************************************
 *
 * FUNCTION:    RsDoI2cSerialBusDescriptor
 *
 * PARAMETERS:  Op                  - Parent resource descriptor parse node
 *              CurrentByteOffset   - Offset into the resource template AML
 *                                    buffer (to track references to the desc)
 *
 * RETURN:      Completed resource node
 *
 * DESCRIPTION: Construct a long "I2cSerialBus" descriptor
 *
 ******************************************************************************/

ASL_RESOURCE_NODE *
RsDoI2cSerialBusDescriptor (
    ACPI_PARSE_OBJECT       *Op,
    UINT32                  CurrentByteOffset)
{
    AML_RESOURCE            *Descriptor;
    ACPI_PARSE_OBJECT       *InitializerOp;
    ASL_RESOURCE_NODE       *Rnode;
    char                    *ResourceSource = NULL;
    UINT8                   *VendorData = NULL;
    UINT16                  ResSourceLength;
    UINT16                  VendorLength;
    UINT16                  DescriptorSize;
    UINT32                  i;


    InitializerOp = Op->Asl.Child;

    /*
     * Calculate lengths for fields that have variable length:
     * 1) Resource Source string
     * 2) Vendor Data buffer
     */
    ResSourceLength = RsGetStringDataLength (InitializerOp);
    VendorLength = RsGetBufferDataLength (InitializerOp);

    DescriptorSize = ACPI_AML_SIZE_LARGE (AML_RESOURCE_I2C_SERIALBUS) +
        ResSourceLength + VendorLength;

    /* Allocate the local resource node and initialize */

    Rnode = RsAllocateResourceNode (DescriptorSize + sizeof (AML_RESOURCE_LARGE_HEADER));

    Descriptor = Rnode->Buffer;
    Descriptor->I2cSerialBus.ResourceLength = DescriptorSize;
    Descriptor->I2cSerialBus.DescriptorType = ACPI_RESOURCE_NAME_SERIAL_BUS;
    Descriptor->I2cSerialBus.RevisionId     = AML_RESOURCE_I2C_REVISION;
    Descriptor->I2cSerialBus.TypeRevisionId = AML_RESOURCE_I2C_TYPE_REVISION;
    Descriptor->I2cSerialBus.Type           = AML_RESOURCE_I2C_SERIALBUSTYPE;
    Descriptor->I2cSerialBus.TypeDataLength = AML_RESOURCE_I2C_MIN_DATA_LEN + VendorLength;

    /* Build pointers to optional areas */

    VendorData = ACPI_ADD_PTR (UINT8, Descriptor, sizeof (AML_RESOURCE_I2C_SERIALBUS));
    ResourceSource = ACPI_ADD_PTR (char, VendorData, VendorLength);

    DbgPrint (ASL_DEBUG_OUTPUT,
        "%16s - Actual: %.2X, Base: %.2X, ResLen: %.2X, VendLen: %.2X, TypLen: %.2X\n",
        "I2cSerialBus", Descriptor->I2cSerialBus.ResourceLength,
        (UINT16) sizeof (AML_RESOURCE_I2C_SERIALBUS), ResSourceLength,
        VendorLength, Descriptor->I2cSerialBus.TypeDataLength);

    /* Process all child initialization nodes */

    for (i = 0; InitializerOp; i++)
    {
        switch (i)
        {
        case 0: /* Slave Address [WORD] (_ADR) */

            Descriptor->I2cSerialBus.SlaveAddress = (UINT16) InitializerOp->Asl.Value.Integer;
            RsCreateWordField (InitializerOp, ACPI_RESTAG_ADDRESS,
                CurrentByteOffset + ASL_RESDESC_OFFSET (I2cSerialBus.SlaveAddress));
            break;

        case 1: /* Slave Mode [Flag] (_SLV) */

            RsSetFlagBits (&Descriptor->I2cSerialBus.Flags, InitializerOp, 0, 0);
            RsCreateBitField (InitializerOp, ACPI_RESTAG_SLAVEMODE,
                CurrentByteOffset + ASL_RESDESC_OFFSET (I2cSerialBus.Flags), 0);
            break;

        case 2: /* Connection Speed [DWORD] (_SPE) */

            Descriptor->I2cSerialBus.ConnectionSpeed = (UINT32) InitializerOp->Asl.Value.Integer;
            RsCreateDwordField (InitializerOp, ACPI_RESTAG_SPEED,
                CurrentByteOffset + ASL_RESDESC_OFFSET (I2cSerialBus.ConnectionSpeed));
            break;

        case 3: /* Addressing Mode [Flag] (_MOD) */

            RsSetFlagBits16 (&Descriptor->I2cSerialBus.TypeSpecificFlags, InitializerOp, 0, 0);
            RsCreateBitField (InitializerOp, ACPI_RESTAG_MODE,
                CurrentByteOffset + ASL_RESDESC_OFFSET (I2cSerialBus.TypeSpecificFlags), 0);
            break;

        case 4: /* ResSource [Optional Field - STRING] */

            if (ResSourceLength)
            {
                /* Copy string to the descriptor */

                strcpy (ResourceSource,
                    InitializerOp->Asl.Value.String);
            }
            break;

        case 5: /* Resource Index */

            if (InitializerOp->Asl.ParseOpcode != PARSEOP_DEFAULT_ARG)
            {
                Descriptor->I2cSerialBus.ResSourceIndex = (UINT8) InitializerOp->Asl.Value.Integer;
            }
            break;

        case 6: /* Resource Usage (consumer/producer) */

            RsSetFlagBits (&Descriptor->I2cSerialBus.Flags, InitializerOp, 1, 1);
            break;

        case 7: /* Resource Tag (Descriptor Name) */

            UtAttachNamepathToOwner (Op, InitializerOp);
            break;

        case 8: /* Vendor Data (Optional - Buffer of BYTEs) (_VEN) */

            RsGetVendorData (InitializerOp, VendorData,
                CurrentByteOffset + sizeof (AML_RESOURCE_I2C_SERIALBUS));
            break;

        default:    /* Ignore any extra nodes */

            break;
        }

        InitializerOp = RsCompleteNodeAndGetNext (InitializerOp);
    }

    return (Rnode);
}


/*******************************************************************************
 *
 * FUNCTION:    RsDoSpiSerialBusDescriptor
 *
 * PARAMETERS:  Op                  - Parent resource descriptor parse node
 *              CurrentByteOffset   - Offset into the resource template AML
 *                                    buffer (to track references to the desc)
 *
 * RETURN:      Completed resource node
 *
 * DESCRIPTION: Construct a long "SPI Serial Bus" descriptor
 *
 ******************************************************************************/

ASL_RESOURCE_NODE *
RsDoSpiSerialBusDescriptor (
    ACPI_PARSE_OBJECT       *Op,
    UINT32                  CurrentByteOffset)
{
    AML_RESOURCE            *Descriptor;
    ACPI_PARSE_OBJECT       *InitializerOp;
    ASL_RESOURCE_NODE       *Rnode;
    char                    *ResourceSource = NULL;
    UINT8                   *VendorData = NULL;
    UINT16                  ResSourceLength;
    UINT16                  VendorLength;
    UINT16                  DescriptorSize;
    UINT32                  i;


    InitializerOp = Op->Asl.Child;

    /*
     * Calculate lengths for fields that have variable length:
     * 1) Resource Source string
     * 2) Vendor Data buffer
     */
    ResSourceLength = RsGetStringDataLength (InitializerOp);
    VendorLength = RsGetBufferDataLength (InitializerOp);

    DescriptorSize = ACPI_AML_SIZE_LARGE (AML_RESOURCE_SPI_SERIALBUS) +
        ResSourceLength + VendorLength;

    /* Allocate the local resource node and initialize */

    Rnode = RsAllocateResourceNode (DescriptorSize + sizeof (AML_RESOURCE_LARGE_HEADER));

    Descriptor = Rnode->Buffer;
    Descriptor->SpiSerialBus.ResourceLength = DescriptorSize;
    Descriptor->SpiSerialBus.DescriptorType = ACPI_RESOURCE_NAME_SERIAL_BUS;
    Descriptor->SpiSerialBus.RevisionId     = AML_RESOURCE_SPI_REVISION;
    Descriptor->SpiSerialBus.TypeRevisionId = AML_RESOURCE_SPI_TYPE_REVISION;
    Descriptor->SpiSerialBus.Type           = AML_RESOURCE_SPI_SERIALBUSTYPE;
    Descriptor->SpiSerialBus.TypeDataLength = AML_RESOURCE_SPI_MIN_DATA_LEN + VendorLength;

    /* Build pointers to optional areas */

    VendorData = ACPI_ADD_PTR (UINT8, Descriptor, sizeof (AML_RESOURCE_SPI_SERIALBUS));
    ResourceSource = ACPI_ADD_PTR (char, VendorData, VendorLength);

    DbgPrint (ASL_DEBUG_OUTPUT,
        "%16s - Actual: %.2X, Base: %.2X, ResLen: %.2X, VendLen: %.2X, TypLen: %.2X\n",
        "SpiSerialBus", Descriptor->SpiSerialBus.ResourceLength,
        (UINT16) sizeof (AML_RESOURCE_SPI_SERIALBUS), ResSourceLength,
        VendorLength, Descriptor->SpiSerialBus.TypeDataLength);

    /* Process all child initialization nodes */

    for (i = 0; InitializerOp; i++)
    {
        switch (i)
        {
        case 0: /* Device Selection [WORD] (_ADR) */

            Descriptor->SpiSerialBus.DeviceSelection = (UINT16) InitializerOp->Asl.Value.Integer;
            RsCreateWordField (InitializerOp, ACPI_RESTAG_ADDRESS,
                CurrentByteOffset + ASL_RESDESC_OFFSET (SpiSerialBus.DeviceSelection));
            break;

        case 1: /* Device Polarity [Flag] (_DPL) */

            RsSetFlagBits16 (&Descriptor->SpiSerialBus.TypeSpecificFlags, InitializerOp, 1, 0);
            RsCreateBitField (InitializerOp, ACPI_RESTAG_DEVICEPOLARITY,
                CurrentByteOffset + ASL_RESDESC_OFFSET (SpiSerialBus.TypeSpecificFlags), 1);
            break;

        case 2: /* Wire Mode [Flag] (_MOD) */

            RsSetFlagBits16 (&Descriptor->SpiSerialBus.TypeSpecificFlags, InitializerOp, 0, 0);
            RsCreateBitField (InitializerOp, ACPI_RESTAG_MODE,
                CurrentByteOffset + ASL_RESDESC_OFFSET (SpiSerialBus.TypeSpecificFlags), 0);
            break;

        case 3: /* Device Bit Length [BYTE] (_LEN) */

            Descriptor->SpiSerialBus.DataBitLength = (UINT8) InitializerOp->Asl.Value.Integer;
            RsCreateByteField (InitializerOp, ACPI_RESTAG_LENGTH,
                CurrentByteOffset + ASL_RESDESC_OFFSET (SpiSerialBus.DataBitLength));
            break;

        case 4: /* Slave Mode [Flag] (_SLV) */

            RsSetFlagBits (&Descriptor->SpiSerialBus.Flags, InitializerOp, 0, 0);
            RsCreateBitField (InitializerOp, ACPI_RESTAG_SLAVEMODE,
                CurrentByteOffset + ASL_RESDESC_OFFSET (SpiSerialBus.Flags), 0);
            break;

        case 5: /* Connection Speed [DWORD] (_SPE) */

            Descriptor->SpiSerialBus.ConnectionSpeed = (UINT32) InitializerOp->Asl.Value.Integer;
            RsCreateDwordField (InitializerOp, ACPI_RESTAG_SPEED,
                CurrentByteOffset + ASL_RESDESC_OFFSET (SpiSerialBus.ConnectionSpeed));
            break;

        case 6: /* Clock Polarity [BYTE] (_POL) */

            Descriptor->SpiSerialBus.ClockPolarity = (UINT8) InitializerOp->Asl.Value.Integer;
            RsCreateByteField (InitializerOp, ACPI_RESTAG_POLARITY,
                CurrentByteOffset + ASL_RESDESC_OFFSET (SpiSerialBus.ClockPolarity));
            break;

        case 7: /* Clock Phase [BYTE] (_PHA) */

            Descriptor->SpiSerialBus.ClockPhase = (UINT8) InitializerOp->Asl.Value.Integer;
            RsCreateByteField (InitializerOp, ACPI_RESTAG_PHASE,
                CurrentByteOffset + ASL_RESDESC_OFFSET (SpiSerialBus.ClockPhase));
            break;

        case 8: /* ResSource [Optional Field - STRING] */

            if (ResSourceLength)
            {
                /* Copy string to the descriptor */

                strcpy (ResourceSource,
                    InitializerOp->Asl.Value.String);
            }
            break;

        case 9: /* Resource Index */

            if (InitializerOp->Asl.ParseOpcode != PARSEOP_DEFAULT_ARG)
            {
                Descriptor->SpiSerialBus.ResSourceIndex = (UINT8) InitializerOp->Asl.Value.Integer;
            }
            break;

        case 10: /* Resource Usage (consumer/producer) */

            RsSetFlagBits (&Descriptor->SpiSerialBus.Flags, InitializerOp, 1, 1);
            break;

        case 11: /* Resource Tag (Descriptor Name) */

            UtAttachNamepathToOwner (Op, InitializerOp);
            break;

        case 12: /* Vendor Data (Optional - Buffer of BYTEs) (_VEN) */

            RsGetVendorData (InitializerOp, VendorData,
                CurrentByteOffset + sizeof (AML_RESOURCE_SPI_SERIALBUS));
            break;

        default:    /* Ignore any extra nodes */

            break;
        }

        InitializerOp = RsCompleteNodeAndGetNext (InitializerOp);
    }

    return (Rnode);
}


/*******************************************************************************
 *
 * FUNCTION:    RsDoUartSerialBusDescriptor
 *
 * PARAMETERS:  Op                  - Parent resource descriptor parse node
 *              CurrentByteOffset   - Offset into the resource template AML
 *                                    buffer (to track references to the desc)
 *
 * RETURN:      Completed resource node
 *
 * DESCRIPTION: Construct a long "UART Serial Bus" descriptor
 *
 ******************************************************************************/

ASL_RESOURCE_NODE *
RsDoUartSerialBusDescriptor (
    ACPI_PARSE_OBJECT       *Op,
    UINT32                  CurrentByteOffset)
{
    AML_RESOURCE            *Descriptor;
    ACPI_PARSE_OBJECT       *InitializerOp;
    ASL_RESOURCE_NODE       *Rnode;
    char                    *ResourceSource = NULL;
    UINT8                   *VendorData = NULL;
    UINT16                  ResSourceLength;
    UINT16                  VendorLength;
    UINT16                  DescriptorSize;
    UINT32                  i;


    InitializerOp = Op->Asl.Child;

    /*
     * Calculate lengths for fields that have variable length:
     * 1) Resource Source string
     * 2) Vendor Data buffer
     */
    ResSourceLength = RsGetStringDataLength (InitializerOp);
    VendorLength = RsGetBufferDataLength (InitializerOp);

    DescriptorSize = ACPI_AML_SIZE_LARGE (AML_RESOURCE_UART_SERIALBUS) +
        ResSourceLength + VendorLength;

    /* Allocate the local resource node and initialize */

    Rnode = RsAllocateResourceNode (DescriptorSize + sizeof (AML_RESOURCE_LARGE_HEADER));

    Descriptor = Rnode->Buffer;
    Descriptor->UartSerialBus.ResourceLength = DescriptorSize;
    Descriptor->UartSerialBus.DescriptorType = ACPI_RESOURCE_NAME_SERIAL_BUS;
    Descriptor->UartSerialBus.RevisionId     = AML_RESOURCE_UART_REVISION;
    Descriptor->UartSerialBus.TypeRevisionId = AML_RESOURCE_UART_TYPE_REVISION;
    Descriptor->UartSerialBus.Type           = AML_RESOURCE_UART_SERIALBUSTYPE;
    Descriptor->UartSerialBus.TypeDataLength = AML_RESOURCE_UART_MIN_DATA_LEN + VendorLength;

    /* Build pointers to optional areas */

    VendorData = ACPI_ADD_PTR (UINT8, Descriptor, sizeof (AML_RESOURCE_UART_SERIALBUS));
    ResourceSource = ACPI_ADD_PTR (char, VendorData, VendorLength);

    DbgPrint (ASL_DEBUG_OUTPUT,
        "%16s - Actual: %.2X, Base: %.2X, ResLen: %.2X, VendLen: %.2X, TypLen: %.2X\n",
        "UartSerialBus", Descriptor->UartSerialBus.ResourceLength,
        (UINT16) sizeof (AML_RESOURCE_UART_SERIALBUS), ResSourceLength,
        VendorLength, Descriptor->UartSerialBus.TypeDataLength);

    /* Process all child initialization nodes */

    for (i = 0; InitializerOp; i++)
    {
        switch (i)
        {
        case 0: /* Connection Speed (Baud Rate) [DWORD] (_SPE) */

            Descriptor->UartSerialBus.DefaultBaudRate = (UINT32) InitializerOp->Asl.Value.Integer;
            RsCreateDwordField (InitializerOp, ACPI_RESTAG_SPEED,
                CurrentByteOffset + ASL_RESDESC_OFFSET (UartSerialBus.DefaultBaudRate));
            break;

        case 1: /* Bits Per Byte [Flags] (_LEN) */

            RsSetFlagBits16 (&Descriptor->UartSerialBus.TypeSpecificFlags, InitializerOp, 4, 3);
            RsCreateMultiBitField (InitializerOp, ACPI_RESTAG_LENGTH,
                CurrentByteOffset + ASL_RESDESC_OFFSET (UartSerialBus.TypeSpecificFlags), 4, 3);
            break;

        case 2: /* Stop Bits [Flags] (_STB) */

            RsSetFlagBits16 (&Descriptor->UartSerialBus.TypeSpecificFlags, InitializerOp, 2, 1);
            RsCreateMultiBitField (InitializerOp, ACPI_RESTAG_STOPBITS,
                CurrentByteOffset + ASL_RESDESC_OFFSET (UartSerialBus.TypeSpecificFlags), 2, 2);
            break;

        case 3: /* Lines In Use [BYTE] (_LIN) */

            Descriptor->UartSerialBus.LinesEnabled = (UINT8) InitializerOp->Asl.Value.Integer;
            RsCreateByteField (InitializerOp, ACPI_RESTAG_LINE,
                CurrentByteOffset + ASL_RESDESC_OFFSET (UartSerialBus.LinesEnabled));
            break;

        case 4: /* Endianness [Flag] (_END) */

            RsSetFlagBits16 (&Descriptor->UartSerialBus.TypeSpecificFlags, InitializerOp, 7, 0);
            RsCreateBitField (InitializerOp, ACPI_RESTAG_ENDIANNESS,
                CurrentByteOffset + ASL_RESDESC_OFFSET (UartSerialBus.TypeSpecificFlags), 7);
            break;

        case 5: /* Parity [BYTE] (_PAR) */

            Descriptor->UartSerialBus.Parity = (UINT8) InitializerOp->Asl.Value.Integer;
            RsCreateByteField (InitializerOp, ACPI_RESTAG_PARITY,
                CurrentByteOffset + ASL_RESDESC_OFFSET (UartSerialBus.Parity));
            break;

        case 6: /* Flow Control [Flags] (_FLC) */

            RsSetFlagBits16 (&Descriptor->UartSerialBus.TypeSpecificFlags, InitializerOp, 0, 0);
            RsCreateMultiBitField (InitializerOp, ACPI_RESTAG_FLOWCONTROL,
                CurrentByteOffset + ASL_RESDESC_OFFSET (UartSerialBus.TypeSpecificFlags), 0, 2);
            break;

        case 7: /* Rx Buffer Size [WORD] (_RXL) */

            Descriptor->UartSerialBus.RxFifoSize = (UINT16) InitializerOp->Asl.Value.Integer;
            RsCreateWordField (InitializerOp, ACPI_RESTAG_LENGTH_RX,
                CurrentByteOffset + ASL_RESDESC_OFFSET (UartSerialBus.RxFifoSize));
            break;

        case 8: /* Tx Buffer Size [WORD] (_TXL) */

            Descriptor->UartSerialBus.TxFifoSize = (UINT16) InitializerOp->Asl.Value.Integer;
            RsCreateWordField (InitializerOp, ACPI_RESTAG_LENGTH_TX,
                CurrentByteOffset + ASL_RESDESC_OFFSET (UartSerialBus.TxFifoSize));
            break;

        case 9: /* ResSource [Optional Field - STRING] */

            if (ResSourceLength)
            {
                /* Copy string to the descriptor */

                strcpy (ResourceSource,
                    InitializerOp->Asl.Value.String);
            }
            break;

        case 10: /* Resource Index */

            if (InitializerOp->Asl.ParseOpcode != PARSEOP_DEFAULT_ARG)
            {
                Descriptor->UartSerialBus.ResSourceIndex = (UINT8) InitializerOp->Asl.Value.Integer;
            }
            break;

        case 11: /* Resource Usage (consumer/producer) */

            RsSetFlagBits (&Descriptor->UartSerialBus.Flags, InitializerOp, 1, 1);

            /*
             * Slave Mode [Flag] (_SLV)
             *
             * Note: There is no SlaveMode argument to the UartSerialBus macro, but
             * we add this name anyway to allow the flag to be set by ASL in the
             * rare case where there is a slave mode associated with the UART.
             */
            RsCreateBitField (InitializerOp, ACPI_RESTAG_SLAVEMODE,
                CurrentByteOffset + ASL_RESDESC_OFFSET (UartSerialBus.Flags), 0);
            break;

        case 12: /* Resource Tag (Descriptor Name) */

            UtAttachNamepathToOwner (Op, InitializerOp);
            break;

        case 13: /* Vendor Data (Optional - Buffer of BYTEs) (_VEN) */

            RsGetVendorData (InitializerOp, VendorData,
                CurrentByteOffset + sizeof (AML_RESOURCE_UART_SERIALBUS));
            break;

        default:    /* Ignore any extra nodes */

            break;
        }

        InitializerOp = RsCompleteNodeAndGetNext (InitializerOp);
    }

    return (Rnode);
}
