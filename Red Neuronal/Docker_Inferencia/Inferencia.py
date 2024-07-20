import tensorflow as tf
import numpy as np

# Ruta al modelo preentrenado
MODEL_PATH = "model.h5"

def predict_with_model(input_array):
    try:
        # Cargar el modelo
        model = tf.keras.models.load_model(MODEL_PATH)
        print("Modelo cargado exitosamente.")

        # Asegurarse de que input_array tenga el formato adecuado
        input_array = np.array([float(x) for x in input_array])
        #input_array = np.asarray(input_array)
        
        if len(input_array.shape) == 1:
            # Añadir una dimensión extra si es un array de una sola dimensión
            input_array = np.expand_dims(input_array, axis=0)

        # Realizar predicción
        predictions = model.predict(input_array)
        
        # Asumiendo que la salida del modelo es de la forma [probabilidades]
        predicted_class = np.argmax(predictions, axis=-1)
        confidence = np.max(predictions, axis=-1)

        # Devuelve la clase predicha y la confianza
        return predicted_class[0], confidence[0]
    except Exception as e:
        print(f"Error al realizar la predicción: {e}")
        return False