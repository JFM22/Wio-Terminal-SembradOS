import requests
from Inferencia import predict_with_model
import numpy as np
from datetime import datetime
import paho.mqtt.client as mqtt
import json
import time

#--CONFIGURACIÓN MQTT--
THE_BROKER = "broker.hivemq.com"
THE_TOPIC  = "idc/wioterminal/UserW1"

# Callback function used when the client receives a CONNACK response from the broker.
def on_connect(client, userdata, flags, rc):
    print("connected to ", client._host, "port: ", client._port)
    print("flags: ", flags, "returned code: ", rc)

# Callback function used when a message is published.
def on_publish(client, userdata, mid):
    print("msg published (mid={})".format(mid))

#--CONFIGURACIÓN ThingSpeak--
read_api_key = "SDFC4QG0TT44L3C3"
channel_id = "2573962"

num_results = 6  # Número de resultados a obtener

# URLs para los datos
url_field1 = f"https://api.thingspeak.com/channels/{channel_id}/fields/1.json?results={num_results}"
url_field2 = f"https://api.thingspeak.com/channels/{channel_id}/fields/2.json?results={num_results}"

def get_field_data(url):
    try:
        response = requests.get(url)
        response.raise_for_status()  # Levanta un error si la solicitud falló
        data = response.json()
        feeds = data['feeds']

        data_values = [feed['field1'] if 'field1' in feed else feed['field2'] for feed in feeds]
        upload_months = [datetime.strptime(feed['created_at'], "%Y-%m-%dT%H:%M:%SZ").month for feed in feeds]

        return data_values, upload_months
    except requests.RequestException as e:
        print(f"Error en la solicitud: {e}")
        return [], []
    except ValueError as e:
        print(f"Error al procesar la respuesta: {e}")
        return [], []

if(__name__=="__main__"):
    while True:
        # Obtener datos de ambos campos
        temperatura_ar, meses_subida_temp = get_field_data(url_field1)
        humedad_ar, _ = get_field_data(url_field2)
        message_to_mqtt = {}
        print(temperatura_ar)
        print(meses_subida_temp)
        print(humedad_ar)
        if (len(temperatura_ar) != num_results or len(humedad_ar) != num_results):
            print("Se necesitan más muestras")
            message_to_mqtt = {"message": "Se necesitan más muestras"}
        else:
            print(f"Últimos {num_results} datos del campo 1: {temperatura_ar}")
            print(f"Últimos {num_results} datos del campo 2: {humedad_ar}") 
            print(f"Últimos {num_results} datos del campo 3: {meses_subida_temp}") 
            #Ahora hay que crear el array de predicciones
            predecir_arr = []
            for i in range(num_results):
                predecir_arr.append(temperatura_ar[i])
                predecir_arr.append(humedad_ar[i])
                predecir_arr.append(meses_subida_temp[i])

            print(len(predecir_arr))
            print(predecir_arr)
            #message_to_mqtt = {"pred":"Llueve","prec":"0.45"}
            result = predict_with_model(predecir_arr)
            if result is not False:
                prediction, confidence = result
                print(f"Predicción: {prediction}, Confianza: {confidence}")
                message_to_mqtt = {
                    "pred":str(prediction),
                    "prec": str(confidence)
                }
            else:
                print("Falló la predicción.")
                message_to_mqtt = {"message": "Se necesitan más muestras"}

        client = mqtt.Client(client_id="", 
                                clean_session=True, 
                                userdata=None, 
                                protocol=mqtt.MQTTv311, 
                                transport="tcp")

        client.on_connect = on_connect
        client.on_publish = on_publish

        client.username_pw_set(username=None, password=None)
        client.connect(THE_BROKER, port=1883, keepalive=60)

        json_message = json.dumps(message_to_mqtt)
        
        client.loop_start()

        client.publish(THE_TOPIC, 
                        payload=json_message, 
                        qos=0, 
                        retain=True)
        client.loop_stop()
        time.sleep(900)