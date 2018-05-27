#include <OneWire.h>

// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library

OneWire  ds(10);  // on pin 10 (a 4.7K resistor is necessary)

unsigned char LED_0F[] = 
{// 0	 1	  2	   3	4	 5	  6	   7	8	 9	  A	   b	C    d	  E    F    -
    0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0x8C,0xBF,0xC6,0xA1,0x86,0xFF,0xbf
};
unsigned char LED[4];
int SCLK = 2;
int RCLK = 3;
int DIO = 4;

void setup(void) {
  //Serial.begin(115200);
  pinMode(SCLK,OUTPUT);
  pinMode(RCLK,OUTPUT);
  pinMode(DIO,OUTPUT);
  LED[0]=1;
  LED[1]=0;
  LED[2]=0;
  LED[3]=4;
  
  LED4_Display();
  //Serial.println("=============start=============");
}

void LED4_Display (void)
{
    unsigned char *led_table;          // 查表指针
    unsigned char i;
    //显示第1位
    led_table = LED_0F + LED[0];
    i = *led_table;
    LED_OUT(i);			
    LED_OUT(0x01);		
    digitalWrite(RCLK,LOW);
    digitalWrite(RCLK,HIGH);
    //显示第2位
    led_table = LED_0F + LED[1];
    i = *led_table;
    LED_OUT(i);		
    LED_OUT(0x02);		
    digitalWrite(RCLK,LOW);
    digitalWrite(RCLK,HIGH);
    //显示第3位
    led_table = LED_0F + LED[2];
    i = *led_table;
    LED_OUT(i);			
    LED_OUT(0x04);	
    digitalWrite(RCLK,LOW);
    digitalWrite(RCLK,HIGH);
    //显示第4位
    led_table = LED_0F + LED[3];
    i = *led_table;
    LED_OUT(i);			
    LED_OUT(0x08);		
    digitalWrite(RCLK,LOW);
    digitalWrite(RCLK,HIGH);
}

void LED_OUT(unsigned char X)
{
    unsigned char i;
    for(i=8;i>=1;i--)
    {
        if (X&0x80) 
            {
                digitalWrite(DIO,HIGH);
                }  
            else 
            {
                digitalWrite(DIO,LOW);
            }
        X<<=1;
            digitalWrite(SCLK,LOW);
            digitalWrite(SCLK,HIGH);
    }
}

void loop(void) { 
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  
  if ( !ds.search(addr)) {
    //Serial.println("No more addresses.");
    //Serial.println();
    ds.reset_search();
    //delay(250);
    for( int ddsspp = 0; ddsspp <200; ddsspp++){
      delay(1);
      LED4_Display();
    }
    return;
  }
  
  //Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    //Serial.write(' ');
    //Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      //Serial.println("CRC is not valid!");
      return;
  }
  //Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      //Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      //Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      //Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      //Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  //delay(750);     // maybe 750ms is enough, maybe not
  for( int ddsspp = 0; ddsspp <500; ddsspp++){
    delay(1);
    LED4_Display();
  }
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  //Serial.print("  Data = ");
  //Serial.print(present, HEX);
  //Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    //Serial.print(data[i], HEX);
    //Serial.print(" ");
  }
  //Serial.print(" CRC=");
  //Serial.print(OneWire::crc8(data, 8), HEX);
  //Serial.println();
  LED4_Display();
  
  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  //Serial.print("  Temperature = ");
  //Serial.print(celsius);
  //Serial.print(" Celsius, ");
  //Serial.print(fahrenheit);
  //Serial.println(" Fahrenheit");
  
  LED[3]=int(celsius)%100/10;
  LED[2]=int(celsius)%10 | 0x80;
  LED[1]=int(celsius*10)%10;
  LED[0]=int(celsius*100)%10;
  LED4_Display();
}
