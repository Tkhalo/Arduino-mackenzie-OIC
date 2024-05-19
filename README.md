# Sistema de Gestão Inteligente de Resíduos Baseado em IoT

Este projeto apresenta um sistema de gestão de resíduos baseado em IoT, projetado para melhorar a limpeza urbana e a saúde pública através de pontos de coleta inteligentes. Esses pontos são equipados com sensores IoT para a separação automática de resíduos recicláveis e não recicláveis, fornecendo feedback imediato aos usuários sobre a segregação adequada dos resíduos e notificando os serviços de coleta quando estiverem cheios.

## Índice
- [Sistema de Gestão Inteligente de Resíduos Baseado em IoT](#sistema-de-gestão-inteligente-de-resíduos-baseado-em-iot)
	- [Índice](#índice)
	- [Introdução](#introdução)
	- [Componentes de Hardware](#componentes-de-hardware)
	- [Componentes de Software](#componentes-de-software)
	- [Arquitetura do Sistema](#arquitetura-do-sistema)
	- [Instruções de Configuração](#instruções-de-configuração)
		- [Configuração do Hardware](#configuração-do-hardware)
		- [Configuração do Software](#configuração-do-software)
	- [Explicação do Código](#explicação-do-código)
		- [Código do Arduino](#código-do-arduino)
		- [Código do Servidor Flask](#código-do-servidor-flask)
	- [Uso](#uso)
	- [Resultados](#resultados)
	- [Referências](#referências)

## Introdução

São Paulo enfrenta um desafio crítico com o descarte inadequado de resíduos, representando uma ameaça significativa à saúde pública. O lixo descartado incorretamente cria um ambiente propício para a proliferação de insetos, ratos e escorpiões, e tem sido associado ao recente aumento nos casos de dengue. Este projeto apresenta um sistema de gestão de resíduos baseado em IoT para melhorar a limpeza urbana e a saúde pública através de pontos de coleta inteligentes. Esses pontos são equipados com sensores IoT para a separação automática de resíduos recicláveis e não recicláveis, fornecendo feedback imediato aos usuários sobre a segregação adequada dos resíduos e notificando os serviços de coleta quando estiverem cheios. Uma página web aumenta a participação da comunidade e a educação sobre resíduos, visando melhorar a limpeza e a responsabilidade ambiental.

## Componentes de Hardware

- **WeMos D1 Mini Lite**: Um microcontrolador baseado no chip ESP-8285 com Wi-Fi integrado.
- **Sensor Ultrassônico HC-SR04**: Usado para medir o nível de resíduos nas lixeiras.
- **Módulo LED RGB KY-016**: Atua como indicador visual para o nível de enchimento das lixeiras.

## Componentes de Software

- **Arduino IDE**: Para programar o WeMos D1 Mini Lite.
- **Visual Studio Code com PlatformIO**: Para desenvolvimento e depuração.
- **Flask**: Um micro framework para Python usado em um servidor na Huawei Cloud.
- **MQTT**: Um protocolo de mensagens leve usado para a comunicação entre os dispositivos IoT e o servidor.

## Arquitetura do Sistema

O sistema consiste em lixeiras inteligentes equipadas com um sensor ultrassônico e um módulo LED RGB. O sensor mede o nível de resíduos e o LED indica o status de enchimento. O microcontrolador WeMos D1 Mini Lite processa os dados do sensor e os envia para um servidor Flask hospedado na Huawei Cloud via MQTT. O servidor processa os dados e atualiza uma interface web para refletir o status das lixeiras.

## Instruções de Configuração

### Configuração do Hardware

1. **Conecte o Sensor Ultrassônico HC-SR04** ao WeMos D1 Mini Lite:
   - VCC ao 3.3V
   - GND ao GND
   - Pino Trigger ao D7
   - Pino Echo ao D6

2. **Conecte o Módulo LED RGB KY-016**:
   - Cátodo Comum ao GND
   - Vermelho ao D4
   - Verde ao D3

### Configuração do Software

1. **Clone o Repositório**:
   ```sh
   git clone https://github.com/Tkhalo/Arduino-mackenzie-OIC.git
   cd Arduino-mackenzie-OIC
   ```

2. **Código do Arduino**:

- Abra arduino_code.ino no Arduino IDE ou no VSCode com PlatformIO.
- Atualize as credenciais de Wi-Fi (ssid e password) e a URL do servidor no código.
- Faça o upload do código para o WeMos D1 Mini Lite.

3. Servidor Flask:
- Instale as bibliotecas Python necessárias:
```sh
pip install flask paho-mqtt flask-cors
```
- Execute o servidor Flask
```sh
python3 server.py
```
## Explicação do Código
### Código do Arduino

```cpp
#include <Arduino.h>
#include <Ultrasonic.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Configurações do Wi-Fi
const char* ssid = "SEU_SSID";
const char* password = "SUA_SENHA";

// Endereço do servidor Flask
const char* serverUrl = "http://SEU_SERVIDOR:5000/update";

// Pinos do HC-SR04
const uint8_t pin_trigger = D7;
const uint8_t pin_echo = D6;

// Pinos do LED
const uint8_t pin_led_red = D4;
const uint88 pin_led_green = D3;

Ultrasonic ultrasonic(pin_trigger, pin_echo);
WiFiClient wifiClient;

// Variáveis para controle dos LEDs e medição de distância
unsigned long previousMillis = 0;
unsigned long startMillis = 0;
const long interval = 500;           // Intervalo em que o LED vermelho pisca
const long stabilityInterval = 30000;  // Intervalo para considerar uma condição estável (30 segundos)
bool isRedLEDOn = false;

void sendFullTrashMessage();

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

    if (distance < 38) {
        if (startMillis == 0) {
            startMillis = currentMillis;
        } else if (currentMillis - startMillis >= stabilityInterval) {
            if (currentMillis - previousMillis >= interval) {
                previousMillis = currentMillis;
                digitalWrite(pin_led_red, !digitalRead(pin_led_red));
            }
            digitalWrite(pin_led_green, LOW);
            if (!isRedLEDOn) {
                sendFullTrashMessage();
                isRedLEDOn = true;
            }
        }
    } else {
        if (currentMillis - startMillis >= stabilityInterval) {
            digitalWrite(pin_led_green, HIGH);
            digitalWrite(pin_led_red, LOW);
        }
        startMillis = 0;
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

    delay(1000);
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
``` 

### Código do Servidor Flask

```python
from flask import Flask, request, jsonify, render_template
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
from flask_cors import CORS
import threading

app = Flask(__name__)
CORS(app)

# Configurações do MQTT
mqtt_server = "ENDEREÇO_DO_SERVIDOR"
mqtt_topic = "sensores/ultrassonico"
mqtt_user = "USUARIO"
mqtt_password = "SENHA"

distance_data = {'distance': None, 'status': 'ok'}

def on_connect(client, userdata, flags, rc):
    print(f"Conectado com código de resultado {rc}")
    client.subscribe(mqtt_topic)

def on_message(client, userdata, msg):
    global distance_data
    message = msg.payload.decode()
    print(f"Mensagem recebida: {message}")
    if message == "full":
        distance_data['status'] = 'full'
    else:
        try:
            distance_data['distance'] = float(message)
            distance_data['status'] = 'ok'
        except ValueError:
            print(f"Valor de distância inválido: {message}")

mqtt_client = mqtt.Client()
mqtt_client.username_pw_set(mqtt_user, mqtt_password)
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

mqtt_client.connect(mqtt_server, 1883, 60)
mqtt_client.loop_start()

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/update', methods=['POST'])
def update_distance():
    global distance_data
    distance = request.form.get('distance')
    if distance is not None:
        if distance == "full":
            distance_data['status'] = 'full'
        else:
            try:
                distance_data['distance'] = float(distance)
                distance_data['status'] = 'ok'
            except ValueError:
                return 'Valor de distância inválido', 400
        publish_mqtt(distance)
        return '', 204
    else:
        return 'Dados inválidos', 400

@app.route('/data', methods=['GET'])
def get_distance():
    if distance_data['distance'] is None:
        return jsonify({"error": "No data available"}), 500
    return jsonify(distance_data)

def publish_mqtt(distance):
    auth = {'username': mqtt_user, 'password': mqtt_password}
    publish.single(mqtt_topic, payload=str(distance), hostname=mqtt_server, auth=auth)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
```

## Uso

1. **Alimente os componentes de hardware**.
2. **Execute o código do Arduino** no WeMos D1 Mini Lite.
3. **Inicie o servidor Flask** na sua instância Huawei Cloud ECS.
4. **Monitore o status** das lixeiras através da interface web fornecida pela aplicação Flask.

## Resultados

- O LED RGB muda para vermelho quando a lixeira está quase cheia e envia uma notificação ao servidor.
- O servidor processa os dados e atualiza a interface web em tempo real.
- Tempos de resposta e métricas de desempenho são documentados e grafados para análise.

## Referências

- [Datasheet do HC-SR04](https://www.electroschematics.com/wp-content/uploads/2013/07/HC-SR04-datasheet-version-2.pdf)
- [Documentação do WeMos D1 Mini Lite](https://www.wemos.cc/en/latest/d1/d1_mini_lite.html)
- [Documentação do Flask](https://flask.palletsprojects.com/)
- [Documentação do MQTT](http://mqtt.org/)
