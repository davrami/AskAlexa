
#include "SSD1306.h"       
#include "OLEDDisplayUi.h" 
#include <WiFi.h>
#include "time.h"
#include <Wire.h>

#include "credentials.h"
#include "images.h"

#include <PubSubClient.h>
#include <BME280I2C.h>
BME280I2C bme;
#define I2C_SDA 5
#define I2C_SCL 4

WiFiClient espClient;
PubSubClient client(espClient);

#define GPIO14 14
#define GPIO13 13

static bool toggleGPIO14 = false;
static bool toggleGPIO13 = false;
static String wifiStatus;
static String mqttStatus;
static String sensorsStatus;

float temp(NAN), hum(NAN), pres(NAN);
float fTemp(NAN), fHum(NAN), fPres(NAN);


const char* mqtt_request = "alx/value/request/a1";
const char* mqtt_response = "alx/value/response/a1";
char data[80];

  
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
static int count = 0;

SSD1306 display(0x3c, 5,4); // OLED display object definition (address, SDA, SCL)
OLEDDisplayUi ui     ( &display );

#define pressure_offset 3.3 // Used to adjust sensor reading to correct pressure for your location

/////////////////////////////////////////////////////////////////////////
// What's displayed along the top line
void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);

  display->drawString(0,0, time_str.substring(0,8));  //HH:MM:SS Sat 05-07-17
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(128,0, time_str.substring(9));
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

// This frame draws a weather icon based on 3-hours of data for the prediction
void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  float trend  = reading[23].pressure - reading[20].pressure;                          // Trend over the last 3-hours
  ForecastToImgTxt(get_forecast_text(reading[23].pressure, trend, look_3hr));          // From forecast and trend determine what image to display
  display->drawXbm(x+0,y+15, icon_width, icon_height, image_table[image]);             // Display corresponding image
  display->drawStringMaxWidth(x+45,y+12,90,String(reading[23].pressure,1)+" hPA");     // Show current air pressure 
  display->drawStringMaxWidth(x+45,y+25,90,String(trend,1)+" "+get_trend_text(trend)); // and pressure trend
}

// This frame shows a weather description based on 3-hours of data for the prediction
void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  float  trend = reading[23].pressure - reading[20].pressure;                             // Get current trend over last 3-hours
  weather_description wx_text = get_forecast_text(reading[23].pressure, trend, look_3hr); // Convert to forecast text based on 3-hours
  ForecastToImgTxt(wx_text);                                                              // Display corresponding text
  display->setFont(ArialMT_Plain_16);
  display->drawStringMaxWidth(x+0,y+10,127,weather_text);
  display->setFont(ArialMT_Plain_10);
}

// This frame draws a graph of pressure (delta) change for the last 24-hours, see Annex* for more details
void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  int gwidth   = 75; // Graph width in pixels
  int gscale   = 30; // Graph height in pixels
  int num_bars = 8;  // Number of bars to display
  #define yscale 8   // Graph +/- y-axis scale  e.g. 8 displays +/-8 and scales data accordingly
  float bar_width = gwidth / (num_bars+1); // Determine bar width based on graph width
  x = 30; // Sets position of graph on screen
  y = 15; // Sets position of graph on screen
  display->drawVerticalLine(x, y, gscale+1);
  display->drawString(x-26,y-6, String(reading[23].temperature + yscale));
  display->drawString(x-26,y+gscale/2-6, String(reading[23].temperature));
  display->drawString(x-26,y+gscale-6, String(reading[23].temperature - yscale));
  //display->drawString(x-30,y+gscale/2-6,String(hr_cnt%24));
  display->drawString(x+2+(bar_width+3)*0, y+gscale,"-24");  // 24hr marker at bar 0
  display->drawString(x+2+(bar_width+3)*2, y+gscale,"-12");  // 12hr marker at bar 2
  display->drawString(x+2+(bar_width+3)*5, y+gscale,"-2");   // 2hr  marker at bar 5
  display->drawString(x+2+(bar_width+3)*7, y+gscale,"0");    // 0hr  marker at bar 7
  int display_points [8] = {0,5,11,17,20,21,22,23}; // Only display time for hours 0,5,11,17,20,21,22,23
  float value;
  for (int bar_num = 0; bar_num < num_bars; bar_num++){      // Now display a bar at each hour position -24, -18, -12, -6, -3, -2, -1 and 0 hour
    value = map(reading[display_points[bar_num]].temperature, reading[23].temperature-yscale, reading[23].temperature+yscale, gscale, 0);
    if (value > gscale) value = gscale;                      // Screen scale is 0 to e.g. 40pixels, this stops drawing beyond graph bounds
    if (value < 0     ) value = 0;                           // 0 is top of graph, this stops drawing beyond graph bounds
    display->drawHorizontalLine(x+bar_num*(bar_width+3)+2, y+value, bar_width);
    for (int yplus=gscale; yplus > value; yplus = yplus - 1) {
      display->drawHorizontalLine(x+bar_num*(bar_width+3)+2, y + yplus, bar_width);
    }
  }
}

