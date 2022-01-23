#define SSD1306    0x3D  // Slave address

#include <TinyI2CMaster.h>
//#include <TinyWireM.h>
#include <Tiny4kOLED.h>

//#define DEBUG

uint8_t width = 128;
uint8_t height = 64;

uint32_t set_temperature=10;
uint32_t read_temperature=0;
uint32_t read_sensor_value=0;
uint32_t read_sensor_resistance=0;
uint32_t last_display_time=0;
boolean active=true;
boolean heating=false;
boolean error=false;  // Set to true when sensor data is invalid


void setup() {
  pinMode(1,INPUT);
  pinMode(3,INPUT);
  pinMode(4,INPUT);
  
  delay(500);
  oled.begin(width, height, sizeof(tiny4koled_init_128x64r), tiny4koled_init_128x64r);
  oled.setFont(FONT8X16);
  oled.clear();
  oled.on();

  initScreen();
}

void loop() {
  read_sensor_value=analogRead(2); // PB4
  read_sensor_resistance=(460*read_sensor_value)/(2482-read_sensor_value);
  
  set_temperature=customMap(analogRead(3),0,1023,29,500); //PB3
  read_temperature = customMap(read_sensor_resistance,53,223, 27,525); 
  
  if(read_temperature>530 || read_temperature<0) error=true;
  else error=false;
  
  // Shut off
  if(set_temperature==29){
    active=false; // Stop the iron
  }else {
    active=true;  // Iron started
  }


  // Heating
  heating=false;
  if(active && !error){
    if(read_temperature<set_temperature){
      heating=true;
    }
  }
  analogWrite(1,heating?255:0);   //PB1 Heating coil
  
  if(millis()-last_display_time>500){
    last_display_time=millis();
    updateScreen();
  }

}

void initScreen(){
  oled.setCursor(0, 0);
  oled.print("SETAT: ");

  oled.setCursor(0, 4);
  oled.print("TEMP: ");

#ifndef DEBUG
  oled.setCursor(0, 6);
  oled.print("sens: ");
#endif
}

void updateScreen(){
  oled.invertOutput(false);
  
  //////////////// Clear text on this line
  oled.setCursor(56, 0);
  oled.fillToEOL(0);
  oled.setCursor(56, 1);
  oled.fillToEOL(0);
  
  // Update with new data
  oled.setCursor(56, 0);
  if(active) oled.print(set_temperature);
  else oled.print("OPRIT");

  
  //////////////// Clear text on this line
  oled.setCursor(42, 4);
  oled.fillToEOL(0);
  oled.setCursor(42, 5);
  oled.fillToEOL(0);
  
  // Update with new data
  oled.setCursor(42, 4);
  if(!error) oled.print(read_temperature);
  else oled.print("EROARE");


#ifndef DEBUG
  //////////////// Clear text on this line
  oled.setCursor(48, 6);
  oled.fillToEOL(0);
  oled.setCursor(48, 7);
  oled.fillToEOL(0);
  
  // Update with new data
  oled.setCursor(48, 6);
  oled.print(read_sensor_value);

  
  if(heating){
    oled.invertOutput(true);
    oled.setCursor(88, 6);
    oled.print("ACTIV");
  }else{
    oled.setCursor(88, 6);
    oled.fillToEOL(0);
    
    oled.setCursor(88, 7);
    oled.fillToEOL(0);
  }
#else
  //////////////// Clear text on this line
  oled.setCursor(0, 6);
  oled.fillToEOL(0);
  oled.setCursor(0, 7);
  oled.fillToEOL(0);
  
  oled.setFont(FONT6X8);
  oled.setCursor(0, 6);
  oled.print(read_sensor_value);
  oled.print(' ');
  oled.print(read_sensor_resistance);
  
  oled.setCursor(0, 7);
  oled.print(read_temperature);
  oled.setFont(FONT8X16);
  
#endif
}


// https://github.com/arduino/ArduinoCore-API/issues/51
int32_t customMap(int64_t x, int64_t in_min, int64_t in_max, int64_t out_min, int64_t out_max){
  int64_t in_range = in_max - in_min;
  int64_t out_range = out_max - out_min;
  
  // compute the numerator
  int64_t num = (x - in_min) * out_range;
  
  // before dividing, add extra for proper round off (towards zero)
  if (out_range >= 0) {
    num += in_range / 2;
  } else {
    num -= in_range / 2;
  }
  
  // divide by input range and add output offset to complete map() compute
  int64_t result = num / in_range + out_min;
  
  // fix "a strange behaviour with negative numbers" (see ArduinoCore-API issue #51)
  //   this step can be deleted if you don't care about non-linear output
  //   behavior extrapolating slightly beyond the mapped input & output range
  if (out_range >= 0) {
    if (in_range * num < 0) return result - 1;
  } else {
    if (in_range * num >= 0) return result + 1;
  }
  return result;
}
