#include <SoftwareSerial.h>
SoftwareSerial BTSerial(3,4); // TX, RX
int num = 0;

void setup() {
   
 Serial.begin(9600);
 BTSerial.begin(9600);

}

void loop() {
  double rubberVal = analogRead(A0);
  Serial.println(rubberVal);
  BTSerial.println(rubberVal);
  
  delay(500);
}
