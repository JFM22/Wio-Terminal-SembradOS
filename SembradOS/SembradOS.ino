// ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀                                                bbbbbbbb                                                           dddddddd                                      
//    SSSSSSSSSSSSSSS                                            b::::::b                                                           d::::::d     OOOOOOOOO        SSSSSSSSSSSSSSS 
//  SS:::::::::::::::S                                           b::::::b                                                           d::::::d   OO:::::::::OO    SS:::::::::::::::S
// S:::::SSSSSS::::::S                                           b::::::b                                                           d::::::d OO:::::::::::::OO S:::::SSSSSS::::::S
// S:::::S     SSSSSSS                                            b:::::b                                                           d:::::d O:::::::OOO:::::::OS:::::S     SSSSSSS
// S:::::S                eeeeeeeeeeee       mmmmmmm    mmmmmmm   b:::::bbbbbbbbb    rrrrr   rrrrrrrrr   aaaaaaaaaaaaa      ddddddddd:::::d O::::::O   O::::::OS:::::S            
// S:::::S              ee::::::::::::ee   mm:::::::m  m:::::::mm b::::::::::::::bb  r::::rrr:::::::::r  a::::::::::::a   dd::::::::::::::d O:::::O     O:::::OS:::::S            
//  S::::SSSS          e::::::eeeee:::::eem::::::::::mm::::::::::mb::::::::::::::::b r:::::::::::::::::r aaaaaaaaa:::::a d::::::::::::::::d O:::::O     O:::::O S::::SSSS         
//   SS::::::SSSSS    e::::::e     e:::::em::::::::::::::::::::::mb:::::bbbbb:::::::brr::::::rrrrr::::::r         a::::ad:::::::ddddd:::::d O:::::O     O:::::O  SS::::::SSSSS    
//     SSS::::::::SS  e:::::::eeeee::::::em:::::mmm::::::mmm:::::mb:::::b    b::::::b r:::::r     r:::::r  aaaaaaa:::::ad::::::d    d:::::d O:::::O     O:::::O    SSS::::::::SS  
//        SSSSSS::::S e:::::::::::::::::e m::::m   m::::m   m::::mb:::::b     b:::::b r:::::r     rrrrrrraa::::::::::::ad:::::d     d:::::d O:::::O     O:::::O       SSSSSS::::S 
//             S:::::Se::::::eeeeeeeeeee  m::::m   m::::m   m::::mb:::::b     b:::::b r:::::r           a::::aaaa::::::ad:::::d     d:::::d O:::::O     O:::::O            S:::::S
//             S:::::Se:::::::e           m::::m   m::::m   m::::mb:::::b     b:::::b r:::::r          a::::a    a:::::ad:::::d     d:::::d O::::::O   O::::::O            S:::::S
// SSSSSSS     S:::::Se::::::::e          m::::m   m::::m   m::::mb:::::bbbbbb::::::b r:::::r          a::::a    a:::::ad::::::ddddd::::::ddO:::::::OOO:::::::OSSSSSSS     S:::::S
// S::::::SSSSSS:::::S e::::::::eeeeeeee  m::::m   m::::m   m::::mb::::::::::::::::b  r:::::r          a:::::aaaa::::::a d:::::::::::::::::d OO:::::::::::::OO S::::::SSSSSS:::::S
// S:::::::::::::::SS   ee:::::::::::::e  m::::m   m::::m   m::::mb:::::::::::::::b   r:::::r           a::::::::::aa:::a d:::::::::ddd::::d   OO:::::::::OO   S:::::::::::::::SS 
//  SSSSSSSSSSSSSSS       eeeeeeeeeeeeee  mmmmmm   mmmmmm   mmmmmmbbbbbbbbbbbbbbbb    rrrrrrr            aaaaaaaaaa  aaaa  ddddddddd   ddddd     OOOOOOOOO      SSSSSSSSSSSSSSS⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀

//Javier Fortuño Martí y Christian Vinader Márquez

#include "TFT_eSPI.h" //TFT LCD library 
#include "DHT.h" // DHT library 
//#include <Seeed_Arduino_FreeRTOS.h>
#include <rpcWiFi.h>
#include <SPI.h>
#include "RTC_SAMD51.h"
#include "DateTime.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
//#include "model.h"

#define NAME_APP "SembradOS"
#define BUZZER_PIN WIO_BUZZER
#define DHTPIN 0 //Define signal pin of DHT sensor 
//#define DHTPIN PIN_WIRE_SCL //Use I2C port as Digital Port */
#define DHTTYPE DHT11 //Define DHT sensor type 

//RTC
RTC_SAMD51 rtc;
DateTime now = DateTime(F(__DATE__), F(__TIME__));

