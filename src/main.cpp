#include <Arduino.h>
#include <Ultrasonic.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>


// Configurações do Wi-Fi
const char* ssid = "wifi";
const char* password = "senha";

// Endereço do servidor Flask
const char* serverUrl = "http://EIP:5000/update";

// Pinos do HC-SR04
const uint8_t pin_trigger = D7;
const uint8_t pin_echo = D6;

// Pinos do LED
const uint8_t pin_led_red = D4;
const uint8_t pin_led_green = D3;

Ultrasonic ultrasonic(pin_trigger, pin_echo);
WiFiClient wifiClient;

// Variáveis para controle dos LEDs e medição de distância
unsigned long previousMillis = 0;
unsigned long startMillis = 0;
const long interval = 500;           // Intervalo em que o LED vermelho pisca
const long stabilityInterval = 30000;  // Intervalo para considerar uma condição estável (30 segundos)
bool isRedLEDOn = false;

void sendFullTrashMessage();  // Declaração antecipada da função

void setup() {
    Serial.begin(9600);
    pinMode(pin_led_red, OUTPUT);
    pinMode(pin_led_green, OUTPUT);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" Conectado ao WiFi!");
}

void checkWifiAndReconnect() {
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.reconnect();
        Serial.println(" Tentando reconectar ao WiFi...");
    }
}

void loop() {
    checkWifiAndReconnect();
    
    unsigned long currentMillis = millis();
    int distance = ultrasonic.read();
    Serial.print("Distância em CM: ");
    Serial.println(distance);

    // Lógica de controle dos LEDs com base na distância
    if (distance < 38) {
        if (startMillis == 0) {
            startMillis = currentMillis;  // Iniciar contagem de tempo
        } else if (currentMillis - startMillis >= stabilityInterval) {
            // Condição estável por mais de 60 segundos
            if (currentMillis - previousMillis >= interval) {
                previousMillis = currentMillis;
                digitalWrite(pin_led_red, !digitalRead(pin_led_red));
            }
            digitalWrite(pin_led_green, LOW);
            if (!isRedLEDOn) {
                sendFullTrashMessage();  // Enviar mensagem para o servidor Flask
                isRedLEDOn = true;
            }
        }
    } else {
        if (currentMillis - startMillis >= stabilityInterval) {
            // Distância maior que 38 cm por mais de 30 segundos
            digitalWrite(pin_led_green, HIGH);
            digitalWrite(pin_led_red, LOW);
        }
        startMillis = 0;  // Resetar contagem de tempo
        isRedLEDOn = false;
    }

    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(wifiClient, serverUrl);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        String httpRequestData = "distance=" + String(distance);
        int httpResponseCode = http.POST(httpRequestData);
        Serial.print("Código de Resposta HTTP: ");
        Serial.println(httpResponseCode);
        http.end();
    }

    delay(1000); // Espera entre leituras para não sobrecarregar o servidor e o sensor
}

void sendFullTrashMessage() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(wifiClient, serverUrl);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        String httpRequestData = "distance=full";
        int httpResponseCode = http.POST(httpRequestData);
        Serial.print("Código de Resposta HTTP para lixeira cheia: ");
        Serial.println(httpResponseCode);
        http.end();
    }
}
