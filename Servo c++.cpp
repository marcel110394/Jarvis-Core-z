
// φ-Core mit Touch-Sensor & Servo-Ausgang – Commander Aichmayr
// φ(i,j) reagiert auf Berührung → steuert einen Servo-Motor proportional zu R(i,j)

#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const int touchPin = T0;     // Touch-Sensor (GPIO 4)
const int servoPin = 18;     // PWM-Ausgang für Servo (GPIO 18)

Servo servo;

float phi[4][4];
float E[4][4];
float M[4][4];
float R[4][4];
float phi_secure[4][4];

const float PHI_THRESHOLD = 1.35;
const float LEARNING_RATE = 0.2;

const char* ssid = "JARVIS_NET";
const char* password = "SecureCommand2025";
WebServer server(80);
String trustedMAC = "AA:BB:CC:DD:EE:FF";

float compute_phi_secure(float trust, float signalQuality, float phi_local, float phi_expected) {
    return trust * signalQuality * (1 + 0.3 * (phi_local - phi_expected));
}

void calculateR() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            R[i][j] = phi[i][j] * E[i][j] * (1 + 0.5 * (phi[i][j] - M[i][j]));
        }
    }
}

void updateMemory() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            M[i][j] = (1.0 - LEARNING_RATE) * M[i][j] + LEARNING_RATE * phi[i][j];
        }
    }
}

void setupNetwork() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    server.on("/status", []() {
        if (WiFi.macAddress() == trustedMAC) {
            server.send(200, "text/plain", "φ-Core online (Servo linked).");
        } else {
            server.send(403, "text/plain", "Unauthorized.");
        }
    });

    server.begin();
}

void setup() {
    Serial.begin(115200);
    setupNetwork();

    servo.attach(servoPin);  // Servo ansteuern
    touchAttachInterrupt(touchPin, NULL, 40);  // Schwelle 40

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            phi[i][j] = 1.0;
            E[i][j] = 1.0;
            M[i][j] = 0.9;
        }
    }
}

void loop() {
    server.handleClient();

    int touchValue = touchRead(touchPin);
    if (touchValue < 40) {
        phi[1][1] += 0.1;
        if (phi[1][1] > 2.0) phi[1][1] = 2.0;
    } else {
        phi[1][1] *= 0.98;
    }

    updateMemory();
    calculateR();

    // Servo-Logik – abhängig von R[1][1]
    int angle = map((int)(R[1][1] * 100), 100, 300, 0, 180);
    angle = constrain(angle, 0, 180);
    servo.write(angle);

    // Debug-Ausgabe
    Serial.print("φ: "); Serial.print(phi[1][1]);
    Serial.print(" | M: "); Serial.print(M[1][1]);
    Serial.print(" | R: "); Serial.print(R[1][1]);
    Serial.print(" | Servo: "); Serial.println(angle);

    delay(500);
}
