#include <WiFi.h>
#include <Wire.h>
#include "RTClib.h"
#include <ESPAsyncWebServer.h>
#include <Adafruit_NeoPixel.h>

// =============== ACCESS POINT SETTINGS ===============
const char* ap_ssid = "akvariumas";      // WiFi network name
const char* ap_password = "";             // No password (open network)
IPAddress local_IP(192,168,0,1);         // ESP32 IP address
IPAddress gateway(192,168,0,1);          // Gateway
IPAddress subnet(255,255,255,0);         // Subnet mask
// ====================================================

#define WS2812_PIN 23
#define WS2812_COUNT 50
#define SERIAL_TX 17
#define SERIAL_RX 16

Adafruit_NeoPixel strip = Adafruit_NeoPixel(WS2812_COUNT, WS2812_PIN, NEO_GRB + NEO_KHZ800);
RTC_DS3231 rtc;
AsyncWebServer server(80);

struct Settings {
  int r=0,g=0,b=0,w=0,sh=8,eh=20,tr=30;
  bool we=0;
  int wr=0,wg=0,wb=0,wsh=20,weh=23;
  int p1h=9,p1d=5,p1i=50,p2h=15,p2d=5,p2i=50;
} s;

unsigned long lp1=0,lp2=0;
bool t1=0,t2=0;

