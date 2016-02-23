/*!***************************************************************************
*!
*! FILE NAME  : tcp.cc
*!
*! DESCRIPTION: TCP, Transport control protocol
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
/****************** TCP DEFINITION SECTION *************************/

//----------------------------------------------------------------------------
//
TCP::TCP()
{
  trace << "TCP created." << endl;
}

//----------------------------------------------------------------------------
//
TCP&
TCP::instance()
{
  static TCP myInstance;
  return myInstance;
}

//----------------------------------------------------------------------------
//
TCPConnection*
TCP::getConnection(IPAddress& theSourceAddress,
                   uword      theSourcePort,
                   uword      theDestinationPort)
{
  TCPConnection* aConnection = NULL;
  // Find among open connections
  uword queueLength = myConnectionList.Length();
  myConnectionList.ResetIterator();
  bool connectionFound = false;
  while ((queueLength-- > 0) && !connectionFound)
  {
    aConnection = myConnectionList.Next();
    connectionFound = aConnection->tryConnection(theSourceAddress,
                      theSourcePort,
                      theDestinationPort);
  }
  if (!connectionFound)
  {
    trace << "Connection not found!" << endl;
    aConnection = NULL;
  }
  else
  {
    trace << "Found connection in queue" << endl;
  }
  return aConnection;
}

//----------------------------------------------------------------------------
//
TCPConnection*
TCP::createConnection(IPAddress& theSourceAddress,
                      uword      theSourcePort,
                      uword      theDestinationPort,
                      InPacket*  theCreator)
{
  TCPConnection* aConnection =  new TCPConnection(theSourceAddress,
      theSourcePort,
      theDestinationPort,
      theCreator);
  myConnectionList.Append(aConnection);
  return aConnection;
}


bool 
TCP::acceptConnection(uword port){
  if(port == 7) {
    return true;
  }
  return false;
}

void 
TCP::connectionEstablished(TCPConnection *theConnection) 
{ 
  trace << "TCP::connectionEstablished" << endl;
  if (theConnection->serverPortNumber() == 7) 
  { 
    trace << "found port 7" << endl;
    TCPSocket* aSocket = new TCPSocket(theConnection); 
    // Create a new TCPSocket. 
    theConnection->registerSocket(aSocket); 
    // Register the socket in the TCPConnection. 
    Job::schedule(new SimpleApplication(aSocket)); 
    // Create and start an application for the connection. 
  } 
}

//----------------------------------------------------------------------------
//
void
TCP::deleteConnection(TCPConnection* theConnection)
{
  myConnectionList.Remove(theConnection);
  delete theConnection;
}

//----------------------------------------------------------------------------
//
TCPConnection::TCPConnection(IPAddress& theSourceAddress,
                             uword      theSourcePort,
                             uword      theDestinationPort,
                             InPacket*  theCreator):
  hisAddress(theSourceAddress),
  hisPort(theSourcePort),
  myPort(theDestinationPort)
{
  trace << "TCP connection created" << endl;
  myTCPSender = new TCPSender(this, theCreator),
  myState = ListenState::instance();
}

//----------------------------------------------------------------------------
//
TCPConnection::~TCPConnection()
{
  trace << "TCP connection destroyed" << endl;
  delete myTCPSender;
}

//----------------------------------------------------------------------------
//
bool
TCPConnection::tryConnection(IPAddress& theSourceAddress,
                             uword      theSourcePort,
                             uword      theDestinationPort)
{
  return (theSourcePort      == hisPort   ) &&
         (theDestinationPort == myPort    ) &&
         (theSourceAddress   == hisAddress);
}


// TCPConnection cont...
void
TCPConnection::Synchronize(udword theSynchronizationNumber) {
  myState->Synchronize(this, theSynchronizationNumber);
  // Handle an incoming SYN segment

  //TODO
}
void
TCPConnection::NetClose() {
  myState->NetClose(this);
  // Handle an incoming FIN segment
  //DONE
}
void
TCPConnection::AppClose() {
  myState->AppClose(this);
}

