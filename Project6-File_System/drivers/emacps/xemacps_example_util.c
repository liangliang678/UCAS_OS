/******************************************************************************
*
* Copyright (C) 2010 - 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xemacps_example_util.c
*
* This file implements the utility functions for the XEmacPs example code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a wsy  01/10/10 First release
* 1.00a asa  01/24/12 A new version of EmacPsUtilEnterLoopback is used for
* 		      Zynq boards. Now there are two versions of
* 		      EmacPsUtilEnterLoopback, one for PEEP and one for Zynq.
* 		      If the example is to be run  on a PEEP board, define PEEP
* 		      in xemacps_example.h.
* 1.01a asa  02/27/12 The sleep value after PHY loopback is setup is reduced
*		      for Zynq.
* 3.0   kpc  01/23/15 Removed PEEP board related code
* 3.2	hk   09/30/15 Added support for TI PHY DP83867
* 3.2   mus  02/20/16 Added support for microblaze.
* 3.9   hk   02/12/19 Use selected speed in loopback mode.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xemacps_example.h"

/************************** Variable Definitions ****************************/

XEmacPs EmacPsInstance;	/* XEmacPs instance used throughout examples */

/*
 * Local MAC address
 */
char EmacPsMAC[] = { 0x00, 0x0a, 0x35, 0x01, 0x02, 0x03 };
char BroadcastMAC[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
char DestMAC[] = { 0xec,0x8e,0xb5,0x59,0x38,0xfb };


/****************************************************************************/
/**
*
* Set the MAC addresses in the frame.
*
* @param    FramePtr is the pointer to the frame.
* @param    DestAddr is the Destination MAC address.
*
* @return   None.
*
* @note     None.
*
*****************************************************************************/
void EmacPsUtilFrameHdrFormatMAC(EthernetFrame * FramePtr, char *DestAddr)
{
	char *Frame = (char *) FramePtr;
	char *SourceAddress = EmacPsMAC;
	s32 Index;

	/* Destination address */
	for (Index = 0; Index < XEMACPS_MAC_ADDR_SIZE; Index++) {
		*Frame++ = *DestAddr++;
	}

	/* Source address */
	for (Index = 0; Index < XEMACPS_MAC_ADDR_SIZE; Index++) {
		*Frame++ = *SourceAddress++;
	}
}

/****************************************************************************/
/**
*
* Set the frame type for the specified frame.
*
* @param    FramePtr is the pointer to the frame.
* @param    FrameType is the Type to set in frame.
*
* @return   None.
*
* @note     None.
*
*****************************************************************************/
void EmacPsUtilFrameHdrFormatType(EthernetFrame * FramePtr, u16 FrameType)
{
	char *Frame = (char *) FramePtr;

	/*
	 * Increment to type field
	 */
	Frame = Frame + 12;
	/*
	 * Do endian swap from little to big-endian.
	 */
	FrameType = Xil_EndianSwap16(FrameType);
	/*
	 * Set the type
	 */
	*(u16 *) Frame = FrameType;
}

/****************************************************************************/
/**
* This function places a pattern in the payload section of a frame. The pattern
* is a  8 bit incrementing series of numbers starting with 0.
* Once the pattern reaches 256, then the pattern changes to a 16 bit
* incrementing pattern:
* <pre>
*   0, 1, 2, ... 254, 255, 00, 00, 00, 01, 00, 02, ...
* </pre>
*
* @param    FramePtr is a pointer to the frame to change.
* @param    PayloadSize is the number of bytes in the payload that will be set.
*
* @return   None.
*
* @note     None.
*
*****************************************************************************/
void EmacPsUtilFrameSetPayloadData(EthernetFrame * FramePtr, u32 PayloadSize)
{
	u32 BytesLeft = PayloadSize;
	u8 *Frame;
	u16 Counter = 0;

	/*
	 * Set the frame pointer to the start of the payload area
	 */
	Frame = (u8 *) FramePtr + XEMACPS_HDR_SIZE;

	/*
	 * Insert 8 bit incrementing pattern
	 */
	while (BytesLeft && (Counter < 256)) {
		*Frame++ = (u8) Counter++;
		BytesLeft--;
	}

	/*
	 * Switch to 16 bit incrementing pattern
	 */
	while (BytesLeft) {
		*Frame++ = (u8) (Counter >> 8);	/* high */
		BytesLeft--;

		if (!BytesLeft)
			break;

		*Frame++ = (u8) Counter++;	/* low */
		BytesLeft--;
	}
}

/****************************************************************************/
/**
* This function verifies the frame data against a CheckFrame.
*
* Validation occurs by comparing the ActualFrame to the header of the
* CheckFrame. If the headers match, then the payload of ActualFrame is
* verified for the same pattern Util_FrameSetPayloadData() generates.
*
* @param    CheckFrame is a pointer to a frame containing the 14 byte header
*           that should be present in the ActualFrame parameter.
* @param    ActualFrame is a pointer to a frame to validate.
*
* @return   XST_SUCCESS if successful, else XST_FAILURE.
*
* @note     None.
*****************************************************************************/
LONG EmacPsUtilFrameVerify(EthernetFrame * CheckFrame,
			   EthernetFrame * ActualFrame)
{
	char *ActualPtr = (char *) ActualFrame;
	u16 BytesLeft;
	u16 Counter;

	/*
	 * Compare the headers
	 */
	/*for (Index = 0; Index < XEMACPS_HDR_SIZE; Index++) {
		if (CheckPtr[Index] != ActualPtr[Index]) {
			return XST_FAILURE;
		}
	}*/

	/*
	 * Get the length of the payload
	 */
	BytesLeft = *(u16 *) &ActualPtr[12];
	/*
	 * Do endian swap from big back to little-endian.
	 */
	BytesLeft = Xil_EndianSwap16(BytesLeft);
	/*
	 * Validate the payload
	 */
	Counter = 0;
	ActualPtr = &ActualPtr[14];

	/*
	 * Check 8 bit incrementing pattern
	 */
	while (BytesLeft && (Counter < 256)) {
		if (*ActualPtr++ != (char) Counter++) {
			return XST_FAILURE;
		}
		BytesLeft--;
	}

	/*
	 * Check 16 bit incrementing pattern
	 */
	while (BytesLeft) {
		if (*ActualPtr++ != (char) (Counter >> 8)) {	/* high */
			return XST_FAILURE;
		}

		BytesLeft--;

		if (!BytesLeft)
			break;

		if (*ActualPtr++ != (char) Counter++) {	/* low */
			return XST_FAILURE;
		}

		BytesLeft--;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* This function sets all bytes of a frame to 0.
*
* @param    FramePtr is a pointer to the frame itself.
*
* @return   None.
*
* @note     None.
*
*****************************************************************************/
void EmacPsUtilFrameMemClear(EthernetFrame * FramePtr)
{
	u32 *Data32Ptr = (u32 *) FramePtr;
	u32 WordsLeft = sizeof(EthernetFrame) / sizeof(u32);

	/* frame should be an integral number of words */
	while (WordsLeft--) {
		*Data32Ptr++ = 0xDEADBEEF;
	}
}


/****************************************************************************/
/**
*
* This function copys data from source to desitnation for n bytes.
*
* @param    Destination is the targeted string to copy to.
* @param    Source is the source string to copy from.
* @param    n is number of bytes to be copied.
*
* @note     This function is similiar to strncpy(), however strncpy will
*           stop either at null byte or n bytes is been copied.
*           This function will copy n bytes without checking the content.
*
*****************************************************************************/
void EmacPsUtilstrncpy(char *Destination, const char *Source, u32 n)
{
	do {
		*Destination++ = *Source++;
	} while (--n != 0);
}


/****************************************************************************/
/**
*
* This function sets the emacps to loopback mode.
*
* @param    EmacPsInstancePtr is the XEmacPs driver instance.
*
* @note     None.
*
*****************************************************************************/
void EmacPsUtilEnterLocalLoopback(XEmacPs * EmacPsInstancePtr)
{
	u32 reg;

	reg = XEmacPs_ReadReg(EmacPsInstancePtr->Config.BaseAddress,
				XEMACPS_NWCTRL_OFFSET);
	XEmacPs_WriteReg(EmacPsInstancePtr->Config.BaseAddress,
			   XEMACPS_NWCTRL_OFFSET,
			   reg | XEMACPS_NWCTRL_LOOPEN_MASK);
}


/****************************************************************************/
/**
*
* This function detects the PHY address by looking for successful MII status
* register contents.
*
* @param    The XEMACPS driver instance
*
* @return   The address of the PHY (defaults to 32 if none detected)
*
* @note     None.
*
*****************************************************************************/
#define PHY_DETECT_REG1 2
#define PHY_DETECT_REG2 3

#define PHY_ID_MARVELL	0x141
#define PHY_ID_TI		0x2000
#define PHY_ID_RTL		0x1c

u32 XEmacPsDetectPHY(XEmacPs * EmacPsInstancePtr)
{
	u32 PhyAddr;
	u32 Status;
	u16 PhyReg1;
	u16 PhyReg2;

	for (PhyAddr = 0; PhyAddr <= 31; PhyAddr++) {
		Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr,
					  PHY_DETECT_REG1, &PhyReg1);

		Status |= XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr,
					   PHY_DETECT_REG2, &PhyReg2);

		if ((Status == XST_SUCCESS) &&
		    (PhyReg1 > 0x0000) && (PhyReg1 < 0xffff) &&
		    (PhyReg2 > 0x0000) && (PhyReg2 < 0xffff)) {
			/* Found a valid PHY address */
			return PhyAddr;
		}
	}

	return PhyAddr;		/* default to 32(max of iteration) */
}


/****************************************************************************/
/**
*
* This function sets the PHY to loopback mode.
*
* @param    The XEMACPS driver instance
* @param    Speed is the loopback speed 10/100 Mbit.
*
* @return   XST_SUCCESS if successful, else XST_FAILURE.
*
* @note     None.
*
*****************************************************************************/
#define PHY_REG0_RESET    0x8000
#define PHY_REG0_LOOPBACK 0x4000
#define PHY_REG0_10       0x0100
#define PHY_REG0_100      0x2100
#define PHY_REG0_1000     0x0140
#define PHY_REG21_10      0x0030
#define PHY_REG21_100     0x2030
#define PHY_REG21_1000    0x0070
#define PHY_LOOPCR		0xFE
#define PHY_REGCR		0x0D
#define PHY_ADDAR		0x0E
#define PHY_RGMIIDCTL	0x86
#define PHY_RGMIICTL	0x32

#define PHY_REGCR_ADDR	0x001F
#define PHY_REGCR_DATA	0x401F

/* RGMII RX and TX tuning values */
#define PHY_TI_RGMII_ZCU102	0xA8
#define PHY_TI_RGMII_VERSALEMU	0xAB

LONG EmacPsUtilMarvellPhyLoopback(XEmacPs * EmacPsInstancePtr,
									u32 Speed, u32 PhyAddr)
{
	LONG Status;
	u16 PhyReg0  = 0;
	u16 PhyReg21  = 0;
	u16 PhyReg22  = 0;

	/*
	 * Setup speed and duplex
	 */
	switch (Speed) {
	case 10:
		PhyReg0 |= PHY_REG0_10;
		PhyReg21 |= PHY_REG21_10;
		break;
	case 100:
		PhyReg0 |= PHY_REG0_100;
		PhyReg21 |= PHY_REG21_100;
		break;
	case 1000:
		PhyReg0 |= PHY_REG0_1000;
		PhyReg21 |= PHY_REG21_1000;
		break;
	default:
		EmacPsUtilErrorTrap("Error: speed not recognized ");
		return XST_FAILURE;
	}

	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);
	/*
	 * Make sure new configuration is in effect
	 */
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy speed");
		return XST_FAILURE;
	}

	/*
	 * Switching to PAGE2
	 */
	PhyReg22 = 0x2;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 22, PhyReg22);

	/*
	 * Adding Tx and Rx delay. Configuring loopback speed.
	 */
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 21, PhyReg21);
	/*
	 * Make sure new configuration is in effect
	 */
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 21, &PhyReg21);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setting Reg 21 in Page 2");
		return XST_FAILURE;
	}
	/*
	 * Switching to PAGE0
	 */
	PhyReg22 = 0x0;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 22, PhyReg22);

	/*
	 * Issue a reset to phy
	 */
	Status  = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	PhyReg0 |= PHY_REG0_RESET;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);

	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error reset phy");
		return XST_FAILURE;
	}

	/*
	 * Enable loopback
	 */
	PhyReg0 |= PHY_REG0_LOOPBACK;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);

	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	}

	EmacpsDelay(1);

	return XST_SUCCESS;
}

