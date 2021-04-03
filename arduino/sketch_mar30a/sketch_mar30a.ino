#include <SoftwareSerial.h>

// HW
const int MODE = 8;
const int RESET = 4;
const int VDD_SWICH = 5;
const int RX_1 = 3;
const int TX_1 = 9;
const int BLANK = 12;

/*My */
const int BUTTON_PIN = 11;

SoftwareSerial targetSerial(BLANK, TX_1);

constexpr uint8_t NOT(uint8_t x) { return x ^ 1; }

namespace vdd {
void on() { digitalWrite(VDD_SWICH, 0); }
void off() { digitalWrite(VDD_SWICH, 1); }
} // namespace vdd

namespace rst {
void off() { digitalWrite(RESET, 1); }
void on() { digitalWrite(RESET, 0); }
} // namespace rst

namespace mode {
void bootloader() { digitalWrite(MODE, 1); }
void program() { digitalWrite(MODE, 0); }
} // namespace mode

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  targetSerial.begin(9600);

  pinMode(MODE, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(VDD_SWICH, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(RX_1, INPUT_PULLUP);
}

namespace INT {
void stop();

void startTimer1() {
  /*Set counter lower and upper byte to 0.*/
  TCNT1L = 0x00;
  TCNT1H = 0x00;
  /*Set timer to normal mode and set prescaler to 1. This should start the
   * timer?*/
  TCCR1B = 0x01;
}

void stopTimer1() {
  /*Set timer to normal mode and set prescaler to 0. This should stop the
   * timer?*/
  TCCR1B = 0x00;
}

unsigned int getTimerTicks() {
  unsigned int ticks = 0;
  /*Get upper and lower counter byte to ticks.*/
  ticks = TCNT1H << 8;
  ticks = TCNT1L;
  return ticks;
}

void start() {
  attachInterrupt(digitalPinToInterrupt(RX_1), INT::stop, FALLING);
  startTimer1();
}

void stop() {
  stopTimer1();
  detachInterrupt(digitalPinToInterrupt(RX_1));
}

} // namespace INT

int send_1byte(uint8_t key) {
  uint8_t zero = 0x00;

  for (int i = 0; i < 16; i++) {
    targetSerial.write(zero);
    delay(40);
  }

  // send header
  const uint8_t header[] = {0xf5, 0xdf, 0xff, 0x00, 0x07};
  for (uint8_t el : header) {
    targetSerial.write(el);
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

  // Start interrupt on falling edge
  INT::start();

  // wait for some time
  delay(200);
}

void send_256_bytes() {
  int max_value = 0;
  int max_digit = 0;
  for (int k = 0; k < 256; k++) {
    int required_time = send_1byte(k);

    // reset after sending

    rst::off();
    delay(200);
    rst::on();
    delay(400);

    Serial.print("Sending: ");
    Serial.print(k);
    Serial.print(" requred time: ");
    Serial.println(required_time);

    if (max_value < required_time) {
      max_value = required_time;
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
  delay(400);
}

void bootload_target() {
  // Začetek delovanja
  vdd::on();
  delay(400);
  rst::on();
  delay(400);
  mode::program();
  delay(400);

  // We send R8C to normall working.

  rst::off();
  delay(300);
  rst::on();
  delay(400);
  vdd::off();

  delay(400);

  // We send R8C to bootloader.
  vdd::on();
  delay(400);
  mode::bootloader();
  delay(400);
  rst::off();
  delay(200);
  rst::on();

  delay(400);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Press button to start cracking process.");

  // my, no code because no code is processed.
  while (digitalRead(BUTTON_PIN) == HIGH) {
    // Naredi nič.
  }

  Serial.println("Start of process");
  delay(400);

  bootload_target();
  send_256_bytes();
  reset_target();

  Serial.println("End of process");

  while (1)
    ;
}
