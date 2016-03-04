/*!***************************************************************************
*!
*! FILE NAME  : http.cc
*!
*! DESCRIPTION: HTTP, Hyper text transfer protocol.
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
#include "tcpsocket.hh"
#include "http.hh"
#include "fs.hh"
#include "tcp.hh"

//#define D_HTTP
#ifdef D_HTTP
#define trace cout
#else
#define trace if(false) cout
#endif

/****************** HTTPServer DEFINITION SECTION ***************************/

HTTPServer::HTTPServer(TCPSocket* theSocket):
mySocket(theSocket){
  reply404 = 
"HTTP/1.0 404 Not found\r\n "
"Content-type: text/html\r\n "
"\r\n "
"<html><head><title>File not found</title></head>"
"<body><h1>404 Not found</h1></body></html>";
  statusReplyOk = "HTTP/1.0 200 OK\r\n"; 
  contentReplyText = "Content-type: text/html\r\n \r\n";
  contentReplyJpeg = "Content-type: image/jpeg\r\n \r\n";
  contentReplyGif = "Content-type: image/gif\r\n \r\n";
}



HTTPServer::~HTTPServer() {
  delete reply404;
  delete statusReplyOk;
  delete contentReplyGif;
  delete contentReplyText;
  delete contentReplyJpeg;
}

void
HTTPServer::doit() 
{ 
  trace << "HTTPServer::doit port: " << mySocket->myConnection->hisPort << endl;
  udword aLength; 
  byte* aData;
    aData = mySocket->Read(aLength);
    cout << "Data dec" << endl;
    if (aLength > 0) 
    { 
      if (strncmp((char*)aData, "GET", 3) == 0) {
        char* filePath = findPathName((char*)aData);
        
        trace << "filePath: " << filePath << endl;
        
        byte* replyFile = 0;
        udword replyLength;
        if (filePath == NULL) {
          //root
          char* firstPos = strchr((char*)aData, ' ');   // First space on line 
          firstPos++;                            // Pointer to first / 
          char* lastPos = strchr(firstPos, ' '); // Last space on line 
          char* pathWithFile = extractString((char*)(firstPos+1), lastPos-firstPos); 
          char* fileName = (char*)(strrchr(pathWithFile ,'/') + 1);
          trace << "astf" << endl;
          if (*fileName == '\0') {
            trace << "oifsoijafe" << endl;
            char* indexFile = "index.htm";
            replyFile = FileSystem::instance().readFile(filePath, indexFile, replyLength);
            mySocket->Write((byte*)statusReplyOk, strlen(statusReplyOk));
            mySocket->Write((byte*)contentReplyText, strlen(contentReplyText));
            trace << "sending index to client" << endl;
            delete fileName;
          } 
          delete pathWithFile;
        } else {
          //parse filename/type
          char* firstPos = strchr((char*)aData, ' ');   // First space on line 
          firstPos++;                            // Pointer to first / 
          char* lastPos = strchr(firstPos, ' '); // Last space on line 
          char* pathWithFile = extractString((char*)(firstPos+1), lastPos-firstPos); 
          char* fileName = strrchr(pathWithFile, '/');
          fileName += 1; //skip '/'
          trace << "fileName: " << fileName << " port: " << mySocket->myConnection->hisPort << endl;
          char* fileType = strrchr(fileName, '.');
          fileType += 1; // skip '.'
          trace << "fileType: " << fileType << " port: "<< mySocket->myConnection->hisPort << endl;
          if (strncmp(fileType, "jpeg", 4) == 0) {
            mySocket->Write((byte*)statusReplyOk, strlen(statusReplyOk));
            mySocket->Write((byte*)contentReplyJpeg, strlen(contentReplyJpeg));
            trace << "found jpeg request" << " port: "<< mySocket->myConnection->hisPort << endl;
          } else if (strncmp(fileType, "gif", 3) == 0) {
            mySocket->Write((byte*)statusReplyOk, strlen(statusReplyOk));
            mySocket->Write((byte*)contentReplyGif, strlen(contentReplyGif));
            trace << "found gif request" << " port: "<< mySocket->myConnection->hisPort << endl;
          } else if (strncmp(fileType, "htm", 3) == 0) {
            mySocket->Write((byte*)statusReplyOk, strlen(statusReplyOk));
            mySocket->Write((byte*)contentReplyText, strlen(contentReplyText));
            trace << "found htm request" << " port: "<< mySocket->myConnection->hisPort << endl;
          }
          replyFile = FileSystem::instance().readFile(filePath, fileName, replyLength);
          delete pathWithFile;          
        }
        if(replyFile == 0){
          //404
          //trace << "404 len: " << strlen(reply404) << endl;
          mySocket->Write((byte*)reply404, strlen(reply404));
        } else {
          //trace << "Before replyFile != 0 port: "<< mySocket->myConnection->hisPort << endl;
          mySocket->Write(replyFile, replyLength);
          //trace << "After replyFile != 0 port: "<< mySocket->myConnection->hisPort << endl;
        }
      }
    } 
    delete aData;

    trace << "Closed Socket " << mySocket->myConnection->hisPort << endl;
    mySocket->Close();

    return;

  //never reached since no EOF is ever gotten except for when fin ack sent form server?
  //check while parameter
  //mySocket->Close();
 
}



