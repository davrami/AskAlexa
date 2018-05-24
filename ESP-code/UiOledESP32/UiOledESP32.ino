
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
#define pressure_offset 3.3
static float temperature_set = 38.0F;
static boolean heater = true;
static boolean heaterSTOP = false;

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
String      time_str, weather_text, weather_extra_text;
int         last_reading_hour, reading_hour, hr_cnt;

enum image_names { // enumerated table used to point to images
  rain_img, sunny_img, mostlysunny_img, cloudy_img, tstorms_img,
} image;

// Define and enumerated type and assign values to expected weather types.
// These values help to determine the average weather preceeding a 'no-change' forecast e.g. rain, rain then mostlysun = -1 (-1 + -1 + 1) resulting on balance = more rain
enum weather_type {unknown     =  4,
                   sunny       =  2,
                   mostlysunny =  1,
                   cloudy      =  0,
                   rain        = -1,
                   tstorms     = -2
                  };

enum weather_description {GoodClearWeather, BecomingClearer,
                          NoChange, ClearSpells, ClearingWithin12hrs, ClearingAndColder,
                          GettingWarmer, WarmerIn2daysRainLikely,
                          ExpectRain, WarmerRainWithin36hrs, RainIn18hrs, RainHighWindsClearAndCool,
                          GalesHeavyRainSnowInWinter
                         };

weather_type current_wx; // Enable the current wx to be recorded

const char* image_table[] PROGMEM = {rain_icon, sunny_icon, mostlysunny_icon, cloudy_icon, tstorms_icon}; // An array of image icons

// An array structure to record pressure, temperaturre, humidity and weather state
typedef struct {
  float pressure;            // air pressure at the designated hour
  float temperature;         // temperature at the designated hour
  float humidity;            // humidity at the designated hour
  weather_type wx_state_1hr; // weather state at 1-hour
  weather_type wx_state_3hr; // weather state at 3-hour point
} wx_record_type;

wx_record_type reading[24]; // An array covering 24-hours to enable P, T, % and Wx state to be recorded for every hour

int wx_average_1hr, wx_average_3hr; // Indicators of average weather

bool look_3hr = true;
bool look_1hr = false;


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

bool update_time() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return false;
  }
  char strftime_buf[64];
  strftime(strftime_buf, sizeof(strftime_buf), "%R:%S   %a %d-%m-%y", &timeinfo);
  time_str = strftime_buf;  // Now is this format HH:MM:SS Sat 05-07-17
  reading_hour = time_str.substring(0, 2).toInt();
  return true;
}