void
TCPConnection::Kill() {
  // Handle an incoming RST segment, can also called in other error conditions
  //Send fin?
  myState->Kill(this);
  //TODO
}
void
TCPConnection::Receive(udword theSynchronizationNumber, byte*  theData, udword theLength) {
  // Handle incoming data
  myState->Receive(this, theSynchronizationNumber, theData, theLength);
  //TODO
}
void
TCPConnection::Acknowledge(udword theAcknowledgementNumber) {
  // Handle incoming Acknowledgement
  myState->Acknowledge(this, theAcknowledgementNumber);
  //TODO
}
void
TCPConnection::Send(byte* theData, udword theLength) {
  // Send outgoing data
  //TODO
  //myState->Send(this, theData, theLength);
  myState->Send(this, theData, theLength);
}

uword
TCPConnection::serverPortNumber(){
  return myPort;
}

void
TCPConnection::registerSocket(TCPSocket* theSocket){
  mySocket = theSocket;
}

udword
TCPConnection::theOffset(){
  return sendNext - firstSeq;
}

byte*
TCPConnection::theFirst(){
  return transmitQueue + theOffset();
}

udword
TCPConnection::theSendLength(){
  trace << "the offset: " << theOffset() << " queueLength: " << queueLength << endl;
  if((queueLength - theOffset()) < 1460) {
      return queueLength - theOffset();
  } else {
      return 1460;
  }
}

//----------------------------------------------------------------------------
// TCPState contains dummies for all the operations, only the interesting ones
// gets overloaded by the various sub classes.


//----------------------------------------------------------------------------
//
void
TCPState::Kill(TCPConnection* theConnection)
{
  // Handle an incoming RST segment, can also called in other error conditions

  trace << "TCPState::Kill" << endl;
  TCP::instance().deleteConnection(theConnection);
}

void
TCPState::Synchronize(TCPConnection* theConnection, udword theSynchronizationNumber)
{
  trace << "TCPState::Synchronize" << endl;
  // Handle an incoming SYN segment
  //TODO
}

void
TCPState::NetClose(TCPConnection* theConnection)
{
  // Handle an incoming FIN segment
  //TODO
  trace << "TCPState::NetClose" << endl;
}

void
TCPState::AppClose(TCPConnection* theConnection)
{
  // Handle close from application
  //TODO
  trace << "TCPState::AppClose" << endl;
}

void
TCPState::Receive(TCPConnection* theConnection, udword theSynchronizationNumber, byte*  theData, udword theLengthn)
{
  // Handle incoming data
  //TODO
  trace << "TCPState::Receive" << endl;
}

void
TCPState::Acknowledge(TCPConnection* theConnection, udword theAcknowledgementNumber)
{
  // Handle incoming Acknowledgement
  //TODO
  trace << "TCPState::Acknowledge" << endl;
}

void
TCPState::Send(TCPConnection* theConnection, byte* theData, udword theLength)
{
  // Send outgoing data
  //TODO
  trace << "TCPState::Send" << endl;
}


//----------------------------------------------------------------------------
//
ListenState*
ListenState::instance()
{
  static ListenState myInstance;
  return &myInstance;
}


void
ListenState::Synchronize(TCPConnection* theConnection, udword theSynchronizationNumber)
{
  if(TCP::instance().acceptConnection(theConnection->myPort)){
    trace << "got SYN on ECHO port" << endl;
    theConnection->receiveNext = theSynchronizationNumber + 1;
    theConnection->receiveWindow = 8 * 1024;
    theConnection->sendNext = get_time();
    // Next reply to be sent.
    theConnection->sentUnAcked = theConnection->sendNext;
    // Send a segment with the SYN and ACK flags set.
    theConnection->myTCPSender->sendFlags(0x12);
    // Prepare for the next send operation.
    theConnection->sendNext += 1;
    // Change state
    theConnection->myState = SynRecvdState::instance();
    trace << "SynRecvdState state set" << endl;
  } else {
    trace << "send RST..." << endl;
    theConnection->sendNext = 0;
    // Send a segment with the RST flag set.
    theConnection->myTCPSender->sendFlags(0x04);
    TCP::instance().deleteConnection(theConnection);
  }

}

