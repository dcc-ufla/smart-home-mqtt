#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>


// Globais
#pragma region Globals

	// Variaveis globais para o WiFi
	#pragma region WiFiVariables

		// Nome da rede WiFi
		const char* ssid = "Silvio";

		// Senha de acesso da rede wifi
		const char* password = "semsenha";

		// Cria o objeto espClient
		WiFiClient espClient; 

	#pragma endregion

	// Variaveis globais para o MQTT
	#pragma region MqttVariables

		// ID do cliente
		#define ID_MQTT "ClientS"

		// Topico a ser subiscrito para envio de dados de status
		char* TOPIC_SEND = "SEND/Fermentador";

		// Topico a ser subiscrito para envio de dados de status
		char* TOPIC_RECEIVE = "RECEIVE/Fermentador";

		// IP do broker
		const char* mqtt_server = "192.168.50.138"; 

		// Porta do Broker MQTT
		int BROKER_PORT = 1883; 

		// Instancia o Cliente MQTT passando o objeto espClient
		PubSubClient MQTT(espClient); 

	#pragma endregion

	// Variavel de modulo
	#pragma region ModulesVariables

		// Seta a saida do rele para D5
		const int relay = 14;

		// Define a area do display LCD
		int lcdColumns = 16;
		int lcdRows = 2;

		// Crian instancia do display
		LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

		// Data wire para porta D3, bus de dados para os sensores
		#define ONE_WIRE_BUS 0

		// Prepara uma instancia para comunicação com os sensores
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

		// Define no controlador de velocidade na saida D6/GND
		const int mosfet = 12;

		// Tempo para pode ativar ou desativar o relé novamente
		int timeToWait;

		// Estado da geladeira
		bool state;

		unsigned long time_now = 0;

		// Tempo parar enviar os dados para a tabela
		int timeToSendTemp = 1800000;
		int time_now_SendTemp = 0;

	#pragma endregion

	// Declaração dos metodos
	#pragma region MethodoCalls

		void _ReconnectMQTT();
		void _ReconectWiFi();
		void _VerifyConectionsMQTT();
		void _InitWifi();
		void _Mqtt_Callback(char* topic, byte* payload, unsigned int length);
		void _InitMQTT();
		void _InitModules();
		void _InitOAT();
		double _GetCorrectionFactor(double temp);
		float _GetTemperature(uint8_t sensorIndex[8]);
		void _ControlFanSpeed (int fanSpeedPercent);
		void _SetSetPoint(float value);
		void _ProcessMsgContent(String msg);

	#pragma endregio
#pragma endregion

#pragma endregion

// Tenta reconectar ao broker
void _ReconnectMQTT()
{
	// Enquanto não conseguir se conectar com o broker
	while (!MQTT.connected())
	{
		Serial.println("Tentando reconectar ao Broker");
		
		String clientId = "ESP8266Client-";
    	clientId += String(random(0xffff), HEX);

		if(MQTT.connect(clientId.c_str(), "username", "password"))
		{
			Serial.println("Conectado com sucesso ao broker");
			MQTT.subscribe(TOPIC_RECEIVE);
		}
		else
		{
			Serial.println(MQTT.connect(ID_MQTT, "username", "password"));
				
			int i = MQTT.state();

			Serial.println(i);

			Serial.println("Falha ao tentar reconectar ao Broker");
			delay(3500);
		}		
	}	
}

// Tenta reconectar ao wifi
void _ReconectWiFi()
{
	WiFi.mode(WIFI_STA);
    //se já está conectado a rede WI-FI, nada é feito. 
    //Caso contrário, são efetuadas tentativas de conexão
    if (WiFi.status() == WL_CONNECTED)
        return;
         
    WiFi.begin(ssid, password); // Conecta na rede WI-FI
     
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
   
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(ssid);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}

// Verifica a conexão do cliente com o broker
void _VerifyConectionsMQTT()
{
	// Caso cliente esteja desconectado do broker
	if(!MQTT.connected())
	{
		Serial.println("Conexão falha com o broker");

		// Tenta reconectar
		_ReconnectMQTT();
	}

	//Serial.println("Sucesso na conexão com o broker");
	//_ReconectWiFi();
}

