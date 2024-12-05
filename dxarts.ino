#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <Servo.h>

// Use pins 11 and 10 to communicate with DFPlayer Mini
static const uint8_t PIN_MP3_TX = 11;  // Connects to module's RX
static const uint8_t PIN_MP3_RX = 10;  // Connects to module's TX
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);

// ultrasonic sensor
const int trigPin = 2;   // Pin for trigger
const int echoPin = 3;   // Pin for echo
long duration;            // Variable to store duration of the echo pulse
int distance = 100;             // Variable to store the calculated distance
const int CHATTER = 4;
const int STATIC = 1;

Servo myservo;  // Create servo object to control a servo
int pos = 0;              // Variable to store the servo position
long tick = millis(); // track last tick

byte volumeLevel = 0; // main chatter 
byte staticVolume = 0; // trigger at certain distance 

// state
int track = 1; // store last track for sound - 1 for main, 2 for static
int servoPos = 0; // store last pos to move to other
long staticTime = 0; // track when static was started
long lastDistanceTime = 0; // for delaying distance measure

// Create the Player object
DFRobotDFPlayerMini player; 

void setup() {
  // servo
  myservo.attach(9);      // Attach the servo on pin 9 to the servo object

  // sensor
  pinMode(trigPin, OUTPUT); // Set trigPin as OUTPUT
  pinMode(echoPin, INPUT);  // Set echoPin as INPUT

  // Init USB serial port for debugging
  Serial.begin(9600);
  // Init serial port for DFPlayer Mini
  softwareSerial.begin(9600);

  // Start communication with DFPlayer Mini
  if (player.begin(softwareSerial)) {
    Serial.println("DFPlayer Mini online.");
    player.volume(30); // Set initial volume to maximum (adjustable later)
    player.play(1);    // Play the first MP3 file on the SD card
  } else {
    Serial.println("Failed to connect to DFPlayer Mini.");
  }
}

void loop() {
  // Trigger the ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read the echo and calculate distance
  // if (millis() - lastDistanceTime >= 700) {
  //   duration = pulseIn(echoPin, HIGH);
  //   distance = (duration * 0.0343) / 2;
  //   lastDistanceTime = millis();
  // }
  
  distance = calc_average();

  // Print distance to Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance > 40) {
    if (track != CHATTER) {
      player.volume(30);
      player.play(CHATTER); // should already be playing, TODO detect when it ends
    }
    if (millis() - tick >= 800) {
      if (pos != 180) {
        pos = 180;
      } else {
        pos = 45;
      }
      tick = millis();
    }
    myservo.write(pos);
    track = CHATTER;
  } else {
    player.volume(20);
    if (track != STATIC || millis() - staticTime >= 3000) {
      player.play(STATIC);
      staticTime = millis();
      myservo.write(180);
    }
    track = STATIC;
  }

  if (player.readType() == DFPlayerError) {
    Serial.println("Error detected!");
  }
}

// denoise sensor readings
float calc_average()
{
  static float average = -1.0 ;
  if (average == -1.0)  // special case for first reading.
  {
    average = read_distance () ;
    return average ;
  }
  average += 0.5 * (read_distance() - average) ; 
  return average ;
}

float read_distance ()
{
  long duration = pulseIn(echoPin, HIGH);
  return (duration/2) / 29.1; //Distance calculation per cm
}