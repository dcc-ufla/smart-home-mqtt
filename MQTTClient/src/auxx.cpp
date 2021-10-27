/*

#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>

// ----------------------------- COMUNICAÇÃO ----------------------------------

const char* ssid = "Silvio";
const char* password = "semsenha";

WiFiServer server(80);
String header;

// ------------------------------ START RELAY ----------------------------------

// Seta a saida do réle para D5/GND/%V
const int relay = 14;

// HIGH = OFF
// LOW = ON

// ------------------------------ END RELAY ----------------------------------

// ------------------------------ START LCD DISPLAY ----------------------------------

// Define o numero de colunas e linhas do display LCD/SDA(azul)-D2/SCL-D1/5V
int lcdColumns = 16;
int lcdRows = 2;

// Cria instancia do display
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

//------------------------------- END LCD DISPLAY ------------------------------

//------------------------------- START TEMPERATURE SENSORS ------------------------------
// Data wire is plugged into port on the ESP8266 3.3V/D3/GND
#define ONE_WIRE_BUS 0

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// Seta variaveis para temperaturas de leitura
float tempSensor1, tempSensor2;

// Seta variaveis para limites definidos de temperatura intera
float setTemp = 18.3;

// Define ranges de valor de temperatura para a parte traseira da geladeira
float mediumTemp = 28;
float highTemp = 30;
float lowTemp = 24;

// Seta variaveis para limites definidos de temperatura intera
float freezerTHTemp;

// Endereço dos sensores
uint8_t sensor1[8] = { 0x28, 0xC1, 0x61, 0x79, 0x97, 0x11, 0x03, 0xC9  };
uint8_t sensor2[8] = { 0x28, 0x32, 0x1B, 0x79, 0x97, 0x09, 0x03, 0xA8 };

//------------------------------- END TEMPERATURE SENSORS ------------------------------


//------------------------------- START MOSFET MODULO ------------------------------

// Define no controlador de velocidade na saida D6/GND
const int mosfet = 12;

//------------------------------- END MOSFET MODULO ------------------------------

// Tempo para pode ativar ou desativar o relé novamente
int timeToWait;

// Estado da geladeira
bool state;

unsigned long time_now = 0;

// Tempo parar enviar os dados para a tabela
int timeToSendTemp = 1800000;
int time_now_SendTemp = 0;

void setup() 
{

  Serial.begin(115200);
  delay(100);

  // Conectar com a rede wifi
  Serial.println("Iniciando WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);  
  Serial.println("");
  Serial.println("Conectando em: ");
  Serial.println(ssid);  

  // Espera conexão
  Serial.println("Esperando conexão");
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  
  // Conectado com sucesso
  Serial.println("");
  Serial.print("Conectado à ");
  Serial.println(ssid);
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  // Iniciando servidor
  server.begin();
  
  // Defini tempo entre leituras como 1 segundo
  timeToWait = 120000;

  // Inicia saida do mosfete
  pinMode(mosfet, OUTPUT);
  Serial.println("pinMode mosfet");

  // Define a saida no range de 1-100
  analogWriteRange(100); 
  analogWriteFreq(10000);

  // Inicia saida do réle
  pinMode(relay, OUTPUT);
  Serial.println("pinMode réle");

  // Inicia o réle como desligado
  digitalWrite(relay, LOW);//DONE
  Serial.println("Desligando réle");
  
  // Inicia instancia dos sensores de temperatura
  sensors.begin(); 

  // Inicia instancia do display lcd
  lcd.begin();

  // Liga luz do display 
  lcd.backlight();

  // Sinaliza que esta iniciando
  lcd.print("Starting");
  Serial.println("Starting");

  // Recebe temperatura primeiro sensor
  tempSensor1 = GetCorrectionFactor(GetTemperature(sensor1));
  Serial.println("Primeira: " + String(tempSensor1, 1));

  // Verifica se a temperatura ta abaixo da definida
  // Se tiver acima do que deve estar liga geladeira
  if(tempSensor1 > setTemp)
  {
    // Liga relé
    digitalWrite(relay, LOW);//DONE
    Serial.println("Réle on");
    state = true;
  }

  // Se estiver abaixo do que deve estar, desliga geladeira
  else if(tempSensor1 < setTemp)
  {
    // Desliga relé
    digitalWrite(relay, HIGH);//DONE
    Serial.println("Réle off");
    state = false;
  }

  // Recebe o momento atual
  time_now = millis();
  time_now_SendTemp = millis();

    // ------------------------------------ OAT ------------------------------------------

  ArduinoOTA.onStart([]() {
    Serial.println("Inicio...");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("nFim!");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progresso: %u%%r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Erro [%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Autenticacao Falhou");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Falha no Inicio");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Falha na Conexao");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Falha na Recepcao");
    else if (error == OTA_END_ERROR) Serial.println("Falha no Fim");
  });
  
  ArduinoOTA.begin();
  Serial.println("Pronto");
  Serial.print("Endereco IP: ");
  Serial.println(WiFi.localIP());
  
  // ------------------------------------ OAT ------------------------------------------
}

void loop() 
{    
  ArduinoOTA.handle();
  
  // Escuta por clientes
  WiFiClient webclient = server.available(); 

  if(webclient)
  {
    Serial.println("Novo client.");

    while(webclient.connected())
    {
      Serial.println("Conectado");   

        // Posiciona o cursor na parte direita superior
        lcd.setCursor(10, 1);
        lcd.print("R");
       
      Serial.println(webclient.read());
      String content = webclient.readString();
      Serial.println(content);
  
      // Caso mensagem contenha 
      if(content.indexOf("SetPoint") > 0)
      {
        Serial.println("SetPoint");
        
        if(content.indexOf("Change") > 0)
        {
          Serial.println("Change");
          
          int start_index = content.indexOf("Value:");
          if(start_index > 0)
          {
            String value = content.substring(start_index + 6, (content.length() - 1));
            Serial.print("Value:");
            Serial.println(value);

            // Muda Temperatura do SetPoit
            SetTemperatureByPost(value);
          }          
        }
      }
      else if(content.indexOf("WaitTime") > 0)
      {
        Serial.println("WaitTime");
        
        if(content.indexOf("Change") > 0)
        {

          Serial.println("Change");
          
          int start_index = content.indexOf("Value:");
          if(start_index > 0)
          {
            String value = content.substring(start_index + 6, (content.length() - 1));
            Serial.print("Value:");
            Serial.println(value);

            // Muda Temperatura do SetPoit
            SetWaitTimeByPost(value);
          }             
        }
      }
      else if(content.indexOf("Request") > 0)
      {
        Serial.println("Request");

        // Posiciona o cursor na parte direita superior
        lcd.setCursor(10, 1);
        lcd.print("S");
        
        Serial.println("Escrevendo JSON");
        StaticJsonBuffer<69> jsonBuffer;
        String data = ((String)"{ \"value1\" : "+ GetCorrectionFactor(GetTemperature(sensor1)) +", \"value2\" : " + setTemp + ", \"value3\" : "+ timeToWait +" }");

        Serial.print("Json ok: ");
        Serial.println(data);

        SendJsonPost(data, "http://maker.ifttt.com/trigger/Status_Atual/with/key/j_oDaW_j1pVmxABRgI7IyBGaaMP5q96A9Ks73rFj4fM");          
                
      }
    }
  }
    // Posiciona o cursor na parte direita superior
  lcd.setCursor(10, 1);
  lcd.print(" ");

  
  // Posiciona o cursor na parte direita superior
  lcd.setCursor(12, 0);

  // Escreve set
  lcd.print("Set");

  // Escreve a temperatura limite na parte inferior direita
  lcd.setCursor(12, 1);
  lcd.print(setTemp);  
  Serial.print("Imprime settemp:");
  Serial.println(setTemp);
 
  // Recebe temperatura primeiro sensor
  tempSensor1 = GetCorrectionFactor(GetTemperature(sensor1));

  // Define local do cursor
  lcd.setCursor(0,0);

  Serial.println("Nova temp: " + String(tempSensor1, 1));

  if(tempSensor1 > -100)
  {
      // Imprime temperatura atual
      lcd.print("Temp:" + String(tempSensor1, 1));
      Serial.println("imprime temperatura LCD");
  }
  else
  {
      // Imprime temperatura atual como erro
      lcd.print("ERRO    ");
      Serial.println("imprime temperatura LCD como erro");
  }
  
  // Verifica se a temperatura ta abaixo da definida
  // Se tiver acima do que deve estar liga geladeira
  if(tempSensor1 > setTemp && state == false && (millis() >= (time_now + timeToWait)))
  {
    // Liga relé
    digitalWrite(relay, LOW);//DONE
    state = true;
    Serial.println("Réle on");
    time_now = millis();
  }

  // Se estiver abaixo do que deve estar, desliga geladeira
  else if(tempSensor1 < setTemp && state == true && (millis() >= (time_now + timeToWait)))
  {
    // Desliga relé
    digitalWrite(relay, HIGH);//DONE
    state = false;
    Serial.println("Réle off");
    time_now = millis();
  }
  
  // Recebe temperatura sengundo sensor
  tempSensor2 = GetCorrectionFactor(GetTemperature(sensor2));
  Serial.println("Nova temp2: " + String(tempSensor2, 1));

  // Define local do cursor
  lcd.setCursor(0,1);

  // Se a temperatura for diferente da de erro
  if(tempSensor2 > -100)
  {
    // Imprime temperatura atual
    lcd.print("Fan: "+ String(tempSensor2, 1));

    Serial.println("Temperatura impressa no LCD");
  }
  // Caso seja erro
  else
  {
    // Imprime temperatura atual como erro
    lcd.print("ERRO");
    Serial.println("erro temperatura sensor 2");
  }

  // Caso temperatura não seja temperatura de erro do sensor
  if(tempSensor2 > -100)
  {
    Serial.println("Temperatura valida para controle de fan");
    
    if(tempSensor2 > -100)
    {
      Serial.println("tempSensor2 abaixo do min");
      controlFanSpeed(0);
    }
    else if(tempSensor2 >= lowTemp && tempSensor2 < mediumTemp)
    {
      Serial.println("tempSensor2 baixa temp");
      controlFanSpeed(60);
    }
    else if(tempSensor2 >= mediumTemp && tempSensor2 < highTemp)
    {
      Serial.println("tempSensor2 media temp");
      controlFanSpeed(80);
    }
    else if(tempSensor2 >= highTemp)
    {
      Serial.println("tempSensor2 alta temp");
      controlFanSpeed(100);
    } 
  }
  // Caso seja verificar fan pelo rele ligado ou não
  else
  {
    Serial.println("Temperatura invalida para controle de fan, controlar por rele");

    // Caso rele esteja ligado
    if(state)
    {
      Serial.println("fan maximo");
      controlFanSpeed(100);
    }
    else
    {
      Serial.println("fan minimo");
      controlFanSpeed(0);
    }
  }

  // Caso o horario atual seja maior que o tempo para enviar os dados para a tabela
  if((millis() >= (time_now_SendTemp + timeToSendTemp)))
  {
    // Posiciona o cursor na parte direita superior
    lcd.setCursor(10, 1);
    lcd.print("S");

    Serial.println("Escrevendo JSON da temperatura para google sheet");
    StaticJsonBuffer<69> jsonBuffer;
    String data = ((String)"{ \"value1\" : "+ GetCorrectionFactor(GetTemperature(sensor1)) + " }");

    Serial.print("Json ok: ");
    Serial.println(data);

    // Envia requisição
    SendJsonPost(data, "http://maker.ifttt.com/trigger/SaveTemp/with/key/j_oDaW_j1pVmxABRgI7IyBGaaMP5q96A9Ks73rFj4fM");        
    
    time_now_SendTemp = millis();
  }

  // Posiciona o cursor na parte direita superior
  lcd.setCursor(10, 1);
  lcd.print(" ");

  
  //delay(500);
}

// Faz alteração da velocidade do cooler externo
void controlFanSpeed (int fanSpeedPercent) 
{
  Serial.print("Fan Speed: ");
  Serial.print(fanSpeedPercent);
  Serial.println("%");  
  analogWrite(mosfet, fanSpeedPercent); // set the fan speed
}

// Requisita temperatura ao sensor 
float GetTemperature(uint8_t sensorIndex[8])
{
  sensors.requestTemperatures();
  float temp = sensors.getTempC(sensorIndex);
  return temp;
}

// Seta temperatura para chamadas de um post
void SetTemperatureByPost(String temperature)
{
  Serial.println("SetTemperatureByPost()");
  Serial.println(temperature);
  Serial.println("String to float:");
  Serial.println(temperature.toFloat());
  
  setTemp = temperature.toFloat();
  Serial.print("SetPoint alterado para ");
  Serial.println(setTemp);
}

// Seta temperatura para chamadas de um post
void SetWaitTimeByPost(String WaitTime)
{
  Serial.println("SetWaitTimeByPost()");
  Serial.println("String to int:");
  Serial.println(WaitTime.toInt());
  timeToWait = WaitTime.toInt();  
  Serial.print("WaitTime alterado para ");
  Serial.println(timeToWait);
}

void SendJsonPost(String data, String url)
{   
  if(WiFi.status() == WL_CONNECTED)
  {
    WiFiClient client;
    HTTPClient http;
    
    Serial.print("Enviando post para: ");
    Serial.println(url);

    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");

    int httpCode = http.POST(data);

    if(httpCode > 0)
    {
      Serial.print("Http Code: ");
      Serial.println(httpCode);
      Serial.print("PayLoad: ");
      String payload = http.getString();
      Serial.println(payload);
    }
    else
    {
      Serial.println("Erro ao fazer POST");
    }
  }
}

double GetCorrectionFactor(double temp)
{
  if(temp < 8)
  {
    return temp + 1.2;
  }
  if(temp > 8 && temp < 30)
  {
    return temp + 1.5;
  }
}

*/