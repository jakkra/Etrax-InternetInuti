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
#include "threads.hh"


//#define D_TCP
#ifdef D_TCP
#define trace cout
#else
#define trace if(false) cout
#endif
/****************** TCPSocket DEFINITION SECTION *************************/

//----------------------------------------------------------------------------
//

TCPSocket::TCPSocket(TCPConnection* theConnection):
myConnection(theConnection),
myReadSemaphore(Semaphore::createQueueSemaphore("Read", 0)),
myWriteSemaphore(Semaphore::createQueueSemaphore("Write", 0))
{

}

TCPSocket::~TCPSocket(){


  //cout << "delete TCPSocket" << endl;
  //myReadSemaphore->signal();
  //myWriteSemaphore->signal();
  delete myReadSemaphore;
  
  //cout << "After delete myRead" << endl;

  delete myWriteSemaphore;

  //cout << "after delete myWrite" << endl;
  //signal()
  //delete myReadSemaphore;
  //delete myWriteSemaphore;
}

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
  if(myConnection->RSTFlag){
    //cout << "RST FLAG DID NOT WRITE becasue was true" << endl;
    return;
  }
  myConnection->Send(theData, theLength); 
  myWriteSemaphore->wait(); // Wait until the data is acknowledged 
}

void 
TCPSocket::socketDataSent() 
{ 
  trace << "TCPSocket: all data has been ack'ed port: " << myConnection->hisPort << endl;
  myWriteSemaphore->signal(); // The data has been acknowledged 
}

void 
TCPSocket::socketEof() 
{ 
  eofFound = true; 
  myReadSemaphore->signal(); 
}

bool
TCPSocket::isEof() {
  return eofFound;
  // True if a FIN has been received from the remote host.
}
  

void
TCPSocket::Close(){
	myConnection->AppClose();
}


SimpleApplication::SimpleApplication(TCPSocket* theSocket):
mySocket(theSocket){

}

void
SimpleApplication::doit() 
{ 
  trace << "SimpleApplication::doit" << endl;
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
        trace << "-------SimpleApplication::doit found q in stream-------" << endl;
        done = true; 
      } else if((char)*aData == 's'){
        trace << "-------SimpleApplication::doit found s in stream-------" << endl;
        byte* space =  (byte*) malloc(1000*1000);
        for (int i = 0; i < 1000*1000; i++)
        {
          space[i] = 'A' + i % (0x5B-0x41);
        }
        mySocket->Write(space, 1000*1000);
        delete space;

      } else if((char)*aData == 'r') {
        trace << "-------SimpleApplication::doit found r in stream-------" << endl;
        byte* space =  (byte*) malloc(10000);
        for (int i = 0; i < 10000; i++)
        {
          space[i] = 'Z' - i % (0x5B-0x41);
        }
        mySocket->Write(space, 10000);
        delete space;
      } else if((char)*aData == 't') {
        trace << "-------SimpleApplication::doit found t in stream-------" << endl;
        byte* space =  (byte*) malloc(360);
        for (int i = 0; i < 360; i++)
        {
          space[i] = '0' + i % 10;
        }
        mySocket->Write(space, 360);
        delete space;
      }

      delete aData; 
    } 
  } 

  mySocket->Close();
}