LONG EmacPsUtilTiPhyLoopback(XEmacPs * EmacPsInstancePtr,
								u32 Speed, u32 PhyAddr)
{
	LONG Status;
	u16 PhyReg0  = 0, LoopbackSpeed = 0;
	u16 RgmiiTuning = PHY_TI_RGMII_ZCU102;

	/*
	 * Setup speed and duplex
	 */
	switch (Speed) {
	case 10:
		PhyReg0 |= PHY_REG0_10;
		break;
	case 100:
		PhyReg0 |= PHY_REG0_100;
		break;
	case 1000:
		PhyReg0 |= PHY_REG0_1000;
		break;
	default:
		EmacPsUtilErrorTrap("Error: speed not recognized ");
		return XST_FAILURE;
	}
	LoopbackSpeed = PhyReg0;

	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);
	/*
	 * Make sure new configuration is in effect
	 */
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy speed");
		return XST_FAILURE;
	}

	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 1, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy speed");
		return XST_FAILURE;
	}

	/* Write PHY_LOOPCR */
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, PHY_LOOPCR);
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_DATA);
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, 0xEF20);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy speed");
		return XST_FAILURE;
	}

	/* Read PHY_LOOPCR */
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, PHY_LOOPCR);
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_DATA);
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy speed");
		return XST_FAILURE;
	}

	/* SW reset */
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0x1F, 0x4000);
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy speed");
		return XST_FAILURE;
	}

	/* issue a reset to phy */
	Status  = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	PhyReg0 |= PHY_REG0_RESET;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);

	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error reset phy");
		return XST_FAILURE;
	}

	EmacpsDelay(1);

	/* enable loopback */
	PhyReg0 = LoopbackSpeed | PHY_REG0_LOOPBACK;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	}

	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0x10, 0x5048);
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0x10, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	}

	/* Write to PHY_RGMIIDCTL */
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, PHY_RGMIIDCTL);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_DATA);
	if ((Platform & PLATFORM_MASK) == PLATFORM_VERSALEMU) {
		RgmiiTuning = PHY_TI_RGMII_VERSALEMU;
	}
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, RgmiiTuning);
		if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error in tuning");
		return XST_FAILURE;
	}

	/* Read PHY_RGMIIDCTL */
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, PHY_RGMIIDCTL);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_DATA);
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error in tuning");
		return XST_FAILURE;
	}

	/* Write PHY_RGMIICTL */
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, PHY_RGMIICTL);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_DATA);
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, 0xD3);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error in tuning");
		return XST_FAILURE;
	}

	/* Read PHY_RGMIICTL */
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, PHY_RGMIICTL);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_DATA);
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error in tuning");
		return XST_FAILURE;
	}

	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0x11, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

