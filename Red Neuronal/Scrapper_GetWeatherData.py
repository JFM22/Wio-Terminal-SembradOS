import numpy as np
import pandas as pd
from datetime import date, timedelta, datetime
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.chrome.service import Service
from bs4 import BeautifulSoup

# URL base para obtener los datos del clima
lookup_URL = 'https://www.wunderground.com/history/daily/es/valencia/LEVC/date/{}-{}-{}.html'

# Rango de fechas para obtener los datos
start_date = datetime(2022, 1, 1)
end_date = start_date + pd.Timedelta(days=730)

# Configuración de las opciones de Chrome
options = webdriver.ChromeOptions()
options.add_argument('headless')

# Ruta al ejecutable de ChromeDriver
chromedriver_path = "D:/Descargas/chromedriver.exe"  # Asegúrate de que esta es la ruta correcta

# Configuración del servicio de ChromeDriver
service = Service(executable_path=chromedriver_path)
driver = webdriver.Chrome(service=service, options=options)

data = []

# Bucle para recorrer las fechas y obtener los datos
while start_date <= end_date:
    print('Gathering data from:', start_date)
    formatted_lookup_URL = lookup_URL.format(start_date.year, start_date.month, start_date.day)
    
    driver.get(formatted_lookup_URL)
    print("Opened URL")

    # Espera hasta que las tablas estén presentes en la página
    tables = WebDriverWait(driver, 20).until(EC.presence_of_all_elements_located((By.CSS_SELECTOR, "table")))

    # Parsear el contenido de la página con BeautifulSoup
    soup = BeautifulSoup(driver.page_source, 'lxml')
    tables = soup.find_all('table')
    if len(tables) > 1:  # Asegúrate de que la tabla existe
        table = tables[1]

        # Obtener los encabezados de la tabla
        table_head = table.findAll('th')
        output_head = []
        for head in table_head:
            output_head.append(head.text.strip())

        # Escribir las filas de la tabla
        output_rows = []
        rows = table.findAll('tr')
        for j in range(1, len(rows)):
            table_row = rows[j]
            columns = table_row.findAll('td')
            output_row = []
            for column in columns:
                value = column.text.strip()
                output_row.append(value)

            if len(output_row) > 0:
                output_row = [start_date.date()] + output_row
                data.append(output_row)

    # Incrementar la fecha en un día
    start_date += timedelta(days=1)

# Escribir los datos en un archivo CSV
print("Now, Write the Data.")
output_head = ["Date"] + output_head
df = pd.DataFrame(data, columns=output_head)
df.to_csv("./WeatherData.csv", index=False)
print("Raw data has been output as CSV for future use.")

# Cerrar el driver
driver.quit()
print("Program Complete!")
