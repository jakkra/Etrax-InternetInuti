/*!***************************************************************************
*!
*! FILE NAME  : tcpsocket.cc
*!
*! DESCRIPTION: TCP Socket
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
#include "timr.h"
}

#include "iostream.hh"
#include "tcp.hh"
#include "ip.hh"
#include "tcpsocket.hh"


//#define D_TCP
#ifdef D_TCP
#define trace cout
#else
#define trace if(false) cout
#endif
/****************** TCPSocket DEFINITION SECTION *************************/

//----------------------------------------------------------------------------
//

byte* 
TCPSocket::Read(udword& theLength) 
{ 
  myReadSemaphore->wait(); // Wait for available data 
  theLength = myReadLength; 
  byte* aData = myReadData; 
  myReadLength = 0; 
  myReadData = 0; 
  return aData; 
}

void 
TCPSocket::socketDataReceived(byte* theData, udword theLength) 
{ 
  myReadData = new byte[theLength]; 
  memcpy(myReadData, theData, theLength); 
  myReadLength = theLength; 
  myReadSemaphore->signal(); // Data is available 
}


void 
TCPSocket::Write(byte* theData, udword theLength) 
{ 
  myConnection->Send(theData, theLength); 
  myWriteSemaphore->wait(); // Wait until the data is acknowledged 
}

void 
TCPSocket::socketDataSent() 
{ 
  myWriteSemaphore->signal(); // The data has been acknowledged 
}

void 
TCPSocket::socketEof() 
{ 
  eofFound = true; 
  myReadSemaphore->signal(); 
}

void
TCPSocket::Close(){
	myConnection->appClose();
}

SimpleApplication::doit() 
{ 
  udword aLength; 
  byte* aData; 
  bool done = false; 
  while (!done && !mySocket->isEof()){ 
    aData = mySocket->Read(aLength); 
    if (aLength > 0) 
    { 
      mySocket->Write(aData, aLength); 
      if ((char)*aData == 'q') 
      { 
        done = true; 
      } 
      delete aData; 
    } 
  } 
  mySocket->Close(); 
}
