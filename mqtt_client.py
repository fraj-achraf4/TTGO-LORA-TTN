# mqtt_client.py
import os
import django
import json
import paho.mqtt.client as mqtt

# --- Étape 1: Charger l'environnement Django ---
# Assurez-vous que "mysit.settings" correspond bien au nom de votre projet.
# Si votre projet s'appelle "projet_iot", ce devrait être "projet_iot.settings".
os.environ.setdefault("DJANGO_SETTINGS_MODULE", "projet_iot.settings") 
django.setup()

# --- Étape 2: Importer le modèle après avoir configuré Django ---
# Assurez-vous que le modèle SensorData se trouve bien dans une app nommée "blog".
from dashboard.models import SensorData

# --- Étape 3: Configuration du client MQTT ---
BROKER = "eu1.cloud.thethings.network"
PORT = 8883
USERNAME = "secondttgotest@ttn"  # Remplacer par l'ID de votre application TTN
PASSWORD = "NNSXS.PRAT7D2VZ7ZS7ETBULFREJQHOEFMHVFTNDZX2RA.IMQTVEDMBRQ2ZFD3EI2WEA7WWUCRAS5MH3ES4HCVXEZDQS2UZFMA"  # Remplacer par votre clé API TTN
TOPIC = "v3/+/devices/+/up"  # Topic plus spécifique pour les messages "uplink"

# --- Étape 4: Fonction de traitement des messages ---
def on_message(client, userdata, msg):
    """
    Cette fonction est appelée chaque fois qu'un message est reçu.
    """
    print(f"[MQTT] Message reçu sur le topic : {msg.topic}")
    try:
        # Décoder le message JSON
        payload = msg.payload.decode('utf-8')
        data = json.loads(payload)

        # Extraire les informations pertinentes de la structure TTN V3
        device_id = data.get("end_device_ids", {}).get("device_id", "inconnu")
        uplink_message = data.get("uplink_message", {})
        decoded_payload = uplink_message.get("decoded_payload", {})

        # Vérifier si le payload décodé contient des données
        if not decoded_payload:
            print(f"[MQTT] Message sans payload décodé pour {device_id}. Ignoré.")
            return

        # Récupérer les valeurs (avec la clé corrigée pour "pressure")
        temperature = decoded_payload.get("temperature")
        # CORRECTION: "pressur" a été remplacé par "pressure"
        pressure = decoded_payload.get("pressur")
        humidity = decoded_payload.get("humidity")
        gaz = decoded_payload.get("gaz")

        # Créer et sauvegarder l'objet dans la base de données Django
        SensorData.objects.create(
            device_id=device_id,
            temperature=temperature,
            humidity=humidity,
            gaz=gaz,
            pressure=pressure,
            raw_payload=data  # Sauvegarde du message complet pour le débogage
        )

        print(f"[MQTT] Données sauvegardées : {device_id} | Temp={temperature} | Hum={humidity} | Gaz={gaz} | Press={pressure}")

    except json.JSONDecodeError:
        print("[Erreur] Le message reçu n'est pas un JSON valide.")
    except Exception as e:
        print(f"[Erreur] Impossible de traiter le message : {e}")


def on_connect(client, userdata, flags, rc):
    """
    Fonction appelée lors de la connexion au broker.
    """
    if rc == 0:
        print("[MQTT] Connecté avec succès au broker TTN !")
        client.subscribe(TOPIC)
        print(f"[MQTT] Souscrit au topic : {TOPIC}")
    else:
        print(f"[Erreur] Échec de la connexion, code de retour : {rc}")


# --- Étape 5: Initialisation et lancement du client ---
client = mqtt.Client()
client.username_pw_set(USERNAME, PASSWORD)
client.tls_set()  # Utiliser une connexion sécurisée TLS pour le port 8883

# Assigner les fonctions de callback
client.on_connect = on_connect
client.on_message = on_message

try:
    print("[MQTT] Connexion au broker TTN...")
    client.connect(BROKER, PORT, 60)
    # loop_forever() est une boucle bloquante qui gère la reconnexion automatiquement
    client.loop_forever()
except Exception as e:
    print(f"[Erreur] Impossible de se connecter au broker MQTT : {e}")