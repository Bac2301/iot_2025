#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const char* ssid = "Galaxy";
const char* password = "02345678";

const char* serverAddress = "http://192.168.87.176:3000/data";

LiquidCrystal_I2C lcd(0x27, 16, 2); 

#define led 12
#define but 15 
#define DHTPIN 27
#define DHTTYPE DHT22
#define soil 32
#define light 33

DHT dht(DHTPIN, DHTTYPE);
WiFiServer server(80);

bool isLedOn = false; 

void setup() {
    pinMode(led, OUTPUT);
    digitalWrite(led, LOW);
    pinMode(but, INPUT_PULLUP); 

    Serial.begin(115200);
    dht.begin();
    
    Wire.begin(21, 22); // SDA = 21, SCL = 22
    lcd.clear();
    lcd.init();       
    lcd.backlight();  
    lcd.setCursor(3, 0);
    lcd.print("Hello World ");
    lcd.setCursor(0, 1);
    lcd.print("UTC2 University");

    // Kết nối WiFi
    WiFi.begin(ssid, password);
    Serial.print("Đang kết nối WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("Đã kết nối WiFi!");
    Serial.print("Địa chỉ IP của ESP32: ");
    Serial.println(WiFi.localIP());

    server.begin();
    Serial.println("Server HTTP đã khởi động trên cổng 80");
}

void send_data() {
    // Đọc dữ liệu từ DHT22
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
        Serial.println(" Lỗi! Không đọc được dữ liệu từ DHT22.");
        return;
    }
    // độ ẩm đất
    int soilvalue = analogRead(soil);
    float soilpercent = map(soilvalue, 0, 4095, 100, 0);
    //cường độ ánh sáng
    int lightvalue = analogRead(light);
    float lightpercent = map(lightvalue, 0, 4095, 0, 100);

    Serial.print(" Nhiệt độ: ");
    Serial.print(temperature);
    Serial.println(" °C");


    Serial.print("Độ ẩm không khí: ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.print("Độ ẩm đất: ");
    Serial.print(soilpercent);
    Serial.println(" %");

    Serial.print("Cường độ ánh sáng: ");
    Serial.print(lightpercent);
    Serial.println(" %");

    Serial.println("------------------------");

    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(temperature);
    //lcd.print((char)223); //  "°" 
    lcd.print(" RH:");
    lcd.print(humidity);
  
    lcd.setCursor(0, 1);
    lcd.print("VWC:");
    lcd.print(soilpercent);

    lcd.print(" LX:");
    lcd.print(lightpercent);

    // send to đến server.js
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverAddress);
        http.addHeader("Content-Type", "application/json");
        http.setTimeout(5000); // Tăng timeout 

        DynamicJsonDocument doc(200);
        doc["temperature"] = temperature;
        doc["humidity"] = humidity;
        doc["soilpercent"] = soilpercent;
        doc["lightpercent"] = lightpercent;

        String jsonData;
        serializeJson(doc, jsonData);

        Serial.print("Dữ liệu JSON gửi đi: ");
        Serial.println(jsonData);

        int httpResponseCode = http.POST(jsonData);
        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Gửi dữ liệu thành công!");
            Serial.print("Mã phản hồi: ");
            Serial.println(httpResponseCode);
            Serial.print("Phản hồi từ server: ");
            Serial.println(response);
        } else {
            Serial.print("Lỗi khi gửi dữ liệu: ");
            Serial.println(httpResponseCode);
        }

        http.end();
    } else {
        Serial.println("WiFi không kết nối!");
    }
}

void handleHttpRequest() {
    WiFiClient client = server.available();
    if (client) {
        Serial.println("New Client.");
        String currentLine = "";
        String request = ""; 
        unsigned long timeout = millis();

        while (client.connected() && millis() - timeout < 1000) { // Timeout 1 giây
            if (client.available()) {
                char c = client.read();
                Serial.write(c);
                currentLine += c;
                if (c == '\n') {
                    if (request == "") {
                        request = currentLine;
                    }
                    if (currentLine.length() == 2) { 
                        if (request.startsWith("GET /led/on ")) {
                            digitalWrite(led, HIGH);
                            isLedOn = true;
                            Serial.println("LED bật bởi giao diện web");
                            Serial.print("Trạng thái GPIO 12: ");
                            Serial.println(digitalRead(led));
                        } else if (request.startsWith("GET /led/off ")) {
                            digitalWrite(led, LOW);
                            isLedOn = false;
                            Serial.println("LED tắt bởi giao diện web");
                            Serial.print("Trạng thái GPIO 12: ");
                            Serial.println(digitalRead(led));
                        }
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();
                        client.println("<!DOCTYPE html>");
                        client.println("<html lang=\"vi\">");
                        client.println("<head>");
                        client.println("<meta charset=\"UTF-8\">");
                        client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
                        client.println("<title>Điều khiển LED - ESP32</title>");
                        client.println("<style>");
                        client.println("body { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f9; padding: 20px; }");
                        client.println("h1 { color: #333; }");
                        client.println(".button { padding: 10px 20px; margin: 10px; font-size: 16px; border: none; border-radius: 5px; cursor: pointer; }");
                        client.println(".button-on { background-color: #4CAF50; color: white; }");
                        client.println(".button-off { background-color: #f44336; color: white; }");
                        client.println(".button:hover { opacity: 0.8; }");
                        client.println("</style>");
                        client.println("</head>");
                        client.println("<body>");
                        client.println("<h1>Điều khiển LED - ESP32</h1>");
                        client.println("<p>Trạng thái LED: <strong>" + String(isLedOn ? "Bật" : "Tắt") + "</strong></p>");
                        client.println("<a href=\"/led/on\"><button class=\"button button-on\">Bật LED</button></a>");
                        client.println("<a href=\"/led/off\"><button class=\"button button-off\">Tắt LED</button></a>");
                        client.println("</body>");
                        client.println("</html>");
                        client.println();
                        break;
                    }
                    currentLine = "";
                }
            }
        }
        client.stop();
        Serial.println("Client Disconnected.");
    }
}

void handleButton() {
    static bool lastButtonState = HIGH; 
    bool buttonState = digitalRead(but); 

    if (lastButtonState == HIGH && buttonState == LOW) {
        isLedOn = !isLedOn;
        digitalWrite(led, isLedOn ? HIGH : LOW);
        Serial.println(isLedOn ? "LED bật bởi nút bấm vật lý" : "LED tắt bởi nút bấm vật lý");
        Serial.print("Trạng thái GPIO 12: ");
        Serial.println(digitalRead(led));
        delay(50); 
    }
    lastButtonState = buttonState; 
}

void loop() {
    static unsigned long lastSensorUpdate = 0;
    if (millis() - lastSensorUpdate >= 2000) {
        send_data();
        lastSensorUpdate = millis();
    }
    handleHttpRequest();
    handleButton();

}