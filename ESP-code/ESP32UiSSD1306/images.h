#define icon_width  40
#define icon_height 40


// Define each of the *icons for display
const char rain_icon[] PROGMEM = {
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0x81, 0xFF, 0xFF, 0xFF, 0x3F, 0x04, 0xFE, 0xFF, 0xFF, 0xDF, 0xF0, 0xFC, 
  0xFF, 0xFF, 0xE7, 0xFF, 0xFB, 0xFF, 0xFF, 0xFB, 0xFF, 0xF3, 0xFF, 0xFF, 
  0xFD, 0xFF, 0xE7, 0xFF, 0xFF, 0xFD, 0xFF, 0x0F, 0xFE, 0x0F, 0xF8, 0xFF, 
  0x8F, 0xFC, 0xE7, 0xFB, 0xFF, 0xC7, 0xF9, 0xF7, 0xE3, 0xFF, 0xC3, 0xF3, 
  0xF3, 0xDF, 0xFF, 0xE8, 0xE7, 0xF9, 0xFF, 0xFF, 0xFE, 0xEF, 0xFD, 0xFF, 
  0xFF, 0xFF, 0xE7, 0xF9, 0xFF, 0xFF, 0xFF, 0xF7, 0xFB, 0xFF, 0xFF, 0xFF, 
  0xF3, 0xFB, 0xFF, 0xFF, 0xFF, 0xF9, 0xF7, 0xFF, 0xFF, 0xFF, 0xFC, 0xCF, 
  0xFF, 0xFF, 0x3F, 0xFE, 0x3F, 0x00, 0x00, 0x80, 0xFF, 0xFF, 0x7D, 0xBF, 
  0xEF, 0xFF, 0xFF, 0xBE, 0xDF, 0xF7, 0xFF, 0x7F, 0xDF, 0xEF, 0xFB, 0xFF, 
  0xBF, 0xEF, 0xF7, 0xFD, 0xFF, 0xDF, 0xF7, 0xFB, 0xFE, 0xFF, 0xEF, 0xFB, 
  0x7D, 0xFF, 0xFF, 0xF7, 0xFD, 0xBE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

const char sunny_icon[] PROGMEM = {
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE3, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xE3, 0xFF, 0xFF, 0xFF, 0xFF, 0xE3, 0xFF, 0xFF, 0xFF, 0xFF, 0xE3, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xE3, 0xFF, 0xFD, 0xDF, 0xFF, 0xE3, 0xFF, 0xF8, 0x8F, 
  0xFF, 0xE3, 0x7F, 0xF0, 0x07, 0xFF, 0xE3, 0x3F, 0xF8, 0x0F, 0xFE, 0xFF, 
  0x1F, 0xFC, 0x1F, 0x7C, 0x00, 0x0E, 0xFE, 0x3F, 0x18, 0x00, 0x1C, 0xFF, 
  0x7F, 0xCC, 0xFF, 0xB1, 0xFF, 0xFF, 0xE6, 0xFF, 0xE7, 0xFF, 0xFF, 0xF3, 
  0xFF, 0xCF, 0xFF, 0xFF, 0xF1, 0xFF, 0x9F, 0xFF, 0xFF, 0xF9, 0xFF, 0x9F, 
  0xFF, 0xFF, 0xF9, 0xFF, 0x9F, 0xFF, 0xFF, 0xF9, 0xFF, 0x9F, 0xFF, 0x01, 
  0xF9, 0xFF, 0x9F, 0x80, 0x01, 0xF9, 0xFF, 0x9F, 0x80, 0x01, 0xF9, 0xFF, 
  0x9F, 0x80, 0xFF, 0xF9, 0xFF, 0x9F, 0xFF, 0xFF, 0xF1, 0xFF, 0x8F, 0xFF, 
  0xFF, 0xF1, 0xFF, 0x8F, 0xFF, 0xFF, 0xE3, 0xFF, 0xE7, 0xFF, 0xFF, 0xC7, 
  0xFF, 0xF3, 0xFF, 0xFF, 0x8D, 0xFF, 0xD8, 0xFF, 0xFF, 0x38, 0x00, 0x8C, 
  0xFF, 0x7F, 0x70, 0x00, 0x07, 0xFF, 0x3F, 0xF8, 0xFF, 0x0F, 0xFE, 0x1F, 
  0xFC, 0xE3, 0x1F, 0xFC, 0x0F, 0xFE, 0xE3, 0x3F, 0xF8, 0x07, 0xFF, 0xE3, 
  0x7F, 0xF0, 0x8F, 0xFF, 0xE3, 0xFF, 0xF8, 0xDF, 0xFF, 0xE3, 0xFF, 0xFD, 
  0xFF, 0xFF, 0xE3, 0xFF, 0xFF, 0xFF, 0xFF, 0xE3, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xE3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

const char mostlysunny_icon[] PROGMEM = {
  0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE, 0xFF, 0xFF, 0xFD, 0x7E, 
  0xFF, 0xFF, 0xFF, 0xFB, 0xBF, 0xEF, 0xFF, 0xFF, 0x17, 0xE0, 0xF7, 0xFF, 
  0xFF, 0xCF, 0x9F, 0xF9, 0xFF, 0xFF, 0xE6, 0x3F, 0xFD, 0xFF, 0xFF, 0xF5, 
  0x7F, 0xFF, 0xFF, 0xFF, 0xFB, 0xFF, 0xFE, 0xFF, 0xFF, 0xF9, 0xFF, 0x00, 
  0xFF, 0xFF, 0xFD, 0x7F, 0x08, 0xFC, 0xFF, 0xFD, 0xBF, 0xE1, 0xF9, 0xFF, 
  0xFD, 0xCF, 0xFF, 0xF7, 0xFF, 0xFD, 0xF7, 0xFF, 0xE7, 0xFF, 0xF9, 0xFB, 
  0xFF, 0xCF, 0xFF, 0xF3, 0xFB, 0xFF, 0x1F, 0xFC, 0x17, 0xF0, 0xFF, 0x1F, 
  0xF9, 0xC7, 0xF7, 0xFF, 0x8F, 0xF3, 0xEF, 0xC7, 0xFF, 0x87, 0xE7, 0xE7, 
  0xBF, 0xFF, 0xD1, 0xCF, 0xF3, 0xFF, 0xFF, 0xFD, 0xDF, 0xFB, 0xFF, 0xFF, 
  0xFF, 0xCF, 0xF3, 0xFF, 0xFF, 0xFF, 0xEF, 0xF7, 0xFF, 0xFF, 0xFF, 0xE7, 
  0xF7, 0xFF, 0xFF, 0xFF, 0xF3, 0xEF, 0xFF, 0xFF, 0xFF, 0xF9, 0x9F, 0xFF, 
  0xFF, 0x7F, 0xFC, 0x7F, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  
const char cloudy_icon[] PROGMEM = {
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0xFF, 0xFF, 0xFF, 0x7F, 0x78, 0xFC, 0xFF, 
  0xFF, 0xBF, 0xFF, 0xF9, 0xFF, 0xFF, 0xCF, 0xFF, 0xF7, 0xFF, 0xFF, 0xF7, 
  0xFF, 0xE7, 0xFF, 0xFF, 0xFB, 0xFF, 0xCF, 0xFF, 0xFF, 0xFB, 0xFF, 0x1F, 
  0xFC, 0x3F, 0xF0, 0xFF, 0xE7, 0xFB, 0xCF, 0xF7, 0xFF, 0xF3, 0xF7, 0xEF, 
  0xCF, 0xFF, 0xF9, 0xEF, 0xF7, 0xBF, 0xFF, 0xFD, 0xCF, 0xF3, 0xFF, 0xFF, 
  0xFD, 0xDF, 0xFB, 0xFF, 0xFF, 0xFF, 0xDF, 0xFB, 0xFF, 0xFF, 0xFF, 0xEF, 
  0xF7, 0xFF, 0xFF, 0xFF, 0xE7, 0xF7, 0xFF, 0xFF, 0xFF, 0xF3, 0xEF, 0xFF, 
  0xFF, 0xFF, 0xF9, 0x9F, 0xFF, 0xFF, 0x7F, 0xFC, 0x7F, 0x00, 0x00, 0x00, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

const char tstorms_icon[] PROGMEM = {
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0x81, 0xFF, 0xFF, 0xFF, 0x3F, 0x04, 0xFE, 0xFF, 0xFF, 0xDF, 0xF0, 0xFC, 
  0xFF, 0xFF, 0xE7, 0xFF, 0xFB, 0xFF, 0xFF, 0xFB, 0xFF, 0xF3, 0xFF, 0xFF, 
  0xFD, 0xFF, 0xE7, 0xFF, 0xFF, 0xFD, 0xFF, 0x0F, 0xFE, 0x0F, 0xF8, 0xFF, 
  0x8F, 0xFC, 0xE7, 0xFB, 0xFF, 0xC7, 0xF9, 0xF7, 0xE3, 0xFF, 0xC3, 0xF3, 
  0xF3, 0xDF, 0xFF, 0xE8, 0xE7, 0xF9, 0xFF, 0xFF, 0xFE, 0xEF, 0xFD, 0xFF, 
  0xFF, 0xFF, 0xE7, 0xF9, 0xFF, 0xFF, 0xFF, 0xF7, 0xFB, 0xFF, 0xFF, 0xFF, 
  0xF3, 0xFB, 0xFF, 0xFF, 0xFF, 0xF9, 0xF7, 0xFF, 0xFF, 0xFF, 0xFC, 0xCF, 
  0xFF, 0xFF, 0x3F, 0xFE, 0x3F, 0x00, 0x00, 0x80, 0xFF, 0xFF, 0x7F, 0x7F, 
  0xFF, 0xFF, 0xFF, 0xBF, 0xBF, 0xFF, 0xFF, 0xFF, 0xDF, 0x8F, 0xFF, 0xFF, 
  0xFF, 0xEF, 0xF7, 0xFF, 0xFF, 0xFF, 0xF7, 0xFB, 0xFF, 0xFF, 0xFF, 0x4F, 
  0xF7, 0xFF, 0xFF, 0xFF, 0xBF, 0xEF, 0xFF, 0xFF, 0xFF, 0xDF, 0xF1, 0xFF, 
  0xFF, 0xFF, 0xDF, 0xFE, 0xFF, 0xFF, 0xFF, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 
  0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xC7, 0xFF, 0xFF, 0xFF, 0xFF, 0xF3, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };