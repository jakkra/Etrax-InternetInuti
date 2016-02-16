/*!***************************************************************************
*!
*! FILE NAME  : Arp.cc
*!
*! DESCRIPTION: Handles the Arp packet layer.
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
#include "arp.hh"
#include "ip.hh"

//#define D_LLC
#ifdef D_LLC
#define trace cout
#else
#define trace if(false) cout
#endif

/********************** LOCAL VARIABLE DECLARATION SECTION *****************/

ARPInPacket::ARPInPacket(byte*      theData,
                         udword     theLength,
                         InPacket*  theFrame):
	InPacket(theData, theLength, theFrame)
{
}

void
ARPInPacket::decode()
{
	trace << "arp decode" << endl;
	ARPHeader *arpHeader = (ARPHeader*) myData;
	if (arpHeader->targetIPAddress == IP::instance().myAddress()) {
		trace << "arp ip match with cd ip" << endl;
		uword hoffs = myFrame->headerOffset();
		byte* temp = new byte[myLength + hoffs];
		byte* aReply = temp + hoffs;
		memcpy(aReply, myData, myLength);
		ARPHeader *arpReplyHeader = (ARPHeader*) aReply; //10 bytes padding after the header exists.		

		uword realOpCode = HILO(2); 
		arpReplyHeader->op = realOpCode; //1, = send, 2 = reply
		arpReplyHeader->senderEthAddress = Ethernet::instance().myAddress();
		arpReplyHeader->senderIPAddress = IP::instance().myAddress();
		arpReplyHeader->targetEthAddress = arpHeader->senderEthAddress;
		arpReplyHeader->targetIPAddress = arpHeader->senderIPAddress;

		this->answer(aReply, myLength + hoffs); //aReply = arp Header
	}

}

void
ARPInPacket::answer(byte* theData, udword theLength)
{
	myFrame->answer(theData, theLength);
}

uword
ARPInPacket::headerOffset()
{
	return myFrame->headerOffset();
}
