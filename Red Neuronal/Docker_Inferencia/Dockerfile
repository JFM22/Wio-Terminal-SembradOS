# Usar una imagen base de Python
FROM python:3.9-slim
# Establecer el directorio de trabajo dentro del contenedor
WORKDIR /app
# Copiar los archivos necesarios al directorio de trabajo del contenedor
COPY main.py Inferencia.py model.h5 /app/
# Instalar las dependencias necesarias
RUN pip install requests paho-mqtt tensorflow numpy
# Comando por defecto para ejecutar el contenedor
CMD ["python", "main.py"]