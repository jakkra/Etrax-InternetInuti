#include "compiler.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
extern "C"
{
#include "system.h"
}

#include "iostream.hh"
#include "ip.hh"
#include "ipaddr.hh"
#include "icmp.hh"

#define trace if(false) cout
static udword seqNum = 0;
byte protocol;
IPAddress srcIP;

IP::IP()
{
	myIPAddress = new IPAddress(0x82, 0xEB, 0xC8, 0x6E);
}

IP&
IP::instance()
{
	static IP myInstance;
	return myInstance;
}


const IPAddress&
IP::myAddress()
{
	return *myIPAddress;
}


IPInPacket::IPInPacket(byte* theData, udword theLength, InPacket* theFrame):
	InPacket(theData, theLength, theFrame)
{
}

void
IPInPacket::decode()
{
	IPHeader *ipHeader = (IPHeader*) myData;
	IPAddress destIP = ipHeader->destinationIPAddress;
	if (destIP == IP::instance().myAddress()) {
		trace << "decoding IP packet" << endl;
		srcIP = ipHeader->sourceIPAddress;
		//little endian nightmare
		byte ipv = ipHeader->versionNHeaderLength >> 4;
		byte headerLength = ipHeader->versionNHeaderLength & 0x0F;
		//byte typeOfService = HILO(ipHeader->typeOfService >> 2);
		uword totalLength = HILO(ipHeader->totalLength);
		byte flags = ipHeader->fragmentFlagsNOffset >> 13;
		//uword fragmentOffset = HILO(ipHeader->fragmentFlagsNOffset  & 0x1FFF);
		//byte timeToLive = HILO(ipHeader->timeToLive);
		protocol = ipHeader->protocol;
		//uword headerChecksum = HILO(ipHeader->headerChecksum);
		trace << "destIpAddress: " << destIP << endl;
		if (ipv == 4 && headerLength == 5 && (flags & 0x3FFF) == 0) {
			trace << "ipv4, headerlength = 5, flag = 0" << endl;
			uword dataLength = totalLength - (headerLength*4);
			if (protocol == 1) { //icmp
				trace << "icmp protocol detected" << endl;
				ICMPInPacket *icmp = new ICMPInPacket(myData + (headerLength)*4, dataLength, this);
				icmp->decode();
				delete icmp;
			} else if (protocol == 6) { //tcp
				trace << "tcp protocol detected" << endl;
			}
		}
	}
}

void
IPInPacket::answer(byte* theData, udword theLength)
{

	theData -= IP::ipHeaderLength;
	IPHeader* ipHeader = (IPHeader*) theData;
	byte ipVersionAndHeader = ((4 << 4) | 5);
	ipHeader->versionNHeaderLength = ipVersionAndHeader;
	ipHeader->typeOfService = 0;
	ipHeader->totalLength = HILO(theLength - myFrame->headerOffset());
	ipHeader->identification = HILO(seqNum);
	seqNum++;
	ipHeader->fragmentFlagsNOffset = 0;
	ipHeader->timeToLive = 64;
	ipHeader->protocol = protocol;
	ipHeader->headerChecksum = 0;
	ipHeader->sourceIPAddress = IP::instance().myAddress();
	ipHeader->destinationIPAddress = srcIP;

	byte headerLength = (ipHeader->versionNHeaderLength & 0x0F)*4;
	ipHeader->headerChecksum = calculateChecksum(theData, headerLength);//Why not HILO?

	myFrame->answer(theData, theLength);

}

uword
IPInPacket::headerOffset()
{
	return myFrame->headerOffset() + IP::ipHeaderLength;
}