//Config. Wifi
const char* ssid = "MIWIFI_5G_z5wx"; 
const char* password = "e5r67uddw5mk";
// Configuración de ThingSpeak
const char* api_key = "TZNMIJZKWEIX2BS6"; // Tu clave de API de ThingSpeak
const char* server = "api.thingspeak.com";
const int port = 80;
bool TSstate = false; //Variable para guardar el estado de la conexión

//Initializations
DHT dht(DHTPIN, DHTTYPE);
TFT_eSPI tft; // Inicializar TFT LCD 

float temperatura; //Assign variable to store temperature 
float humedad;//dht.readHumidity(); //Assign variable to store humidity 
int light; //Assign variable to store light sensor values
int soil_moisture;
int sensorPin = A1;
int sensorValue = 0;
int gatherDataFromSensor1 = true;

//Broker MQTT
const char *ID = "Wio-Terminal-Client";  // Name of our device, must be unique
const char *subTopic = "idc/wioterminal/UserW1";  // Topic to subcribe to
const char *serverMQTT = "broker.hivemq.com"; // Server URL
String nombreMQTT = "hivemq";
bool MQTTstate = false; //Estado de la conexión con el broker
String err_msg = "No hay conexion"; //Mensaje predeterminado

WiFiClient wifiClient;
PubSubClient client(wifiClient);

String prediction = "No llueve"; 
float probability;

// Enums para los menús y botones
enum MenuType {
    MAIN_MENU,
    SENSORS_MENU,
    PREDICT_MENU,
    SETTINGS_MENU
};

enum ButtonType {
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_SELECT,
    BUTTON_BACK
};

const char* OUTPUTS[] = {
    "Despejado",
    "Podria llover",
    "Llueve"
};

class App {
  public:
    //App() {}
    App(TFT_eSPI &tft) //Constructor
        : tft(tft), spr(&tft), spr_sensores(&tft), currentMenu(MAIN_MENU), menuSelectedOption(0) { //asignaciones del tft, spr, currentMenu y MenuSelectedOption
        //setMenu(MAIN_MENU);
        printMenuFunction = &App::nop; //Función nop del objeto App que es basicamente una función vacía
        buttonPressedFunction = &App::nop;
        updateMenuFunction = &App::nop;
    }
    //Iniciar la aplicación
    void start(){
      tft.fillScreen(TFT_BLACK);
      tft.fillCircle(160, 120, 50, 0x4380); // Círculo verde
      tft.fillCircle(160, 120, 40, TFT_WHITE); // Círculo blanco
      tft.fillRect(140, 110, 40, 20, 0x4380); // Raya horizontal
      tft.fillRect(150, 100, 20, 40, 0x4380); // Raya vertical
      tft.drawString("Iniciando...", 5, 220);

      tft.setTextColor(TFT_WHITE);
      tft.setTextSize(2);
      tft.setTextDatum(TC_DATUM); //Centrado
      tft.drawString(NAME_APP, 160, 184); 
      tft.setTextDatum(TL_DATUM);
      
      char notes[] = "aabbC";
      int beats[] = { 1, 1, 1, 1, 2};
      int tempo = 200;
      for(int i = 0; i < sizeof(notes)/sizeof(notes[0]); i++) {
          this->playNote(notes[i], beats[i] * tempo);
          delay(tempo / 5);    /* delay between notes */
      }     
      tft.drawString("Iniciando......", 5, 220);
    }
    void setMenu(MenuType menu) {
      //if(xSemaphoreTake(changingMenuSemaphore, portMAX_DELAY) == pdTRUE){
      spr.deleteSprite();
      spr_sensores.deleteSprite();
      currentMenu = menu;
      Serial.print(menu);
      switch (menu) {
          //Asignamos las direcciones de las funciones de gestión del menú
          case MAIN_MENU:
              printMenuFunction = &App::printMainMenu; //Le decimos que está dentro de App o no lo aceptará
              buttonPressedFunction = &App::buttonPressedMainMenu;
              updateMenuFunction = &App::nop;
              menuSelectedOption = 0;
              break;
          case SENSORS_MENU:
              printMenuFunction = &App::printSensorsMenu;
              buttonPressedFunction = &App::buttonPressedSensorsMenu;
              updateMenuFunction = &App::updateSensorsMenu;
              break;
          case PREDICT_MENU:
              printMenuFunction = &App::printPredictMenu;
              buttonPressedFunction = &App::buttonPressedPredictMenu;
              updateMenuFunction = &App::updatePredictMenu;
              break;
          case SETTINGS_MENU:
              printMenuFunction = &App::printSettingsMenu;
              buttonPressedFunction = &App::buttonPressedSettingsMenu;
              updateMenuFunction = &App::updateSettingMenu;
              break;
      }
      //}
      Serial.println(menu);
      //printMenuFunction es un puntero a función, así que hay que desreferenciarlo y entonces ejecutarlo
      (this->*printMenuFunction)();
      (this->*updateMenuFunction)();
      //xSemaphoreGive(changingMenuSemaphore);
    }
    //Manejo de inputs de botones a nivel interno de la app
    void handleButtonPress(ButtonType button) {
        //paso de hacer más funciones para cada menú, que se maneje por if's
        //Manejo acciones botones en menu main
        if(currentMenu == MAIN_MENU){
          switch (button) {
            case BUTTON_UP:
                menuSelectedOption = (menuSelectedOption - 1 + 3) % 3;
                playNote('a', 30);
                //delay(200); Mejor hacer el delay en la función principal de manejo de botones
                break;
            case BUTTON_DOWN:
                menuSelectedOption = (menuSelectedOption + 1) % 3;
                playNote('a', 30);
                //delay(200);
                break;
            case BUTTON_SELECT:
                setMenu(static_cast<MenuType>(menuSelectedOption + 1)); //casteo a MenuType para pasar como argumento
                playNote('f', 30);
                return;
                //delay(200);
                break;
            }
        //Manejo acciones botones en menu sensores
        }else if(currentMenu == SENSORS_MENU){
          switch (button){
            case BUTTON_BACK:
                setMenu(MAIN_MENU);
                playNote('a', 30);
                //delay(200);
                return;
                break;
          }
        //Manejo acciones botones en menu predecir
        }else if(currentMenu == PREDICT_MENU){
          switch (button){
            case BUTTON_BACK:
                setMenu(MAIN_MENU);
                playNote('a', 30);
                //delay(200);
                return;
                break;
          }
        //Manejo acciones botones en menu ajustes
        }else if(currentMenu == SETTINGS_MENU){
          switch (button){
            case BUTTON_BACK:
                setMenu(MAIN_MENU);
                playNote('a', 30);
                //delay(200);
                return;
                break;
          }
        }
        (this->*buttonPressedFunction)(); //actualizamos la pantalla
    }
    void update() { 
        (this->*updateMenuFunction)();
    }
    //Tonos (Sonido)
    void playTone(int tone, int duration) {
      for (long i = 0; i < duration * 1000L; i += tone * 2) {
          digitalWrite(BUZZER_PIN, HIGH);
          delayMicroseconds(tone);
          digitalWrite(BUZZER_PIN, LOW);
          delayMicroseconds(tone);
          }
    }
    //Notas (Sonido)
    void playNote(char note, int duration) {
        char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
        int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };

        // play the tone corresponding to the note name
        for (int i = 0; i < 8; i++) {
            if (names[i] == note) {
                playTone(tones[i], duration);
            }
        }
    }
    //Función pública para actualizar los valores de los sensores
    void updateSensorsData(){
      updateSensorsMenu();
    }

  private:
    //Printeo de cada menú de la aplicación
    void printMainMenu() {
        tft.fillScreen(0x067F);
        tft.setTextSize(3);

        tft.fillRect(0, menuSelectedOption * 33 + 72, 180, 31, TFT_BLUE);
        tft.drawRect(0,71,180,0,TFT_BLACK); //draw rectangle with fill color 
        tft.setTextColor(TFT_BLACK);

        tft.fillRect(180, 0, 140, 240, colorFondo1);

        tft.drawRect(180, 0, 0, 240, TFT_BLACK);

        tft.fillCircle(30, 35, 20, 0x4380);
        tft.fillCircle(30, 35, 15, TFT_WHITE);
        tft.fillRect(18, 30, 25, 10, 0x4380);
        tft.fillRect(26, 23, 10, 25, 0x4380);

        tft.setTextSize(2);
        tft.drawString(NAME_APP, 56, 30);

        buttonPressedMainMenu();
    }
    void printSensorsMenu() {
        tft.fillScreen(0x067F); //Fill background with white color
        tft.fillRect(0,0,320,54,colorFondo1);
        tft.setTextColor(TFT_WHITE); //Setting text color
        tft.setTextSize(2); //Setting text size 
        tft.setTextDatum(TC_DATUM);
        tft.drawString("Datos de sensores",160,20); //Drawing a text string 
        tft.setTextDatum(TL_DATUM);

        tft.drawFastVLine(150,54,190,TFT_DARKGREEN); //Drawing verticle line
        tft.drawFastHLine(0,140,320,TFT_DARKGREEN); //Drawing horizontal line

        //Setting temperature
        tft.setTextColor(TFT_BLACK);
        tft.setTextSize(2);
        tft.drawString("Temperature",10,65);
        tft.setTextSize(3);
        //tft.drawNumber(t,50,95); //Display temperature values 
        tft.drawString("C",90,95);

        //Setting humidity
        tft.setTextSize(2);
        tft.drawString("Humidity",25,160);
        tft.setTextSize(3);
        //tft.drawNumber(h,30,190); //Display humidity values 
        tft.drawString("%RH",70,190);

        //Setting soil moisture
        tft.setTextSize(2);
        tft.drawString("Soil Moisture",160,65);
        tft.setTextSize(3);
        //tft.drawNumber(sensorValue,200,95); //Display sensor values as percentage  
        tft.drawString("%",240,95);
        
        //Setting light 
        tft.setTextSize(2);
        tft.drawString("Light",200,160);
        tft.setTextSize(3);
      //Map sensor values 
        Serial.printf("luz: %d", light);
        //tft.drawNumber(light,205,190); //Display sensor values as percentage  
        tft.drawString("%",245,190);
    }
    void printPredictMenu() {
        // Implementación del menú de predicción
        tft.fillScreen(0x067F);
        tft.fillRect(0,0,320,54,colorFondo1);
        tft.setTextDatum(TC_DATUM);
        tft.setTextSize(2);
        tft.setTextColor(TFT_WHITE);
        tft.drawString(F("Predecir el tiempo"), 160, 20);
        tft.setTextColor(TFT_BLACK);
        tft.drawFastVLine(160,80,60, TFT_BLACK);
        tft.setTextPadding(0);
        tft.setTextSize(2);
        tft.drawString(F("Temp (C)"), 80, 140);
        tft.drawString(F("R.H."), 240, 140);
        tft.setTextSize(4);
        tft.setTextDatum(TL_DATUM);
    }
    void printSettingsMenu() {
        // Implementación del menú de configuración
        tft.fillScreen(0x067F);
        tft.fillRect(0,0,320,54,colorFondo1);
        tft.setTextDatum(TC_DATUM);
        tft.setTextSize(2);
        tft.setTextColor(TFT_WHITE);
        tft.drawString(F("Informacion"), 160, 20);
        tft.setTextColor(TFT_BLACK);

        tft.setTextDatum(TL_DATUM);
        tft.setTextSize(3);
        tft.drawString(F("WiFi"), 5, 64);
        tft.drawString(F("MQTT"), 5, 100);
        tft.drawString(F("ThingSpeak"), 5, 131);
        tft.drawString(F("Broker"), 5, 162);

        tft.setTextDatum(TR_DATUM);
        tft.setTextSize(2);
        tft.drawString(nombreMQTT, 299, 165);
        tft.setTextSize(1);
        tft.drawString(String(subTopic), 299, 180);
        tft.setTextDatum(TL_DATUM);

        tft.setTextSize(1);
        tft.drawString("Javier & Christian, SembradOS", 5, 228);
    }
    //Actualiza el menú sensores con los nuevos datos de los sensores
    void updateSensorsMenu() {
        // Aquí se actualizarían los valores de los sensores en el menú de sensores
        //if(currentMenu == SENSORS_MENU && xSemaphoreTake(changingMenuSemaphore, 0) == pdTRUE){
        spr_sensores.deleteSprite();
        spr_sensores.setTextColor(TFT_BLACK);
        spr.setTextPadding(0);

        spr_sensores.createSprite(35,21);
        spr_sensores.fillSprite(0x067F);
        spr_sensores.setTextSize(3);
        spr_sensores.createSprite(35,21);
        spr_sensores.drawString(String(temperatura),0,0);
        spr_sensores.pushSprite(50,95);

        spr_sensores.deleteSprite();
        spr_sensores.createSprite(35,21);
        spr_sensores.fillSprite(0x067F);
        spr_sensores.drawString(String(humedad),0,0);
        spr_sensores.pushSprite(30,190);

        spr_sensores.deleteSprite();
        spr_sensores.createSprite(40,21);
        spr_sensores.fillSprite(0x067F);
        spr_sensores.drawString(String(soil_moisture),0,0);
        spr_sensores.pushSprite(200,95);

        spr_sensores.deleteSprite();
        spr_sensores.createSprite(35,21);
        spr_sensores.fillSprite(0x067F);
        spr_sensores.drawString(String(light),0,0);
        spr_sensores.pushSprite(205,190);
        spr_sensores.deleteSprite();

        //xSemaphoreGive(changingMenuSemaphore);
        //}
    }
    void updatePredictMenu(){
      tft.setTextDatum(TC_DATUM);
      tft.setTextPadding(20);
      tft.setTextColor(TFT_BLACK, 0x067F);
      tft.setTextSize(4);
      tft.drawString(String(round(humedad*10)/10), 80, 90);
      tft.drawString(String(round(temperatura)), 240, 90);
      tft.setTextPadding(320);
      tft.setTextSize(2);
      if(err_msg != ""){
        tft.drawString(err_msg, 160 ,200);
        //tft.setTextSize(1);
        //tft.drawString("Si el problema persiste, comprueba las conexiones", 160 ,212);
      }else{
        tft.drawString(prediction + " (" + String(round(probability*100)) + "%)", 160 ,200);
      }
      tft.setTextDatum(TL_DATUM);
    }
    void updateSettingMenu(){
      spr.deleteSprite();
      spr.createSprite(33,33);
      spr.fillSprite(0x067F);
      spr.fillCircle(16, 16, 16, TFT_BLACK); //280, 75, 17, TFT_BLACK
      if(WiFi.status() == WL_CONNECTED){
        spr.fillCircle(16, 16, 14, TFT_GREEN);
      }else{
        spr.fillCircle(16, 16, 14, TFT_RED);
      }
      spr.pushSprite(263,59);

      spr.deleteSprite();
      spr.createSprite(33,33);
      spr.fillSprite(0x067F);
      spr.fillCircle(16, 16, 16, TFT_BLACK); //280, 75, 17, TFT_BLACK
      if(MQTTstate){
        spr.fillCircle(16, 16, 14, TFT_GREEN);
      }else{
        spr.fillCircle(16, 16, 14, TFT_RED);
      }
      spr.pushSprite(263,93);

      spr.deleteSprite();
      spr.createSprite(33,33);
      spr.fillSprite(0x067F);
      spr.fillCircle(16, 16, 16, TFT_BLACK); //280, 75, 17, TFT_BLACK
      if(TSstate){
        spr.fillCircle(16, 16, 14, TFT_GREEN);
      }else{
        spr.fillCircle(16, 16, 14, TFT_RED);
      }
      spr.pushSprite(263,128);
    }
    //Printeo de logos en el menú principal
    void drawGraphLogo() {

        spr.deleteSprite();
        spr.createSprite(105,101);
        spr.fillSprite(colorFondo1);
        int dataPoints[] = {20, 50, 70, 40, 90, 30, 60};

        // Calcular el número total de puntos de datos
        int numDataPoints = sizeof(dataPoints) / sizeof(dataPoints[0]);

        // Definir el tamaño y los límites del gráfico
        int graphWidth = 99;
        int graphHeight = 90;
        int graphX = 5;
        int graphY = 0;
        int maxValue = 100;

        // Dibujar el eje X
        spr.drawLine(graphX, graphY + graphHeight, graphX + graphWidth, graphY + graphHeight, TFT_BLACK);

        // Dibujar el eje Y
        spr.drawLine(graphX, graphY, graphX, graphY + graphHeight, TFT_BLACK);

        // Dibujar las líneas conectando los puntos de datos
        int interval = graphWidth / (numDataPoints - 1);
        for (int i = 0; i < numDataPoints - 1; i++) {
          int x1 = graphX + i * interval;
          int y1 = graphY + graphHeight - map(dataPoints[i], 0, maxValue, 0, graphHeight);
          int x2 = graphX + (i + 1) * interval;
          int y2 = graphY + graphHeight - map(dataPoints[i + 1], 0, maxValue, 0, graphHeight);
          spr.drawLine(x1, y1, x2, y2, TFT_RED);
        }

        // Dibujar los puntos de datos
        for (int i = 0; i < numDataPoints; i++) {
          int x = graphX + i * interval;
          int y = graphY + graphHeight - map(dataPoints[i], 0, maxValue, 0, graphHeight);
          spr.fillCircle(x, y, 3, TFT_RED);
        }
        spr.pushSprite(200,70);
    }
    void drawIALogo() {
        // Limpiar el sprite
        spr.deleteSprite();
        spr.createSprite(105,101);
        spr.fillSprite(colorFondo1);
        spr.setTextSize(4);
        //spr.setTextDatum(MC_DATUM); // Alineación en el centro horizontal
        spr.drawString("Tiny", 0, 0);
        spr.setTextSize(7);
        spr.drawString("ML", 0, 31);
        spr.setTextSize(3);
        spr.pushSprite(200,70);
    }
    void drawOptionsLogo() {
        spr.deleteSprite();
        spr.createSprite(105,101);
        spr.fillSprite(colorFondo1);
        spr.fillCircle(50, 50, 50, TFT_BLACK);
        spr.fillCircle(50, 50, 14, colorFondo1);

        spr.fillCircle(25, 5, 14, colorFondo1);
        spr.fillCircle(75, 5, 14, colorFondo1);

        spr.fillCircle(0, 50, 15, colorFondo1);
        spr.fillCircle(100, 50, 15, colorFondo1);

        spr.fillCircle(25, 95, 14, colorFondo1);
        spr.fillCircle(75, 95, 14, colorFondo1);

        spr.pushSprite(200,70);
    }
    //Manejo de input de los botones a nivel visual de los menús
    void buttonPressedMainMenu() {
        char* cat[] = {"Sensores", "Predecir", "Ver Config."};
        spr.deleteSprite();
        //spr.createSprite(179,168);
        spr.createSprite(179,100);
        spr.fillScreen(0x067F);
        spr.fillRect(0,menuSelectedOption*33, 180,34,TFT_BLUE);
        spr.setTextSize(3);
        //spr.drawString(cat[menu_selected_option],10,8); //draw a text string 
        //spr.pushSprite(0,menu_selected_option*33+72);

        spr.setTextColor(TFT_BLACK); //set text color 
        spr.drawString("Sensores",10,6); //draw a text string 
        spr.drawRect(0,33,180,0,TFT_BLACK); //draw rectangle with fill color 

        spr.drawString("Predecir",10,39); //draw a text string 
        spr.drawRect(0,66,180,0,TFT_BLACK); //draw rectangle with fill color 

        spr.drawString("Info.",10,72); //draw a text string 
        spr.drawRect(0,99,180,0,TFT_BLACK); //draw rectangle with fill color 
        spr.pushSprite(0,73);
        //spr.createSprite(179,69);
        spr.deleteSprite();
        spr.createSprite(179,63);
        spr.fillScreen(0x067F);
        spr.setTextSize(1);
        switch(menuSelectedOption){
          case 0:
            spr.drawString("En esta ventana se muestran", 5, 0);
            spr.drawString("los datos recogidos por los", 5, 9);
            spr.drawString("diferentes sensores", 5, 18);
            spr.pushSprite(0,177);
            drawGraphLogo();
            break;
          case 1:
            spr.drawString("En esta ventana se muestra", 5, 0);
            spr.drawString("la temp. y humedad actual", 5, 9);
            spr.drawString("junto con la prediccion del", 5, 18);
            spr.drawString("tiempo esperado", 5, 27);
            spr.pushSprite(0,177);
            drawIALogo();
            break;
          case 2:
            //drawOptionsLogo();
            spr.drawString("En esta ventana se muestra el", 5, 0);
            spr.drawString("estado actual del sistema", 5, 9);
            spr.pushSprite(0,177);
            drawOptionsLogo();
          break;
        }
    }
    void buttonPressedSensorsMenu() {//vacio
        // Lógica para el manejo de botones en el menú de sensores
    }
    void buttonPressedPredictMenu() {//vacio
        // Lógica para el manejo de botones en el menú de predicción
    }
    void buttonPressedSettingsMenu() {//vacio
        // Lógica para el manejo de botones en el menú de configuración
    }
    //Función NOP
    void nop(){}

    //Variables de la APP
    TFT_eSPI &tft;
    TFT_eSprite spr;
    TFT_eSprite spr_sensores;
    MenuType currentMenu;
    int menuSelectedOption;
    uint16_t colorFondo1 = 0x04FA;

    //Punteros a funciones DE la propia App (no sirven externas)
    void (App::*printMenuFunction)(); 
    void (App::*buttonPressedFunction)();
    void (App::*updateMenuFunction)();
};