const char html[] PROGMEM = R"HTML(
<!DOCTYPE HTML>
<html>
   <head>
      <meta name="viewport" content="width=device-width,initial-scale=1">
      <meta charset="UTF-8">
      <title>Akvariumas</title>
      <style>*{margin:0;padding:0;box-sizing:border-box}body{font-family:Arial;background:#0f2027;color:#e0e0e0;padding:10px}h1{text-align:center;color:#4fc3f7;font-size:2rem;margin:10px 0}#t{text-align:center;font-size:2.5rem;color:#81d4fa;margin:20px 0;font-weight:bold}.s{background:rgba(255,255,255,0.05);border-radius:10px;padding:15px;margin:10px 0;border:1px solid rgba(79,195,247,0.2)}.s h2{color:#4fc3f7;font-size:1.2rem;margin-bottom:10px;border-bottom:1px solid rgba(79,195,247,0.3);padding-bottom:5px}.g{margin:10px 0}.g label{display:block;margin-bottom:5px;color:#b0bec5}input[type=range]{width:100%;height:6px;border-radius:5px;outline:none;-webkit-appearance:none;margin:5px 0}input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:20px;height:20px;border-radius:50%;background:#4fc3f7;cursor:pointer}input[type=number],input[type=text]{width:100%;padding:8px;background:rgba(255,255,255,0.1);border:1px solid rgba(79,195,247,0.3);border-radius:5px;color:#e0e0e0}.r{background:linear-gradient(90deg,#1a1a1a,#ff5252)}.gr{background:linear-gradient(90deg,#1a1a1a,#69f0ae)}.bl{background:linear-gradient(90deg,#1a1a1a,#448aff)}.wh{background:linear-gradient(90deg,#1a1a1a,#fff)}.df{background:linear-gradient(90deg,#37474f,#4fc3f7)}.c{display:flex;align-items:center;gap:10px}input[type=checkbox]{width:20px;height:20px}.g2{display:grid;grid-template-columns:1fr 1fr;gap:10px}.g3{display:grid;grid-template-columns:1fr 1fr 1fr;gap:10px}.v{color:#4fc3f7;font-weight:bold}.btn{width:100%;padding:10px;background:#4fc3f7;color:#000;border:none;border-radius:5px;font-size:1rem;cursor:pointer;font-weight:bold;margin-top:10px}.btn:active{background:#81d4fa}</style>
   </head>
   <body>
      <h1>🐠 Akvariumas</h1>
      <div id="t">00:00</div>
      <div class="s">
         <h2>🔆 LED</h2>
         <div class="g"><label>R: <span class="v" id="vR">0</span>%</label><input type="range" class="r" min="0" max="100" value="0" oninput="u('R',this.value)"></div>
         <div class="g"><label>G: <span class="v" id="vG">0</span>%</label><input type="range" class="gr" min="0" max="100" value="0" oninput="u('G',this.value)"></div>
         <div class="g"><label>B: <span class="v" id="vB">0</span>%</label><input type="range" class="bl" min="0" max="100" value="0" oninput="u('B',this.value)"></div>
         <div class="g"><label>W: <span class="v" id="vW">0</span>%</label><input type="range" class="wh" min="0" max="100" value="0" oninput="u('W',this.value)"></div>
         <div class="g2">
            <div class="g"><label>Start:</label><input type="number" min="0" max="23" value="8" id="sh" onchange="u('SH',this.value)"></div>
            <div class="g"><label>End:</label><input type="number" min="0" max="23" value="20" id="eh" onchange="u('EH',this.value)"></div>
         </div>
         <div class="g"><label>Transition: <span class="v" id="vT">30</span>m</label><input type="range" class="df" min="5" max="120" value="30" step="5" oninput="u('TR',this.value)"></div>
      </div>
      <div class="s">
         <h2>✨ WS2812</h2>
         <div class="c"><input type="checkbox" id="we" onchange="u('WE',this.checked?1:0)"><label>Enable</label></div>
         <div class="g"><label>R: <span class="v" id="vWR">0</span></label><input type="range" class="r" min="0" max="255" value="0" oninput="u('WR',this.value)"></div>
         <div class="g"><label>G: <span class="v" id="vWG">0</span></label><input type="range" class="gr" min="0" max="255" value="0" oninput="u('WG',this.value)"></div>
         <div class="g"><label>B: <span class="v" id="vWB">0</span></label><input type="range" class="bl" min="0" max="255" value="0" oninput="u('WB',this.value)"></div>
         <div class="g2">
            <div class="g"><label>On:</label><input type="number" min="0" max="23" value="20" id="wsh" onchange="u('WSH',this.value)"></div>
            <div class="g"><label>Off:</label><input type="number" min="0" max="23" value="23" id="weh" onchange="u('WEH',this.value)"></div>
         </div>
      </div>
      <div class="s">
         <h2>💧 Pump #1</h2>
         <div class="g2">
            <div class="g"><label>Hour:</label><input type="number" min="0" max="23" value="9" id="p1h" onchange="u('P1H',this.value)"></div>
            <div class="g"><label>Sec:</label><input type="number" min="1" max="60" value="5" id="p1d" onchange="u('P1D',this.value)"></div>
         </div>
         <div class="g"><label>Power: <span class="v" id="vP1">50</span>%</label><input type="range" class="df" min="0" max="100" value="50" oninput="u('P1I',this.value)"></div>
      </div>
      <div class="s">
         <h2>💧 Pump #2</h2>
         <div class="g2">
            <div class="g"><label>Hour:</label><input type="number" min="0" max="23" value="15" id="p2h" onchange="u('P2H',this.value)"></div>
            <div class="g"><label>Sec:</label><input type="number" min="1" max="60" value="5" id="p2d" onchange="u('P2D',this.value)"></div>
         </div>
         <div class="g"><label>Power: <span class="v" id="vP2">50</span>%</label><input type="range" class="df" min="0" max="100" value="50" oninput="u('P2I',this.value)"></div>
      </div>
      <div class="s">
         <h2>🕐 Set Time</h2>
         <div class="g3">
            <div class="g"><label>Hour:</label><input type="number" min="0" max="23" value="12" id="th"></div>
            <div class="g"><label>Minute:</label><input type="number" min="0" max="59" value="0" id="tm"></div>
            <div class="g"><label>Second:</label><input type="number" min="0" max="59" value="0" id="ts"></div>
         </div>
         <button class="btn" onclick="setTime()">Set Time</button>
      </div>
      <script>
      function u(p,v){fetch('/s?p='+p+'&v='+v);let d={'R':'vR','G':'vG','B':'vB','W':'vW','TR':'vT','WR':'vWR','WG':'vWG','WB':'vWB','P1I':'vP1','P2I':'vP2'};if(d[p])document.getElementById(d[p]).textContent=v}
      function load(){fetch('/g').then(r=>r.json()).then(d=>{document.querySelector('#vR').textContent=d.r;document.querySelector('input.r').value=d.r;document.querySelector('#vG').textContent=d.g;document.querySelectorAll('input.gr')[0].value=d.g;document.querySelector('#vB').textContent=d.b;document.querySelectorAll('input.bl')[0].value=d.b;document.querySelector('#vW').textContent=d.w;document.querySelector('input.wh').value=d.w;document.querySelector('#sh').value=d.sh;document.querySelector('#eh').value=d.eh;document.querySelector('#vT').textContent=d.tr;document.querySelectorAll('input.df')[0].value=d.tr;document.querySelector('#we').checked=d.we;document.querySelector('#vWR').textContent=d.wr;document.querySelectorAll('input.r')[1].value=d.wr;document.querySelector('#vWG').textContent=d.wg;document.querySelectorAll('input.gr')[1].value=d.wg;document.querySelector('#vWB').textContent=d.wb;document.querySelectorAll('input.bl')[1].value=d.wb;document.querySelector('#wsh').value=d.wsh;document.querySelector('#weh').value=d.weh;document.querySelector('#p1h').value=d.p1h;document.querySelector('#p1d').value=d.p1d;document.querySelector('#vP1').textContent=d.p1i;document.querySelectorAll('input.df')[1].value=d.p1i;document.querySelector('#p2h').value=d.p2h;document.querySelector('#p2d').value=d.p2d;document.querySelector('#vP2').textContent=d.p2i;document.querySelectorAll('input.df')[2].value=d.p2i})}
      function setTime(){let h=document.getElementById('th').value;let m=document.getElementById('tm').value;let s=document.getElementById('ts').value;fetch('/st?h='+h+'&m='+m+'&s='+s).then(()=>alert('Time set!'))}
      window.onload=load;
      setInterval(()=>{fetch('/t').then(r=>r.text()).then(t=>document.getElementById('t').textContent=t)},1000)
      </script>
   </body>
</html>
)HTML";

