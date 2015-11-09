/*--------------------------------------------------
HTTP 1.1 Webserver as AccessPoint for ESP8266 
for ESP8266 adapted Arduino IDE

by Stefan Thesen 08/2015 - free for anyone

modifications by Gerhard Spitzlsperger Oct 2015
see licence file.

Does HTTP 1.1 with defined connection closing.
Handles empty requests in a defined manner.
Handle requests for non-exisiting pages correctly.

This demo is intended to switch a BlueberyE UnoSWU 
plug/socket using a ESP8266 based board. This 
script can serve as basis for own developments,
e. g. adding sensors etc.
--------------------------------------------------*/

#include <ESP8266WiFi.h>

const char* ssid = "UnoSWU";
const char* password = "1234";  // set to "" for open access point w/o passwortd

unsigned long ulReqcount;


// Create an instance of the server on Port 80
WiFiServer server(80);

void setup() 
{
  // setup globals
  ulReqcount=0; 
  
  // prepare GPIO2
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);
  
  // start serial
  Serial.begin(9600);
  delay(1);
  
  // AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  server.begin();
}

void loop() 
{ 
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) 
  {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  unsigned long ultimeout = millis()+250;
  while(!client.available() && (millis()<ultimeout) )
  {
    delay(1);
  }
  if(millis()>ultimeout) 
  { 
    Serial.println("client connection time-out!");
    return; 
  }
  
  // Read the first line of the request
  String sRequest = client.readStringUntil('\r');
  //Serial.println(sRequest);
  client.flush();
  
  // stop client, if request is empty
  if(sRequest=="")
  {
    Serial.println("empty request! - stopping client");
    client.stop();
    return;
  }
  
  // get path; end of path is either space or ?
  // Syntax is e.g. GET /?pin=MOTOR1STOP HTTP/1.1
  String sPath="",sParam="", sCmd="";
  String sGetstart="GET ";
  int iStart,iEndSpace,iEndQuest;
  iStart = sRequest.indexOf(sGetstart);
  if (iStart>=0)
  {
    iStart+=+sGetstart.length();
    iEndSpace = sRequest.indexOf(" ",iStart);
    iEndQuest = sRequest.indexOf("?",iStart);
    
    // are there parameters?
    if(iEndSpace>0)
    {
      if(iEndQuest>0)
      {
        // there are parameters
        sPath  = sRequest.substring(iStart,iEndQuest);
        sParam = sRequest.substring(iEndQuest,iEndSpace);
      }
      else
      {
        // NO parameters
        sPath  = sRequest.substring(iStart,iEndSpace);
      }
    }
  }
  
  ///////////////////////////////////////////////////////////////////////////////
  // output parameters to serial, you may connect e.g. an Arduino and react on it
  ///////////////////////////////////////////////////////////////////////////////
  if(sParam.length()>0)
  {
    int iEqu=sParam.indexOf("=");
    if(iEqu>=0)
    {
      sCmd = sParam.substring(iEqu+1,sParam.length());
      Serial.println(sCmd);
    }
  }
  
  
  ///////////////////////////
  // format the html response
  ///////////////////////////
  String sResponse,sHeader;
  
  ////////////////////////////
  // 404 for non-matching path
  ////////////////////////////
  if(sPath!="/")
  {
    sResponse="<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server.</p></body></html>";
    
    sHeader  = "HTTP/1.1 404 Not found\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }
  ///////////////////////
  // format the html page
  ///////////////////////
  else
  {
    //////////////////////
    // react on parameters
    //////////////////////
    if (sCmd.length()>0)
    {
      // write received command to html page
      //sResponse += "Kommando:" + sCmd + "<BR>";
      
      // switch GPIO
      if(sCmd.indexOf("FUNCTION1ON")>=0)
      {
        digitalWrite(2, 1);
      }
      else if(sCmd.indexOf("FUNCTION1OFF")>=0)
      {
        digitalWrite(2, 0);
      }
    }
    // read pin2 status
    int pin2high = digitalRead(2);
    
    ulReqcount++;
    sResponse  = "<html><head><title>Demo f&uumlr BlueberryE UnoSWU</title></head><body>";
    sResponse += "<font color=\"#000000\"><body>";
    sResponse += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">";
    sResponse += "<h1>BlueberryE UnoSWU</h1>";
    sResponse += "<p> Diese einfache Seite dient als Demo, um die Netzspannung, durch eine BlueberryE UnoSWU Steckdose mit integrierten USB ";
    sResponse += "Power Supply, mittels eines auf einem ESP8266 laufenden Webservers ein- und auszuschalten.";
    sResponse += "";
    sResponse += "<FONT SIZE=+1>";

    if (pin2high) { 
      sResponse += "<p>UnoSWU Netzspannung ist eingeschaltet.</p>";
      sResponse += "<p><a href=\"?pin=FUNCTION1OFF\"><button>ausschalten</button></a></p>";
    } else {
      sResponse += "<p>UnoSWU Netzspannung ist ausgeschaltet.</p>";
      sResponse += "<p><a href=\"?pin=FUNCTION1ON\"><button>einschalten</button></a>";
    }
      
    //sResponse += "<p>Funktion 2 <a href=\"?pin=FUNCTION2ON\"><button>einschalten</button></a>&nbsp;<a href=\"?pin=FUNCTION2OFF\"><button>ausschalten</button></a></p>";
    
    
    sResponse += "<FONT SIZE=-2>";
    sResponse += "<BR>Aufrufz&auml;hler="; 
    sResponse += ulReqcount;
    sResponse += "<BR>";
    sResponse += "Stefan Thesen 08/2015<BR>";
    sResponse += "</body></html>";
    
    sHeader  = "HTTP/1.1 200 OK\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }
  
  // Send the response to the client
  client.print(sHeader);
  client.print(sResponse);
  
  // and stop the client
  client.stop();
  Serial.println("Client disonnected");
}

