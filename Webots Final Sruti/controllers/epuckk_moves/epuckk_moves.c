#include <webots/robot.h>
#include <webots/light_sensor.h>
#include <webots/distance_sensor.h>
#include <webots/motor.h>
#include <webots/led.h>
#include <stdio.h>

#define TIME_STEP 32
#define MAX_SPEED 6.28
#define CYCLE_TIME 570.0 
#define NUM_LIGHT_SENSORS 8

// Sensors and motors
WbDeviceTag left_motor, right_motor;
WbDeviceTag ps[8];  // 8 proximity sensors
WbDeviceTag ls[8];  // 8 light sensors
WbDeviceTag led;

// expressions for handling cycle logic
bool exploration_done = false;
bool second_cycle_started = false;
double max_average_intensity = 0.0;  // Records the highest average light intensity

// Initializing devices
void initialize_devices() {
  left_motor = wb_robot_get_device("left wheel motor");
  right_motor = wb_robot_get_device("right wheel motor");
  wb_motor_set_position(left_motor, INFINITY);
  wb_motor_set_position(right_motor, INFINITY);

  // Initializing proximity sensors
  for (int i = 0; i < 8; i++) {
    char ps_name[4];
    sprintf(ps_name, "ps%d", i);
    ps[i] = wb_robot_get_device(ps_name);
    wb_distance_sensor_enable(ps[i], TIME_STEP);
  }

  // Initializing light sensors
  for (int i = 0; i < NUM_LIGHT_SENSORS; i++) {
    char ls_name[4];
    sprintf(ls_name, "ls%d", i);
    ls[i] = wb_robot_get_device(ls_name);
    wb_light_sensor_enable(ls[i], TIME_STEP);
  }

  // Initializing LED
  led = wb_robot_get_device("led0");
  wb_led_set(led, 0);  // To keep LED turned off initially
}


// Calculating average light intensity from all 8 sensors
double get_average_light_intensity() {
  double total_intensity = 0.0;
  for (int i = 0; i < NUM_LIGHT_SENSORS; i++) {
    total_intensity += wb_light_sensor_get_value(ls[i]);
  }
  return total_intensity / NUM_LIGHT_SENSORS;
}

// Blinking LED at stop
void blink_led() {
  if (wb_motor_get_velocity(left_motor) == 0.0 && wb_motor_get_velocity(right_motor) == 0.0) {
    wb_led_set(led, 1); 
    wb_robot_step(TIME_STEP); 
    wb_led_set(led, 0);  
  }
}


// Processing sensors and controlling the robot
void process_sensors_and_control() {
  double current_time = wb_robot_get_time();

  // Begins second cycle 
  if (current_time >= CYCLE_TIME && !second_cycle_started) {
    second_cycle_started = true;
    exploration_done = true;  // Transitions to second cycle
    printf("Epuck begins its second cycle!");
  }

  // Gets proximity sensor values
  int ps_values[8];
  for (int i = 0; i < 8; i++) {
    ps_values[i] = (int)wb_distance_sensor_get_value(ps[i]);
  }

  // Gets current average light intensity in integer
  double current_average_intensity = get_average_light_intensity();

  // Tracks the highest average light intensity while exploring (first cycle)
  if (!exploration_done) {
    if (current_average_intensity > max_average_intensity) {
      max_average_intensity = current_average_intensity;
      printf("New highest average light intensity recorded: %d\n", (int)max_average_intensity);
    }
  } else {
    // Compares current average intensity with the max recorded intensity at second cycle
    if (current_average_intensity >= (max_average_intensity - 20.0)) { //-20 threshold from the max recorded intensity
      printf("Light intensity near max recorded intensity: stopping at average light intensity %d\n", (int)current_average_intensity);
      wb_motor_set_velocity(left_motor, 0.0);  // Stop the left motor
      wb_motor_set_velocity(right_motor, 0.0); // Stop the right motor
      blink_led();  // Blink the LED three times

      // Stops the robot completely and make the LED blink continuously
      while (true) {
        wb_led_set(led, 1);  // Turn LED on
        wb_robot_step(1000); // Wait for 1 second
        wb_led_set(led, 0);  // Turn LED off
        wb_robot_step(1000); // Wait for 1 second
      }
    }
  }

  // Movement logic (executed only if the robot hasn't stopped yet)
  int left_speed = (int)(MAX_SPEED);
  int right_speed = (int)(MAX_SPEED);

  bool front_obstacle = ps_values[0] > 80.0 || ps_values[7] > 80.0;
  bool side_left = ps_values[5] > 80.0;
  bool side_right = ps_values[2] > 80.0;

  if (front_obstacle) {
    if (side_left && side_right) {
      left_speed = (int)(-0.5 * MAX_SPEED);
      right_speed = (int)(-0.5 * MAX_SPEED);
    } else if (side_left) {
      left_speed = (int)(MAX_SPEED);
      right_speed = (int)(0.1 * MAX_SPEED);
    } else if (side_right) {
      left_speed = (int)(0.1 * MAX_SPEED);
      right_speed = (int)(MAX_SPEED);
    } else {
      left_speed = (int)(MAX_SPEED);
      right_speed = (int)(-0.1 * MAX_SPEED);
    }
  } else {
    if (side_left && !side_right) {
      left_speed = (int)(0.1 * MAX_SPEED);
      right_speed = (int)(MAX_SPEED);
    } else if (!side_left && !side_right) {
      left_speed = (int)(0.5 * MAX_SPEED);
      right_speed = (int)(0.5 * MAX_SPEED);
    } else if (!side_left && side_right) {
      left_speed = (int)(0.1 * MAX_SPEED);
      right_speed = (int)(MAX_SPEED);
    }
  }

  // Set motor speeds
  wb_motor_set_velocity(left_motor, left_speed);
  wb_motor_set_velocity(right_motor, right_speed);
}

int main() {
  wb_robot_init();
  initialize_devices();

  while (wb_robot_step(TIME_STEP) != -1) {
    process_sensors_and_control();
  }

  wb_robot_cleanup();
  return 0;
}