// This frame draws a weather icon based on 1-hour of data for the prediction
void drawFrame4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  reading[23].pressure = (reading[23].pressure + read_pressure())/2;                 // Update rolling average, gets reset on the hour transition
  float  trend = reading[23].pressure - reading[22].pressure;                        // Get short-term trend for the last 1-hour
  weather_description wx_text = get_forecast_text(read_pressure(), trend, look_1hr); // Convert to forecast text based on 1-hours
  ForecastToImgTxt(wx_text);
  display->drawXbm(x+0,y+15, icon_width, icon_height, image_table[image]);           // Display corresponding image
  display->drawStringMaxWidth(x+45,y+12,90,"1-Hr forecast");
  display->drawStringMaxWidth(x+45,y+22,90,String(read_pressure(),1)+" hPA");
  display->drawStringMaxWidth(x+47,y+32,90,String(trend,1)+" "+get_trend_text(trend));
}

// This frame shows a weather description based on 1-hour of data for the prediction
void drawFrame5(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  reading[23].pressure = (reading[23].pressure + read_pressure())/2;                 // Update rolling average
  float  trend = reading[23].pressure - reading[22].pressure;                        // Get short-term trend
  //weather_description wx_text = get_forecast_text(read_pressure(), trend, look_1hr); // Convert to forecast text based on 1-hours
  //ForecastToImgTxt(wx_text);
  display->drawString(x+0,y+10,WiFi.localIP().toString());
  display->setFont(ArialMT_Plain_16);
  display->drawStringMaxWidth(x+0,y+18,127,weather_text);
  display->setFont(ArialMT_Plain_10);
}

float read_pressure(){
   //Serial.println("read");  
  int reading = (pres/100.0F+pressure_offset)*10; // Rounded result to 1-decimal place
  return (float)reading/10;
}

// Convert pressure trend to text
String get_trend_text(float trend){
  String trend_str = "Steady"; // Default weather state
  if (trend > 3.5)                          { trend_str = "Rising fast";  }
  else if (trend >   1.5  && trend <= 3.5)  { trend_str = "Rising";       }
  else if (trend >   0.25 && trend <= 1.5)  { trend_str = "Rising slow";  }
  else if (trend >  -0.25 && trend <  0.25) { trend_str = "Steady";       }
  else if (trend >= -1.5  && trend < -0.25) { trend_str = "Falling slow"; }
  else if (trend >= -3.5  && trend < -1.5)  { trend_str = "Falling";      }
  else if (trend <= -3.5)                   { trend_str = "Falling fast"; }
  return trend_str;
}

