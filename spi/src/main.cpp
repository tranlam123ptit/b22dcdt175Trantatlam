#include <Arduino.h>
#include <SPI.h>

#define BMP_CS   5
#define SPI_CLOCK 1000000 

#define BMP280_REG_ID         0xD0
#define BMP280_REG_RESET      0xE0
#define BMP280_REG_STATUS     0xF3
#define BMP280_REG_CTRL_MEAS  0xF4
#define BMP280_REG_CONFIG     0xF5
#define BMP280_REG_PRESS_MSB  0xF7
#define BMP280_REG_TEMP_MSB   0xFA

struct {
  uint16_t dig_T1;
  int16_t dig_T2, dig_T3;
  uint16_t dig_P1;
  int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
} bmp280_calib;

void setup() {
  Serial.begin(115200);
  SPI.begin();
  pinMode(BMP_CS, OUTPUT);
  digitalWrite(BMP_CS, HIGH);
  
  initBMP280();
  readCalibrationData();
}

void loop() {
  float temperature = readTemperature();
  float pressure = readPressure();
  
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  
  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println(" Pa");
  
  delay(2000);
}

void initBMP280() {
  writeRegister(BMP280_REG_RESET, 0xB6);
  delay(100);
  
  writeRegister(BMP280_REG_CONFIG, 0x14); 
  writeRegister(BMP280_REG_CTRL_MEAS, 0x57);
}

uint8_t readRegister(uint8_t reg) {
  digitalWrite(BMP_CS, LOW);
  SPI.transfer(reg);
  uint8_t value = SPI.transfer(0);
  digitalWrite(BMP_CS, HIGH);
  return value;
}

void writeRegister(uint8_t reg, uint8_t value) {
  digitalWrite(BMP_CS, LOW);
  SPI.transfer(reg & 0x7F); 
  SPI.transfer(value);
  digitalWrite(BMP_CS, HIGH);
}

void readCalibrationData() {
  bmp280_calib.dig_T1 = read16(0x88);
  bmp280_calib.dig_T2 = readS16(0x8A);
  bmp280_calib.dig_T3 = readS16(0x8C);
  
  bmp280_calib.dig_P1 = read16(0x8E);
  bmp280_calib.dig_P2 = readS16(0x90);
  bmp280_calib.dig_P3 = readS16(0x92);
  bmp280_calib.dig_P4 = readS16(0x94);
  bmp280_calib.dig_P5 = readS16(0x96);
  bmp280_calib.dig_P6 = readS16(0x98);
  bmp280_calib.dig_P7 = readS16(0x9A);
  bmp280_calib.dig_P8 = readS16(0x9C);
  bmp280_calib.dig_P9 = readS16(0x9E);
}

uint16_t read16(uint8_t reg) {
  return (readRegister(reg) << 8) | readRegister(reg + 1);
}

int16_t readS16(uint8_t reg) {
  return (int16_t)read16(reg);
}

float readTemperature() {
  uint8_t buffer[3];
  digitalWrite(BMP_CS, LOW);
  SPI.transfer(BMP280_REG_TEMP_MSB | 0x80);
  for(int i = 0; i < 3; i++) {
    buffer[i] = SPI.transfer(0);
  }
  digitalWrite(BMP_CS, HIGH);
  
  int32_t adc_T = ((uint32_t)buffer[0] << 12) | ((uint32_t)buffer[1] << 4) | ((buffer[2] >> 4) & 0x0F);
  
  int32_t var1, var2, T;
  var1 = ((((adc_T >> 3) - ((int32_t)bmp280_calib.dig_T1 << 1))) * ((int32_t)bmp280_calib.dig_T2)) >> 11;
  var2 = (((((adc_T >> 4) - ((int32_t)bmp280_calib.dig_T1)) * ((adc_T >> 4) - ((int32_t)bmp280_calib.dig_T1))) >> 12) * ((int32_t)bmp280_calib.dig_T3)) >> 14;
  
  int32_t t_fine = var1 + var2;
  T = (t_fine * 5 + 128) >> 8;
  
  return T / 100.0;
}

float readPressure() {
  readTemperature(); 
  
  uint8_t buffer[3];
  digitalWrite(BMP_CS, LOW);
  SPI.transfer(BMP280_REG_PRESS_MSB | 0x80);
  for(int i = 0; i < 3; i++) {
    buffer[i] = SPI.transfer(0);
  }
  digitalWrite(BMP_CS, HIGH);
  
  int32_t adc_P = ((uint32_t)buffer[0] << 12) | ((uint32_t)buffer[1] << 4) | ((buffer[2] >> 4) & 0x0F);
  
  
  return adc_P; 
}