#define PHY_REG0_ANE 0x1000
#define PHY_REG0_RestartAN 0x200
#define PHY_REG0_FULL_DUPLEX 0x100
#define PHY_REG0_1000M_SPEED0 0x0
#define PHY_REG0_1000M_SPEED1 0x40
#define PHY_REG0_100M_SPEED0 0x2000
#define PHY_REG0_100M_SPEED1 0x0
#define PHY_REG0_10M_SPEED0 0x0
#define PHY_REG0_10M_SPEED1 0x0

#define RTL821x_EXT_PAGE_SELECT 0x1e
#define RTL821x_PAGE_SELECT 0x1f
#define RTL8211E_TX_DELAY 0x2
#define RTL8211E_RX_DELAY 0x4
#define RTL8211E_MAGIC 0xb400 /* Undocumented */
#define RTL8211x_PHY_INER 0x12
#define RTL8211x_PHY_INTR_ENA 0x9f01
#define RTL8211x_PHY_INTR_DIS 0x0000
/* RTL8211x PHY Interrupt Status Register */
#define RTL8211x_PHY_INSR 0x13

/* link status ok, speed and duplex solved */
#define RTL8211E_PHYSR_LINK_STATUS 0xc00
#define RTL8211E_PHYSR_SPEED_MASK 0xc000
#define RTL8211E_PHYSR_SPEED_1G 0x8000
#define RTL8211E_PHYSR_SPEED_100M 0x4000
#define RTL8211E_PHYSR_SPEED_10M 0x0000
#define RTL8211E_PHYSR_DUPLEX_MASK 0x2000
#define RTL8211E_PHYSR_HALF_DUPLEX 0x0000
#define RTL8211E_PHYSR_FULL_DUPLEX 0x2000