void update_time_and_data() {
  while (!update_time());
  if (reading_hour != last_reading_hour) { // If the hour has advanced, then shift readings left and record new values at array element [23]
    for (int i = 0; i < 23; i++) {
      reading[i].pressure     = reading[i + 1].pressure;
      reading[i].temperature  = reading[i + 1].temperature;
      reading[i].wx_state_1hr = reading[i + 1].wx_state_1hr;
      reading[i].wx_state_3hr = reading[i + 1].wx_state_3hr;
    }
    int readPressure = (fPres / 100.0F + pressure_offset) * 10; // Rounded result to 1-decimal place
    reading[23].pressure     = (float)readPressure / 10; // Update time=now with current value of pressure
    reading[23].wx_state_1hr = current_wx;
    reading[23].wx_state_3hr = current_wx;
    last_reading_hour        = reading_hour;
    hr_cnt++;
    wx_average_1hr = reading[22].wx_state_1hr + current_wx;           // Used to predict 1-hour forecast extra text
    wx_average_3hr = 0;
    for (int i = 23; i >= 21; i--) {                                  // Used to predict 3-hour forecast extra text
      wx_average_3hr = wx_average_3hr + (int)reading[i].wx_state_3hr; // On average the last 3-hours of weather is used for the 'no change' forecast - e.g. more of the same?
    }
  }
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
  if (szRx[0] == 's' || szRx[0] == 'S') {
    szRx.remove(0, 1);
    Serial.println(szRx);

    temperature_set = szRx.toFloat() ;
  }
  if (szRx == "L14" || szRx == "l14") {
    toggleGPIO14 = LOW;
    heaterSTOP = false;
    digitalWrite(GPIO14, toggleGPIO14);
  }
  if (szRx == "H14" || szRx == "h14") {
    toggleGPIO14 = HIGH;
    heaterSTOP = true;
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
  if (szRx == "Astatus") {
    char payloadData[100];
    String("{\"deviceStatus\": \"a1\", \"temp\": "+String(fTemp)+", \"hum\": "+String(fHum)+", \"setTemp\": "+temperature_set+", \"G14\": "+toggleGPIO14+" }").toCharArray(payloadData, 100 );
    client.publish("alx/value/response/a1", payloadData);
  }

  String value = "\"temperature\": " + String(fTemp) ;
  String value2 = ", \"humidity\": " + String(fHum) ;
  value = value + value2;
  String payload2 = "{ \"devices\": \"a1\",\"payload\": {" + value + "}}";
  payload2.toCharArray(data, (payload2.length() + 1));
}


// Convert forecast text to a corresponding image for display together with a record of the current weather
void ForecastToImgTxt(weather_description wx_text) {
  if      (wx_text == GoodClearWeather)           {
    image = sunny_img;
    current_wx = sunny;
    weather_text = "Good clear weather";
  }
  else if (wx_text == BecomingClearer)            {
    image = mostlysunny_img;
    current_wx = mostlysunny;
    weather_text = "Becoming clearer";
  }
  else if (wx_text == NoChange)                   {
    image = cloudy_img;
    current_wx = cloudy;
    weather_text = "No change, clearing";
  }
  else if (wx_text == ClearSpells)                {
    image = mostlysunny_img;
    current_wx = mostlysunny;
    weather_text = "Clear spells";
  }
  else if (wx_text == ClearingWithin12hrs)        {
    image = mostlysunny_img;
    current_wx = mostlysunny;
    weather_text = "Clearing within 12-hrs";
  }
  else if (wx_text == ClearingAndColder)          {
    image = mostlysunny_img;
    current_wx = mostlysunny;
    weather_text = "Clearing and colder";
  }
  else if (wx_text == GettingWarmer)              {
    image = mostlysunny_img;
    current_wx = mostlysunny;
    weather_text = "Getting warmer";
  }
  else if (wx_text == WarmerIn2daysRainLikely)    {
    image = rain_img;
    current_wx = rain;
    weather_text = "Warmer in 2-days, rain likely";
  }
  else if (wx_text == ExpectRain)                 {
    image = rain_img;
    current_wx = rain;
    weather_text = "Expect rain";
  }
  else if (wx_text == WarmerRainWithin36hrs)      {
    image = rain_img;
    current_wx = rain;
    weather_text = "Warmer, rain within 36-hrs";
  }
  else if (wx_text == RainIn18hrs)                {
    image = rain_img;
    current_wx = rain;
    weather_text = "Rain in 18-hrs";
  }
  else if (wx_text == RainHighWindsClearAndCool)  {
    image = rain_img;
    current_wx = rain;
    weather_text = "Rain, high winds, clear and cool";
  }
  else if (wx_text == GalesHeavyRainSnowInWinter) {
    image = tstorms_img;
    current_wx = tstorms;
    weather_text = "Heavy rain, in winter snow";
  }
}

// Convert pressure and trend to a weather description either for 1 or 3 hours with the boolean true/false switch
weather_description get_forecast_text(float pressure_now, float trend, bool range) {
  String trend_str = get_trend_text(trend);
  weather_description wx_text = NoChange; //As a default forecast
  image = cloudy_img; // Generally when there is 'no change' then cloudy is the conditions
  if (pressure_now >= 1022.68 )                                                          {
    wx_text = GoodClearWeather;
  }
  if (pressure_now >= 1022.7  && trend_str  == "Falling fast")                           {
    wx_text = WarmerRainWithin36hrs;
  }
  if (pressure_now >= 1013.2  && pressure_now <= 1022.68 &&
      (trend_str == "Steady" || trend_str == "Rising slow"))                              {
    wx_text = NoChange;
    (range ? wx_history_3hr() : wx_history_1hr());
  }
  if (pressure_now >= 1013.2 && pressure_now <= 1022.68 &&
      (trend_str == "Rising" || trend_str == "Rising fast"))                              {
    wx_text = GettingWarmer;
  }
  if (pressure_now >= 1013.2 && pressure_now <= 1022.68 && trend_str == "Rising slow")   {
    wx_text = BecomingClearer;
  }
  if (pressure_now >= 1013.2 && pressure_now <= 1022.68 &&
      (trend_str == "Falling slow" || trend_str == "Falling fast"))                       {
    wx_text = ExpectRain;
  }
  if (pressure_now >= 1013.2 && pressure_now <= 1022.68 && trend_str  == "Steady")       {
    wx_text = ClearSpells;
    (range ? wx_history_3hr() : wx_history_1hr());
  };
  if (pressure_now <= 1013.2 && (trend_str == "Falling slow" || trend_str == "Falling")) {
    wx_text = RainIn18hrs;
  }
  if (pressure_now <= 1013.2  &&  trend_str == "Falling fast")                           {
    wx_text = RainHighWindsClearAndCool;
  }
  if (pressure_now <= 1013.2  &&
      (trend_str == "Rising" || trend_str == "Rising slow" || trend_str == "Rising fast"))      {
    wx_text = ClearingWithin12hrs;
  }
  if (pressure_now <= 1009.14 && trend_str  == "Falling fast")                           {
    wx_text = GalesHeavyRainSnowInWinter;
  }
  if (pressure_now <= 1009.14 && trend_str  == "Rising fast")                            {
    wx_text = ClearingAndColder;
  }
  return wx_text;
}

// Convert pressure trend to text
String get_trend_text(float trend) {
  String trend_str = "Steady"; // Default weather state
  if (trend > 3.5)                          {
    trend_str = "Rising fast";
  }
  else if (trend >   1.5  && trend <= 3.5)  {
    trend_str = "Rising";
  }
  else if (trend >   0.25 && trend <= 1.5)  {
    trend_str = "Rising slow";
  }
  else if (trend >  -0.25 && trend <  0.25) {
    trend_str = "Steady";
  }
  else if (trend >= -1.5  && trend < -0.25) {
    trend_str = "Falling slow";
  }
  else if (trend >= -3.5  && trend < -1.5)  {
    trend_str = "Falling";
  }
  else if (trend <= -3.5)                   {
    trend_str = "Falling fast";
  }
  return trend_str;
}

// Convert 1-hr weather history to text
void wx_history_1hr() {
  if      (wx_average_1hr >  0) weather_extra_text = ", expect sun";
  else if (wx_average_1hr == 0) weather_extra_text = ", mainly cloudy";
  else if (wx_average_1hr <  0) weather_extra_text = ", expect rain";
  else weather_extra_text = "";
}

// Convert 3-hr weather history to text
void wx_history_3hr() {
  if      (wx_average_3hr >  0) weather_extra_text = ", expect sun";
  else if (wx_average_3hr == 0) weather_extra_text = ", mainly cloudy";
  else if (wx_average_3hr <  0) weather_extra_text = ", expect rain";
  else weather_extra_text = "";
}



void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  /*display->setTextAlignment(TEXT_ALIGN_RIGHT);
    display->setFont(ArialMT_Plain_10);
    display->drawString(128, 50, "dev: a1");
    display->drawString(128, 0, String(WiFi.localIP().toString()));*/
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0, 0, time_str.substring(0, 8)); //HH:MM:SS Sat 05-07-17
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(128, 0, time_str.substring(9));
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