//----------------------------------------------------------------------------
//
SynRecvdState*
SynRecvdState::instance()
{
  static SynRecvdState myInstance;
  return &myInstance;
}

void
SynRecvdState::Acknowledge(TCPConnection* theConnection, udword theAcknowledgementNumber) {
//DONE
  trace << "SynRecvdState::Acknowledge" << endl;
  if (theAcknowledgementNumber == theConnection->sendNext) {
    trace<<"EstablishedState::instance()" << endl;
    theConnection->myState = EstablishedState::instance();
    TCP::instance().connectionEstablished(theConnection);
  } else {
    trace << "Wrong ackNbr" << endl;
    theConnection->Kill();
  }
    /*
    if (theAcknowledgementNumber > theConnection->sentUnAcked) {
    trace << "ACK received, changing to EstablishedState" << endl;
    // Setting the last acked segment
    theConnection->sentUnAcked = theAcknowledgementNumber;
    
    // Changing state to established
    theConnection->myState = EstablishedState::instance();
    */
 // }

}



//----------------------------------------------------------------------------
//
EstablishedState*
EstablishedState::instance()
{
  static EstablishedState myInstance;
  return &myInstance;
}


//----------------------------------------------------------------------------
//
void
EstablishedState::NetClose(TCPConnection* theConnection)
{
  trace << "EstablishedState::NetClose" << endl;

  // Update connection variables and send an ACK
  //DONE
  (theConnection->receiveNext) += 1;
  theConnection->myTCPSender->sendFlags(0x10);

  theConnection->mySocket->socketEof();

  // Normally the application would be notified next and nothing
  // happen until the application calls appClose on the connection.
  // Since we don't have an application we simply call appClose here instead.

}

//----------------------------------------------------------------------------
//
void
EstablishedState::Receive(TCPConnection* theConnection,
                          udword theSynchronizationNumber,
                          byte*  theData,
                          udword theLength)
{
  trace << "EstablishedState::Receive" << endl;

  if(theSynchronizationNumber == theConnection->receiveNext){

    theConnection->receiveNext += theLength;
    theConnection->sentUnAcked = theConnection->sendNext; 
    theConnection->myTCPSender->sendFlags(0x10);

    
    theConnection->mySocket->socketDataReceived(theData, theLength);
  }
  
  // Delayed ACK is not implemented, simply acknowledge the data
  // by sending an ACK segment, then echo the data using Send.

}

void
EstablishedState::Acknowledge(TCPConnection* theConnection, udword theAcknowledgementNumber) {
  trace << "EstablishedState::Acknowledge" << endl;
  if (theAcknowledgementNumber > theConnection->sentUnAcked) {
    trace << "rec ack greater than unAcked" << endl;
    // Setting the last acked segment
    theConnection->sentUnAcked = theAcknowledgementNumber;

  }
  trace << "theAcknowledgementNumber: " << theAcknowledgementNumber << 
  "sendNext: " << theConnection->sendNext << endl;

  if(theConnection->theOffset() == theConnection->queueLength) {
    theConnection->mySocket->socketDataSent();
  } else {
    trace << "EstablishedState::Acknowledge sendFromQueue" << endl;
    theConnection->myTCPSender->sendFromQueue();
  }
}

void
EstablishedState::Send(TCPConnection* theConnection, byte*  theData, udword theLength) {
  //TODO
  theConnection->transmitQueue = theData;
  theConnection->queueLength = theLength;
  theConnection->firstSeq = theConnection->sendNext;

  theConnection->myTCPSender->sendFromQueue();

}
void
EstablishedState::AppClose(TCPConnection* theConnection){
  trace << "EstablishedState::AppClose" << endl;
  theConnection->myState = FinWait1State::instance();
  theConnection->myTCPSender->sendFlags(0x11); //Send FIN
  theConnection->sendNext = theConnection->sendNext + 1;
}