//Documento JSON de como máximo 2 campos
const size_t bufferSize = JSON_OBJECT_SIZE(2);
DynamicJsonDocument jsonDoc(bufferSize);
//Callback de la conexión al broker MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String payl = "";
  //guardamos el payload
  for (int i=0;i<length;i++) {
    payl += (char)payload[i];
    Serial.print((char)payload[i]);
  }
  //Deserializamos el Json
  DeserializationError error = deserializeJson(jsonDoc, payl);
  if (error) {
    Serial.print("Error al parsear JSON: ");
    Serial.println(error.c_str());
    err_msg = "Algo ha salido mal";
    jsonDoc.clear();
    return;
  }
  //Si hay un campo "message" es porque ha ocurrido algo
  if(!jsonDoc["message"].isNull()){
    err_msg = jsonDoc["message"].as<String>();
    jsonDoc.clear();
    return;
  }

  int pred = jsonDoc["pred"].as<int>();
  //pred es un número, por lo que hay que sacar la predicción que indica
  prediction = OUTPUTS[pred];
  probability = jsonDoc["prec"].as<float>();
  err_msg = "";
  jsonDoc.clear();
  Serial.println();
}

//Reconectarse al broker MQTT
void reconnect() {
  // Loop until we're reconnected
  int maxloops = 0;
  //en un principio se intentaba reconectar 3 veces, pero por ahora que lo intente solo 1 vez
  while (!client.connected() && maxloops<1)
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(ID)) {
      //client.setKeepAlive(3600);
      Serial.println("connected");
      client.subscribe(subTopic);
      Serial.print("Subcribed to: ");
      Serial.println(subTopic);
      MQTTstate = true;
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      //Serial.println(" try again in 5 seconds");
      MQTTstate = false;
      // Wait 5 seconds before retrying
      delay(5000);
    }
    maxloops = maxloops+1;
  }
}

