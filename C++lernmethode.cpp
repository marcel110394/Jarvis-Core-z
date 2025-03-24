
// φ-Core Kontrolliertes Netzwerk – mit Lernmodul aktiviert (M-Update)
// Commander M. Aichmayr – φ-Lernen implementiert

#include <WiFi.h>
#include <WebServer.h>

float phi[4][4];
float E[4][4];
float M[4][4];
float R[4][4];
float phi_secure[4][4];

const float PHI_THRESHOLD = 1.35;
const float PHI_SECURE_THRESHOLD = 1.3;
const float LEARNING_RATE = 0.2; // λ – Lernrate

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
            server.send(200, "text/plain", "φ-Core status online (with learning).");
        } else {
            server.send(403, "text/plain", "Unauthorized.");
        }
    });

    server.begin();
}

void setup() {
    Serial.begin(115200);
    setupNetwork();

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            phi[i][j] = 1.0;
            E[i][j] = 1.0;
            M[i][j] = 0.9;
        }
    }
    calculateR();
}

void loop() {
    server.handleClient();

    updateMemory();    // Gedächtnisaktualisierung
    calculateR();      // Antwort neu berechnen

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float trust = 1.0;
            float signalQuality = 1.0;
            phi_secure[i][j] = compute_phi_secure(trust, signalQuality, phi[i][j], 1.0);
            if (phi_secure[i][j] < 1.0) {
                Serial.println("φ-Sicherheitsalarm: Zelle " + String(i) + "," + String(j));
            }
        }
    }

    delay(1000);
}
