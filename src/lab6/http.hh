/*!***************************************************************************
*!
*! FILE NAME  : tcpsocket.hh
*! 
*! DESCRIPTION: TCPSocket is the TCP protocols stacks interface to the
*!              application.
*! 
*!***************************************************************************/

#ifndef http_hh
#define http_hh

/****************** INCLUDE FILES SECTION ***********************************/

#include "job.hh"

/****************** CLASS DEFINITION SECTION ********************************/

/*****************************************************************************
*%
*% CLASS NAME   : TCPSocket
*%
*% BASE CLASSES : None
*%
*% CLASS TYPE   : Singleton
*%
*% DESCRIPTION  : Handles the interace beetwen TCP and the application.
*%
*% SUBCLASSING  : None.
*%
*%***************************************************************************/
class HTTPServer : public Job
{
 public:
  HTTPServer(TCPSocket* theSocket);
  // Constructor. The socket is created by class TCP when a connection is
  // established. create the semaphores
  ~HTTPServer();
  // Destructor. destroy the semaphores.

  //
  // Interface to application
  //

char* extractString(char* thePosition, udword theLength);

udword contentLength(char* theData, udword theLength);

char* decodeBase64(char* theEncodedString);

char* decodeForm(char* theEncodedForm);

char* findPathName(char* str);

void doit();

 private:
  TCPSocket* mySocket;
  const char* reply404;
  const char* statusReplyOk;
  const char* contentReplyText;
  const char* contentReplyJpeg;
  const char* contentReplyGif;
  const char* replyUnAut;
};



#endif
/****************** END OF FILE tcpsocket.hh *********************************/