//Conectarse al Wifi
void startWiFi(){
  Serial.println("Inicializando...");
  WiFi.begin(ssid, password);
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000) {
      delay(500);
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("Conectado a la red WiFi");
      TSstate = true;
    } else {
      Serial.println("");
      Serial.println("No se pudo conectar a la red WiFi en 5 segundos. Continuando sin conexión.");
    }
    Serial.println(WiFi.status());
}

App app = App(tft);
void setup() {
    Serial.begin(9600);
    rtc.begin();
    if (!Serial){delay(2000);}

    dht.begin(); //Start DHT sensor 
    tft.begin();
    tft.setRotation(3); //set TFT LCD rotation 
    tft.fillScreen(TFT_BLACK);
    tft.setTextPadding(320);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString("Iniciando.", 5, 220);

    startWiFi();
    tft.drawString("Iniciando...", 5, 220);
    rtc.adjust(now);
    now = rtc.now();

    pinMode(WIO_5S_UP, INPUT);
    pinMode(WIO_5S_DOWN, INPUT);
    pinMode(WIO_KEY_A, INPUT_PULLUP);
    pinMode(WIO_KEY_C, INPUT);
    pinMode(WIO_KEY_B, INPUT);
    pinMode(WIO_5S_PRESS, INPUT_PULLUP);
    pinMode(WIO_BUZZER, OUTPUT);
    pinMode(WIO_LIGHT, INPUT); //Set light sensor pin as INPUT

    //iniciar aplicación
    app.start();
    tft.drawString("Iniciando.........", 5, 220);
    //Primera lectura de sensores
    temperatura = dht.readTemperature();
    humedad = dht.readHumidity();
    sensorValue = analogRead(sensorPin); //Store sensor values 
    soil_moisture = map(sensorValue,1023,400,0,100); //Map sensor values
    light = analogRead(WIO_LIGHT);
    light = map(light,0,1023,0,100);

    //Configuramos la conexión con el broker MQTT
    client.setServer(serverMQTT, 1883);
    client.setCallback(callback);
    reconnect();
    app.setMenu(MAIN_MENU);
}

