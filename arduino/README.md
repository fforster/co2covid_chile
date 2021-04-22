# Sistema sensor de C02 con MHZ19B y comunicación LoRa con E32TTL100.

## Funcionamineto esperado nodo sensor (Arduino UNO):

Al inicializar, aparecerán textos en el display que indicarán qué parte del sistema está partiendo, por lo que se verán los textos (en este orden) "HOLA", LORA", "DONE", "HEAT", "DONE"):

- En "LORA" se inicializa el transceptor y si es necesario se puede configurar sus parámetros. 

- En "HEAT" se espera 180 seg (variable tHeat*1000) para que el sensor se caliente, para luego entrar al loop, donde se consulta la concentración de CO2, se expone en pantalla, si es sobre 800 ppm además sonará un buzzer (pasivo, dos sonidos de 300ms, 2100Hz).

- Posteriormente se ingresa a un ciclo de escucha en el que, si llega una solicitud de envío, si corresponde se envía y rompe el ciclo de escucha, delo contrario se espera un segundo, hasta cumplir el minuto o salir de ahi.

## Funcionamineto esperado nodo sensora (Node MCU):

Una vez conectado a wifi a AWS y Lora con los nodos sensores, interroga a cada nodo y luego sube la información a AWS.

## Archivos de configuración:

Además de los archivos .ino, se debe configurar los siguientes archivos:

- aws_credentials.h: credenciales para comunicación con AWS. Variables awsEndpoint, certificatePemCrt, privatePemKey, caPemCrt.

- NodeMCU_config.h: configuración de receptor. Variable idRoot (ID de ubicación, 3 letras mayúsculas), numeroSensores (entre 0 y 99), ssid (nombre de red wifi), password (pwd de red wifi)

- Sensor_config.h: configuración de sensor. Variable StringId (ID de ubicación + número de 0 a 99 en formato de dos dígitos, e.g. "01")


Pablo M. y F. Förster