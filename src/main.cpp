#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

// Pour OTAA
static const u1_t PROGMEM APPEUI[8] = { 0x77, 0x48, 0x62, 0x5A, 0x07, 0x40, 0xD4, 0x70 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

static const u1_t PROGMEM DEVEUI[8] = { 0x25, 0x25, 0x07, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

static const u1_t PROGMEM APPKEY[16] = { 0xFD, 0x8C, 0x52, 0x0C, 0xE9, 0x6A, 0xA1, 0xAE, 0x9A, 0x51, 0x5B, 0x6D, 0x39, 0x10, 0xDD, 0x8C };
void os_getDevKey (u1_t* buf) { memcpy_P(buf, APPKEY, 16);}

static osjob_t sendjob;

// Intervalle d'envoi (30 secondes)
const unsigned TX_INTERVAL = 30;

// Configuration des broches pour le TTGO T-Beam
const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 23,
    .dio = {26, 33, 32},
};

// Fonction pour préparer le payload binaire compatible avec le décodeur
void preparePayload(uint8_t *payload) {
    // Valeurs fixes comme demandé (multipliées par 100 pour éviter les floats)
    int32_t gas = 8479629;       // 84796.29 * 100
    int16_t humidity = 6076;     // 60.76 * 100
    int32_t pressure = 10125700; // 101257 * 100
    int16_t temperature = 5000;  // 50.00 * 100
    
    // Structure du payload (identique à ce que le décodeur attend)
    payload[0] = temperature >> 8;    // Octet haut température
    payload[1] = temperature & 0xFF;  // Octet bas température
    payload[2] = humidity >> 8;       // Octet haut humidité
    payload[3] = humidity & 0xFF;      // Octet bas humidité
    payload[4] = pressure >> 24;       // Octet 1 pression
    payload[5] = pressure >> 16;       // Octet 2 pression
    payload[6] = pressure >> 8;        // Octet 3 pression
    payload[7] = pressure & 0xFF;      // Octet 4 pression
    payload[8] = gas >> 24;            // Octet 1 gaz
    payload[9] = gas >> 16;            // Octet 2 gaz
    payload[10] = gas >> 8;            // Octet 3 gaz
    payload[11] = gas & 0xFF;          // Octet 4 gaz
    
    // Affichage des valeurs pour débogage
    Serial.print("Température: "); Serial.println(temperature / 100.0);
    Serial.print("Humidité: "); Serial.println(humidity / 100.0);
    Serial.print("Pression: "); Serial.println(pressure / 100.0);
    Serial.print("Gaz: "); Serial.println(gas / 100.0);
}

void do_send(osjob_t* j);

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
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
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
            Serial.println(F("Unknown event"));
            break;
    }
}

void do_send(osjob_t* j){
    // Vérifier si une transmission est déjà en cours
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Préparer le payload binaire
        uint8_t payload[12];
        preparePayload(payload);
        
        // Afficher le payload en hexadécimal pour vérification
        Serial.print("Payload hex: ");
        for(int i = 0; i < 12; i++) {
            if(payload[i] < 0x10) Serial.print('0');
            Serial.print(payload[i], HEX);
            Serial.print(' ');
        }
        Serial.println();
        
        // Envoyer les données
        LMIC_setTxData2(1, payload, sizeof(payload), 0);
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println(F("Démarrage..."));
    
    // Initialisation de LMIC
    os_init();
    LMIC_reset();
    
    // Lancer la tâche d'envoi
    do_send(&sendjob);
}

void loop() {
    os_runloop_once();
}