//Temporizadores
unsigned long lastTimeSensorRead = millis();
unsigned long lastTimeSensorSend = millis();
unsigned long lastWifiRefresh = millis();
unsigned long reconnectDelay = millis();

void loop() {
  //client es la conexión con el broker MQTT
  //Para que no se quede en bucle en reconnect si no hay conexión
  if (!client.connected() && WiFi.status() == WL_CONNECTED && (labs((long)(millis() - reconnectDelay)) > 20000)) {
    reconnect();
    delay(100);
    reconnectDelay = millis();
  //Si está conectado, revisa el MQTT por nuevos mensajes
  } else if (client.connected() && WiFi.status() == WL_CONNECTED){
    client.loop();
  }

  //Detecta si se están pulsando botones
  while(TaskButtonHandler()){} //Puede que el usuario quiera seguir interactuando por el menú

  //Lee datos de sensores cada 5 segundos
  if(labs((long)(millis() - lastTimeSensorRead)) > 5000){//cada 5s
    TaskReadSensors();
    lastTimeSensorRead = millis();
  }

  //Envía datos de sensores a ThingSpeak cada x minutos
  if(labs((long)(millis() - lastTimeSensorSend)) > 800000){//cada 15 minutos = 900000ms
    //Si no está conectado al Wifi, que se conecte.
    if(WiFi.status() != WL_CONNECTED){
      startWiFi();
    }
    //Si está conectado al Wifi, que envie datos a TS
    if (WiFi.status() == WL_CONNECTED){
      TaskSendDataToThingSpeak();
      int maxcount = 0;
      //Si no es capaz de establecer la conexión con TS
      while(!TSstate && maxcount<2 && WiFi.status() == WL_CONNECTED){ //Si no ha podido enviar datos, que lo vuelva a intentar 2 veces
        //startWiFi();
        TaskSendDataToThingSpeak();
        delay(500);
        maxcount = maxcount+1;
      }
    }
    lastTimeSensorSend = millis();
  }
  //Refrescar conexión WiFi cada x tiempo para evitar problemas
  if(labs((long)(millis() - lastWifiRefresh)) > 1700000){//cada ~30min
    startWiFi();
    reconnect();
    lastWifiRefresh = millis();
  }

  //actualizamos la aplicación
  app.update();
  delay(50);
}

