#include <SoftwareSerial.h>


// HW
const int MODE = 3;
const int RESET = 4;
const int VDD_SWICH = 5;
const int RX_1 = 8;
const int TX_1 = 9;
/*moje*/
const int START_TIPKA = 11;



SoftwareSerial targetSerial(RX_1, TX_1);

constexpr uint8_t NOT(uint8_t x){
  return x^1;
}

namespace vdd{
  void on(){
    digitalWrite(VDD_SWICH, 0);
  }
  void off(){
    digitalWrite(VDD_SWICH, 1);
  }
}

namespace rst{
  void off(){
    digitalWrite(RESET, 1);
  }
  void on(){
    digitalWrite(RESET, 0);
  }
}

namespace mode{
  void bootloader(){
    digitalWrite(MODE, 1);
  }
  void program(){
    digitalWrite(MODE, 0);
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  targetSerial.begin(9600);

  pinMode(MODE, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(VDD_SWICH, OUTPUT);
  /*moje*/
  pinMode(START_TIPKA, INPUT);
}

int send_1byte(uint8_t key){
  uint8_t zero = 0x00;

  for (int i=0; i<16; i++){
    targetSerial.write(zero);
    delay(30);
  }

  rst::off();
  delay(20);
  rst::on();
  
  delay(100);

  // send key
  targetSerial.write(key);
  targetSerial.write(zero);
  targetSerial.write(zero);
  targetSerial.write(zero);
  targetSerial.write(zero);
  targetSerial.write(zero);
  targetSerial.write(zero);

  // send query
  targetSerial.write(0x70);
  volatile int start_time = millis();
  targetSerial.read();
  volatile int stop_time = millis();

  // calculate time
  int duration = stop_time - start_time;
  return duration;
}

void send_256_bytes(){
  int max_value = 0;
  int max_digit = 0;
  for (int k=0; k<256; k++){
    int requred_time = send_1byte(k);
    Serial.print("Sending: ");
    Serial.print(k);
    Serial.print(" requred time: ");
    Serial.println(requred_time);

    if (max_value < requred_time){
      max_value = requred_time;
      max_digit = k;
    }
  }
  Serial.print("Max digit: ");
  Serial.println(max_digit);
}


void reset_target(){
  mode::program();
  delay(50);
  rst::off();
  delay(20);
  rst::on();
  delay(50);
  vdd::off();

  delay(100);
}

void bootload_target(){
  //Začetek delovanja
  vdd::on();
  rst::on();
  mode::program();
  
  delay(100);
  
  //Spravimo v navadno delovanje R8C
  rst::off();
  delay(20);
  rst::on();
  delay(50);
  vdd::off();
  
  delay(100);

  //Spravimo R8C v bootloader delovanje
  vdd::on();
  delay(50);
  mode::bootloader();
  delay(50);
  rst::off();
  delay(20);
  rst::on();

  delay(30);
}


void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Hello world");
 
  /*moje, da dokler ne pritisneš tipke, ne naredi nič*/
  while (digitalRead(START_TIPKA) == HIGH) {
  //Naredi nič.
  }

  Serial.println("Zacetek delovanja");

  
  bootload_target();
  send_256_bytes();
  reset_target();

   Serial.println("Konec delovanja");

  while(1);
}