/*
  void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->drawXbm(x + 34, y + 13, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(34 + x, 45 + y, WiFi.localIP().toString());
  }*/

// This frame draws a weather icon based on 3-hours of data for the prediction
void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  float trend  = reading[23].pressure - reading[20].pressure;                          // Trend over the last 3-hours
  ForecastToImgTxt(get_forecast_text(reading[23].pressure, trend, look_3hr));          // From forecast and trend determine what image to display
  display->drawXbm(x + 0, y + 15, icon_width, icon_height, image_table[image]);        // Display corresponding image
  display->drawStringMaxWidth(x + 45, y + 12, 90, String(reading[23].pressure, 1) + " hPA"); // Show current air pressure
  display->drawStringMaxWidth(x + 45, y + 25, 90, String(trend, 1) + " " + get_trend_text(trend)); // and pressure trend
}

void drawFrameStatus(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {

  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0 + x, 10 + y, wifiStatus );
  display->drawString(0 + x, 20 + y, mqttStatus );
  display->drawString(0 + x, 30 + y, sensorsStatus );
}


void drawFrameTest(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  char str[4];
  dtostrf(fTemp, 3, 2, str );
  display->drawString(0 + x, 10 + y, "Now: " + String(str) + "C");
  display->drawHorizontalLine(0 + x, 35 + y, 148 + y);
  dtostrf(temperature_set, 3, 2, str );
  display->drawString(0 + x, 38 + y, "Set: " + String(str) + "C");

  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(120 + x, 38 + y, (heater || heaterSTOP ? "OFF" : "ON"));

}

void bmeFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0 + x, 10 + y, "Temp: " + String(fTemp) + " C");
  display->drawString(0 + x, 20 + y, "Hum: " + String(fHum) + " % HR");
  int readPressure = (fPres / 100.0F + pressure_offset) * 10; // Rounded result to 1-decimal place

  display->drawString(0 + x, 30 + y, "Pres: " + String((float)readPressure / 10) + " Pa");
}

// This frame draws a weather icon based on 3-hours of data for the prediction
void drawFrame5(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  float trend  = reading[23].pressure - reading[20].pressure;                          // Trend over the last 3-hours
  ForecastToImgTxt(get_forecast_text(reading[23].pressure, trend, look_3hr));          // From forecast and trend determine what image to display
  display->drawXbm(x + 0, y + 15, icon_width, icon_height, image_table[image]);        // Display corresponding image
  display->drawStringMaxWidth(x + 45, y + 12, 90, String(reading[23].pressure, 1) + " hPA"); // Show current air pressure
  display->drawStringMaxWidth(x + 45, y + 25, 90, String(trend, 1) + " " + get_trend_text(trend)); // and pressure trend

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

void vLecturaTemp() {
    if (((float)temperature_set - 2.0F) > fTemp && !heaterSTOP ) {
      Serial.println("ON");
      toggleGPIO14 = LOW;
      digitalWrite(GPIO14, toggleGPIO14);
      heater = false; //inverse logic
    } else{
      toggleGPIO14 = HIGH;
      digitalWrite(GPIO14, toggleGPIO14);
      Serial.println("OFF");
      heater = true; //inverse logic
    }
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
  float p, t;
  Serial.begin(115200);
  pinMode(GPIO14, OUTPUT);
  pinMode(GPIO13, OUTPUT);
  digitalWrite(GPIO14, HIGH);
  digitalWrite(GPIO13, HIGH);

  //pinMode(GPIO2, INPUT);

  ui.setTargetFPS(20);
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
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  Serial.println("\r\nConnecting to: " + String(ssid));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("WiFi connected\r\nIP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

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
  last_reading_hour = reading_hour;
  wx_average_1hr = 0; // Until we get a better idea
  wx_average_3hr = 0; // Until we get a better idea
}


void loop() {
  int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0) {
    count++;
    if (count % 300 == 0) {
      Serial.println(count);
      vLecturaBme280(&fTemp, &fHum, &fPres);
      getLocalTime(&timeinfo);
      vLecturaStatus();
      update_time();
      update_time_and_data();
      Serial.println(fTemp);
      if (count == 400) {
        int readPressure = (fPres / 100.0F + pressure_offset) * 10; // Rounded result to 1-decimal place
        for (int i = 0; i <= 23; i++) { // At the start all array values are the same as a baseline
          reading[i].pressure     = (float)readPressure / 10;     // A rounded to 1-decimal place version of pressure
          reading[i].temperature  = fTemp; // Although not used, but avialable
          reading[i].humidity     = fHum;    // Although not used, but avialable
          reading[i].wx_state_1hr = unknown;               // To begin with
          reading[i].wx_state_3hr = unknown;              // To begin with
        }
      }
      /*for (int i = 0; i < 24; i++) {
        Serial.println(String(i) + " " + String(reading[i].pressure));
      }*/
      vLecturaTemp();
    }
    vLecturaDigitalWrite();


    delay(remainingTimeBudget);
  }
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

}
