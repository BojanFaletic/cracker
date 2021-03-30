#include <SoftwareSerial.h>


// HW
const int MODE = 3;
const int RESET = 4;
const int VDD_SWICH = 5;
const int RX_1 = 8;
const int TX_1 = 9;


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
}

int send_1byte(uint8_t key){
  uint8_t zero = 0x00;

  for (int i=0; i<16; i++){
    targetSerial.write(zero);
    delay(30);
  }

  rst::off();
  vdd::off();

  delay(100);
  vdd::on();
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
  mode::bootloader();
  rst::off();
  delay(100);
  vdd::off();
}

void bootload_target(){
  vdd::on();
  rst::on();
  mode::bootloader();

  delay(100);

  mode::program();
  rst::off();
}


void loop() {
  // put your main code here, to run repeatedly:

  Serial.println("Hello world");

  // program
  bootload_target();
  send_256_bytes();
  reset_target();
}