//----------------------------------------------------------------------------
//
CloseWaitState*
CloseWaitState::instance()
{
  static CloseWaitState myInstance;
  return &myInstance;
}

void
CloseWaitState::AppClose(TCPConnection* theConnection) {
  //DONE
  trace << "CloseWaitState::AppClose" << endl;
  theConnection->myTCPSender->sendFlags(0x11);
  theConnection->myState = LastAckState::instance();
  
  theConnection->Kill(); // should not be done here, ACK should come from linus first
}

FinWait1State*
FinWait1State::instance()
{
  static FinWait1State  myInstance;
  return &myInstance;
}

void
FinWait1State::Acknowledge(TCPConnection* theConnection, udword theAcknowledgementNumber) {
  //DONE
  trace << "FinWait1State ack" << endl;
  trace << "theConnection->sendNext in FinWait1State::ACK..." << theConnection->sendNext << endl;
  if (theConnection->sendNext == theAcknowledgementNumber) {
    trace << "Correct ack number" << endl;
    theConnection->myState = FinWait2State::instance();
  } else {
    trace << "Incorrect ack number" << endl;
  }
}


FinWait2State*
FinWait2State::instance()
{
  static FinWait2State   myInstance;
  return &myInstance;
}

void
FinWait2State::NetClose(TCPConnection* theConnection) {
  trace << "FinWait2State::NetClose" << endl;
    theConnection->receiveNext += 1;
    theConnection->myTCPSender->sendFlags(0x10);
    theConnection->Kill();
    //theConnection->myState TimeWait::instance(); //Perhaps add with timeout
}
/*
TimeWait*
TimeWait::instance()
{
  static TimeWait   myInstance;
  return &myInstance;
}
*/
//----------------------------------------------------------------------------
//
LastAckState*
LastAckState::instance()
{
  static LastAckState myInstance;
  return &myInstance;
}
void
LastAckState::Acknowledge(TCPConnection* theConnection, udword theAcknowledgementNumber) {
  //DONE
  trace << "LastAckState::Acknowledge" << endl;
  if (theConnection->receiveNext == theAcknowledgementNumber) {
    trace << "Correct LastAck ack number" << endl;
    theConnection->Kill();
  } else {
    trace << "INcorrect LastAck ack number, terminiating anyway" << endl;
    theConnection->Kill();
  }
}

//----------------------------------------------------------------------------
//
TCPSender::TCPSender(TCPConnection* theConnection,
                     InPacket*      theCreator):
  myConnection(theConnection),
  myAnswerChain(theCreator->copyAnswerChain()) // Copies InPacket chain!
{
}

//----------------------------------------------------------------------------
//
TCPSender::~TCPSender()
{
  myAnswerChain->deleteAnswerChain();
}
void
TCPSender::sendFlags(byte theFlags)
{
  trace << "TCPSender::sendFlags" << endl;
  // Decide on the value of the length totalSegmentLength.
  // Allocate a TCP segment.
  uword hoffs = myAnswerChain->headerOffset();
  uword totalSegmentLength = TCP::tcpHeaderLength; //data = 0
  byte* anAnswer = new byte[hoffs + totalSegmentLength];
  anAnswer+= hoffs;
  // Calculate the pseudo header checksum
  TCPPseudoHeader* aPseudoHeader =
    new TCPPseudoHeader(myConnection->hisAddress,
                        totalSegmentLength);
  uword pseudosum = aPseudoHeader->checksum();
  delete aPseudoHeader;
  // Create the TCP segment.
  TCPHeader* aTCPHeader = (TCPHeader*) anAnswer;
  aTCPHeader->sourcePort = HILO(myConnection->myPort);
  aTCPHeader->destinationPort = HILO(myConnection->hisPort);
  aTCPHeader->sequenceNumber = LHILO(myConnection->sendNext);
  aTCPHeader->acknowledgementNumber = LHILO(myConnection->receiveNext);
  //byte headerShifted = (TCP::tcpHeaderLength/4) << 4;  //32 bit words => (/4)

  //uword temp = aTCPHeader->headerLength;
  aTCPHeader->headerLength = (byte) (TCP::tcpHeaderLength << 2);
  //aTCPHeader->headerLength = headerShifted;
  //aTCPHeader->headerLength |= temp; // so we don't overwrite flags/reserved bits
  aTCPHeader->flags = theFlags;
  aTCPHeader->windowSize = HILO(myConnection->receiveWindow);
  aTCPHeader->urgentPointer = 0;
  aTCPHeader->checksum = 0;

  // Calculate the final checksum.
  aTCPHeader->checksum = calculateChecksum(anAnswer,
                         totalSegmentLength,
                         pseudosum);
  // Send the TCP segment.
  trace << "SENDING Tcp Packet! Src port: " << myConnection->myPort << " Dest port: " << 
  myConnection->hisPort << " Seq#: " << myConnection->sendNext << " Ack#: " << myConnection->receiveNext << endl;
  myAnswerChain->answer(anAnswer, totalSegmentLength);
  // Deallocate the dynamic memory
}

