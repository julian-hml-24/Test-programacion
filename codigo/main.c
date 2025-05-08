
// Definición de pines para sensores
#define pinSensorTemp A2      // Pin analógico para el sensor de temperatura (LM35)
#define pinSensorLuz A1       // Pin analógico para el sensor de luminosidad (LDR)
#define pinPotenciometro A0   // Pin analógico para el potenciómetro (ajuste de umbrales)
#define pulsador 2            // Pin digital para el pulsador (entrada digital)

// Definición de pines para actuadores
#define pinLedNaranja 12      // Pin digital para el LED naranja
#define pinLedRojo 11         // Pin digital para el LED rojo
#define pinLedAmarillo 10     // Pin digital para el LED amarillo
#define pinLedVerde 9         // Pin digital para el LED verde
#define pinZumbador 8         // Pin digital para el zumbador (buzzer)

// Variables para almacenar lecturas de sensores
float temperatura = 0.0;      // Almacena el valor de temperatura en °C
int luminosidad = 0;          // Almacena el valor de luminosidad (0-1023)
int valorPotenciometro = 0;   // Almacena el valor del potenciómetro (0-1023)

// Umbrales ajustables
float umbralTempAlta = 30.0;  // Umbral de temperatura alta (ajustable con potenciómetro)
float umbralTempBaja = 20.0;  // Umbral de temperatura baja (ajustable con potenciómetro)
int umbralLuzAlta = 800;      // Umbral de luminosidad alta (ajustable con potenciómetro)
int umbralLuzBaja = 300;      // Umbral de luminosidad baja (ajustable con potenciómetro)

// Variables para control de modos
int modoOperacion = 0;        // Almacena el modo de operación actual (0: automático, 1: manual, etc.)
const int numModos = 2;       // Número total de modos de operación

// Variables para la alarma
bool alarmaActivada = false;  // Estado de la alarma (activada/desactivada)
bool estadoAlarma = false;    // Estado actual del parpadeo de la alarma

// Configuración inicial
void setup() {
  // Configuración de pines de sensores como entradas
  pinMode(pinSensorTemp, INPUT);
  pinMode(pinSensorLuz, INPUT);
  pinMode(pinPotenciometro, INPUT);

  attachInterrupt(digitalPinToInterrupt(pulsador), cambiarModo, FALLING);

  // Configuración de pines de actuadores como salidas
  pinMode(pinLedRojo, OUTPUT);
  pinMode(pinLedAmarillo, OUTPUT);
  pinMode(pinLedVerde, OUTPUT);
  pinMode(pinLedNaranja, OUTPUT);
  pinMode(pinZumbador, OUTPUT);

  // Inicialización de la comunicación serial (para depuración)
  Serial.begin(9600);
}

// Bucle principal
void loop() {
  // Leer datos de los sensores
  sensores();

  // Analizar los datos según el modo de operación
  analizarDatos();

  // Controlar la alarma si está activada
  if (alarmaActivada) {
    controlarAlarma();
  }

  // Pequeño retardo para evitar lecturas demasiado rápidas
  delay(100);
}

// Función para leer los sensores
void sensores() {
  // Sensado de temperatura (LM35)
  int LM35_s = analogRead(pinSensorTemp);
  float voltajeTemp = (float)LM35_s * 5.0 / 1023.0;
  temperatura = voltajeTemp * 100.0;  // Convertir a grados Celsius

  // Sensado de luminosidad (LDR)
  luminosidad = analogRead(pinSensorLuz);

  // Lectura del potenciómetro
  valorPotenciometro = analogRead(pinPotenciometro);
}

