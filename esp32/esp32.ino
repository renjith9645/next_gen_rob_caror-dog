#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// ================= WIFI =================
const char *ssid = "OnePlus 12R";
const char *password = "12244687";

// ================= TELEGRAM =================
#define BOT_TOKEN "8735905267:AAF-54q_wJZRivWnv-JWVKCmB572n9vT6Z0"
#define CHAT_ID "1012506978"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ================= SERVER =================
WebServer server(80);

// ===== MOTOR PINS =====
#define L1 5
#define L2 18
#define R1 19
#define R2 21

// ===== HEAD MOTOR =====
#define H1 22
#define H2 23

// ===== DEVICES =====
#define LED_PIN 33
#define BUZZER_PIN 4
#define METAL_PIN 34

bool gunActive = false;

// ================= MOTOR =================
void stopAll(){
  digitalWrite(L1,LOW);
  digitalWrite(L2,LOW);
  digitalWrite(R1,LOW);
  digitalWrite(R2,LOW);
  server.send(200,"text/plain","OK");
}

void forward(){
  digitalWrite(L1,HIGH);
  digitalWrite(L2,LOW);
  digitalWrite(R1,HIGH);
  digitalWrite(R2,LOW);
  server.send(200,"text/plain","OK");
}

void backward(){
  digitalWrite(L1,LOW);
  digitalWrite(L2,HIGH);
  digitalWrite(R1,LOW);
  digitalWrite(R2,HIGH);
  server.send(200,"text/plain","OK");
}

void left(){
  digitalWrite(L1,LOW);
  digitalWrite(L2,HIGH);
  digitalWrite(R1,HIGH);
  digitalWrite(R2,LOW);
  server.send(200,"text/plain","OK");
}

void right(){
  digitalWrite(L1,HIGH);
  digitalWrite(L2,LOW);
  digitalWrite(R1,LOW);
  digitalWrite(R2,HIGH);
  server.send(200,"text/plain","OK");
}

// ================= HEAD =================
void headForward(){
  digitalWrite(H1,HIGH);
  digitalWrite(H2,LOW);
  server.send(200,"text/plain","OK");
}

void headLeft(){
  digitalWrite(H1,LOW);
  digitalWrite(H2,HIGH);
  server.send(200,"text/plain","OK");
}

void headStop(){
  digitalWrite(H1,LOW);
  digitalWrite(H2,LOW);
  server.send(200,"text/plain","OK");
}

// ================= PERSON ALERT =================
void personDetected(){

  if(!gunActive){
    digitalWrite(LED_PIN,HIGH);
    delay(150);
    digitalWrite(LED_PIN,LOW);
  }

  server.send(200,"text/plain","OK");
}

// ================= GUN =================
void gunOn(){
  gunActive = true;
  digitalWrite(LED_PIN,HIGH);
  server.send(200,"text/plain","OK");
}

void gunOff(){
  gunActive = false;
  digitalWrite(LED_PIN,LOW);
  server.send(200,"text/plain","OK");
}

// ================= BUZZER =================
void buzzerOn(){
  digitalWrite(BUZZER_PIN,HIGH);
  server.send(200,"text/plain","OK");
}

void buzzerOff(){
  digitalWrite(BUZZER_PIN,LOW);
  server.send(200,"text/plain","OK");
}

// ================= METAL SENSOR =================
void getMetalStatus(){

  int metal = digitalRead(METAL_PIN);

  if(metal == HIGH){
    digitalWrite(BUZZER_PIN,HIGH);
    server.send(200,"text/plain","1");
  }
  else{
    digitalWrite(BUZZER_PIN,LOW);
    server.send(200,"text/plain","0");
  }
}

// ================= SETUP =================
void setup(){

  Serial.begin(115200);

  pinMode(L1,OUTPUT);
  pinMode(L2,OUTPUT);
  pinMode(R1,OUTPUT);
  pinMode(R2,OUTPUT);

  pinMode(H1,OUTPUT);
  pinMode(H2,OUTPUT);

  pinMode(LED_PIN,OUTPUT);
  pinMode(BUZZER_PIN,OUTPUT);

  pinMode(METAL_PIN,INPUT);

  // ===== WIFI =====
  WiFi.begin(ssid,password);

  Serial.print("Connecting");

  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");

  IPAddress ip = WiFi.localIP();

  Serial.print("ESP32 IP: ");
  Serial.println(ip);

  // ===== TELEGRAM SSL =====
  client.setInsecure();

  // ===== SEND IP TO TELEGRAM =====
  String msg = "ESP32 ONLINE\nIP: " + ip.toString();
  bot.sendMessage(CHAT_ID,msg,"");

  // ===== ROUTES =====
  server.on("/forward",forward);
  server.on("/back",backward);
  server.on("/left",left);
  server.on("/right",right);
  server.on("/stop",stopAll);

  server.on("/head_forward",headForward);
  server.on("/head_left",headLeft);
  server.on("/head_stop",headStop);

  server.on("/person_detected",personDetected);

  server.on("/buzzer_on",buzzerOn);
  server.on("/buzzer_off",buzzerOff);

  server.on("/gun_on",gunOn);
  server.on("/gun_off",gunOff);

  server.on("/metal_status",getMetalStatus);

  server.begin();

  Serial.println("Web Server Started");
}

// ================= LOOP =================
void loop(){
  server.handleClient();
}