//Tarea que gestiona los sensores
void TaskReadSensors() {
    if(gatherDataFromSensor1){
      temperatura = dht.readTemperature();
      sensorValue = analogRead(sensorPin); //Store sensor values 
      soil_moisture = map(sensorValue,1023,400,0,100); //Map sensor values
      Serial.println("soild moisture: "+String(soil_moisture));
    }else{
      humedad = dht.readHumidity();
      light = analogRead(WIO_LIGHT);
      light = map(light,0,1023,0,100);
      Serial.println("light: "+String(light));
    }
    gatherDataFromSensor1 = !gatherDataFromSensor1;
}

//Tarea que gestiona los inputs de los botones a nivel de inputs físicos
bool TaskButtonHandler() {
      if (digitalRead(WIO_KEY_B) == LOW) {
          app.handleButtonPress(BUTTON_DOWN);
          delay(200);//vTaskDelay(200 / portTICK_PERIOD_MS);
          return true;
      } else if (digitalRead(WIO_KEY_A) == LOW) {
          app.handleButtonPress(BUTTON_UP);
          delay(200);//vTaskDelay(200 / portTICK_PERIOD_MS);
          return true;
      } else if (digitalRead(WIO_5S_PRESS) == LOW) {
          app.handleButtonPress(BUTTON_SELECT);
          delay(200);//vTaskDelay(200 / portTICK_PERIOD_MS);
          return true;
      } else if (digitalRead(WIO_KEY_C) == LOW) {
          app.handleButtonPress(BUTTON_BACK);
          delay(200);//vTaskDelay(200 / portTICK_PERIOD_MS);
          return true;
      }
    return false;
}

