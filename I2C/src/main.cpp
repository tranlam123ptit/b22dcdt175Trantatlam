#include <Wire.h>
#include <BH1750.h>

#define I2C_SDA 21
#define I2C_SCL 22

BH1750 lightMeter;

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
  
  Serial.println("BH1750 đã sẵn sàng!");
}

void loop() {
  float lux = lightMeter.readLightLevel();
  
  Serial.print("Ánh sáng: ");
  Serial.print(lux);
  Serial.println(" lx");
  
  if (lux < 50) {
    Serial.println("Rất tối");
  } else if (lux < 200) {
    Serial.println("Tối");
  } else if (lux < 500) {
    Serial.println("Trung bình");
  } else if (lux < 1000) {
    Serial.println("Sáng");
  } else {
    Serial.println("Rất sáng");
  }
  
  Serial.println();
  delay(1000);
}