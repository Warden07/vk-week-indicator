#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecureBearSSL.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <time.h>
#include <coredecls.h>
extern "C" {
#include <user_interface.h>
}
#include "AppConfig.h"
#include "Calendar.h"
#include "Trust.h"

Adafruit_ST7789 screen(TFT_CS,TFT_DC,TFT_RST);
WiFiManager wifi;
BearSSL::PublicKey signingKey(SIGNING_PUBLIC_KEY);
BearSSL::HashSHA256 signingHash;
BearSSL::SigningVerifier signingVerifier(&signingKey);
String apName, apPassword;
String updateMessage="UPDATE: WAIT WIFI";
bool portalUsed=false, updateTried=false, firstDraw=true;
bool lastConnected=false, lastPortal=false;
unsigned long bootAt=0, portalAt=0, retryAt=0, frameAt=0, updateAt=0;
volatile bool synchronized=false;
volatile unsigned long syncAt=0;
int lastSecond=-1;

void line(int y,const String& value,int size=2,uint16_t color=ST77XX_WHITE) {
  screen.setTextSize(size); screen.setTextColor(color); screen.setCursor(8,y); screen.print(value);
}
void portalScreen() {
  screen.fillScreen(ST77XX_BLACK);
  line(8,"WIFI SETUP",2,ST77XX_CYAN);
  line(43,"Connect phone to:",1);
  line(62,apName,2);
  line(94,"Setup password:",1);
  line(111,apPassword,2,ST77XX_GREEN);
  line(145,"Open browser:",1);
  line(162,"192.168.4.1",2);
  line(198,"Save YOUR hotspot name",1);
  line(212,"and password. Then enable",1);
  line(226,"hotspot on your phone.",1);
}
void openPortal() {
  if(wifi.getConfigPortalActive()) return;
  portalUsed=true; portalAt=millis();
  char value[13]; snprintf(value,sizeof(value),"Wc%08lX",os_random());
  apPassword=value;
  wifi.startConfigPortal(apName.c_str(),apPassword.c_str());
  Serial.printf("Setup AP: %s\nSetup password: %s\nOpen http://192.168.4.1\n",apName.c_str(),apPassword.c_str());
  firstDraw=true;
}
void drawClock() {
  time_t now=time(nullptr); tm local={};
  const bool valid=synchronized && now>1700000000;
  if(valid) localtime_r(&now,&local);
  if(!firstDraw && valid && local.tm_sec==lastSecond) return;
  lastSecond=valid?local.tm_sec:-1; firstDraw=false;
  screen.fillScreen(ST77XX_BLACK);
  line(6,WiFi.status()==WL_CONNECTED?"WIFI CONNECTED":"WIFI OFFLINE",1,ST77XX_CYAN);
  char buf[32];
  if(valid) strftime(buf,sizeof(buf),"%H:%M:%S",&local); else strcpy(buf,"--:--:--");
  line(25,buf,4);
  if(valid) strftime(buf,sizeof(buf),"%d.%m.%Y",&local); else strcpy(buf,"Waiting for NTP");
  line(67,buf,2);
  screen.drawFastHLine(8,92,224,ST77XX_BLUE);
  if(valid) {
    Study study=studyWeek(local.tm_year+1900,local.tm_mon+1,local.tm_mday);
    line(103,study.week<0?"HOLIDAY":"STUDY WEEK",2,ST77XX_CYAN);
    if(study.week>0) { line(128,String(study.week),6,ST77XX_GREEN); line(181,"Semester "+String(study.semester),1); }
  } else line(115,"SYNCING TIME",2,ST77XX_YELLOW);
  line(200,updateMessage,1);
  if(valid) {
    unsigned long age=(millis()-syncAt)/1000;
    line(214,"NTP age: "+String(age/60)+" min",1,age>86400?ST77XX_YELLOW:ST77XX_WHITE);
  }
  line(228,String("v")+FW_VERSION+"  USB: type wifi",1);
}
void checkUpdate() {
  updateTried=true; updateAt=millis(); updateMessage="CHECKING GITHUB";
  firstDraw=true; drawClock();
  BearSSL::X509List roots(ROOT_CA);
  BearSSL::WiFiClientSecure tls;
  tls.setTrustAnchors(&roots); tls.setTimeout(5000);

  HTTPClient http; http.setTimeout(5000);
  String base=UPDATE_BASE;
  if(!http.begin(tls,base+"build.txt")) { updateMessage="UPDATE: URL ERROR"; return; }
  int status=http.GET();
  if(status!=200) {
    updateMessage="UPDATE HTTP "+String(status); http.end(); return;
  }
  // A bounded, numeric build number. No arbitrary URLs from the network.
  const int length=http.getSize();
  if(length<1 || length>12) { updateMessage="BAD BUILD FILE"; http.end(); return; }
  char text[13]={};
  const size_t got=http.getStream().readBytes(text,length);
  http.end();
  if(got!=size_t(length)) { updateMessage="UPDATE READ ERROR"; return; }
  String clean=text; clean.trim();
  unsigned long remote=0;
  if(clean.length()<1 || clean.length()>9) { updateMessage="BAD BUILD FILE"; return; }
  for(unsigned i=0;i<clean.length();++i) {
    if(clean[i]<'0' || clean[i]>'9') { updateMessage="BAD BUILD FILE"; return; }
    remote=remote*10+(clean[i]-'0');
  }
  if(remote<=FW_BUILD) { updateMessage="UP TO DATE"; return; }
  if(ESP.getFreeSketchSpace()<262144) { updateMessage="UPDATE: NO SPACE"; return; }
  updateMessage="INSTALLING UPDATE"; firstDraw=true; drawClock();
  ESPhttpUpdate.rebootOnUpdate(false);
  ESPhttpUpdate.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
  const auto result=ESPhttpUpdate.update(tls,base+String(remote)+"/firmware.bin.signed");
  if(result==HTTP_UPDATE_OK) {
    updateMessage="RESTARTING"; firstDraw=true; drawClock(); delay(500); ESP.restart();
  } else {
    updateMessage="UPDATE FAILED";
    Serial.printf("Update failed: %d\n",ESPhttpUpdate.getLastError());
  }
}
void setup() {
  Serial.begin(115200); bootAt=millis();
  screen.init(240,240); screen.setRotation(TFT_ROTATION); screen.setTextWrap(false);
  drawClock();
  wifi.setDebugOutput(false); wifi.setConfigPortalBlocking(false);
  wifi.setConnectTimeout(12); wifi.setSaveConnectTimeout(12);
  wifi.setConfigPortalTimeout(300); wifi.setShowPassword(false);
  wifi.setBreakAfterConfig(true);
  const char* menu[]={"wifi","exit"};
  wifi.setMenu(menu,2);
  Update.installSignature(&signingHash,&signingVerifier);
  apName="WeekClock-"+String(ESP.getChipId(),HEX);
  WiFi.persistent(false); WiFi.mode(WIFI_STA); WiFi.setAutoReconnect(true);
  WiFi.begin();
  settimeofday_cb([](){ synchronized=true; syncAt=millis(); });
  configTime(TIME_ZONE,"pool.ntp.org","time.google.com","time.cloudflare.com");
  if(WiFi.SSID().isEmpty()) openPortal();
  Serial.println("Commands: wifi = setup; update = check GitHub");
}
void loop() {
  wifi.process();
  bool connected=WiFi.status()==WL_CONNECTED;
  bool portal=wifi.getConfigPortalActive();
  if(portal && (connected || millis()-portalAt>PORTAL_TIME_MS)) {
    wifi.stopConfigPortal(); portal=false; firstDraw=true;
  }
  if(!connected && !portal) {
    if(!portalUsed && millis()-bootAt>=CONNECT_WAIT_MS) openPortal();
    if(millis()-retryAt>=15000) { retryAt=millis(); WiFi.reconnect(); }
  }
  static String command;
  while(Serial.available()) {
    char c=Serial.read();
    if(c=='\n' || c=='\r') {
      command.trim();
      if(command=="wifi") openPortal();
      if(command=="update") updateTried=false;
      command="";
    } else if(command.length()<16) command+=c;
  }
  portal=wifi.getConfigPortalActive();
  if(connected!=lastConnected || portal!=lastPortal) firstDraw=true;
  if(connected && !lastConnected) updateTried=false;
  lastConnected=connected; lastPortal=portal;
  if(connected && synchronized && !portal && (!updateTried || millis()-updateAt>=UPDATE_INTERVAL_MS)) {
    checkUpdate(); firstDraw=true;
  }
  if(millis()-frameAt>=1000 || firstDraw) {
    frameAt=millis();
    if(portal) { if(firstDraw) { portalScreen(); firstDraw=false; } }
    else drawClock();
  }
  delay(5);
}
