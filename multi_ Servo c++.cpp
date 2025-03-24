
// φ-Core Multi-Servo Version – Schulter & Ellenbogen (φ[1][1], φ[1][2])
// Commander M. Aichmayr – Koordinierte Gelenkbewegung

#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const int touchPin = T0;     // Touchsensor → Schultersteuerung (φ[1][1])
const int servoPinA = 18;    // Servo A – Schulter
const int servoPinB = 19;    // Servo B – Ellenbogen

Servo servoA;
Servo servoB;

float phi[4][4];
float E[4][4];
float M[4][4];
float R[4][4];

const float PHI_THRESHOLD = 1.35;
const float LEARNING_RATE = 0.2;

const char* ssid = "JARVIS_NET";
const char* password = "SecureCommand2025";
WebServer server(80);
String trustedMAC = "AA:BB:CC:DD:EE:FF";

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
            server.send(200, "text/plain", "φ-Core online – Multi-Servo aktiv.");
        } else {
            server.send(403, "text/plain", "Unauthorized.");
        }
    });

    server.begin();
}

void setup() {
    Serial.begin(115200);
    setupNetwork();

    servoA.attach(servoPinA);  // Schulter
    servoB.attach(servoPinB);  // Ellenbogen
    touchAttachInterrupt(touchPin, NULL, 40);  // Touch für Schulter

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

    // SENSOR: Touch steuert φ[1][1] (Schulter)
    int touchValue = touchRead(touchPin);
    if (touchValue < 40) {
        phi[1][1] += 0.1;
        if (phi[1][1] > 2.0) phi[1][1] = 2.0;
    } else {
        phi[1][1] *= 0.98;
    }

    // SIMULATION: φ[1][2] steigt passiv leicht → Ellenbogen synchronisiert
    phi[1][2] = phi[1][1] * 0.85;

    updateMemory();
    calculateR();

    // SERVOS:
    int angleA = map((int)(R[1][1] * 100), 100, 300, 0, 160);
    int angleB = map((int)(R[1][2] * 100), 100, 300, 0, 140);
    angleA = constrain(angleA, 0, 160);
    angleB = constrain(angleB, 0, 140);

    servoA.write(angleA);
    servoB.write(angleB);

    // DEBUG:
    Serial.print("φA: "); Serial.print(phi[1][1]);
    Serial.print(" | R_A: "); Serial.print(R[1][1]);
    Serial.print(" | ServoA: "); Serial.print(angleA);
    Serial.print(" || φB: "); Serial.print(phi[1][2]);
    Serial.print(" | R_B: "); Serial.print(R[1][2]);
    Serial.print(" | ServoB: "); Serial.println(angleB);

    delay(500);
}
