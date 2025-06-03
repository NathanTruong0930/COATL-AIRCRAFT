#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>

// === WiFi Configuration ===
const char* ssid = "Etherium";
const char* password = "ladylink";

WebServer server(80);

// === Pin Configuration ===
#define GAIN1_PIN 18
#define GAIN2_PIN 17
#define GAIN3_PIN 9
#define CS_PIN     8
#define SPI_SCK    36
#define SPI_MOSI   35
#define SPI_MISO   37

// === Constants ===
#define V_REF       4.098
//#define VIRTUAL_GND 4.0
#define MAX_ADC     16383
#define V_ADC_MAX   (3 * V_REF)
#define FEEDBACK_CAP 1e-10

// === Globals ===
bool manualGain = false;
int activeGain = 1;
uint16_t adcRaw = 0;
float voltage = 0.0, charge = 0.0;
String gainLabel = "Gain 1 (3.9x)";
String chargeStr = "";
String chargeSign = "";
float testVoltage = 4.5;
bool testMode = true, clippingWarning = false;

// === Serial Log ===
constexpr uint16_t LOG_DEPTH = 120;
String logBuf[LOG_DEPTH];
uint16_t logHead = 0;

void addLogLine(const String& ln) {
  logBuf[logHead] = ln;
  logHead = (logHead + 1) % LOG_DEPTH;
}

void handleLog() {
  String out;
  for (uint16_t i = 0; i < LOG_DEPTH; ++i) {
    uint16_t idx = (logHead + i) % LOG_DEPTH;
    if (logBuf[idx].length()) out += logBuf[idx] + "\n";
  }
  server.send(200, "text/plain", out);
}

// === Gain Switching ===
void setGain(int g) {
  digitalWrite(GAIN1_PIN, g == 1);
  digitalWrite(GAIN2_PIN, g == 2);
  digitalWrite(GAIN3_PIN, g == 3);
  activeGain = g;
  gainLabel = (g == 1) ? "Gain 1 (3.9x)" : (g == 2) ? "Gain 2 (15x)" : "Gain 3 (47x)";
}

void detectGain(float deltaV) {
  if (!manualGain) {
    if (abs(deltaV) <= 0.2) setGain(3);
    else if (abs(deltaV) <= 0.6) setGain(2);
    else setGain(1);
  }
  chargeSign = (deltaV >= 0) ? "+" : "-";
}

uint16_t readMAX1032() {
  uint8_t r[3] = {0};
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0b10000000);
  r[0] = SPI.transfer(0x00);
  r[1] = SPI.transfer(0x00);
  r[2] = SPI.transfer(0x00);
  digitalWrite(CS_PIN, HIGH);
  return (((r[1] << 8) | r[2]) >> 2) & 0x3FFF;
}

void calculateDerivedValues() {
  adcRaw = testMode ? (uint16_t)((testVoltage / V_ADC_MAX) * MAX_ADC + 0.5) : readMAX1032();
  voltage = (adcRaw / (float)MAX_ADC) * V_ADC_MAX;
  float deltaV = voltage /*- VIRTUAL_GND*/;
  detectGain(deltaV);
  charge = deltaV * FEEDBACK_CAP;
  chargeStr = chargeSign + String(abs(charge) * 1e12, 4) + " pC";
  clippingWarning = (voltage > /*VIRTUAL_GND +*/ 4.75);
}

// === Particle Estimates ===
String particleEstimate(float q, String t) {
  float minS, maxS;
  if (t == "Volcanic Ash") { minS = 1e-6; maxS = 1e-5; }
  else if (t == "Wildfire Smoke") { minS = 1e-8; maxS = 1e-6; }
  else if (t == "Mineral Dust") { minS = 7.7e-7; maxS = 2.34e-6; }
  else return "";

  float dMin = 2 * sqrt(abs(q) / (4 * PI * maxS));
  float dMax = 2 * sqrt(abs(q) / (4 * PI * minS));
  return String(t) + ": " + String(dMin * 1e6, 2) + "–" + String(dMax * 1e6, 2) + " µm";
}