//Tarea que envía los datos a la plataforma
void TaskSendDataToThingSpeak(){
  WiFiClient clientTS;

  //String solo para printear por Serial
  String url = "GET /update?api_key=" + String(api_key);
  url += "&field1=" + String(temperatura);
  url += "&field2=" + String(humedad);
  url += "&field3=" + String(soil_moisture);
  url += "&field4=" + String(light)+ " HTTP/1.1\n\n";

  Serial.print("Solicitando URL: ");
  Serial.println(url);
  
  //Nos conectamos a TS, si no podemos, detenemos el envío y reintentamos
  if (!clientTS.connect(server, port)) {
      Serial.println("Connection failed.");
      Serial.println("Waiting 5 seconds before retrying...");
      delay(4000);
      TSstate = false; //No hemos podido conectarnos
      clientTS.stop();
      return;
  }

  //Subida de datos
  Serial.println("Soil_moisture: "+String(round(soil_moisture))+" ,light: "+String(round(light)));
  String postStr = String(api_key);
  postStr += "&field1=" + String(temperatura);
  postStr += "&field2=" + String(humedad);
  postStr += "&field3=" + String(round(soil_moisture));
  postStr += "&field4=" + String(round(light));
  postStr += "\r\n\r\n";

  //make an HTTP POST request
  clientTS.print("POST /update HTTP/1.1\n");
  clientTS.print("Host: api.thingspeak.com\n");
  clientTS.print("Connection: close\n");
  clientTS.print("X-THINGSPEAKAPIKEY: "+String(api_key)+"\n");
  clientTS.print("Content-Type: application/x-www-form-urlencoded\n");
  clientTS.print("Content-Length: ");
  clientTS.print(postStr.length());
  clientTS.print("\n\n");
  clientTS.print(postStr); //aquí los datos anteriores

  int maxloops = 0;

  //wait for the server's reply to become available
  while (!clientTS.available() && maxloops < 1500) {
      maxloops++;
      delay(1); //delay 1 msec
  }
  if (clientTS.available() > 0) {
      //read back one line from the server
      String line = clientTS.readString(); // Read from the server response
      // Proceed various line-endings
      line.replace("\r\n", "\n");
      line.replace('\r', '\n');
      line.replace("\n", "\r\n");
      Serial.println(line);
      TSstate = true;
  } else {
      Serial.println("client.available() timed out ");
      TSstate = false;
  }

  Serial.println("Closing connection.");
  clientTS.stop();
}