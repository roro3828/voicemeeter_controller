int sensorPin = 5;
char t;
void setup() {
  t='A';
  Serial.begin(9600);
}
void loop(){
  unsigned long sensorValue = analogRead(sensorPin);
  if(Serial.available()>0){
    t=Serial.read();
  }
  
  Serial.write(0xFF);
  Serial.write(0x01);
  Serial.write((sensorValue>>0)&0xFF);
  Serial.write((sensorValue>>8)&0xFF);
  Serial.write((sensorValue>>16)&0xFF);
  Serial.write((sensorValue>>24)&0xFF);

  delay(100);
}