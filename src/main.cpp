#define BLYNK_TEMPLATE_ID "TMPL6h7gDyIT1"
#define BLYNK_TEMPLATE_NAME "sistem parkir"
#define BLYNK_AUTH_TOKEN "GANTI_TOKEN_BARU_JIKA_DIPERLUKAN"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

// ================= WIFI =================

char ssid[] = "NAMA_WIFI";
char pass[] = "PASSWORD_WIFI";

// ================= PIN =================

// Servo
#define SERVO_MASUK 18
#define SERVO_KELUAR 19

// Tombol
#define BTN_MASUK 32
#define BTN_KELUAR 33

// IR Beam
#define IR_MASUK 25
#define IR_KELUAR 26

// HC-SR04
#define TRIG1 5
#define ECHO1 17

#define TRIG2 16
#define ECHO2 4

// LED Slot 1
#define LED_MERAH1 13
#define LED_HIJAU1 12

// LED Slot 2
#define LED_MERAH2 14
#define LED_HIJAU2 27

// =========================================

Servo servoMasuk;
Servo servoKeluar;

bool slot1Terisi = false;
bool slot2Terisi = false;

bool gerbangMasukTerbuka = false;
bool gerbangKeluarTerbuka = false;

int mobilMasuk = 0;
int mobilKeluar = 0;
int mobilDiArea = 0;

// =========================================

float bacaJarak(int trig, int echo) {

  digitalWrite(trig, LOW);
  delayMicroseconds(2);

  digitalWrite(trig, HIGH);
  delayMicroseconds(10);

  digitalWrite(trig, LOW);

  long durasi = pulseIn(echo, HIGH, 30000);

  return durasi * 0.034 / 2.0;
}

// =========================================

void updateBlynk() {

  int slotTerisi = 0;

  if (slot1Terisi) slotTerisi++;
  if (slot2Terisi) slotTerisi++;

  int slotKosong = 2 - slotTerisi;

  bool parkirPenuh = slot1Terisi && slot2Terisi;

  Blynk.virtualWrite(V0, slot1Terisi ? 1 : 0);
  Blynk.virtualWrite(V1, slot2Terisi ? 1 : 0);

  Blynk.virtualWrite(V2, mobilDiArea);

  Blynk.virtualWrite(V3, slotKosong);
  Blynk.virtualWrite(V4, slotTerisi);

  if (parkirPenuh)
    Blynk.virtualWrite(V5, "PARKIRAN PENUH");
  else
    Blynk.virtualWrite(V5, "TERSEDIA");
}

// =========================================

void cekSlotParkir() {

  float jarak1 = bacaJarak(TRIG1, ECHO1);
  float jarak2 = bacaJarak(TRIG2, ECHO2);

  slot1Terisi = (jarak1 <= 10);
  slot2Terisi = (jarak2 <= 10);

  // SLOT 1

  if (slot1Terisi) {

    digitalWrite(LED_HIJAU1, HIGH);
    digitalWrite(LED_MERAH1, LOW);

  } else {

    digitalWrite(LED_HIJAU1, LOW);
    digitalWrite(LED_MERAH1, HIGH);
  }

  // SLOT 2

  if (slot2Terisi) {

    digitalWrite(LED_HIJAU2, HIGH);
    digitalWrite(LED_MERAH2, LOW);

  } else {

    digitalWrite(LED_HIJAU2, LOW);
    digitalWrite(LED_MERAH2, HIGH);
  }
}

// =========================================

BLYNK_CONNECTED() {

  Blynk.syncAll();
}

// =========================================

void setup() {

  Serial.begin(115200);

  // Servo

  servoMasuk.attach(SERVO_MASUK);
  servoKeluar.attach(SERVO_KELUAR);

  servoMasuk.write(90);
  servoKeluar.write(90);

  // Tombol

  pinMode(BTN_MASUK, INPUT_PULLUP);
  pinMode(BTN_KELUAR, INPUT_PULLUP);

  // IR Beam

  pinMode(IR_MASUK, INPUT_PULLUP);
  pinMode(IR_KELUAR, INPUT_PULLUP);

  // Ultrasonik

  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);

  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  // LED

  pinMode(LED_MERAH1, OUTPUT);
  pinMode(LED_HIJAU1, OUTPUT);

  pinMode(LED_MERAH2, OUTPUT);
  pinMode(LED_HIJAU2, OUTPUT);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

// =========================================

void loop() {

  Blynk.run();

  cekSlotParkir();

  bool parkirPenuh = slot1Terisi && slot2Terisi;

  // =====================
  // GERBANG MASUK
  // =====================

  if (digitalRead(BTN_MASUK) == LOW) {

    if (!parkirPenuh) {

      servoMasuk.write(0);

      gerbangMasukTerbuka = true;

      Serial.println("Gerbang Masuk Dibuka");

    } else {

      Serial.println("PARKIRAN PENUH");
    }

    delay(300);
  }

  // =====================
  // IR MASUK
  // =====================
  // kalau dia ternyata high, ganti ke high
  if (digitalRead(IR_MASUK) == LOW &&
      gerbangMasukTerbuka) {

    mobilMasuk++;
    mobilDiArea++;

    servoMasuk.write(90);

    gerbangMasukTerbuka = false;

    Serial.print("Mobil Di Area : ");
    Serial.println(mobilDiArea);

    delay(300);
  }

  // =====================
  // GERBANG KELUAR
  // =====================
  
  if (digitalRead(BTN_KELUAR) == LOW) {

    servoKeluar.write(0);

    gerbangKeluarTerbuka = true;

    Serial.println("Gerbang Keluar Dibuka");

    delay(300);
  }

  // =====================
  // IR KELUAR
  // =====================
  // Ganti LOW menjadi HIGH jika sensor beam aktif HIGH
  if (digitalRead(IR_KELUAR) == LOW &&
      gerbangKeluarTerbuka) {

    if (mobilDiArea > 0) {

      mobilKeluar++;
      mobilDiArea--;
    }

    servoKeluar.write(90);

    gerbangKeluarTerbuka = false;

    Serial.print("Mobil Di Area : ");
    Serial.println(mobilDiArea);

    delay(300);
  }

  updateBlynk();

  delay(100);
}