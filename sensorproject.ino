const int reset_pin = 12;
const int shift_clock_pin = 11;
const int latch_clock_pin = 10;
const int out_enable_pin = 9;
const int serial_input_pin = 8;
const int sensor_a_pin = 2;
const int sensor_b_pin = 3;
const int start_pin = 4;

const int max_score = 99;

byte current_number = 0;
byte prev_sensor_value_a = HIGH;
byte prev_sensor_value_b = HIGH;
bool max_score_reached = false;
bool start_pin_state = HIGH;
bool game_started = false;
unsigned long start_time = 0;

// controlling 7 segment display
const uint8_t digit_patterns[10][8] = { 
  {0, 0, 0, 0, 0, 0, 1, 1}, //  0
  {1, 0, 0, 1, 1, 1, 1, 1}, //  1
  {0, 0, 1, 0, 0, 1, 0, 1}, //  2
  {0, 0, 0, 0, 1, 1, 0, 1}, //  3
  {1, 0, 0, 1, 1, 0, 0, 1}, //  4
  {0, 1, 0, 0, 1, 0, 0, 1}, //  5
  {0, 1, 0, 0, 0, 0, 0, 1}, //  6
  {0, 0, 0, 1, 1, 1, 1, 1}, //  7
  {0, 0, 0, 0, 0, 0, 0, 1}, //  8
  {0, 0, 0, 1, 1, 0, 0, 1}  //  9
};

void initialize_display(void) {
  pinMode(reset_pin, OUTPUT);
  pinMode(shift_clock_pin, OUTPUT);
  pinMode(latch_clock_pin, OUTPUT);
  pinMode(out_enable_pin, OUTPUT);
  pinMode(serial_input_pin, OUTPUT);

//74HC595 shift register states
  digitalWrite(reset_pin, HIGH);  
  digitalWrite(shift_clock_pin, LOW);
  digitalWrite(latch_clock_pin, LOW);
  digitalWrite(out_enable_pin, LOW);
  digitalWrite(serial_input_pin, LOW);
}

void write_byte(uint8_t number, bool last) {
  digitalWrite(latch_clock_pin, LOW);
  
//controlling shifting for 74HC595 at the digit array
  for (int i = 7; i >= 0; i--) {
    digitalWrite(serial_input_pin, digit_patterns[number][i]);
    digitalWrite(shift_clock_pin, HIGH);
    digitalWrite(shift_clock_pin, LOW);
  }
  digitalWrite(latch_clock_pin, HIGH);

  if (last) {
    digitalWrite(out_enable_pin, HIGH);
  }
}

void write_high_and_low_number(uint8_t tens, uint8_t ones) {
  write_byte(tens, false);
  write_byte(ones, true);
}

void show_results(byte number) {
  if (number > max_score) {
    write_high_and_low_number(9, 9);
  } else {
    write_high_and_low_number(number / 10, number % 10);
  }
}

void game_logic(int score_increment) {
  if (!max_score_reached) {
    current_number = min(current_number + score_increment, max_score);

   

    if (current_number >= max_score) {
      max_score_reached = true;
      unsigned long elapsed_time = (millis() - start_time);
      Serial.print("Maximum score reached in ");
      Serial.print(elapsed_time / 60000); // Minutes
      Serial.print(" minutes, ");
      Serial.print((elapsed_time / 1000) % 60); // Seconds
      Serial.print(".");
      Serial.print((elapsed_time % 1000) / 100); // Tenths of seconds
      Serial.println(" seconds!");
    }
  }
}

void check_sensor(int sensor_pin, byte &prev_sensor_value, int score_increment) {
  if (!max_score_reached) {
    byte sensor_value = digitalRead(sensor_pin);

    if (sensor_value == LOW && prev_sensor_value == HIGH) {
      game_logic(score_increment);
    }

    prev_sensor_value = sensor_value;
  }
}

void checkStartPinChange() {
  static bool last_start_pin_state = HIGH;

  if (digitalRead(start_pin) == LOW && last_start_pin_state == HIGH) {
    last_start_pin_state = LOW;
    game_started = !game_started;

    if (game_started) {
      current_number = 0;
      max_score_reached = false;
      start_time = millis(); 
      Serial.println("Game started!");
    }
  } else if (digitalRead(start_pin) == HIGH) {
    last_start_pin_state = HIGH;
  }
}

void setup() {
  initialize_display();
  pinMode(sensor_a_pin, INPUT_PULLUP);
  pinMode(sensor_b_pin, INPUT_PULLUP);
  pinMode(start_pin, INPUT_PULLUP);
  Serial.begin(9600);
  digitalWrite(start_pin, HIGH);
}

void loop() {
  checkStartPinChange();

  if (game_started) {
    check_sensor(sensor_a_pin, prev_sensor_value_a,  1); // adds 1 points
    check_sensor(sensor_b_pin, prev_sensor_value_b,  10); // adds 10 points

    show_results(current_number);
    digitalWrite(out_enable_pin, LOW);
    delay(5);
  }
}