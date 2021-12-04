#include<Wire.h>
#include <SoftwareSerial.h>

const int MPU_ADDR = 0x68;    
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;   
float angleAcX, angleAcY, angleAcZ;
float angleGyX, angleGyY, angleGyZ;
float angleFiX, angleFiY, angleFiZ;

const float RADIAN_TO_DEGREE = 180 / 3.14159;  
const float DEG_PER_SEC = 32767 / 250;    // 1초에 회전하는 각도
const float ALPHA = 1 / (1 + 0.04);  // GyX, GyY, GyZ 값의 범위 : -32768 ~ +32767 (16비트 정수범위)


unsigned long now = 0;   // 현재 시간 저장용 변수
unsigned long past = 0;  // 이전 시간 저장용 변수
float dt = 0;           // 한 사이클 동안 걸린 시간 변수 

float averAcX, averAcY, averAcZ;
float averGyX, averGyY, averGyZ;
SoftwareSerial BTSerial(4,3);

void setup() {
  Serial.begin(9600);
  initSensor();
  caliSensor();   
  past = millis(); // past에 현재 시간 저장  
  BTSerial.begin(9600);
}

void loop() {
  getData(); 
  getDT();

  angleAcX = atan(AcY / sqrt(pow(AcX, 2) + pow(AcZ, 2)));
  angleAcX *= RADIAN_TO_DEGREE;
  angleAcY = atan(-AcX / sqrt(pow(AcY, 2) + pow(AcZ, 2)));
  angleAcY *= RADIAN_TO_DEGREE;
 
  
  // 가속도 현재 값에서 초기평균값을 빼서 센서값에 대한 보정
  angleGyX += ((GyX - averGyX) / DEG_PER_SEC) * dt;  //각속도로 변환
  angleGyY += ((GyY - averGyY) / DEG_PER_SEC) * dt;
  angleGyZ += ((GyZ - averGyZ) / DEG_PER_SEC) * dt;

  // 보다 정확한 값을 위해 상보필터 사용, 상보필터 처리를 위한 임시각도 저장
  float angleTmpX = angleFiX + angleGyX * dt;
  float angleTmpY = angleFiY + angleGyY * dt;
  float angleTmpZ = angleFiZ + angleGyZ * dt;

  // (상보필터 값 처리) 임시 각도에 0.96가속도 센서로 얻어진 각도 0.04의 비중을 두어 현재 각도를 구함.
  angleFiX = ALPHA * angleTmpX + (1.0 - ALPHA) * angleAcX;
  angleFiY = ALPHA * angleTmpY + (1.0 - ALPHA) * angleAcY;
  angleFiZ = angleGyZ;    // Z축은 자이로 센서만을 이용하열 구함.
  
  //Serial.print("\t FilteredX:");
  //Serial.print(angleFiX);
  Serial.print("\t FilteredY:");  // 상체의 각도 측정에 필요한 y축만 출력함.
  Serial.println(angleFiY);
  //Serial.print("\t FilteredZ:");
  //Serial.println(angleFiZ);
 
  
  //BTSerial.print("\t FilteredX:");
  //BTSerial.print(angleFiX);
  //BTSerial.print("\t FilteredY:");
  BTSerial.println(angleFiY);
  //BTSerial.print("\t FilteredZ:");
  //BTSerial.println(angleFiZ); 


  delay(500);
  
}


void initSensor() {
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);   // I2C 통신용 어드레스(주소)
  Wire.write(0x6B);        
  Wire.write(0);
  Wire.endTransmission(true);
}

void getData() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);   
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);  // AcX 주소 이후의 14byte의 데이터를 요청
  AcX = Wire.read() << 8 | Wire.read(); //두 개의 나뉘어진 바이트를 하나로 이어 붙여서 각 변수에 저장
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  Tmp = Wire.read() << 8 | Wire.read();
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();
}

// loop 한 사이클동안 걸리는 시간을 알기위한 함수
void getDT() {
  now = millis();   
  dt = (now - past) / 1000.0;  
  past = now;
}

// 센서의 초기값을 10회 정도 평균값으로 구하여 저장하는 함수
void caliSensor() {
  float sumAcX = 0 , sumAcY = 0, sumAcZ = 0;
  float sumGyX = 0 , sumGyY = 0, sumGyZ = 0;
  getData();
  for (int i=0;i<10;i++) {
    getData();
    sumAcX+=AcX;  sumAcY+=AcY;  sumAcZ+=AcZ;
    sumGyX+=GyX;  sumGyY+=GyY;  sumGyZ+=GyZ;
    delay(50);
  }
  averAcX=sumAcX/10;  averAcY=sumAcY/10;  averAcZ=sumAcY/10;
  averGyX=sumGyX/10;  averGyY=sumGyY/10;  averGyZ=sumGyZ/10;
}