// Convert forecast text to a corresponding image for display together with a record of the current weather
void ForecastToImgTxt(weather_description wx_text){
  if      (wx_text == GoodClearWeather)           {image = sunny_img;       current_wx = sunny;        weather_text = "Good clear weather";}
  else if (wx_text == BecomingClearer)            {image = mostlysunny_img; current_wx = mostlysunny;  weather_text = "Becoming clearer";}
  else if (wx_text == NoChange)                   {image = cloudy_img;      current_wx = cloudy;       weather_text = "No change, clearing";}
  else if (wx_text == ClearSpells)                {image = mostlysunny_img; current_wx = mostlysunny;  weather_text = "Clear spells";}
  else if (wx_text == ClearingWithin12hrs)        {image = mostlysunny_img; current_wx = mostlysunny;  weather_text = "Clearing within 12-hrs";}
  else if (wx_text == ClearingAndColder)          {image = mostlysunny_img; current_wx = mostlysunny;  weather_text = "Clearing and colder";}
  else if (wx_text == GettingWarmer)              {image = mostlysunny_img; current_wx = mostlysunny;  weather_text = "Getting warmer";}
  else if (wx_text == WarmerIn2daysRainLikely)    {image = rain_img;        current_wx = rain;         weather_text = "Warmer in 2-days, rain likely";}
  else if (wx_text == ExpectRain)                 {image = rain_img;        current_wx = rain;         weather_text = "Expect rain";}
  else if (wx_text == WarmerRainWithin36hrs)      {image = rain_img;        current_wx = rain;         weather_text = "Warmer, rain within 36-hrs";}
  else if (wx_text == RainIn18hrs)                {image = rain_img;        current_wx = rain;         weather_text = "Rain in 18-hrs";}
  else if (wx_text == RainHighWindsClearAndCool)  {image = rain_img;        current_wx = rain;         weather_text = "Rain, high winds, clear and cool";}
  else if (wx_text == GalesHeavyRainSnowInWinter) {image = tstorms_img;     current_wx = tstorms;      weather_text = "Gales, heavy rain, in winter snow";}
}

// Convert pressure and trend to a weather description either for 1 or 3 hours with the boolean true/false switch
weather_description get_forecast_text(float pressure_now, float trend, bool range) {
  String trend_str = get_trend_text(trend);
  weather_description wx_text = NoChange; //As a default forecast 
  image = cloudy_img; // Generally when there is 'no change' then cloudy is the conditions
  if (pressure_now >= 1022.68 )                                                          {wx_text = GoodClearWeather;}
  if (pressure_now >= 1022.7  && trend_str  == "Falling fast")                           {wx_text = WarmerRainWithin36hrs;}
  if (pressure_now >= 1013.2  && pressure_now <= 1022.68 && 
     (trend_str == "Steady" || trend_str == "Rising slow"))                              {wx_text = NoChange; (range?wx_history_3hr():wx_history_1hr()); }
  if (pressure_now >= 1013.2 && pressure_now <= 1022.68 &&
     (trend_str == "Rising" || trend_str == "Rising fast"))                              {wx_text = GettingWarmer;}
  if (pressure_now >= 1013.2 && pressure_now <= 1022.68 && trend_str == "Rising slow")   {wx_text = BecomingClearer;}
  if (pressure_now >= 1013.2 && pressure_now <= 1022.68 && 
     (trend_str == "Falling slow" || trend_str == "Falling fast"))                       {wx_text = ExpectRain;}
  if (pressure_now >= 1013.2 && pressure_now <= 1022.68 && trend_str  == "Steady")       {wx_text = ClearSpells; (range?wx_history_3hr():wx_history_1hr());};
  if (pressure_now <= 1013.2 && (trend_str == "Falling slow" || trend_str == "Falling")) {wx_text = RainIn18hrs;}
  if (pressure_now <= 1013.2  &&  trend_str == "Falling fast")                           {wx_text = RainHighWindsClearAndCool;}
  if (pressure_now <= 1013.2  && 
     (trend_str == "Rising" || trend_str=="Rising slow"||trend_str=="Rising fast"))      {wx_text = ClearingWithin12hrs;}
  if (pressure_now <= 1009.14 && trend_str  == "Falling fast")                           {wx_text = GalesHeavyRainSnowInWinter;}
  if (pressure_now <= 1009.14 && trend_str  == "Rising fast")                            {wx_text = ClearingAndColder;}
  return wx_text;
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

///////////////////////////////////////////////////////////////////////////////////////////////////////

// This array keeps function pointers to all frames
// frames are the single views that slide in
FrameCallback frames[] = { drawFrame1, drawFrame2, drawFrame3, drawFrame4, drawFrame5};

// how many frames are there?
int frameCount = 5;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = { msOverlay };
int overlaysCount = 1;





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
}

void vLecturaBme280(float *pfTemp, float *pfHum, float *pfPres) {
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);
  bme.read(*pfPres, *pfTemp, *pfHum, tempUnit, presUnit);
  
}



