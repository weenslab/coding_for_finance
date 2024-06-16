#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Pin I2C
#define I2C_SDA 33
#define I2C_SCL 32

// Address I2C OLED
#define OLED_ADDR 0x3C

// Deklarasi object untuk OLED display
Adafruit_SSD1306 oled(128, 64, &Wire, -1);

// SSID dan password Wi-Fi AP
const char ssid[] = "<your_wifi_name>";
const char pass[] = "<yuor_wifi_password>";

// Koneksi HTTP, request, dan response
HTTPClient http;
const String currentURL = "http://api.coindesk.com/v1/bpi/currentprice/BTC.json";
const String historyURL = "http://api.coindesk.com/v1/bpi/historical/close.json";
String response = "";

void setup()
{
  // Setup serial communication
  Serial.begin(115200);

  // Inisialisasi pin I2C
  Wire.begin(I2C_SDA, I2C_SCL, 400000);
  
  // Inisialisasi OLED
  oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  oled.clearDisplay();
  
  // Connect to a WiFi access point
  Serial.printf("Connecting to %s ...\n", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi connect failed! Rebooting...\n");
    delay(1000);
    ESP.restart();
  }
  Serial.printf("Connected\n");
  Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
}

void loop()
{
  // Request data hari ini
  http.begin(currentURL);
  http.GET();
  response = http.getString();
  http.end();

  // Parse JSON
  StaticJsonDocument<2000> doc;
  DeserializationError error = deserializeJson(doc, response);
  if (error)
  {
    Serial.printf("JSON parseObject() failed\n");
    return;
  }  

  // Ambil nilai time dan price hari ini
  String time = doc["time"]["updated"].as<String>();
  double price = doc["bpi"]["USD"]["rate_float"].as<double>();

  // Request data kemarin
  http.begin(historyURL);
  http.GET();
  response = http.getString();
  http.end();

  // Parse JSON
  StaticJsonDocument<2000> history_doc;
  DeserializationError history_error = deserializeJson(history_doc, response);
  if (history_error)
  {
    Serial.printf("JSON parseObject() failed\n");
    return;
  }  

  // Ambil nilai price kemarin
  JsonObject bpi_obj = history_doc["bpi"].as<JsonObject>();
  double yesterday_price;
  for (JsonPair kv : bpi_obj)
    yesterday_price = kv.value().as<double>();

  // Hitung persentase perubahan harga
  bool is_up = price > yesterday_price;
  double percent_change = is_up ? ((price-yesterday_price)/yesterday_price)*100 : ((yesterday_price-price)/yesterday_price)*100;
  String sign = is_up ? "+" : "";

  // Format text
  String txt0 = "BTC/USD " + sign + String(percent_change, 1) + "%";
  String txt1 = String(price, 2);
  String txt2 = "Last updated " + time.substring(13, 18);

  // Tampilkan hasil
  oled.clearDisplay();
  oled.setTextColor(WHITE);
  oled.setTextSize(1);
  print_center(txt0, 0, 0);
  oled.setTextSize(2);
  print_center(txt1, 0, 18);
  oled.setTextSize(1);
  print_center(txt2, 0, 50);
  oled.display();

  // Delay
  delay(1000);
}

// Fungsi untuk print text ke OLED dengan alignment center
void print_center(const String str, int x, int y)
{
  int16_t x1, y1;
  uint16_t w, h;
  oled.getTextBounds(str, x, y, &x1, &y1, &w, &h);
  oled.setCursor((x-w/2)+(128/2), y);
  oled.print(str);
}