void sendToArduino() {
  DateTime now = rtc.now();
  int cm = now.hour() * 60 + now.minute();
  int sm = s.sh * 60;
  int em = s.eh * 60;
  int se = sm + s.tr;
  int ss = em - s.tr;
  
  int r=0,g=0,b=0,w=0;
  
  if(cm>=sm && cm<se) {
    float p = (float)(cm-sm)/s.tr;
    r=(s.r*255/100)*p;
    g=(s.g*255/100)*p;
    b=(s.b*255/100)*p;
    w=(s.w*255/100)*p;
  } 
  else if(cm>=se && cm<ss) {
    r=s.r*255/100;
    g=s.g*255/100;
    b=s.b*255/100;
    w=s.w*255/100;
  }
  else if(cm>=ss && cm<em) {
    float p=1.0-(float)(cm-ss)/s.tr;
    r=(s.r*255/100)*p;
    g=(s.g*255/100)*p;
    b=(s.b*255/100)*p;
    w=(s.w*255/100)*p;
  }
  
  int p1=0,p2=0;
  unsigned long m=millis();
  
  if(now.hour()==s.p1h && now.minute()==0) {
    if(!t1){t1=1;lp1=m;}
  }else{t1=0;}
  
  if(t1 && (m-lp1<s.p1d*1000UL)) p1=s.p1i*255/100;
  
  if(now.hour()==s.p2h && now.minute()==0) {
    if(!t2){t2=1;lp2=m;}
  }else{t2=0;}
  
  if(t2 && (m-lp2<s.p2d*1000UL)) p2=s.p2i*255/100;
  
  Serial2.printf("R:%d,G:%d,B:%d,W:%d,P1:%d,P2:%d\n",r,g,b,w,p1,p2);
  Serial.printf("->ARD R:%d G:%d B:%d W:%d P1:%d P2:%d\n",r,g,b,w,p1,p2);
}