#define RTL8211E_AUTO_NEGOTIATION_COMPLETE 0x20


LONG EmacPsUtilRtlPhyLoopback(XEmacPs * EmacPsInstancePtr,
									u32 Speed, u32 PhyAddr)
{
	LONG Status;
	u16 PhyReg0  = 0;
	
    /*
     * Setup speed and duplex
     */
    PhyReg0 = 0;
    switch (Speed) {
        case 10:
            PhyReg0 |= PHY_REG0_10M_SPEED0 | PHY_REG0_10M_SPEED1;
            break;
        case 100:
            PhyReg0 |= PHY_REG0_100M_SPEED0 | PHY_REG0_100M_SPEED1;
            break;
        case 1000:
            PhyReg0 |= PHY_REG0_1000M_SPEED0 | PHY_REG0_1000M_SPEED1;
            break;
        default:
            EmacPsUtilErrorTrap("Error: speed not recognized ");
            return XST_FAILURE;
    }
	PhyReg0 |= PHY_REG0_FULL_DUPLEX;

	/*
	 * Issue a reset to phy
	 */
	PhyReg0 |= PHY_REG0_RestartAN;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);

	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error reset phy");
		return XST_FAILURE;
	} else {
		xil_printf("Reset phy success! new reg value: %x\n\r", PhyReg0);
	}

	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, RTL8211x_PHY_INER, RTL8211x_PHY_INTR_ENA);
	

	/* According to a sample driver there is a 0x1c config register on the
     * 0xa4 extension page (0x7) layout. It can be used to disable/enable
     * the RX/TX delays otherwise controlled by RXDLY/TXDLY pins. It can
     * also be used to customize the whole configuration register:
     * 8:6 = PHY Address, 5:4 = Auto-Negotiation, 3 = Interface Mode Select,
     * 2 = RX Delay, 1 = TX Delay, 0 = SELRGV (see original PHY datasheet
     * for details).
     */
	// page select
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, RTL821x_PAGE_SELECT, 0x7);
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, RTL821x_EXT_PAGE_SELECT, 0xa4);

	Status  = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0x1c, &PhyReg0);
	PhyReg0 |= RTL8211E_RX_DELAY | RTL8211E_TX_DELAY;
	// PhyReg0 &= ~(RTL8211E_RX_DELAY | RTL8211E_TX_DELAY);
	PhyReg0 |= RTL8211E_MAGIC;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0x1c, PhyReg0);

	// return to page 0
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, RTL821x_PAGE_SELECT, 0x0);


	//PhyReg0 |= PHY_REG0_FULL_DUPLEX;
	//Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);

	/*Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	xil_printf("reg : %x\n\r", PhyReg0);
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0 | PHY_REG0_LOOPBACK);
	xil_printf("reg : %x\n\r", PhyReg0);
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	xil_printf("reg : %x\n\r", PhyReg0);*/

    /*
     * Enable loopback
     */
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
    PhyReg0 |= PHY_REG0_LOOPBACK;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);

	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	} else {
		xil_printf("setup phy loopback success\n\r");
	}

	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, RTL8211x_PHY_INSR, &PhyReg0);

	xil_printf("waiting phy link ok ..");
	u16 physr = 0;
	do {
		xil_printf("..");
		EmacpsDelay(10);
		Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0x11, &physr);
	} while ((physr & RTL8211E_PHYSR_LINK_STATUS) != RTL8211E_PHYSR_LINK_STATUS);
	xil_printf(" [Done]\n\r");

	PhyReg0 = 0;
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	} else {
		xil_printf("reg0 : %x\n\r", PhyReg0);
	}

	PhyReg0 = 0;
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0x1, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	} else {
		xil_printf("reg0x1 : %x\n\r", PhyReg0);
	}

	PhyReg0 = 0;
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0x10, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	} else {
		xil_printf("reg0x10 : %x\n\r", PhyReg0);
	}

	PhyReg0 = 0;
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0x11, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	} else {
		xil_printf("reg0x11 : %x\n\r", PhyReg0);
	}

	return XST_SUCCESS;
}

