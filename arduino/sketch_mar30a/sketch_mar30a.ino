#include <SoftwareSerial.h>


// HW
const int MODE = 3;
const int RESET = 4;
const int VDD_SWICH = 5;
const int RX_1 = 8;
const int TX_1 = 9;
/*My*/
const int BUTTON_PIN  = 11;



SoftwareSerial targetSerial(RX_1, TX_1);

constexpr uint8_t NOT(uint8_t x) {
  return x ^ 1;
}

namespace vdd {
void on() {
  digitalWrite(VDD_SWICH, 0);
}
void off() {
  digitalWrite(VDD_SWICH, 1);
}
}

namespace rst {
void off() {
  digitalWrite(RESET, 1);
}
void on() {
  digitalWrite(RESET, 0);
}
}

namespace mode {
void bootloader() {
  digitalWrite(MODE, 1);
}
void program() {
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
  /*My*/
  pinMode(BUTTON_PIN, INPUT);
}

int send_1byte(uint8_t key) {
  uint8_t zero = 0x00;

  for (int i = 0; i < 16; i++) {
    targetSerial.write(zero);
    delay(40);
  }

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

  delay(100);
  rst::off();
  delay(200);
  rst::on();

  delay(100);
}

void send_256_bytes() {
  int max_value = 0;
  int max_digit = 0;
  for (int k = 0; k < 256; k++) {
    int requred_time = send_1byte(k);
    Serial.print("Sending: ");
    Serial.print(k);
    Serial.print(" requred time: ");
    Serial.println(requred_time);

    if (max_value < requred_time) {
      max_value = requred_time;
      max_digit = k;
    }
  }
  Serial.print("Max digit: ");
  Serial.println(max_digit);
}


void reset_target() {
  mode::program();
  delay(400);
  rst::off();
  delay(200);
  rst::on();
  delay(400);
  vdd::off();

  delay(100);
}

void bootload_target() {
  //Začetek delovanja
  vdd::on();
  delay(100);
  rst::on();
  delay(100);
  mode::program();

  delay(100);

  //We send R8C to normall working.
  rst::off();
  delay(200);
  rst::on();
  delay(400);
  vdd::off();

  delay(500);

  //We send R8C to bootloader.
  vdd::on();
  delay(400);
  mode::bootloader();
  delay(400);
  rst::off();
  delay(200);
  rst::on();

  delay(100);
}


void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Press button to start crecking process.");

  /*my, no code because no code is procesed.*/
  while (digitalRead(BUTTON_PIN) == HIGH) {
    //Naredi nič.
  }

  Serial.println("Start of process");


  bootload_target();
  send_256_bytes();
  reset_target();

  Serial.println("End of process");

  while (1);
}
