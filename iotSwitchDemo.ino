#include <iotfw.h>
#include "iotSwitchDemo.h"
#define DEBUG 1
iotfw iotfw;
#ifdef USESECURE
extern httpsserver::HTTPSServer secureServer;
extern httpsserver::HTTPServer server;
extern httpsserver::ResourceParameters *params;
extern httpsserver::HTTPRequest * __req;
extern httpsserver::HTTPResponse * __res;
#else
extern WebServer server;
#endif
extern struct tm * timeinfo;

int ledState;
const char demoRootHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html> <html>
<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">
<title>IOT Framework Switch Demo</title>
<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}
.button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}
.button-on {background-color: #1abc9c;}
.button-on:active {background-color: #16a085;}
.button-off {background-color: #34495e;}
.button-off:active {background-color: #2c3e50;}
p {font-size: 14px;color: #888;margin-bottom: 10px;}
</style>
</head>
<body>
<h1>IOT Switch Demo</h1>
<h3>%TIME% %TIMESTATUS%</h3>
<h3>Network State: %NETSTATE%</h3>
%LED%
<br /><a href="/net">localy defined network configuration</a>
<br /><a href="/configNetTZ">Network config from IOTFW</a>
%FOOTER_LINKS%
</body></html>
)rawliteral";
//String genHTML(void);

// handle root is just a core function
void handle_root() {
  //iotfw.send(genHTML());
  sendTok((char *) demoRootHTML);
}
#ifdef USESECURE
void handle_on_off(httpsserver::HTTPRequest * req, httpsserver::HTTPResponse * res) {
  params = req->getParams();
  __req=req;
  __res=res;
  Serial.print(" got "); Serial.print(params->getQueryParameterCount()); Serial.println("Args");
  std::string setValue;
  bool hasSet = params->getQueryParameter("set",setValue);
  if(hasSet) {
    //Serial.println("Has set parameter");
    if(setValue=="off") {ledState = LOW;}
    if(setValue=="on") {ledState = HIGH;}
  } else {
    //Serial.println("Does not have set parameter");
  }
  req->discardRequestBody();
#else
void handle_on_off() {
  Serial.print(" got "); Serial.print(server.args()); Serial.println("Args");
  for (uint8_t i = 0; i < server.args(); i++) {
    Serial.print("    Arg "); Serial.print(i); Serial.print(" : "); Serial.print(server.argName(i)); Serial.print(" = "); Serial.println(server.arg(i));
    if(server.argName(i).substring(0,3).equals("set") && server.arg(i).substring(0,3).equals("off")) {
        ledState = LOW;
    } else if(server.argName(i).substring(0,3).equals("set") && server.arg(i).substring(0,2).equals("on")) {
        ledState = HIGH;
    } else {
      Serial.println("Unknown command");
    }
    
  }
#endif
  digitalWrite(LEDPIN, ledState);
  handle_root();
}
String switchDemoTokHandler(const String& var) {
  String ret = "__UNSET__";   
  if (var=="LED") {
    if(ledState == HIGH)
      {ret="<p>LED: ON</p><a class=\"button button-off\" href=\"/on_off?set=off\">OFF</a>\n";}
    else
      {ret="<p>LED: OFF</p><a class=\"button button-on\" href=\"/on_off?set=on\">ON</a>\n";}
  }
  return(ret);
}
/*
String genHTML(){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>IOT Framework Switch Demo</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #1abc9c;}\n";
  ptr +=".button-on:active {background-color: #16a085;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>IOT Switch Demo</h1>\n";
  ptr +="<h3>";
  ptr +=asctime(timeinfo);
  ptr +=" ";
  ptr +=ascTimeStatus();
  ptr +="</h3>\n";
  ptr +="<h3>Network State: ";
  ptr +=ascNetState();
  ptr +="</h3>\n";
  
  if(ledState == HIGH)
    {ptr +="<p>LED: ON</p><a class=\"button button-off\" href=\"/on_off?set=off\">OFF</a>\n";}
  else
    {ptr +="<p>LED: OFF</p><a class=\"button button-on\" href=\"/on_off?set=on\">ON</a>\n";}
  ptr +="<br /><a href=\"/net\">localy defined network configuration</a>\n";
  ptr +="<br /><a href=\"/configNetTZ\">Network config from IOTFW</a>\n";
  ptr +=footer_links;
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;

}
*/
void setup() {
  // Start the Serial Monitor
  Serial.begin(115200);
  pinMode(LEDPIN, OUTPUT);
  iotfw.begin();
  iotfw.setRoot(handle_root);
  iotfw.setUsrTokHandler(switchDemoTokHandler);
#ifdef USESECURE
  httpsserver::ResourceNode * netRS    = new httpsserver::ResourceNode("/net", "GET", &handle_configNetTZ);
  httpsserver::ResourceNode * on_offRS = new httpsserver::ResourceNode("/on_off", "GET", &handle_on_off);
  server.registerNode(netRS);
  server.registerNode(on_offRS);
  secureServer.registerNode(on_offRS);
#else
  iotfw.on("/net",  handle_configNetTZ);
  iotfw.on("/on_off", handle_on_off);
#endif
}
void loop(){
  iotfw.wDelay(1);
}
