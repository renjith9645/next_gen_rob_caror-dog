#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// ================= WIFI =================
const char *ssid = "OnePlus 12R";
const char *password = "12244687";

// ================= TELEGRAM =================
#define BOT_TOKEN "YOUR_BOT_TOKEN"
#define CHAT_ID  "YOUR_CHAT_ID"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

WebServer server(80);

// ===== MOTOR PINS =====
#define L1 5
#define L2 18
#define R1 19
#define R2 21

#define H1 22
#define H2 23

#define LED_PIN 2
#define BUZZER_PIN 4
#define METAL_PIN 34

bool gunActive = false;

// ================= MOTOR =================
void stopAll(){
  Serial.println("STOP pressed");
  digitalWrite(L1, LOW);
  digitalWrite(L2, LOW);
  digitalWrite(R1, LOW);
  digitalWrite(R2, LOW);
}

void forward(){
  Serial.println("FORWARD pressed");
  digitalWrite(L1, HIGH);
  digitalWrite(L2, LOW);
  digitalWrite(R1, HIGH);
  digitalWrite(R2, LOW);
}

void backward(){
  Serial.println("BACKWARD pressed");
  digitalWrite(L1, LOW);
  digitalWrite(L2, HIGH);
  digitalWrite(R1, LOW);
  digitalWrite(R2, HIGH);
}

void left(){
  Serial.println("LEFT pressed");
  digitalWrite(L1, LOW);
  digitalWrite(L2, HIGH);
  digitalWrite(R1, HIGH);
  digitalWrite(R2, LOW);
}

void right(){
  Serial.println("RIGHT pressed");
  digitalWrite(L1, HIGH);
  digitalWrite(L2, LOW);
  digitalWrite(R1, LOW);
  digitalWrite(R2, HIGH);
}

// ===== HEAD =====
void headForward(){
  Serial.println("HEAD FORWARD pressed");
  digitalWrite(H1, HIGH);
  digitalWrite(H2, LOW);
}

void headLeft(){
  Serial.println("HEAD LEFT pressed");
  digitalWrite(H1, LOW);
  digitalWrite(H2, HIGH);
}

void headStop(){
  Serial.println("HEAD STOP pressed");
  digitalWrite(H1, LOW);
  digitalWrite(H2, LOW);
}

// ===== PERSON DETECTED =====
void personDetected(){
  Serial.println("PERSON DETECTED SIGNAL");

  if(!gunActive){  // Gun priority
    digitalWrite(LED_PIN, HIGH);
    delay(150);
    digitalWrite(LED_PIN, LOW);
  }

  bot.sendMessage(CHAT_ID, "🚨 PERSON DETECTED!", "");
}

// ===== GUN BUTTON =====
void gunOn(){
  Serial.println("GUN PRESSED");
  gunActive = true;
  digitalWrite(LED_PIN, HIGH);
}

void gunOff(){
  Serial.println("GUN RELEASED");
  gunActive = false;
  digitalWrite(LED_PIN, LOW);
}

// ===== BUZZER =====
void buzzerOn(){
  Serial.println("BUZZER ON");
  digitalWrite(BUZZER_PIN, HIGH);
}

void buzzerOff(){
  Serial.println("BUZZER OFF");
  digitalWrite(BUZZER_PIN, LOW);
}

// ===== METAL SENSOR =====
void getMetalStatus(){
  int metal = digitalRead(METAL_PIN);

  if(metal == HIGH){
    Serial.println("⚠ METAL DETECTED!");
    digitalWrite(BUZZER_PIN, HIGH);
   
  }
  else{
    digitalWrite(BUZZER_PIN, LOW);
  }

  server.send(200, "text/plain", metal == HIGH ? "1" : "0");
}

void setup(){
  Serial.begin(115200);

  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(R1, OUTPUT);
  pinMode(R2, OUTPUT);
  pinMode(H1, OUTPUT);
  pinMode(H2, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(METAL_PIN, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");

  IPAddress ip = WiFi.localIP();
  Serial.print("ESP32 IP: ");
  Serial.println(ip);

  client.setInsecure();

  String message = "📡 *ESP32 Connected*\n";
  message += "🛜 WiFi: " + String(ssid) + "\n";
  message += "🌐 IP: " + ip.toString();

  bot.sendMessage(CHAT_ID, message, "Markdown");

  // ===== ROUTES =====
  server.on("/forward", forward);
  server.on("/back", backward);
  server.on("/left", left);
  server.on("/right", right);
  server.on("/stop", stopAll);

  server.on("/head_forward", headForward);
  server.on("/head_left", headLeft);
  server.on("/head_stop", headStop);

  server.on("/person_detected", personDetected);
  server.on("/buzzer_on", buzzerOn);
  server.on("/buzzer_off", buzzerOff);

  server.on("/gun_on", gunOn);
  server.on("/gun_off", gunOff);

  server.on("/metal_status", getMetalStatus);

  server.begin();
  Serial.println("Web Server Started");
}

void loop(){
  server.handleClient();
}