LONG EmacPsUtilRtlPhyInit(XEmacPs * EmacPsInstancePtr,
							u32 *Speed, u32 PhyAddr)
{
	LONG Status;
	u16 PhyReg0  = 0;

	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0x4, 0x0141);

	/*
	 * Issue a reset to phy
	 */
	// PhyReg0 |= PHY_REG0_100M_SPEED0 | PHY_REG0_100M_SPEED1 | PHY_REG0_FULL_DUPLEX;
	// PhyReg0 |= PHY_REG0_RestartAN;
	PhyReg0 = PHY_REG0_ANE | PHY_REG0_RESET;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);

	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error reset phy");
		return XST_FAILURE;
	} else {
		xil_printf("Reset phy success! new reg value: %x\n\r", PhyReg0);
	}

	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, RTL8211x_PHY_INER, RTL8211x_PHY_INTR_ENA);
	

	/* According to a sample driver there is a 0x1c config register on the
     * 0xa4 extension page (0x7) layout. It can be used to disable/enable
     * the RX/TX delays otherwise controlled by RXDLY/TXDLY pins. It can
     * also be used to customize the whole configuration register:
     * 8:6 = PHY Address, 5:4 = Auto-Negotiation, 3 = Interface Mode Select,
     * 2 = RX Delay, 1 = TX Delay, 0 = SELRGV (see original PHY datasheet
     * for details).
     */
	// page select
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, RTL821x_PAGE_SELECT, 0x7);
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, RTL821x_EXT_PAGE_SELECT, 0xa4);

	Status  = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0x1c, &PhyReg0);
	PhyReg0 |= RTL8211E_RX_DELAY | RTL8211E_TX_DELAY;
	// PhyReg0 &= ~(RTL8211E_RX_DELAY | RTL8211E_TX_DELAY);
	// PhyReg0 |= RTL8211E_MAGIC;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0x1c, PhyReg0);

	// return to page 0
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, RTL821x_PAGE_SELECT, 0x0);

	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, RTL8211x_PHY_INSR, &PhyReg0);

	xil_printf("waiting phy setup link ..");
	u16 physr = 0;
	do {
		xil_printf(".");
		EmacpsDelay(10);
		Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0x11, &physr);
	} while ((physr & RTL8211E_PHYSR_LINK_STATUS) != RTL8211E_PHYSR_LINK_STATUS);
	xil_printf(" [Done]\n\r");

	xil_printf("waiting phy Auto-Negotiation Complete  ..");
    u16 bmsr = 0;
    do {
        xil_printf(".");
        EmacpsDelay(10);
        Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0x1, &bmsr);
    } while ((bmsr & RTL8211E_AUTO_NEGOTIATION_COMPLETE) !=
             RTL8211E_AUTO_NEGOTIATION_COMPLETE);
    xil_printf(" [Done]\n\r");


	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0x11, &physr);
	u32 _speed = physr & RTL8211E_PHYSR_SPEED_MASK;
	switch(_speed) {
		case RTL8211E_PHYSR_SPEED_10M:
			*Speed = 10;
			break;
		case RTL8211E_PHYSR_SPEED_100M:
			*Speed = 100;
			break;
		case RTL8211E_PHYSR_SPEED_1G:
			*Speed = 1000;
			break;
		default:
			xil_printf("Unknown speed!\n\r");
			return XST_FAILURE;
	}
	u32 duplex = physr & RTL8211E_PHYSR_DUPLEX_MASK;
	xil_printf("[Link solved] Speed :%d M, %s Duplex\n\r",
		*Speed,	duplex == 0 ? "Half" : "Full");

	/*PhyReg0 = 0;
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	} else {
		xil_printf("reg0 : %x\n\r", PhyReg0);
	}

	PhyReg0 = 0;
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0x1, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	} else {
		xil_printf("reg0x1 : %x\n\r", PhyReg0);
	}

	PhyReg0 = 0;
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0x10, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	} else {
		xil_printf("reg0x10 : %x\n\r", PhyReg0);
	}

	PhyReg0 = 0;
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0x11, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	} else {
		xil_printf("reg0x11 : %x\n\r", PhyReg0);
	}*/

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function provides delays in seconds
*
* @param    Delay in seconds
*
* @return   None.
*
* @note     for microblaze the delay is not accurate and it need to tuned.
*
*****************************************************************************/
void EmacpsDelay(u32 delay)
{
	unsigned long count=0;
	while(count < (delay * 0xffff))
	{
		count++;
	}
}