void controlWS2812() {
  DateTime now = rtc.now();
  int h = now.hour();
  
  if(s.we && h>=s.wsh && h<s.weh) {
    strip.fill(strip.Color(s.wr,s.wg,s.wb));
  } else {
    strip.fill(strip.Color(0,0,0));
  }
  strip.show();
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, SERIAL_RX, SERIAL_TX);
  
  Wire.begin(21,22);
  if(!rtc.begin()) {
    Serial.println("RTC ERROR!");
    while(1);
  }
  
  strip.begin();
  strip.show();
  
  // Configure Access Point
  Serial.println("Starting Access Point...");
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ap_ssid, ap_password);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.println("\n=================================");
  Serial.println("ACCESS POINT STARTED");
  Serial.print("Network Name: ");
  Serial.println(ap_ssid);
  Serial.println("Password: (none - open network)");
  Serial.print("IP Address: ");
  Serial.println(IP);
  Serial.println("Connect to 'akvariumas' WiFi");
  Serial.println("Then open: http://192.168.4.1");
  Serial.println("=================================\n");
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send_P(200,"text/html",html);
  });
  
  server.on("/t", HTTP_GET, [](AsyncWebServerRequest *req) {
    DateTime now=rtc.now();
    char buf[6];
    sprintf(buf,"%02d:%02d",now.hour(),now.minute());
    req->send(200,"text/plain",buf);
  });
  
  server.on("/s", HTTP_GET, [](AsyncWebServerRequest *req) {
    if(req->hasParam("p") && req->hasParam("v")) {
      String p=req->getParam("p")->value();
      int v=req->getParam("v")->value().toInt();
      
      if(p=="R")s.r=v;
      else if(p=="G")s.g=v;
      else if(p=="B")s.b=v;
      else if(p=="W")s.w=v;
      else if(p=="SH")s.sh=v;
      else if(p=="EH")s.eh=v;
      else if(p=="TR")s.tr=v;
      else if(p=="WE")s.we=v;
      else if(p=="WR")s.wr=v;
      else if(p=="WG")s.wg=v;
      else if(p=="WB")s.wb=v;
      else if(p=="WSH")s.wsh=v;
      else if(p=="WEH")s.weh=v;
      else if(p=="P1H")s.p1h=v;
      else if(p=="P1D")s.p1d=v;
      else if(p=="P1I")s.p1i=v;
      else if(p=="P2H")s.p2h=v;
      else if(p=="P2D")s.p2d=v;
      else if(p=="P2I")s.p2i=v;
    }
    req->send(200,"text/plain","OK");
  });
  
  server.on("/g", HTTP_GET, [](AsyncWebServerRequest *req) {
    char json[256];
    sprintf(json,"{\"r\":%d,\"g\":%d,\"b\":%d,\"w\":%d,\"sh\":%d,\"eh\":%d,\"tr\":%d,\"we\":%d,\"wr\":%d,\"wg\":%d,\"wb\":%d,\"wsh\":%d,\"weh\":%d,\"p1h\":%d,\"p1d\":%d,\"p1i\":%d,\"p2h\":%d,\"p2d\":%d,\"p2i\":%d}",
      s.r,s.g,s.b,s.w,s.sh,s.eh,s.tr,s.we,s.wr,s.wg,s.wb,s.wsh,s.weh,s.p1h,s.p1d,s.p1i,s.p2h,s.p2d,s.p2i);
    req->send(200,"application/json",json);
  });
  
  server.on("/st", HTTP_GET, [](AsyncWebServerRequest *req) {
    if(req->hasParam("h") && req->hasParam("m") && req->hasParam("s")) {
      int h=req->getParam("h")->value().toInt();
      int m=req->getParam("m")->value().toInt();
      int sec=req->getParam("s")->value().toInt();
      rtc.adjust(DateTime(2024,1,1,h,m,sec));
      Serial.printf("Time set to: %02d:%02d:%02d\n",h,m,sec);
      req->send(200,"text/plain","OK");
    } else {
      req->send(400,"text/plain","ERROR");
    }
  });
  
  server.begin();
  Serial.println("Server started!");
}

void loop() {
  static unsigned long lu=0;
  
  if(millis()-lu>=1000) {
    lu=millis();
    sendToArduino();
    controlWS2812();
  }
  
  delay(10);
}