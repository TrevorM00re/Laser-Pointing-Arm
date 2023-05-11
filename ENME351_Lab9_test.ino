#include <math.h>

#define X_MAX .3
#define X_MIN .1
#define Y_MAX .3
#define Y_MIN 0
#define L1 .132
#define L2 .1315

#define BLUE 12
#define YELLOW 11
#define PINK 10
#define ORANGE 9

#define ON 1
#define OFF 0

//Pin and Input definitions
int new_pot_x;
int curr_pot_x = 0;
int x_pin = A4;

int new_pot_y;
int curr_pot_y = 0;
int y_pin = A3;

int pot_angle;
int angle_pin = A2;

int photo_pin = A0;

int laser_state = OFF;
int laser_button = A1;

int pin_shoulder = 3;
int pin_elbow = 4;
int pin_effector = 5;
int pin_laser = 8;

int stepper_blue = 12;
int stepper_pink = 10;
int stepper_yellow = 11;
int stepper_orange = 9;

int stepper_state = BLUE;

//Setting initial values
float x_coord = (X_MAX + X_MIN)/2;
float y_coord = (Y_MAX + Y_MIN)/2;
float angle_direction = 0;

float shoulder_pos = 1500;
float elbow_pos = 1500;
float effector_pos = 1500;

float L3;
float new_b;
float gamma;
float new_alpha;
float curr_b = 53;
float curr_alpha = 65;

//Various timers
long curr_time; //Overall time
long prev_time = 0;
long overall_time = 0;

long curr_laser_time; //Controls speed at which laser can switch
long prev_laser_time = 0;

bool elbow_updated = true;
bool shoulder_updated = true;
bool effector_updated = true;