void
TCPSender::sendData(byte* theData, udword theLength) {
  //TODO
  trace << "TCPSender::sendData" << endl;
  // Decide on the value of the length totalSegmentLength.
  // Allocate a TCP segment.
  uword hoffs = myAnswerChain->headerOffset();
  uword totalSegmentLength = theLength + TCP::tcpHeaderLength; //data = 0
  byte* anAnswer = new byte[hoffs + totalSegmentLength];
  anAnswer+= hoffs;
  memcpy(anAnswer+TCP::tcpHeaderLength, theData, theLength);
  // Calculate the pseudo header checksum
  TCPPseudoHeader* aPseudoHeader =
    new TCPPseudoHeader(myConnection->hisAddress,
                        totalSegmentLength);
  uword pseudosum = aPseudoHeader->checksum();
  delete aPseudoHeader;
  // Create the TCP segment.
  TCPHeader* aTCPHeader = (TCPHeader*) anAnswer;
  aTCPHeader->sourcePort = HILO(myConnection->myPort);
  aTCPHeader->destinationPort = HILO(myConnection->hisPort);
  aTCPHeader->sequenceNumber = LHILO(myConnection->sendNext);
  aTCPHeader->acknowledgementNumber = LHILO(myConnection->receiveNext);
  //byte headerShifted = (TCP::tcpHeaderLength/4) << 4;  //32 bit words => (/4)

  //uword temp = aTCPHeader->headerLength;
  aTCPHeader->headerLength = (byte) (TCP::tcpHeaderLength << 2);
  //aTCPHeader->headerLength = headerShifted;
  //aTCPHeader->headerLength |= temp; // so we don't overwrite flags/reserved bits
  aTCPHeader->flags = 0x18;
  aTCPHeader->windowSize = HILO(myConnection->receiveWindow);
  aTCPHeader->urgentPointer = 0;
  aTCPHeader->checksum = 0;

  // Calculate the final checksum.
  aTCPHeader->checksum = calculateChecksum(anAnswer,
                         totalSegmentLength,
                         pseudosum);
  // Send the TCP segment.
  trace << "SENDING Tcp Packet DATA! Src port: " << myConnection->myPort << " Dest port: " << 
  myConnection->hisPort << " Seq#: " << myConnection->sendNext << " Ack#: " << myConnection->receiveNext << endl;
  myAnswerChain->answer(anAnswer, totalSegmentLength);
  myConnection->sendNext += theLength; //Increase seq

  // Deallocate the dynamic memory
}


void
TCPSender::sendFromQueue(){
  trace << "TCPSender::sendFromQueue" << endl;
  udword theWindowSize = myConnection->myWindowSize - (myConnection->sendNext - myConnection->sentUnAcked);
  sendData(myConnection->theFirst(), MIN(theWindowSize, myConnection->theSendLength()));
}
//----------------------------------------------------------------------------
//
TCPInPacket::TCPInPacket(byte*           theData,
                         udword          theLength,
                         InPacket*       theFrame,
                         IPAddress&      theSourceAddress):
  InPacket(theData, theLength, theFrame),
  mySourceAddress(theSourceAddress)
{
}