LONG EmacPsUtilEnterLoopback(XEmacPs * EmacPsInstancePtr, u32 Speed)
{
	LONG Status;
	u16 PhyIdentity;
	u32 PhyAddr;

	/*
	 * Detect the PHY address
	 */
	PhyAddr = XEmacPsDetectPHY(EmacPsInstancePtr);

	if (PhyAddr >= 32) {
		EmacPsUtilErrorTrap("Error detect phy");
		return XST_FAILURE;
	}

	if (XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, PHY_DETECT_REG1, &PhyIdentity) != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Phy Read Error");
	}

	if (PhyIdentity == PHY_ID_MARVELL) {
		Status = EmacPsUtilMarvellPhyLoopback(EmacPsInstancePtr, Speed, PhyAddr);
	} else if (PhyIdentity == PHY_ID_TI) {
		Status = EmacPsUtilTiPhyLoopback(EmacPsInstancePtr, Speed, PhyAddr);
	} else if (PhyIdentity == PHY_ID_RTL) {
		Status = EmacPsUtilRtlPhyLoopback(EmacPsInstancePtr, Speed, PhyAddr);
	} else {
		xil_printf("phy identity: %d\n\r", (uint32_t) PhyIdentity);
		EmacPsUtilErrorTrap("Unknown phy identity");
	}

	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is called by example code when an error is detected. It
* can be set as a breakpoint with a debugger or it can be used to print out
* the given message if there is a UART or STDIO device.
*
* @param    Message is the text explaining the error
*
* @return   None
*
* @note     None
*
*****************************************************************************/
void EmacPsUtilErrorTrap(const char *Message)
{
	static u32 Count = 0;

	Count++;

	xil_printf("Error: %s\r\n", Message);
	for(;;);
}
