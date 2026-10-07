#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


// =====================================================
// CONFIGURAÇÃO DO WI-FI
// =====================================================

const char* WIFI_SSID = "Tomelin_";
const char* WIFI_PASSWORD = "1hora=50pila";


// =====================================================
// CONFIGURAÇÃO DO DHT22
// =====================================================

#define DHT_PIN 4
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);


// =====================================================
// CONFIGURAÇÃO DO OLED
// =====================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define OLED_SDA 21
#define OLED_SCL 22

#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);


// =====================================================
// SERVIDOR HTTP
// =====================================================

WebServer server(80);


// =====================================================
// VARIÁVEIS DOS SENSORES
// =====================================================

float temperatura = 0.0;
float umidade = 0.0;


// =====================================================
// LER DHT22
// =====================================================

bool lerSensor() {

  float novaTemperatura = dht.readTemperature();
  float novaUmidade = dht.readHumidity();

  // Verifica se houve erro
  if (isnan(novaTemperatura) || isnan(novaUmidade)) {

    Serial.println("ERRO: Falha ao ler o DHT22!");

    return false;
  }

  temperatura = novaTemperatura;
  umidade = novaUmidade;

  return true;
}


// =====================================================
// ATUALIZAR OLED
// =====================================================

void atualizarOLED() {

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  // -----------------------------
  // Título
  // -----------------------------

  display.setTextSize(1);

  display.setCursor(0, 0);

  display.println("ESP32 - DHT22");


  // -----------------------------
  // Temperatura
  // -----------------------------

  display.setCursor(0, 11);

  display.print("Temp: ");

  display.print(temperatura, 1);

  display.println(" C");


  // -----------------------------
  // Umidade
  // -----------------------------

  display.setCursor(0, 22);

  display.print("Umid: ");

  display.print(umidade, 1);

  display.println(" %");


  // Envia tudo para o OLED
  display.display();
}


// =====================================================
// ROTA PRINCIPAL
// GET /
// =====================================================

void paginaInicial() {

  String resposta;

  resposta += "ESP32 DHT22 API\n";
  resposta += "\n";
  resposta += "Endpoints:\n";
  resposta += "GET /dados\n";
  resposta += "GET /temperatura\n";
  resposta += "GET /umidade\n";

  server.send(
    200,
    "text/plain",
    resposta
  );
}


// =====================================================
// ROTA /dados
// GET /dados
// =====================================================

void obterDados() {

  // Tenta atualizar a leitura
  if (!lerSensor()) {

    server.send(
      500,
      "application/json",
      "{\"erro\":\"Falha ao ler o DHT22\"}"
    );

    return;
  }


  // Monta JSON

  String json = "{";

  json += "\"temperatura\":";
  json += String(temperatura, 2);

  json += ",";

  json += "\"umidade\":";
  json += String(umidade, 2);

  json += "}";


  // Responde HTTP

  server.send(
    200,
    "application/json",
    json
  );
}


// =====================================================
// ROTA /temperatura
// GET /temperatura
// =====================================================

void obterTemperatura() {

  if (!lerSensor()) {

    server.send(
      500,
      "application/json",
      "{\"erro\":\"Falha ao ler o DHT22\"}"
    );

    return;
  }

  String json = "{";

  json += "\"temperatura\":";
  json += String(temperatura, 2);

  json += "}";


  server.send(
    200,
    "application/json",
    json
  );
}


// =====================================================
// ROTA /umidade
// GET /umidade
// =====================================================

void obterUmidade() {

  if (!lerSensor()) {

    server.send(
      500,
      "application/json",
      "{\"erro\":\"Falha ao ler o DHT22\"}"
    );

    return;
  }

  String json = "{";

  json += "\"umidade\":";
  json += String(umidade, 2);

  json += "}";


  server.send(
    200,
    "application/json",
    json
  );
}


// =====================================================
// CONECTAR WI-FI
// =====================================================

void conectarWiFi() {

  Serial.println();
  Serial.println("==============================");
  Serial.println("CONEXAO WI-FI");
  Serial.println("==============================");

  Serial.print("Rede: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();

  Serial.println("Wi-Fi conectado!");

  Serial.print("IP do ESP32: ");

  Serial.println(
    WiFi.localIP()
  );

  Serial.print("RSSI: ");

  Serial.print(
    WiFi.RSSI()
  );

  Serial.println(" dBm");
}


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  delay(1000);


  // ===================================================
  // DHT22
  // ===================================================

  dht.begin();


  // ===================================================
  // OLED
  // ===================================================

  Wire.begin(
    OLED_SDA,
    OLED_SCL
  );

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        OLED_ADDRESS
      )) {

    Serial.println("ERRO: OLED nao encontrado!");

    while (true) {

      delay(1000);
    }
  }


  // Tela inicial

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 0);

  display.println("ESP32 DHT22");

  display.setCursor(0, 12);

  display.println("Conectando WiFi...");

  display.display();


  // ===================================================
  // WI-FI
  // ===================================================

  conectarWiFi();


  // ===================================================
  // PRIMEIRA LEITURA
  // ===================================================

  if (lerSensor()) {

    atualizarOLED();

  } else {

    display.clearDisplay();

    display.setTextSize(1);

    display.setCursor(0, 0);

    display.println("DHT22");

    display.setCursor(0, 12);

    display.println("Erro no sensor");

    display.display();
  }


  // ===================================================
  // ROTAS HTTP
  // ===================================================

  server.on(
    "/",
    HTTP_GET,
    paginaInicial
  );


  server.on(
    "/dados",
    HTTP_GET,
    obterDados
  );


  server.on(
    "/temperatura",
    HTTP_GET,
    obterTemperatura
  );


  server.on(
    "/umidade",
    HTTP_GET,
    obterUmidade
  );


  // ===================================================
  // INICIAR SERVIDOR
  // ===================================================

  server.begin();

  Serial.println();
  Serial.println("==============================");
  Serial.println("SERVIDOR HTTP INICIADO");
  Serial.println("==============================");

  Serial.print("Acesse: http://");

  Serial.print(
    WiFi.localIP()
  );

  Serial.println("/dados");
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  // Processa requisições HTTP

  server.handleClient();


  // Atualiza sensor/OLED a cada 2 segundos

  static unsigned long ultimaLeitura = 0;

  unsigned long agora = millis();

  if (agora - ultimaLeitura >= 2000) {

    ultimaLeitura = agora;

    if (lerSensor()) {

      atualizarOLED();


      // Mostra também no Serial

      Serial.print("Temperatura: ");

      Serial.print(
        temperatura,
        1
      );

      Serial.print(" C | Umidade: ");

      Serial.print(
        umidade,
        1
      );

      Serial.println(" %");
    }
  }
}