void setup() { 
  float p,t;
  Serial.begin(115200);
  pinMode(GPIO14, OUTPUT);
  pinMode(GPIO13, OUTPUT);
  digitalWrite(GPIO14, HIGH);
  digitalWrite(GPIO13, HIGH);
 
  if (!StartWiFi(ssid,password)) Serial.println("Failed to start WiFi Service after 20 attempts");;
  configTime(1, 3600, "pool.ntp.org");
  while(!Serial) {} // Wait

  
  Wire.begin(I2C_SDA, I2C_SCL);
  while(!bme.begin())
  {
    Serial.println("Could not find BME280 sensor!");
  }
  switch(bme.chipModel())
    {
       case BME280::ChipModel_BME280:
         Serial.println("Found BME280 sensor! Success.");
         break;
       case BME280::ChipModel_BMP280:
         Serial.println("Found BMP280 sensor! No Humidity available.");
         break;
       default:
         Serial.println("Found UNKNOWN sensor! Error!");
    }
  
  Serial.println("Found a sensor continuing");
  if (!StartMqtt()) Serial.println("Failed to start MQTT Service after 20 attempts");;
 
  last_reading_hour = reading_hour;
  wx_average_1hr = 0; // Until we get a better idea
  wx_average_3hr = 0; // Until we get a better idea
  
  // An ESP is capable of rendering 60fps in 80Mhz mode but leaves little time for anything else, run at 160Mhz mode or just set it to about 30 fps
  ui.setTargetFPS(20);
  ui.setIndicatorPosition(BOTTOM);         // You can change this to TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorDirection(LEFT_RIGHT);    // Defines where the first frame is located in the bar
  ui.setFrameAnimation(SLIDE_LEFT);        // You can change the transition that is used SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrames(frames, frameCount);        // Add frames
  ui.setOverlays(overlays, overlaysCount); // Add overlays
  ui.init(); // Initialising the UI will init the display too.
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
}

void loop() {

  int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0) { // Do some work here if required
    count++;
    if (count % 200 == 0) {
      if(count == 800){
         for (int i = 0; i <= 23; i++){ // At the start all array values are the same as a baseline 
          /*reading[i].pressure     = read_pressure();       // A rounded to 1-decimal place version of pressure
          reading[i].temperature  = pres; // Although not used, but avialable
          reading[i].humidity     = temp;    // Although not used, but avialable
          reading[i].wx_state_1hr = unknown;               // To begin with  
          reading[i].wx_state_3hr = unknown;  */             // To begin with 
        }
      }
        update_time_and_data();
        //vLecturaBme280();
        //printBME280Data(&Serial);
        vLecturaBme280(&fTemp, &fHum, &fPres);
        Serial.println(fTemp);

        struct tm timeinfo;
          if(!getLocalTime(&timeinfo)){
            Serial.println("Failed to obtain time");
          }
                
        Serial.println(&timeinfo, "%A, %d %B %y %H:%M:%S"); // Displays: Saturday, 24 June 17 14:05:49

        for (int i = 0; i < 24;i++){
          //Serial.println(String(i)+" "+String(reading[i].pressure));
        }
    }

    
    delay(remainingTimeBudget);
  }
  
   if (!client.connected()) {
      StartMqtt();
    }
  client.loop();
}

void update_time_and_data(){
  while (!update_time());
  if (reading_hour != last_reading_hour) { // If the hour has advanced, then shift readings left and record new values at array element [23]
    for (int i = 0; i < 23;i++){
      reading[i].pressure     = reading[i+1].pressure;
      reading[i].temperature  = reading[i+1].temperature;
      reading[i].wx_state_1hr = reading[i+1].wx_state_1hr;
      reading[i].wx_state_3hr = reading[i+1].wx_state_3hr;
    }
    //reading[23].pressure     = read_pressure(); // Update time=now with current value of pressure
    reading[23].wx_state_1hr = current_wx;
    reading[23].wx_state_3hr = current_wx;
    last_reading_hour        = reading_hour;
    hr_cnt++;
    wx_average_1hr = reading[22].wx_state_1hr + current_wx;           // Used to predict 1-hour forecast extra text
    wx_average_3hr = 0;
    for (int i=23;i >= 21; i--){                                      // Used to predict 3-hour forecast extra text 
      wx_average_3hr = wx_average_3hr + (int)reading[i].wx_state_3hr; // On average the last 3-hours of weather is used for the 'no change' forecast - e.g. more of the same?
    }
  }  
}