// Inicializa conexcao ao wifi
void _InitWifi()
{
	delay(10);

	// Conectando ao WiFi
	Serial.println();
	Serial.print("Conectando em ");
	Serial.println(ssid);

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
	delay(500);
	Serial.print(".");
	}

	randomSeed(micros());

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
}

// Callback para as chamadas dos topicos subiscritos
void _Mqtt_Callback(char* topic, byte* payload, unsigned int length)
{
	// Variavel para conter mensagem
	String msg;

	// Atribui conteudo da mensagem a variavel
	for (int i = 0; i < length; i++)
	{
		char c = (char)payload[i];
		msg += c;
	}
	
	Serial.print("Conteudo do topico: ");

	// Imprime conteudo da mensagem
	Serial.println(msg);

	// Processa o conteudo da mensagem para poder fazer as ações necessarias
	_ProcessMsgContent(msg);
}

// Inicias os parametros de conexao MQTT
void _InitMQTT()
{
	MQTT.setServer(mqtt_server, 1883); 
    MQTT.setCallback(_Mqtt_Callback); 
}

// Inicia os modulos
void _InitModules()
{
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
	digitalWrite(relay, LOW);
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

	//TODO Analisar necessidae disso, as vezes somente a primeira requisição do loop é o suficiente
	
	// Recebe temperatura primeiro sensor
	tempSensor1 = _GetCorrectionFactor(_GetTemperature(sensor1));
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
}