//----------------------------------------------------------------------------
//
InPacket*
TCPInPacket::copyAnswerChain()
{
  return myFrame->copyAnswerChain();
}

void
TCPInPacket::decode()
{
  // Extract the parameters from the TCP header which define the
  // connection.
  TCPHeader* aTCPHeader = (TCPHeader*) myData;
  myDestinationPort = HILO(aTCPHeader->destinationPort);
  mySourcePort = HILO(aTCPHeader->sourcePort);
  mySequenceNumber = LHILO(aTCPHeader->sequenceNumber);
  myAcknowledgementNumber = LHILO(aTCPHeader->acknowledgementNumber);

  trace << "Incoming TCP packet! Src port: " << mySourcePort << " Dest port: " << 
  myDestinationPort << " Seq#: " << mySequenceNumber << " Ack#: " << myAcknowledgementNumber << endl;

  TCPConnection* aConnection =
    TCP::instance().getConnection(mySourceAddress,
                                  mySourcePort,
                                  myDestinationPort);
  if (!aConnection)
  {
    // Establish a new connection.
    aConnection =
      TCP::instance().createConnection(mySourceAddress,
                                       mySourcePort,
                                       myDestinationPort,
                                       this);
    if ((aTCPHeader->flags & 0x02) != 0)
    {
      // State LISTEN. Received a SYN flag.
      aConnection->Synchronize(mySequenceNumber);
    }
    else
    {
      // State LISTEN. No SYN flag. Impossible to continue.
      trace << "no SYN flag in new conn" << endl;
      aConnection->Kill();
    }
  }
  else
  {
    //DONE
    trace << "Decoding incoming flags in TCP layer." << endl;
    aConnection->myWindowSize = HILO(aTCPHeader->windowSize);
    if ((aTCPHeader->flags & 0x18) == 0x18) { //ACK and PSH flag
      trace << "found ACK and PSH flag" << endl;
      aConnection->Receive(mySequenceNumber, myData + TCP::tcpHeaderLength, myLength - TCP::tcpHeaderLength);

    } else if ((aTCPHeader->flags & 0x11) == 0x11) { //ACK and FIN flag
      trace << "found ACK and FIN flag" << endl;
      aConnection->Acknowledge(myAcknowledgementNumber);
      aConnection->NetClose();
    } else if ((aTCPHeader->flags & 0x10) == 0x10) { //ACK flag
      trace << "found ACK flag" << endl;
      aConnection->Acknowledge(myAcknowledgementNumber);
    }  
    if ((aTCPHeader->flags & 0x04) == 0x04) { //RST flag
      trace << "RST FLAG" << endl;
      aConnection->Kill();
    }
    if ((aTCPHeader->flags & 0x02) == 0x02) {// SYN flag
      trace << "Received syn flag, should not?" << endl;
      //aConnection->Synchronize(mySequenceNumber);
    }
  }
}

void
TCPInPacket::answer(byte* theData, udword theLength) {
  //TODO
  trace << "TCPInPacket::anwer" << endl;
}

uword
TCPInPacket::headerOffset() {
  return myFrame->headerOffset() + TCP::tcpHeaderLength; //DONE?
}

//----------------------------------------------------------------------------
//
TCPPseudoHeader::TCPPseudoHeader(IPAddress& theDestination,
                                 uword theLength):
  sourceIPAddress(IP::instance().myAddress()),
  destinationIPAddress(theDestination),
  zero(0),
  protocol(6)
{
  tcpLength = HILO(theLength);
}

//----------------------------------------------------------------------------
//
uword
TCPPseudoHeader::checksum()
{
  return calculateChecksum((byte*)this, 12);
}




/****************** END OF FILE tcp.cc *************************************/

