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

#define D_TCP
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
  // Handle an incoming FIN segment
  //TODO
}
void
TCPConnection::AppClose() {
  // Handle close from application
  //TODO
}
void
TCPConnection::Kill() {
  // Handle an incoming RST segment, can also called in other error conditions
  //TODO
}
void
TCPConnection::Receive(udword theSynchronizationNumber, byte*  theData, udword theLength) {
  // Handle incoming data
  //TODO
}
void
TCPConnection::Acknowledge(udword theAcknowledgementNumber) {
  // Handle incoming Acknowledgement
  //TODO
}
void
TCPConnection::Send(byte*  theData, udword theLength) {
  // Send outgoing data
  //TODO
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
  switch (theConnection->myPort)
  {
  case 7:
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
    break;
  default:
    trace << "send RST..." << endl;
    theConnection->sendNext = 0;
    // Send a segment with the RST flag set.
    theConnection->myTCPSender->sendFlags(0x04);
    TCP::instance().deleteConnection(theConnection);
    break;
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
//TODO

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

  // Go to NetClose wait state, inform application
  theConnection->myState = CloseWaitState::instance();

  // Normally the application would be notified next and nothing
  // happen until the application calls appClose on the connection.
  // Since we don't have an application we simply call appClose here instead.

  // Simulate application Close...
  theConnection->AppClose();
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

  // Delayed ACK is not implemented, simply acknowledge the data
  // by sending an ACK segment, then echo the data using Send.

}

void
EstablishedState::Acknowledge(TCPConnection* theConnection, udword theAcknowledgementNumber) {
  //TODO
}

void
EstablishedState::Send(TCPConnection* theConnection, byte*  theData, udword theLength) {
  //TODO
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
  //TODO
}








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
  //TODO
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
  // Decide on the value of the length totalSegmentLength.
  // Allocate a TCP segment.
  uword totalSegmentLength = TCP::tcpHeaderLength;
  byte* anAnswer = new byte[totalSegmentLength];
  // Calculate the pseudo header checksum
  TCPPseudoHeader* aPseudoHeader =
    new TCPPseudoHeader(myConnection->hisAddress,
                        totalSegmentLength);
  uword pseudosum = aPseudoHeader->checksum();
  delete aPseudoHeader;
  // Create the TCP segment.
  aTCPHeader = (TCPHeader*) anAnswer;
  aTCPHeader->sourcePort = theConnection->myPort;
  aTCPHeader->destinationPort = theConnection->hisPort;
  aTCPHeader->sequenceNumber = theConnection->sendNext;
  aTCPHeader->acknowledgementNumber = theConnection->receiveNext;
  aTCPHeader->headerLength = TCP::headerLength;
  byte flags = 0x12; //ack and syn = 1, rest 0's
  aTCPHeader->flags = flags;


  // Calculate the final checksum.
  aTCPHeader->checksum = calculateChecksum(anAnswer,
                         totalSegmentLength,
                         pseudosum);
  // Send the TCP segment.
  myAnswerChain->answer(anAnswer,
                        totalSegmentLength);
  // Deallocate the dynamic memory
  delete anAnswer;
}

void
TCPSender::sendData(byte* theDate, udword theLength) {
  //TODO
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
  mySourcePort = aTCPHeader->sourcePort;
  myDestinationPort = aTCPHeader->destinationPort;


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
      aConnection->Kill();
    }
  }
  else
  {
    // Connection was established. Handle all states.

  }
}

void
TCPInPacket::answer(byte* theData, udword theLength) {
  //TODO
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