// Inicia os dados da comunicação OAT do dispositivos
void _InitOAT()
{
	ArduinoOTA.onStart([]() 
	{
		Serial.println("Inicio...");
	});

	ArduinoOTA.onEnd([]() 
	{
		Serial.println("nFim!");
	});

	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) 
	{
		Serial.printf("Progresso: %u%%r", (progress / (total / 100)));
	});

	ArduinoOTA.onError([](ota_error_t error) 
	{
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
}

// Faz uma publicação em um topico 
bool _Publish(char* topic, char* content)
{
	// Se estiver conectado
	if(MQTT.connected())
	{
		Serial.println("Conectado para fazer publicação");

		// Publica
		MQTT.publish(topic, content);

		Serial.print("Conteudo: ");
		Serial.println(content);

		Serial.print("Publicado no topico: ");
		Serial.println(topic);

		// Publicação feita com sucesso
		return true;
	}

	// Falha na publicação
	return false;
}

void setup() 
{
  	// put your setup code here, to run once:
	Serial.begin(9600);
	delay(100);
	_InitWifi();
	_InitModules();
	_InitOAT();
    _InitMQTT();
}

void loop() 
{
	_VerifyConectionsMQTT();
	MQTT.loop();

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
	//Serial.print("Imprime settemp:");
	//Serial.println(setTemp);

	// Recebe temperatura primeiro sensor
	tempSensor1 = _GetCorrectionFactor(_GetTemperature(sensor1));

	// Define local do cursor
	lcd.setCursor(0,0);

	//Serial.println("Nova temp: " + String(tempSensor1, 1));

	// Caso não seja uma leitura errada
	if(tempSensor1 > -100)
	{
		// Imprime temperatura atual
		lcd.print("Temp:" + String(tempSensor1, 1));
		//Serial.println("imprime temperatura LCD");
	}
	
	// Ouve um erro na leitura do sensor
	else
	{
		// Imprime temperatura atual como erro
		lcd.print("ERRO    ");
		//Serial.println("imprime temperatura LCD como erro");
	}

	// Caso temperatura não seja temperatura de erro do sensor
	if(tempSensor2 > -100)
	{
		//Serial.println("Temperatura valida para controle de fan");

		if(tempSensor2 > -100)
		{
			//Serial.println("tempSensor2 abaixo do min");
			_ControlFanSpeed(0);
		}
		else if(tempSensor2 >= lowTemp && tempSensor2 < mediumTemp)
		{
			//Serial.println("tempSensor2 baixa temp");
			_ControlFanSpeed(60);
		}
		else if(tempSensor2 >= mediumTemp && tempSensor2 < highTemp)
		{
			//Serial.println("tempSensor2 media temp");
			_ControlFanSpeed(80);
		}
		else if(tempSensor2 >= highTemp)
		{
			//Serial.println("tempSensor2 alta temp");
			_ControlFanSpeed(100);
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
			_ControlFanSpeed(100);
		}
		else
		{
			Serial.println("fan minimo");
			_ControlFanSpeed(0);
		}
	}

	// Posiciona o cursor na parte direita superior
	lcd.setCursor(10, 1);
	lcd.print(" ");
}

// Corrige a leitura da temperatura dos sensores
double _GetCorrectionFactor(double temp)
{
	// Caso temperatura seja menor que 8
	if(temp <= 8)
	{
		// Corrige o sensor com 1.2 graus positivos
		return temp + 1.2;
	}
	// Caso temperatura seja maior que 8 e menor que 30
	if(temp > 8 && temp < 30)
	{
		// Corrige o sensor com 1.5 graus positivos
		return temp + 1.5;
	}
}

// Requisita temperatura ao sensor 
float _GetTemperature(uint8_t sensorIndex[8])
{
	// Pede tempertaura
	sensors.requestTemperatures();
	float temp = sensors.getTempC(sensorIndex);

	// Retorna valor da temperatura
	return temp;
}

// Faz alteração da velocidade do cooler externo
void _ControlFanSpeed (int fanSpeedPercent) 
{
	//Serial.print("Fan Speed: ");
	//Serial.print(fanSpeedPercent);
	//Serial.println("%");  

	// set the fan speed
	analogWrite(mosfet, fanSpeedPercent); 
}

// Processa mensagem recebida
void _ProcessMsgContent(String msg)
{
	Serial.println("Iniciando processamento da mensagem recebida!");

	// Encontra o separador do campo de alvo e ação
	int indexComma = msg.indexOf(";");

	// Recebe a ação a ser realizada
	String action = msg.substring(0, indexComma);

	Serial.print("Action: ");
	Serial.println(action);

	// Recebe alvo da ação
	String target = msg.substring(indexComma + 1, msg.length());

	Serial.print("Target: ");
	Serial.println(target);

	if(target.indexOf("sended") < 0)
	{
		// Caso seja uma ação de set
		if(action.equals("set"))
		{
			Serial.print("String to float:");
			// Transforma valor da mensagem em float
			float setTemp = target.toFloat();
			Serial.println(setTemp);
			
			// Altera temperatura do SetPoint
			_SetSetPoint(setTemp);
		}

		// Caso seja uma mensagem requisitando alguma informação
		else if(action.equals("get"))
		{
			// Caso seja temperatura atual
			if(target.equals("temp"))
			{ 
				char prefix[7] = "Temp: ";
				char msg2Send[12]; 

				sprintf(msg2Send,"%s = %.2f",prefix,tempSensor1);

				Serial.print("Ms2Send Temp: ");
				Serial.println(msg2Send);

				// Publica no topico 
				_Publish(TOPIC_SEND, msg2Send);
			}

			// Caso seja temperatura de setpoint definida
			else if(target.equals("setpoint"))
			{
				char prefix[7] = "Set: ";
				char msg2Send[12]; 

				sprintf(msg2Send,"%s = %.2f",prefix,setTemp);

				Serial.print("Ms2Send SetPoint: ");
				Serial.println(msg2Send);

				// Publica no topico
				_Publish(TOPIC_SEND, msg2Send);
			}
		}
	}
}

// Metodo para alterar temperatura do SetPoint
void _SetSetPoint(float value)
{
	Serial.print("Alterando SetPoin para: ");
  	Serial.println(value);

	// Altera temperatura
	setTemp = value;

	Serial.print("SetPoint alterado com sucesso!");
}