#include <DHT.h>
#define DHTPIN 4
#define DHTTYPE DHT22
#define GAS_SENSOR_PIN A0  // Cảm biến MQ-135 nối với chân A0
#define LED 2

#define WINDOW_SIZE 5  // Kích thước cửa sổ WMA

DHT dht(DHTPIN, DHTTYPE);

// Lưu lịch sử PM2.5, Nhiệt độ và Độ ẩm
float pm25_history[WINDOW_SIZE] = {0};
float temp_history[WINDOW_SIZE] = {0};
float hum_history[WINDOW_SIZE] = {0};
int histIndex = 0;

// Tính WMA cho một dãy số
float computeWMA(float arr[], int size) {
  float numerator = 0;
  float denominator = 0;
  for (int i = 0; i < size; i++) {
    int weight = size - i;  // Trọng số cao hơn cho các giá trị gần nhất
    numerator += arr[i] * weight;
    denominator += weight;
  }
  return numerator / denominator;
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(LED, OUTPUT);
}

void loop() {
  delay(1000);

  // Đọc cảm biến DHT22
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("❌Failed to read from DHT Sensor!");
    return;
  }

  // Đọc cảm biến MQ-135 (PM2.5 thực tế)
  int gasLevel = analogRead(GAS_SENSOR_PIN);
  // Quy đổi giá trị analog sang µg/m³ cho PM2.5 (Công thức có thể thay đổi tùy thuộc vào cảm biến và điều kiện)
  float pm25 = map(gasLevel, 0, 1023, 0, 500);  // Bạn cần điều chỉnh theo cảm biến và môi trường

  // Cập nhật dữ liệu vào mảng WMA
  pm25_history[histIndex] = pm25;
  temp_history[histIndex] = t;
  hum_history[histIndex] = h;
  histIndex = (histIndex + 1) % WINDOW_SIZE;

  // Tính WMA cho PM2.5, Nhiệt độ và Độ ẩm
  float pm25_wma = computeWMA(pm25_history, WINDOW_SIZE);
  float temp_wma = computeWMA(temp_history, WINDOW_SIZE);
  float hum_wma = computeWMA(hum_history, WINDOW_SIZE);

  // In thông số
  Serial.println("=== DỮ LIỆU MÔI TRƯỜNG ===");
  Serial.print("Độ ẩm: "); Serial.print(h); Serial.println(" %");
  Serial.print("Nhiệt độ: "); Serial.print(t); Serial.println(" °C");
  Serial.print("PM2.5: "); Serial.print(pm25); Serial.println(" µg/m³");
  Serial.print("PM2.5 WMA: "); Serial.print(pm25_wma); Serial.println(" µg/m³");
  Serial.print("Nhiệt độ WMA: "); Serial.print(temp_wma); Serial.println(" °C");
  Serial.print("Độ ẩm WMA: "); Serial.print(hum_wma); Serial.println(" %");

  // Cảnh báo nếu PM2.5 WMA vượt quá mức cho phép
  if (pm25_wma > 150) {
    digitalWrite(LED, HIGH);
    Serial.println("⚠️Cảnh báo: Mức ô nhiễm cao!");
  } else {
    digitalWrite(LED, LOW);
  }

  delay(3000);
}