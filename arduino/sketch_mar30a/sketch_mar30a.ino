#include <SoftwareSerial.h>

// HW
const int MODE = 3;
const int RESET = 4;
const int VDD_SWICH = 5;
const int RX_1 = 8;
const int TX_1 = 9;


SoftwareSerial targetSerial(RX_1, TX_1);


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

  digitalWrite(RESET, 0);
  digitalWrite(VDD_SWICH, 0);

  delay(100);
  digitalWrite(VDD_SWICH, 1);
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


void finish_sequence(){
  digitalWrite(MODE, 1);
  digitalWrite(RESET, 0);
  delay(100);
  digitalWrite(VDD_SWICH, 0);
}

void reset_target(){
  digitalWrite(VDD_SWICH, 1);
  digitalWrite(RESET, 1);
  digitalWrite(MODE, 1);

  delay(100);

  digitalWrite(MODE, 0);
  digitalWrite(RESET, 0);
}


void loop() {
  // put your main code here, to run repeatedly:

  Serial.println("Hello world");

  // program
  reset_target();
  send_256_bytes();
  finish_sequence();
}
