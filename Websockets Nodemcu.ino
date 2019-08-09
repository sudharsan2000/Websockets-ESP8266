#include<ESP8266WiFi.h>
#include<ESP8266WebServer.h>
#include<WebSocketsServer.h>

int state=0;
int ledpin = LED_BUILTIN;
ESP8266WebServer server;
WebSocketsServer webSocket = WebSocketsServer(81);
char *ssid = "Sudhar..!";
char *password = "ni6ga2rd";
char webpage[] PROGMEM = R"=====(
<html>
<head>
  <script>
    var Socket;
    function init()
    {
      Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
      Socket.onmessage = function(event)
      {
        document.getElementById("rxConsole").value += event.data;
      }
    }
    function sendText()
    {
      Socket.send(document.getElementById("txBar").value);
      document.getElementbyId("txBar").value="";
    }
    function sendBrightness()
    {
      Socket.send("*" + document.getElementById("brightness").value);
      document.getElementbyId("txBar").value="";
    }
  </script>
<body onload="javascript:init()">
<div>
    <textarea id = "rxConsole"> </textarea>
</div>
<hr/>
<div>
  <input type ="text" id="txBar" onkeydown="if(event.keyCode==13) sendText();"/>
</div>
<hr/>
<div>
  <input type ="range" min =0 max = 1023 value=512 id="brightness" oninput="sendBrightness();" />
</div>
</body>
</html>
)=====";
void setup()
{
  pinMode(ledpin,OUTPUT);
  WiFi.begin(ssid,password);
  Serial.begin(9600);
  while(WiFi.status() !=WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("IP address:");
  Serial.print(WiFi.localIP());

  server.on("/",[](){
    server.send_P(200,"text/html",webpage);
    });
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}
void webSocketEvent(uint8_t num,WStype_t type,uint8_t *payload,size_t length)
{
  if(payload[0] == '*')
  {
    uint16_t brightness = (uint16_t) strtol((const char*)&payload[1],NULL,10);
    analogWrite(ledpin,1024-brightness);
    Serial.println(brightness);
  }
  else if(type == WStype_TEXT)
  {
    for(int i=0;i<length;i++)
    {
      Serial.print((char)payload[i]);
    }
    Serial.println();
  }
  
}
void loop()
{
  server.handleClient();
  webSocket.loop();
  if(Serial.available()>0)
  {
    char c[] = {(char) Serial.read()};
    webSocket.broadcastTXT(c,sizeof(c));
    Serial.println("broadcasted");
  }
}
