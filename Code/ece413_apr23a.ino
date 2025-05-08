#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>

// --- WIFI CONFIG ---
const char* ssid = "Etherium";
const char* password = "ladylink";

// --- SPI & ADC CONFIG ---
#define CS_PIN        9
#define SPI_SCK       36
#define SPI_MOSI      35
#define SPI_MISO      37
#define V_REF         4.098
#define ADC_RANGE_BYTE 0b10000110
#define MODE_CONTROL_BYTE 0b10000000

WebServer server(80);
float voltage = 0.0;
float charge = 0.0; // Placeholder for actual calculation
float particleSize = 0.0; // Placeholder for actual calculation

// --- Function Declarations ---
void configureADC();
uint16_t readMAX1032();
void calculateDerivedValues();
void handle_OnConnect();

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

  configureADC();

  server.on("/", HTTP_GET, handle_OnConnect);
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();
}

// --- ADC CONFIG & READ ---
void configureADC() {
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(MODE_CONTROL_BYTE);
  digitalWrite(CS_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(CS_PIN, LOW);
  SPI.transfer(ADC_RANGE_BYTE);
  digitalWrite(CS_PIN, HIGH);
  delayMicroseconds(100);
}

uint16_t readMAX1032() {
  uint8_t response[3] = {0};

  digitalWrite(CS_PIN, LOW);
  SPI.transfer(MODE_CONTROL_BYTE);
  response[0] = SPI.transfer(0x00);
  response[1] = SPI.transfer(0x00);
  response[2] = SPI.transfer(0x00);
  digitalWrite(CS_PIN, HIGH);

  return (((response[1] << 8) | response[2]) >> 2) & 0x3FFF;
}

// --- PARTICLE MATH (mocked logic for now) ---
void calculateDerivedValues() {
  uint16_t adcRaw = readMAX1032();
  voltage = (adcRaw / 16383.0) * (3 * V_REF);
  charge = voltage * 1e-12; // Placeholder: pretend 1V = 1pC
  // Particle size estimation formula can depend on material; mock for now:
  particleSize = pow(charge, 1.0/3.0) * 1e6; // microns, fake scaling
}

// --- WEB PAGE ---
void handle_OnConnect() {
  calculateDerivedValues();

  String page = "<html><head><title>Particle Viewer</title></head><body>";
  page += "<h1>Electrostatic Dust Analyzer</h1>";
  page += "<p><b>Voltage:</b> " + String(voltage, 4) + " V</p>";
  page += "<p><b>Charge:</b> " + String(charge, 6) + " C (simulated)</p>";
  page += "<p><b>Particle Size:</b> " + String(particleSize, 2) + " μm (mock)</p>";

  // Placeholder: Add dropdown for particle types here
  // page += "<p>Particle Type: [TBD]</p>";

  page += "</body></html>";
  server.send(200, "text/html", page);
}
