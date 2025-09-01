#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

// --- Configuration BME688 ---
Adafruit_BME680 bme; // I2C

// --- Configuration LoRaWAN ---
//
// Pour OTAA, remplissez ces clés depuis votre console TTN
//
// Pour l'App EUI, vous devrez peut-être inverser l'ordre des octets (LSB).
// Par exemple, si votre App EUI sur TTN est 01 02 03 04 05 06 07 08,
// vous devrez peut-être l'écrire comme { 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01 }
// Faites de même pour le Dev EUI. L'App Key est généralement en MSB.
//
// Pour OTAA
static const u1_t PROGMEM APPEUI[8] = { 0x77, 0x48, 0x62, 0x5A, 0x07, 0x40, 0xD4, 0x70 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

static const u1_t PROGMEM DEVEUI[8] = { 0x25, 0x25, 0x07, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

static const u1_t PROGMEM APPKEY[16] = { 0xFD, 0x8C, 0x52, 0x0C, 0xE9, 0x6A, 0xA1, 0xAE, 0x9A, 0x51, 0x5B, 0x6D, 0x39, 0x10, 0xDD, 0x8C };
void os_getDevKey (u1_t* buf) { memcpy_P(buf, APPKEY, 16);} 

// Charge utile (payload) à envoyer. 11 octets.
// 2 pour la température, 2 pour l'humidité, 4 pour la pression, 3 pour le gaz
static uint8_t payload[11];
static osjob_t sendjob;

// Planification de la prochaine transmission
const unsigned TX_INTERVAL = 20; // en secondes

// Configuration des broches pour le TTGO T-Beam
const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 23,
    .dio = {26, 33, 32},
};

void do_send(osjob_t* j); // Déclaration anticipée

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            LMIC_setLinkCheckMode(0);
            // Lancer l'envoi périodique après avoir rejoint le réseau
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.print(F("Received "));
              Serial.print(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            // Planifier la prochaine transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}

void do_send(osjob_t* j){
    // Vérifier si une transmission est déjà en cours
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // --- Lecture des données du BME688 ---
        if (!bme.performReading()) {
            Serial.println("Failed to perform reading :(");
            return;
        }

        Serial.print("Temperature = "); Serial.print(bme.temperature); Serial.println(" *C");
        Serial.print("Pressure = "); Serial.print(bme.pressure / 100.0); Serial.println(" hPa");
        Serial.print("Humidity = "); Serial.print(bme.humidity); Serial.println(" %");
        Serial.print("Gas = "); Serial.print(bme.gas_resistance / 1000.0); Serial.println(" KOhms");
        Serial.println();

        // --- Préparation de la charge utile (payload) ---
        // Pour économiser de l'espace, nous envoyons les valeurs comme des entiers.
        // Il faudra les diviser par 100 ou 10 du côté du serveur pour retrouver les décimales.
        int16_t temp = bme.temperature * 100;
        uint16_t hum = bme.humidity * 100;
        uint32_t pres = bme.pressure; // Pression en Pascals
        uint32_t gas = bme.gas_resistance; // Résistance au gaz en Ohms

        payload[0] = temp >> 8;
        payload[1] = temp;
        payload[2] = hum >> 8;
        payload[3] = hum;
        payload[4] = pres >> 24;
        payload[5] = pres >> 16;
        payload[6] = pres >> 8;
        payload[7] = pres;
        payload[8] = gas >> 16;
        payload[9] = gas >> 8;
        payload[10] = gas;

        // Préparer les données à envoyer sur le port 1
        LMIC_setTxData2(1, payload, sizeof(payload), 0);
        Serial.println(F("Packet queued"));
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println(F("Starting"));

    // --- Initialisation BME688 ---
    if (!bme.begin()) {
        Serial.println(F("Could not find a valid BME688 sensor, check wiring!"));
        while (1);
    }
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150); // 320*C for 150 ms

    // --- Initialisation LMIC ---
    os_init();
    LMIC_reset();

    // Lancer la procédure de join (rejoindre le réseau)
    LMIC_startJoining();
    
    // La première transmission sera déclenchée par l'événement EV_JOINED
}

void loop() {
    os_runloop_once();
}
