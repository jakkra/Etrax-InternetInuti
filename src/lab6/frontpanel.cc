/*!***************************************************************************
*!
*! FILE NAME  : FrontPanel.cc
*!
*! DESCRIPTION: Handles the LED:s
*!
*!***************************************************************************/

/****************** INCLUDE FILES SECTION ***********************************/

#include "compiler.h"

#include "iostream.hh"
#include "frontpanel.hh"
#include "tcp.hh"
//#define D_FP
#ifdef D_FP
#define trace cout
#else
#define trace if(false) cout
#endif

/****************** FrontPanel DEFINITION SECTION ***************************/

static byte write_out_register_shadow = 0x78;


LED::LED(byte theLedNumber)
{
  myLedBit = 4 << theLedNumber;
  // Constructor: initiate the bitmask 'myLedBit'. 8/16/32, N/S/C
}
// //----------------------------------------------------------------------------
// //
void LED::on()
{
  *(VOLATILE byte*)0x80000000 = write_out_register_shadow &= ~myLedBit ;
}

// //----------------------------------------------------------------------------
// //
void LED::off()
{
  *(VOLATILE byte*)0x80000000 = write_out_register_shadow |= myLedBit ;
}

// //----------------------------------------------------------------------------
// //
void LED::toggle()
{
  *(VOLATILE byte*)0x80000000 = write_out_register_shadow ^= myLedBit;
}

//--------------------------------------------------------------------------------
// StatusLEDTimer
StatusLEDTimer::StatusLEDTimer(Duration blinkPeriod) {
  this->timerInterval(blinkPeriod);
  startPeriodicTimer();
}

void StatusLEDTimer::timerNotify() {
  FrontPanel::instance().notifyLedEvent(FrontPanel::statusLedId);
}

//--------------------------------------------------------------------------------
// CDLEDTimer
CDLEDTimer::CDLEDTimer(Duration blinkPeriod) {
  this->timerInterval(blinkPeriod);
  startPeriodicTimer();
}

void CDLEDTimer::timerNotify() {
  FrontPanel::instance().notifyLedEvent(FrontPanel::cdLedId);
}

//--------------------------------------------------------------------------------
// NetworkLEDTimer
NetworkLEDTimer::NetworkLEDTimer(Duration blinkTime) {
  myBlinkTime = blinkTime;
}

void NetworkLEDTimer::start() {
  this->timeOutAfter(myBlinkTime);
}

void NetworkLEDTimer::timeOut() {
  FrontPanel::instance().notifyLedEvent(FrontPanel::networkLedId);
}

//----------------------------------------------------------------------------
// FrontPanel
//

FrontPanel::FrontPanel():
  Job(),
  mySemaphore(Semaphore::createQueueSemaphore("FP", 0)),
  myNetworkLED(networkLedId),
  myCDLED(cdLedId),
  myStatusLED(statusLedId)
{
  //ax_printf("Starting FrontPanel.\n");
  Job::schedule(this);
}

FrontPanel& FrontPanel::instance()
{
  static FrontPanel myInstance;
  return myInstance;

// Returns the instance of FrontPanel, used for accessing the FrontPanel
}

void FrontPanel::packetReceived()
{
  myNetworkLED.on();
  myNetworkLEDTimer->start();
// turn Network led on and start network led timers
}

void FrontPanel::notifyLedEvent(uword theLedId)
{
  switch (theLedId) {
  case statusLedId:
    statusLedEvent = true;
    break;
  case cdLedId:
    cdLedEvent = true;
    break;
  case networkLedId:
    netLedEvent = true;
    break;
  default:
    break;
  }
  mySemaphore->signal();
// Called from the timers to notify that a timer has expired.
  // Sets an event flag and signals the semaphore.
}

void FrontPanel::doit() {
  myNetworkLEDTimer = new NetworkLEDTimer(Clock::tics*5);
  myStatusLEDTimer = new StatusLEDTimer(Clock::tics * 33);
  myCDLEDTimer = new CDLEDTimer(Clock::seconds * 120);
  
  //ax_printf("Timers created for LEDS\n");
  while (true) {
    mySemaphore->wait();
    if (statusLedEvent) {
      myStatusLED.toggle();
      statusLedEvent = false;
    }
    if (cdLedEvent) {
      myCDLED.toggle();
      cdLedEvent = false;
      trace << "Found pack in retrx" << endl;
      cout << "Core " << ax_coreleft_total() << endl;
    }
    if (netLedEvent) {
      myNetworkLED.toggle();
      netLedEvent = false;
    }
  }

}



/****************** END OF FILE FrontPanel.cc ********************************/
