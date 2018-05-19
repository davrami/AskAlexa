
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include "OLEDDisplayUi.h"
#include "images.h"

#include <PubSubClient.h>
#include <WiFi.h>
#include <BME280I2C.h>
BME280I2C bme;
#define I2C_SDA 5
#define I2C_SCL 4
#define GPIO14 14
#define GPIO13 13

#define GPIO2 2
#include "credentials.h"
int status = WL_IDLE_STATUS;


WiFiClient espClient;
PubSubClient client(espClient);

SSD1306  display(0x3c, I2C_SDA, I2C_SCL);
OLEDDisplayUi ui( &display );
const char* mqtt_request = "alx/value/request/a1";
const char* mqtt_response = "alx/value/response/a1";
char data[80];
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;
struct tm timeinfo;
float fTemp(NAN), fHum(NAN), fPres(NAN);
static int count = 0;
static String wifiStatus;
static String mqttStatus;
static String sensorsStatus;
static bool toggleGPIO14 = false;
static bool toggleGPIO13 = false;

static bool bPrevi = HIGH;


// utility function for digital clock display: prints leading 0
String twoDigits(int digits) {
  if (digits < 10) {
    String i = '0' + String(digits);
    return i;
  }
  else {
    return String(digits);
  }
}

void vLecturaBme280(float *pfTemp, float *pfHum, float *pfPres) {
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);
  bme.read(*pfPres, *pfTemp, *pfHum, tempUnit, presUnit);
}

void vLecturaStatus() {
  if (!client.connected()) {
    wifiStatus = "WIFI: ...";
  } else {
    wifiStatus = "WIFI: OK";
  }
  if (client.state() != 0) {
    mqttStatus = "MQTT: ...";
  } else {
    mqttStatus = "MQTT: OK";
  }
  if (!bme.begin()) {
    sensorsStatus = "SENSORS: ...";
  } else {
    sensorsStatus = "SENSORS: OK";
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Client", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      char ip[50];
      WiFi.localIP().toString().toCharArray(ip, 50);

      client.publish("alx/init/a1", ip);
      client.subscribe("alx/value/request/a1");
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  String szRx = "";

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    char receivedChar = (char)payload[i];
    szRx += receivedChar;
  }
  Serial.println(szRx);

  float fTemp(NAN), fHum(NAN), fPres(NAN);
  vLecturaBme280(&fTemp, &fHum, &fPres);

  if (szRx == "temp") {
    char temp[10];
    String(fTemp).toCharArray(temp, 10);
    client.publish("alx/value/response/a1", temp);
  };
  if (szRx == "hum") {
    char hum[10];
    String(fHum).toCharArray(hum, 10);
    client.publish("alx/value/response/a1", hum);
  };
  if (szRx == "pres") {
    char pres[10];
    String(fPres).toCharArray(pres, 10);
    client.publish("alx/value/esponse/a1", pres);
  };
  if (szRx == "L14" || szRx == "l14") {
    toggleGPIO14 = LOW;
    digitalWrite(GPIO14, toggleGPIO14);
  }
  if (szRx == "H14" || szRx == "h14") {
    toggleGPIO14 = HIGH;
    digitalWrite(GPIO14, toggleGPIO14);
  }
  if (szRx == "L13" || szRx == "l13") {
    toggleGPIO13 = LOW;
    digitalWrite(GPIO13, toggleGPIO13);
  }
  if (szRx == "H13" || szRx == "h13") {
    toggleGPIO13 = HIGH;
    digitalWrite(GPIO13, toggleGPIO13);
  }

  String value = "\"temperature\": " + String(fTemp) ;
  String value2 = ", \"humidity\": " + String(fHum) ;
  value = value + value2;
  String payload2 = "{ \"devices\": \"a1\",\"payload\": {" + value + "}}";
  payload2.toCharArray(data, (payload2.length() + 1));


  /*if(szRx == "2L") digitalWrite(ledPin,LOW);
    if(szRx == "2H") digitalWrite(ledPin,HIGH);
    if(szRx == "13L") digitalWrite(ledGreen,LOW);
    if(szRx == "13H") digitalWrite(ledGreen,HIGH);*/
}



void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(128, 50, "dev: a1");
  display->drawString(128, 0, String(WiFi.localIP().toString()));
}

void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->drawXbm(x + 34, y + 13, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(34 + x, 45 + y, WiFi.localIP().toString());
}

void drawFrameStatus(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {

  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  display->drawString(0 + x, 9 + y, wifiStatus );
  display->drawString(0 + x, 25 + y, mqttStatus );
  display->drawString(0 + x, 39 + y, sensorsStatus );
}

void drawFrameTest(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0 + x, 10 + y, asctime(&timeinfo));
}


void bmeFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  display->drawString(0 + x, 10 + y, "Temp: " + String(fTemp) + " C");
  display->drawString(0 + x, 26 + y, "Hum: " + String(fHum) + " % HR");
  display->drawString(0 + x, 40 + y, "Pres: " + String(fPres) + " Pa");
}

void vLecturaDigitalWrite() {
  //bool bAra = digitalRead(GPIO2);

  /*
    if(bAra != bPrevi){
      delay(200);
      Serial.println(digitalRead(GPIO2));

      if(bAra == LOW){
         Serial.println("Flanc de baixada detectat." + String(bAra));
               bPrevi = bAra;

      }
      if (bAra == HIGH){
        Serial.println("Flanc de subida detectat." + String(bAra));
              bPrevi = bAra;

      }
    }*/
}


// This array keeps function pointers to all frames
// frames are the single views that slide in
FrameCallback frames[] = {drawFrameStatus, drawFrameTest, drawFrame1, bmeFrame};

// how many frames are there?
int frameCount = 4;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = { msOverlay };
int overlaysCount = 1;

void setup() {
  Serial.begin(115200);
  pinMode(GPIO14, OUTPUT);
  pinMode(GPIO13, OUTPUT);
  digitalWrite(GPIO14, HIGH);
  digitalWrite(GPIO13, HIGH);

  //pinMode(GPIO2, INPUT);

  ui.setTargetFPS(30);
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);
  ui.setIndicatorDirection(LEFT_RIGHT);
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);
  // Add frames
  ui.setFrames(frames, frameCount);
  // Add overlays
  ui.setOverlays(overlays, overlaysCount);
  // Initialising the UI will init the display too.
  ui.init();
  display.flipScreenVertically();

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, password);
    delay(10000);
  }

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  Serial.println("Connected to wifi");
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  display.init();
  display.flipScreenVertically();

  Wire.begin(I2C_SDA, I2C_SCL);
  if (!bme.begin())
  {
    Serial.println("Could not find BME280 sensor!");
  }

}


void loop() {
  int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0) {
    count++;
    if (count % 200 == 0) {
      Serial.println(count);
      vLecturaBme280(&fTemp, &fHum, &fPres);
      getLocalTime(&timeinfo);
      vLecturaStatus();
    }
    vLecturaDigitalWrite();

    delay(remainingTimeBudget);
  }
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

}
