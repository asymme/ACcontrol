#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define ON_PIN 4
#define OFF_PIN 5

// SSID
const char* _ssid = "********";
// PASSWARD
const char* _password = "********";
// HOST
String _host = "http://localhost/accontrol";
// ディープスリープ時間(us)
unsigned long _deepSleepTime = 60 * 60e6;

// TOUT無効(電源電圧測定の為)
ADC_MODE(ADC_VCC);


// HTTP GET
String getRequest(String url) {
  HTTPClient http;
  http.begin(url);
  String payload = "";
  int httpCode = http.GET();
  if(httpCode > 0) {
    if(httpCode == HTTP_CODE_OK) {
      payload = http.getString();
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
  return payload;
}


// HTTP POST
void postRequest(String url, String param) {
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  int httpCode = http.POST(param);
  if(httpCode > 0) {
    if(httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
    }
  } else {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}


// ピンON
void pinON() {
  digitalWrite(ON_PIN, 1);
  delay(100);
  digitalWrite(ON_PIN, 0);
}
// ピンOFF
void pinOFF() {
  digitalWrite(OFF_PIN, 1);
  delay(100);
  digitalWrite(OFF_PIN, 0);
}


void setup() {
  Serial.begin(74880);
  pinMode(ON_PIN, OUTPUT);
  pinMode(OFF_PIN, OUTPUT);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(_ssid, _password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    // Connection failed
    WiFi.disconnect();
    pinON();
    ESP.deepSleep(_deepSleepTime, WAKE_RF_DEFAULT);
  }
    
  // スケジュール確認
  String state = getRequest(_host + "/schedule/?q=now");
  if(!state.equals("")) {
    // リレー切り替え
    if(state.equals("1")) {
      pinON();
    } else if(state.equals("0")) {
      pinOFF();
    }
    
    // 電源電圧
    float vcc = (float)ESP.getVcc() / 1000.0;
    String q_vcc = (!isnan(vcc)) ? (String)vcc : "";
    String param = "vcc=" + q_vcc + "&state=" + state;
    postRequest(_host + "/log/", param);
  }

  // 次回までのスリープ時間を取得
  String t = getRequest(_host + "/schedule/?q=next");
  if(!t.equals("")) {
    _deepSleepTime = t.toInt() * 1e6;
  }
  ESP.deepSleep(_deepSleepTime, WAKE_RF_DEFAULT);
}

void loop() {
  delay(500);
}