// Función para analizar los datos según el modo
void analizarDatos() {
  // Restablecer estado de LEDs
  digitalWrite(pinLedVerde, LOW);
  digitalWrite(pinLedAmarillo, LOW);
  digitalWrite(pinLedRojo, LOW);
  digitalWrite(pinLedNaranja, LOW);

  // Analizar según el modo de operación
  switch (modoOperacion) {
    case 0: // Modo temperatura
      analizarTemperatura();
      break;

    case 1: // Modo luminosidad
      analizarLuminosidad();
      break;

    case 2: // Modo combinado
      analizarCombinado();
      break;
  }
}

// Función para analizar temperatura
void analizarTemperatura() {
  const float margenPrecaucion = 2.0;  // Margen para zona de precaución
  if (temperatura < (umbralTempBaja - margenPrecaucion)) {
    // Temperatura baja (LED verde)
    digitalWrite(pinLedVerde, HIGH);
    alarmaActivada = false;
  } else if (temperatura <= (umbralTempAlta + margenPrecaucion)) {
    // Temperatura en zona de precaución (LED amarillo)
    digitalWrite(pinLedAmarillo, HIGH);
    alarmaActivada = false;
  } else {
    // Temperatura alta (LED rojo + alarma)
    digitalWrite(pinLedRojo, HIGH);
    alarmaActivada = true;
  }
  //indicar el modo de operación Temperatura
  digitalWrite(pinLedNaranja, HIGH);
  delay (500);
  digitalWrite(pinLedNaranja, LOW);
}

// Función para analizar luminosidad
void analizarLuminosidad() {
  if (luminosidad < umbralLuzBaja) {
    // Luz baja (LED verde)
    digitalWrite(pinLedVerde, HIGH);
    alarmaActivada = false;
  } else if (luminosidad < umbralLuzAlta) {
    // Luz media (LED amarillo)
    digitalWrite(pinLedAmarillo, HIGH);
    alarmaActivada = false;
  } else {
    // Luz alta (LED rojo + alarma)
    digitalWrite(pinLedRojo, HIGH);
    alarmaActivada = true;
  }
  //indicar el modo de operación Luminosidad
  digitalWrite(pinLedNaranja, HIGH);
  delay (250);
  digitalWrite(pinLedNaranja, LOW);
}

// Función para analizar modo combinado
void analizarCombinado() {
  bool condicionTemperatura = (temperatura > umbralTempAlta);
  bool condicionLuz = (luminosidad > umbralLuzAlta);

  if (condicionTemperatura && condicionLuz) {
    // Condición crítica: temperatura alta y mucha luz (LED rojo + alarma)
    digitalWrite(pinLedRojo, HIGH);
    alarmaActivada = true;
  } else if (condicionTemperatura || condicionLuz) {
    // Condición de precaución: o temperatura alta o mucha luz (LED amarillo)
    digitalWrite(pinLedAmarillo, HIGH);
    alarmaActivada = false;
  } else {
    // Condición normal: temperatura y luz en rangos normales (LED verde)
    digitalWrite(pinLedVerde, HIGH);
    alarmaActivada = false;
  }
  //indicar el modo de operación Combinado
  digitalWrite(pinLedNaranja, HIGH);
  delay (75);
  digitalWrite(pinLedNaranja, LOW);
}

// Función para controlar la alarma (parpadeo)
void controlarAlarma() {
  estadoAlarma = !estadoAlarma;  // Invertir el estado de la alarma

  if (estadoAlarma) {
    tone(pinZumbador, 1000);  // Tono de 1kHz
  } else {
    noTone(pinZumbador);      // Apagar el tono
  }
}

void cambiarModo() {
    // Uso de variable volatile para garantizar coherencia en interrupciones
    static unsigned long ultimoTiempoInterrupcion = 0;
    unsigned long tiempoActual = millis();
    
    // Debounce (anti-rebote) para el botón
    if (tiempoActual - ultimoTiempoInterrupcion > 200) {
        // Incrementar modo y volver a 0 si excede el máximo
        modoOperacion = (modoOperacion + 1) % 3;
    }
    
    ultimoTiempoInterrupcion = tiempoActual;
}
