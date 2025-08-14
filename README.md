# TTGO LoRa "Hello, World!" pour The Things Network (PlatformIO)

![Version du projet](https://img.shields.io/badge/version-1.0.0-blue)
![Licence](https://img.shields.io/badge/license-MIT-green)
![Framework](https://img.shields.io/badge/framework-PlatformIO-orange.svg)

Projet de démarrage simple pour envoyer un message "Hello, World!" depuis un nœud TTGO LoRa vers The Things Network (TTN) v3, en utilisant Visual Studio Code avec l'extension PlatformIO.

---

## Objectif du projet

Ce projet est un **"Hello, World!" pour le LoRaWAN**. Il a pour but de valider la chaîne de communication complète : de l'appareil physique (TTGO) jusqu'à la plateforme cloud (TTN), en passant par une passerelle LoRaWAN. C'est la première étape essentielle avant de construire des applications plus complexes.

## Matériel requis

* **Nœud LoRa :** Un [TTGO T-Beam](https://www.lilygo.cc/products/t-beam) ou une carte ESP32+LoRa similaire.
* **Antenne LoRa :** Connectez-la toujours avant de mettre la carte sous tension ! Utilisez la bonne fréquence pour votre région (868 MHz pour l'Europe, 915 MHz pour l'Amérique du Nord, etc.).
* **Câble Micro-USB** pour la programmation.

## Logiciels et Prérequis

1.  **Compte The Things Network v3 :** Nécessaire pour enregistrer votre appareil. [Créez un compte ici](https://www.thethingsnetwork.org/).
2.  **Visual Studio Code :** L'éditeur de code. [Télécharger ici](https://code.visualstudio.com/).
3.  **Extension PlatformIO IDE :** À installer depuis le marketplace de VSCode. Elle gérera automatiquement les bibliothèques et la compilation.
4.  **Git :** Pour cloner le projet. [Télécharger ici](https://git-scm.com/).
5.  **Une passerelle (Gateway) LoRaWAN** à portée, configurée et connectée à The Things Network.

---

## Guide d'installation et de configuration

### 1. Configuration sur The Things Network (TTN)

Cette étape est identique : vous devez déclarer votre appareil sur le réseau.

1.  Connectez-vous à la [console TTN v3](https://console.thethingsnetwork.org/) et sélectionnez votre région.
2.  Allez dans une **Application**, puis dans **"End devices"** et cliquez sur **"Add end device"**.
3.  Utilisez l'onglet **"Enter end device specifics manually"**.
    * **Frequency plan :** Choisissez celui de votre région (ex: `Europe 863-870 MHz (SF9 for RX2 - recommended)`).
    * **LoRaWAN version :** `LoRaWAN Specification 1.0.2`.
    * **Activation mode :** `Over-the-air activation (OTAA)`.
    * Cliquez sur **"Generate"** pour les `DevEUI`, `AppEUI` et `AppKey`.
4.  Cliquez sur **"Register end device"**.

Une fois l'appareil créé, allez sur sa page et copiez les trois valeurs suivantes (cliquez sur l'icône `<>` pour voir le format C-style `lsb`).
* `Device EUI`
* `Application EUI`
* `App Key`

### 2. Configuration et téléversement avec PlatformIO

1.  Clonez ce dépôt sur votre ordinateur.
2.  Ouvrez le dossier du projet dans Visual Studio Code. PlatformIO devrait automatiquement détecter le fichier `platformio.ini` et installer les dépendances nécessaires.

3.  **Entrez vos clés secrètes :**
    Ouvrez le fichier `src/main.cpp`. Cherchez la section de configuration et collez les clés que vous avez récupérées de TTN.

    ```c++
    // Fichier : src/main.cpp

    // TODO: Remplacez les valeurs suivantes par celles de votre console TTN
    // Les clés doivent être en format LSB (Little Endian).
    // Copiez-les depuis la console TTN en cliquant sur l'icône "<>"
    static const u1_t PROGMEM APPEUI[8]= { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static const u1_t PROGMEM DEVEUI[8]= { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static const u1_t PROGMEM APPKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    ```

4.  **TTN Payload Decoder :**
Pour décoder les données envoyées par le TTGO LoRa dans The Things Network (TTN), vous pouvez utiliser le script JavaScript suivant.

# Étapes :
1. Connectez-vous à votre console TTN.
2. Allez dans **Applications** → sélectionnez votre **device**.
3. Ouvrez l’onglet **Payload formatters** → **Uplink**.
4. Choisissez **JavaScript** comme format.
5. Collez le code suivant : payload.js


5.  **Compilez et téléversez :**
    * Branchez votre TTGO à votre ordinateur.
    * Dans la barre de statut bleue en bas de VSCode, cliquez sur l'icône en forme de flèche (→) qui correspond à **"PlatformIO: Upload"**.
    * PlatformIO va compiler le code et le téléverser sur la carte.

6.  **Vérifiez le fonctionnement :**
    * Cliquez sur l'icône en forme de prise (🔌) dans la barre de statut pour ouvrir le **Moniteur Série**.
    * Vous devriez voir les logs de la librairie LMIC. Après `EV_JOINING`, un `EV_JOINED` confirmera la connexion au réseau.
    * Rendez-vous sur la console TTN, dans la section **"Live data"** de votre appareil. Vous devriez y voir arriver vos messages !

### Fichier `platformio.ini` de référence

Ce fichier est le cœur de la configuration pour PlatformIO. Il doit être à la racine de votre projet.

```ini
[env:ttgo-t-beam]
platform = espressif32
board = ttgo-t-beam
framework = arduino
lib_deps =
    mcci-catena/MCCI LoRaWAN LMIC library @ ^4.1.1
monitor_speed = 115200