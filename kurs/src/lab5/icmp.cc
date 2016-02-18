/*!***************************************************************************
*!
*! FILE NAME  : icmp.cc
*!
*! DESCRIPTION: Handles the icmp packet layer.
*!
*!***************************************************************************/

/****************** INCLUDE FILES SECTION ***********************************/


#include "compiler.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
extern "C"
{
#include "system.h"
}

#include "iostream.hh"
#include "icmp.hh"
#include "ip.hh"

//#define D_LLC
#ifdef D_LLC
#define trace cout
#else
#define trace if(false) cout
#endif

/********************** LOCAL VARIABLE DECLARATION SECTION *****************/

ICMPInPacket::ICMPInPacket(byte*      theData,
                           udword     theLength,
                           InPacket*  theFrame):
	InPacket(theData, theLength, theFrame)
{
}

void
ICMPInPacket::decode()
{
	ICMPHeader *thisHeader = (ICMPHeader*) myData;
	trace << "Value icmp type: " << hex << int(thisHeader->type) << endl;

	if (thisHeader->type == 8) // icmp echo request
	{
		trace << "icmp echo request found" << endl;
		uword hoffs = myFrame->headerOffset();
		byte* temp = new byte[myLength + hoffs + ICMPInPacket::icmpHeaderLen];
		byte* aReply = temp + hoffs;

		memcpy(aReply, myData, myLength);

		ICMPHeader *replyHeader = (ICMPHeader*) aReply;
		replyHeader->type = 0;

		// Adjust ICMP checksum...
		uword oldSum = thisHeader->checksum;
		uword newSum = oldSum + 0x8;
		replyHeader->checksum = newSum;
		myFrame->answer(aReply, myLength);

	}

}

void
ICMPInPacket::answer(byte* theData, udword theLength)
{
	myFrame->answer(theData, theLength);
}

uword
ICMPInPacket::headerOffset()
{
	return myFrame->headerOffset() + icmpEchoHeaderLen;
}
