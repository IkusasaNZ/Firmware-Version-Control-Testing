#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>   // for HTTP GET/POST
#include <ESP8266httpUpdate.h>   // for OTA updates (ESPhttpUpdate.update)
#include <Updater.h>              // for manual OTA (Update.begin / Update.write)


#define LED_PIN LED_BUILTIN   // On NodeMCU, LED_BUILTIN = GPIO2 (D4)

const char* ssid = "George";
const char* password = "QUINTINLIDDLE";

const char* firmwareUrl = "https://github.com/IkusasaNZ/Firmware-Version-Control-Testing/releases/download/esp8266_firmware/firmware.bin";
//  https://github.com/IkusasaNZ/Firmware-Version-Control-Testing/releases/download/esp8266_firmware/firmware.bin

const char* versionUrl = "https://raw.githubusercontent.com/IkusasaNZ/Firmware-Version-Control-Testing/refs/heads/master/version.txt";

const char* currentVersion = "1.0.0";   // device’s version


// =============================
// Function Prototypes
// =============================
void connectToWiFi();
void checkForUpdate();
String fetchLatestVersion();
bool downloadAndApplyFirmware();

// =============================
// Setup
// =============================
void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  delay(100);

  connectToWiFi();

  Serial.println("\nDevice Ready");
  Serial.println("Current Firmware: " + String(currentVersion));

  checkForUpdate();
}

void loop() {
  // blink LED while running normally
  digitalWrite(LED_PIN, LOW);
  delay(1000);
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
}

// =============================
// WiFi Connect
// =============================
void connectToWiFi() {
  Serial.printf("Connecting to %s", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
}

// =============================
// Update Check
// =============================
void checkForUpdate() {
  Serial.println("Checking for firmware update...");
  String latest = fetchLatestVersion();

  if (latest == "") {
    Serial.println("Failed to fetch latest version.");
    return;
  }

  Serial.println("Latest: " + latest);
  if (latest != currentVersion) {
    Serial.println("New firmware available!");
    if (downloadAndApplyFirmware()) {
      Serial.println("Update successful. Rebooting...");
      ESP.restart();
    } else {
      Serial.println("Update failed.");
    }
  } else {
    Serial.println("Device is up to date.");
  }
}

// =============================
// Fetch latest version string
// =============================
String fetchLatestVersion() {
  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure();
  http.begin(client, versionUrl);
  int code = http.GET();

  if (code == HTTP_CODE_OK) {
    String ver = http.getString();
    ver.trim();
    http.end();
    return ver;
  } else {
    Serial.printf("HTTP error: %d\n", code);
    http.end();
    return "";
  }
}

// =============================
// Download + Apply firmware
// =============================
bool downloadAndApplyFirmware() {
  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure();
  http.begin(client, firmwareUrl);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  
  int code = http.GET();

  if (code != HTTP_CODE_OK) {
    Serial.printf("Firmware download failed. HTTP code: %d\n", code);
    http.end();
    return false;
  }

  int contentLength = http.getSize();
  if (contentLength <= 0) {
    Serial.println("Invalid content length");
    http.end();
    return false;
  }

  WiFiClient* stream = http.getStreamPtr();

  Serial.printf("Starting OTA: %d bytes\n", contentLength);
  if (!Update.begin(contentLength)) {
    Serial.printf("Update begin failed: %s\n", Update.getErrorString().c_str());
    http.end();
    return false;
  }

  size_t written = Update.writeStream(*stream);
  if (written != (size_t)contentLength) {
    Serial.printf("Write mismatch: %d written / %d expected\n", written, contentLength);
    Update.end();
    http.end();
    return false;
  }

  if (!Update.end()) {
    Serial.printf("Update end failed: %s\n", Update.getErrorString().c_str());
    http.end();
    return false;
  }

  Serial.println("OTA Update OK!");
  http.end();
  return true;
}