bool update_time(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return false;
  }
  //See http://www.cplusplus.com/reference/ctime/strftime/
  //Serial.println(&timeinfo, "%A, %d %B %y %H:%M:%S"); // Displays: Saturday, 24 June 17 14:05:49
  char strftime_buf[64];
  strftime(strftime_buf, sizeof(strftime_buf), "%R:%S   %a %d-%m-%y", &timeinfo);
  time_str = strftime_buf;  // Now is this format HH:MM:SS Sat 05-07-17
  reading_hour = time_str.substring(0,2).toInt();
  return true;
}

int StartWiFi(const char* ssid, const char* password){
  int connAttempts = 0;
  Serial.println("\r\nConnecting to: "+String(ssid));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(".");
    if(connAttempts > 20) return false;
    connAttempts++;
  }
  Serial.print("WiFi connected\r\nIP address: ");
  Serial.println(WiFi.localIP());
  return true;
}

int StartMqtt(){
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  int connAttempts = 0;
  Serial.println("\r\nConnecting MQTT: " + String(mqtt_server));

  while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
      if (client.connect("Client", mqtt_username, mqtt_password)) {
        char ip[50];
        WiFi.localIP().toString().toCharArray(ip, 50);
        client.publish("alx/init/a1", ip);
        client.subscribe("alx/value/request/a1");  
      } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(8000);
      }
      delay(500);
      Serial.print(".");
      if(connAttempts > 20) return false;
      connAttempts++;
    }
  Serial.println("Mqtt connected");
  return true;
}


void printBME280Data
(
   Stream* client
)
{
   BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
   BME280::PresUnit presUnit(BME280::PresUnit_Pa);
   bme.read(pres, temp, hum, tempUnit, presUnit);

   client->print("Temp: ");
   client->print(temp);
   client->print("°"+ String(tempUnit == BME280::TempUnit_Celsius ? 'C' :'F'));
   client->print("\t\tHumidity: ");
   client->print(hum);
   client->print("% RH");
   client->print("\t\tPressure: ");
   client->print(pres);
   client->println(" Pa");
}

/*
FRAME-3 description
// This frame draws a graph of pressure (delata) change for the last 24-hours, see Annex* for more details
// Draws a 'relative value' chart using reading[23] as the baseline
// +8 |
// +7 |--  
//    : 
// +1 |--  --  --  --  --  --  --   
//  0 +-24+-18+-12+-8-+-3-+-2-+-1-+-0-+
// -1 |  
// -2 |
//    The 'reading' array holds values for Pressure, Temperature, Humidity and Wx State for the last 24-hours
//    [00][01][02][03][04][05][06][07][08][09][10][11][12][13][14][15][16][17][18][19][20][21][22][23] Values are shifted left <-- each hour
//     ^-23Hr              ^-18Hr                  ^-12Hr                  ^-6Hr       ^-3 ^-2 ^-1 ^0Hr
//     P  ~ readings in each array position
//     T  ~ readings in each array position
//     %  ~ readings in each array position
//     Wx ~ readings in each array position

// Forecast basics:
// Look at the pressure change over3 hours
// If pressure is descending, then a low pressure area is approaching 
// If pressure is ascending , then a low is passing or a high pressure is coming
// When pressure is changing rapidly (>6hPa/3 hours), it will be windy (or potentially windy) 

// More detailed:
// Pressure falling slowly (0.5 - 3 hPa in 3h): low is weak, dying or moving slowly. You might get some rain but typically no high winds.
// Pressure falling moderately (3-6 hPa/3h): rapid movement or deepening low. Moderate winds and rain and a warm front.
                                           : the low is passing fast, the day after tomorrow will typically be fine. 
// Pressure falling fast (6-12 hPa/3h) : Storm conditions highly likely.
// Pressure rises are connected with gradually drier weather

 */