//The method findPathName expects a string like GET /private/private.htm HTTP/1.0
//return the request filepathname
char* 
HTTPServer::findPathName(char* str) 
{ 
  char* firstPos = strchr(str, ' ');     // First space on line 
  firstPos++;                            // Pointer to first / 
  char* lastPos = strchr(firstPos, ' '); // Last space on line 
  char* thePath = 0;                     // Result path 
  if ((lastPos - firstPos) == 1) 
  { 
    // Is / only 
    thePath = 0;                         // Return NULL 
  } 
  else 
  { 
    // Is an absolute path. Skip first /. 
    thePath = extractString((char*)(firstPos+1), 
                            lastPos-firstPos); 
    if ((lastPos = strrchr(thePath, '/')) != 0) 
    { 
      // Found a path. Insert -1 as terminator. 
      *lastPos = '\xff'; 
      *(lastPos+1) = '\0'; 
      while ((firstPos = strchr(thePath, '/')) != 0) 
      { 
        // Insert -1 as separator. 
        *firstPos = '\xff'; 
      } 
    } 
    else 
    { 
      // Is /index.html 
      delete thePath; thePath = 0; // Return NULL 
    } 
  } 
  return thePath; 
}

//----------------------------------------------------------------------------
//
// Allocates a new null terminated string containing a copy of the data at
// 'thePosition', 'theLength' characters long. The string must be deleted by
// the caller.
//
char*
HTTPServer::extractString(char* thePosition, udword theLength)
{
  char* aString = new char[theLength + 1];
  strncpy(aString, thePosition, theLength);
  aString[theLength] = '\0';
  return aString;
}

//----------------------------------------------------------------------------
//
// Will look for the 'Content-Length' field in the request header and convert
// the length to a udword
// theData is a pointer to the request. theLength is the total length of the
// request.
//
udword
HTTPServer::contentLength(char* theData, udword theLength)
{
  udword index = 0;
  bool   lenFound = false;
  const char* aSearchString = "Content-Length: ";
  while ((index++ < theLength) && !lenFound)
  {
    lenFound = (strncmp(theData + index,
                        aSearchString,
                        strlen(aSearchString)) == 0);
  }
  if (!lenFound)
  {
    return 0;
  }
  trace << "Found Content-Length!" << endl;
  index += strlen(aSearchString) - 1;
  char* lenStart = theData + index;
  char* lenEnd = strchr(theData + index, '\r');
  char* lenString = this->extractString(lenStart, lenEnd - lenStart);
  udword contLen = atoi(lenString);
  trace << "lenString: " << lenString << " is len: " << contLen << endl;
  delete [] lenString;
  return contLen;
}

//----------------------------------------------------------------------------
//
// Decode user and password for basic authentication.
// returns a decoded string that must be deleted by the caller.
//
char*
HTTPServer::decodeBase64(char* theEncodedString)
{
  static const char* someValidCharacters =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
  
  int aCharsToDecode;
  int k = 0;
  char  aTmpStorage[4];
  int aValue;
  char* aResult = new char[80];

  // Original code by JH, found on the net years later (!).
  // Modify on your own risk. 
  
  for (unsigned int i = 0; i < strlen(theEncodedString); i += 4)
  {
    aValue = 0;
    aCharsToDecode = 3;
    if (theEncodedString[i+2] == '=')
    {
      aCharsToDecode = 1;
    }
    else if (theEncodedString[i+3] == '=')
    {
      aCharsToDecode = 2;
    }

    for (int j = 0; j <= aCharsToDecode; j++)
    {
      int aDecodedValue;
      aDecodedValue = strchr(someValidCharacters,theEncodedString[i+j])
        - someValidCharacters;
      aDecodedValue <<= ((3-j)*6);
      aValue += aDecodedValue;
    }
    for (int jj = 2; jj >= 0; jj--) 
    {
      aTmpStorage[jj] = aValue & 255;
      aValue >>= 8;
    }
    aResult[k++] = aTmpStorage[0];
    aResult[k++] = aTmpStorage[1];
    aResult[k++] = aTmpStorage[2];
  }
  aResult[k] = 0; // zero terminate string

  return aResult;  
}

//------------------------------------------------------------------------
//
// Decode the URL encoded data submitted in a POST.
//
char*
HTTPServer::decodeForm(char* theEncodedForm)
{
  char* anEncodedFile = strchr(theEncodedForm,'=');
  anEncodedFile++;
  char* aForm = new char[strlen(anEncodedFile) * 2]; 
  // Serious overkill, but what the heck, we've got plenty of memory here!
  udword aSourceIndex = 0;
  udword aDestIndex = 0;
  
  while (aSourceIndex < strlen(anEncodedFile))
  {
    char aChar = *(anEncodedFile + aSourceIndex++);
    switch (aChar)
    {
     case '&':
       *(aForm + aDestIndex++) = '\r';
       *(aForm + aDestIndex++) = '\n';
       break;
     case '+':
       *(aForm + aDestIndex++) = ' ';
       break;
     case '%':
       char aTemp[5];
       aTemp[0] = '0';
       aTemp[1] = 'x';
       aTemp[2] = *(anEncodedFile + aSourceIndex++);
       aTemp[3] = *(anEncodedFile + aSourceIndex++);
       aTemp[4] = '\0';
       udword anUdword;
       anUdword = strtoul((char*)&aTemp,0,0);
       *(aForm + aDestIndex++) = (char)anUdword;
       break;
     default:
       *(aForm + aDestIndex++) = aChar;
       break;
    }
  }
  *(aForm + aDestIndex++) = '\0';
  return aForm;
}

/************** END OF FILE http.cc *************************************/