void setup() {
  //Pin inputs
  pinMode(x_pin, INPUT);
  pinMode(y_pin, INPUT);
  pinMode(angle_pin, INPUT);
  pinMode(laser_button, INPUT);
  pinMode(photo_pin, INPUT);

  //Pin outputs
  pinMode(pin_shoulder, OUTPUT);
  pinMode(pin_elbow, OUTPUT);

  pinMode(pin_laser, OUTPUT);

  pinMode(stepper_blue, OUTPUT);
  pinMode(stepper_yellow, OUTPUT);
  pinMode(stepper_orange, OUTPUT);
  pinMode(stepper_pink, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  //Read and process positional inputs
  new_pot_x = analogRead(x_pin);
  new_pot_y = analogRead(y_pin);
  pot_angle = analogRead(angle_pin);
  x_coord += (map(new_pot_x, 0.0, 1023.0, -1.0, 2.0))/5000.0;
  y_coord += (map(new_pot_y, 0.0, 1023, -1.0, 2.0))/5000.0;
  angle_direction = map(pot_angle, 0, 1023, -1, 2);

  //Checking valid inputs
  if (x_coord > X_MAX) {
    x_coord = X_MAX;
  } else if (x_coord < X_MIN) {
    x_coord = X_MIN;
  }

  if (y_coord > Y_MAX) {
    y_coord = Y_MAX;
  } else if (y_coord < Y_MIN) {
    y_coord = Y_MIN;
  }

  //Inverse kinematics to find needed arm angles
  L3 = sqrt(pow(x_coord, 2) + pow(y_coord, 2));
  new_b = acos((pow(L3, 2) - pow(L2, 2) - pow(L1, 2)) / (2 * L1 * L2));
  gamma = PI - new_b;
  new_alpha = atan(y_coord/x_coord) + asin(sin(gamma)*L1/L3);

  new_b *= (4068.0/71.0);
  new_alpha *= (4068.0/71.0);

  //Move stepper
  move_stepper();

  //Updating laser pointer
  change_laser();

  //Data output for processing GUI
  Serial.print(x_coord, 3);

  Serial.print(" ");
  Serial.print(y_coord, 3);

  Serial.print(" ");
  Serial.print(new_alpha);

  Serial.print(" ");
  Serial.print(new_b);

  Serial.print(" ");
  Serial.print(90 - new_alpha - new_b);

  Serial.print(" ");
  Serial.print(angle_direction);

  Serial.print(" ");
  Serial.print(laser_state);

  Serial.print(" ");
  Serial.println(analogRead(photo_pin));

  //Setup for servo actuation
  //Shoulder and effector servos set to false if no movement to prevent stuttering
  if (new_alpha == curr_alpha && new_alpha == curr_alpha &&
      new_b == curr_b && new_b == curr_b) {
    shoulder_updated = true;
    effector_updated = true;
    elbow_updated = false;
  } else {
    shoulder_updated = false;
    effector_updated = false;
    elbow_updated = false;
  }

  //Ensuring code runs every 20ms for servo pulse width
  Serial.print(" ");
  curr_time = micros();
  delayMicroseconds(20000 - (curr_time - overall_time));
  overall_time = micros();

  //Determining signal lengths for all 3 servos
  effector_pos = curr_alpha - curr_b;
  shoulder_pos = map(curr_alpha, 0.0, 180.0, 500, 2500.0);
  elbow_pos = map(curr_b, 0.0, 180.0, 300.0, 2300.0);
  effector_pos = map(90 - effector_pos, 0, 180, 300, 2300);
  
  //Starting servo PWM signals
  digitalWrite(pin_shoulder, HIGH);
  digitalWrite(pin_elbow, HIGH);
  digitalWrite(pin_effector, HIGH);
  prev_time = micros();
  curr_time = micros();

  //Looping till all servos updated
  while (!elbow_updated || !shoulder_updated || !effector_updated) {
    //Checking for proper pulse width on elbow
    if (curr_time - prev_time > elbow_pos && !elbow_updated) {
      digitalWrite(pin_elbow, LOW);
      elbow_updated = true;
    }
    
    //Checking for proper pulse width on shoulder
    if (curr_time - prev_time > shoulder_pos && !shoulder_updated) {
      digitalWrite(pin_shoulder, LOW);
      shoulder_updated = true;
    }

//Checking for proper pulse width on effector
    if (curr_time - prev_time > effector_pos && !effector_updated) {
      digitalWrite(pin_effector, LOW);
      effector_updated = true;
    }

    curr_time = micros();
  }

  //Don't update actuated values if invalid
  if (!isnan(new_alpha) && !isnan(new_b)) {
    curr_b = new_b;
    curr_alpha = new_alpha;
  }
}

void adjust_servos() {

}

void move_stepper() {
  //Move CCW  
  if (angle_direction > 0) {
    if (stepper_state == BLUE) {
      digitalWrite(stepper_blue, LOW);
      digitalWrite(stepper_yellow, LOW);
      digitalWrite(stepper_pink, HIGH);
      digitalWrite(stepper_orange, LOW);

      stepper_state = PINK;
    } else if (stepper_state == PINK) {
      digitalWrite(stepper_blue, LOW);
      digitalWrite(stepper_yellow, HIGH);
      digitalWrite(stepper_pink, LOW);
      digitalWrite(stepper_orange, LOW);

      stepper_state = YELLOW;
    } else if (stepper_state == YELLOW) {
      digitalWrite(stepper_blue, LOW);
      digitalWrite(stepper_yellow, LOW);
      digitalWrite(stepper_pink, LOW);
      digitalWrite(stepper_orange, HIGH);

      stepper_state = ORANGE;
    } else {
      digitalWrite(stepper_blue, HIGH);
      digitalWrite(stepper_yellow, LOW);
      digitalWrite(stepper_pink, LOW);
      digitalWrite(stepper_orange, LOW);

      stepper_state = BLUE;
    }
  } else if (angle_direction < 0) { //Move CW
    if (stepper_state == BLUE) {
      digitalWrite(stepper_blue, LOW);
      digitalWrite(stepper_yellow, LOW);
      digitalWrite(stepper_pink, HIGH);
      digitalWrite(stepper_orange, LOW);

      stepper_state = ORANGE;
    } else if (stepper_state == PINK) {
      digitalWrite(stepper_blue, LOW);
      digitalWrite(stepper_yellow, HIGH);
      digitalWrite(stepper_pink, LOW);
      digitalWrite(stepper_orange, LOW);

      stepper_state = BLUE;
    } else if (stepper_state == YELLOW) {
      digitalWrite(stepper_blue, LOW);
      digitalWrite(stepper_yellow, LOW);
      digitalWrite(stepper_pink, LOW);
      digitalWrite(stepper_orange, HIGH);

      stepper_state = PINK;
    } else {
      digitalWrite(stepper_blue, HIGH);
      digitalWrite(stepper_yellow, LOW);
      digitalWrite(stepper_pink, LOW);
      digitalWrite(stepper_orange, LOW);

      stepper_state = YELLOW;
    }
  }
}

void change_laser() {
  curr_laser_time = micros();

  //Throttle laser changing to once per second and require low ambient light
  if (analogRead(laser_button) > 256 && curr_laser_time - prev_laser_time > 1000000 && analogRead(photo_pin) < 500) {
    if (laser_state == ON) {
      laser_state = OFF;
      digitalWrite(pin_laser, LOW);
    } else {
      laser_state = ON;
      digitalWrite(pin_laser, HIGH);
    }

    prev_laser_time = curr_laser_time;
  }  
}