// === Web Page Handlers ===
void handleMain() {
  calculateDerivedValues();
  String page = "<!DOCTYPE html><html><head><title>Dust Analyzer</title>";
  page += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  page += "<style>";
  page += "body{margin:0;padding:0;font-family:monospace;background:#A1A6A8;";
  page += "background-image:url('https://github.com/NathanTruong0930/COATL-AIRCRAFT/blob/main/Code/Volcano2.png?raw=true');";
  page += "background-repeat:no-repeat;background-position:center top;background-size:contain;height:auto;min-height:100vh;overflow-y:scroll;}";
  page += "#dashboard{position:absolute;top:60vh;left:50%;transform:translateX(-50%);background-color:transparent;padding:1vh;text-align:center;width:90vw;max-width:450px;font-size:2vh;}";
  page += "h1{font-size:3vh;margin:0.5vh 0;}p{font-size:2vh;margin:1vh 0;}button{font-family:monospace;background-color:#E87749;color:black;padding:1vh 2vh;border:none;cursor:pointer;font-size:2vh;margin-top:2vh;}";
  page += "</style></head><body>";
  page += "<script>window.onload=function(){document.getElementById('dashboard').scrollIntoView({behavior:'smooth',block:'center'});}</script>";
  page += "<div id='dashboard'><h1>Electrostatic Dust Analyzer</h1>";
  page += "<p><b>Charge:</b> " + chargeStr + "</p>";
  page += "<p><b>Particle Size Estimates:</b></p>";
  page += "<p>" + particleEstimate(charge, "Volcanic Ash") + "</p>";
  page += "<p>" + particleEstimate(charge, "Wildfire Smoke") + "</p>";
  page += "<p>" + particleEstimate(charge, "Mineral Dust") + "</p>";
  page += "<form action='/details'><button>Technical Details</button></form></div></body></html>";
  server.send(200, "text/html", page);
}

void handleDetails() {
  calculateDerivedValues();
  String page = "<html><head><title>Details</title>";
  page += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  page += "<style>body{background:#A1A6A8;color:black;font-family:monospace;padding:20px;}button{background:#E87749;color:black;font-family:monospace;padding:10px;border:none;cursor:pointer;}";
  page += ".radio-group{display:flex;gap:10px;margin-top:10px;}";
  page += "input[type=radio]{display:none;}label{padding:6px 12px;border:1px solid black;border-radius:5px;cursor:pointer;}input[type=radio]:checked+label{background:#E87749;color:black;}</style>";
  page += "<script>function upd(){fetch('/log').then(r=>r.text()).then(t=>{let p=document.getElementById('log');p.textContent=t;p.scrollTop=p.scrollHeight;});}setInterval(upd,1000);window.onload=upd;</script></head><body>";
  page += "<a href='/' style='text-decoration:none;color:black;font-weight:bold;font-size:18px;'>&#8592; Main Page</a>";
  page += "<h2>Technical Details</h2>";
  page += "<p><b>Charge:</b> " + chargeStr + "</p>";
  page += "<p><b>Voltage:</b> " + String(voltage, 4) + " V</p>";
  page += "<p><b>ADC Bits:</b> " + String(adcRaw) + "</p>";
  page += "<p><b>Gain Stage:</b> " + gainLabel + "</p>";
  if (clippingWarning) page += "<p style='color:red'><b>⚠️ Clipping risk!</b></p>";

  // Gain toggle
  page += "<form method='POST' action='/toggleGain'><button>Toggle " + String(manualGain ? "Auto" : "Manual") + " Gain</button></form>";

  // Gain selectors
  if (manualGain) {
    page += "<form method='POST' action='/selectGain'><div class='radio-group'>";
    for (int g = 1; g <= 3; g++) {
      String val = "gain" + String(g);
      String checked = (activeGain == g) ? "checked" : "";
      String label = (g == 1) ? "Gain 1 (3.9x)" : (g == 2) ? "Gain 2 (15x)" : "Gain 3 (47x)";
      page += "<input type='radio' id='" + val + "' name='gain' value='" + String(g) + "' " + checked + ">";
      page += "<label for='" + val + "'>" + label + "</label>";
    }
    page += "</div><button>Apply Gain</button></form>";
  }

  // Serial log monitor
  page += "<h3>Serial Monitor</h3><pre id='log' style='background:#000;color:#0f0;padding:8px;height:300px;overflow-y:scroll;font-family:monospace;'></pre>";
  page += "</body></html>";
  server.send(200, "text/html", page);
}

void handleToggleGain() {
  manualGain = !manualGain;
  if (!manualGain) setGain(1); // reset to auto start
  server.sendHeader("Location", "/details");
  server.send(303);
}

void handleSelectGain() {
  if (server.hasArg("gain")) {
    int g = server.arg("gain").toInt();
    if (g >= 1 && g <= 3) setGain(g);
  }
  server.sendHeader("Location", "/details");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  pinMode(CS_PIN, OUTPUT); digitalWrite(CS_PIN, HIGH);
  pinMode(GAIN1_PIN, OUTPUT);
  pinMode(GAIN2_PIN, OUTPUT);
  pinMode(GAIN3_PIN, OUTPUT);
  setGain(1);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());

  server.on("/", handleMain);
  server.on("/details", handleDetails);
  server.on("/log", handleLog);
  server.on("/toggleGain", HTTP_POST, handleToggleGain);
  server.on("/selectGain", HTTP_POST, handleSelectGain);
  server.begin();
}

void loop() {
  server.handleClient();
  if (testMode && Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    float entered = input.toFloat();
    if (entered >= 0.0 && entered <= V_ADC_MAX) {
      testVoltage = entered;
      Serial.println("Simulated input: " + String(testVoltage));
      addLogLine("Manual input: " + String(testVoltage));
    }
  }
}
