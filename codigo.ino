#include <Adafruit_LiquidCrystal.h>
#include <stdlib.h>
#include <math.h>

int pinAnalogico = A0;
int valor = 0;
float *voltaje = (float*) malloc(sizeof(float));  // Puntero voltaje
float *amplitud = (float*) malloc(sizeof(float));  // Puntero amplitud
float *frecuencia = (float*) malloc(sizeof(float));  // Puntero frecuencia
unsigned long ultimaHora = 0;
Adafruit_LiquidCrystal *lcd = new Adafruit_LiquidCrystal(0);  // Puntero al objeto LCD

const int numLecturas = 100;
float *lecturas = (float*) malloc(numLecturas * sizeof(float));  // Arreglo dinámico para almacenar las lecturas
int indiceLectura = 0;

void setup() {
  Serial.begin(9600);
  lcd->begin(16, 2);  // Iniciar la pantalla LCD
  *amplitud = 0.0;
  *frecuencia = 0.0;
}

void loop() {
  valor = analogRead(pinAnalogico);

  // Convertir el valor analógico a voltaje
  *voltaje = (valor / 1023.0) * 5.0;

  // Cálculo de amplitud pico-pico
  if (*voltaje > *amplitud) {
    *amplitud = *voltaje;
  }

  // Cálculo de frecuencia (simplificado)
  unsigned long tiempoActual = millis();
  if (*voltaje >= 2.5 && (tiempoActual - ultimaHora) > 500) {
    *frecuencia = 1000.0 / (tiempoActual - ultimaHora);
    ultimaHora = tiempoActual;
  }

  // Obtener lecturas para identificar la forma de onda
  obtenerLecturas();

  // Mostrar valores en la pantalla LCD
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->print("Amp: ");
  lcd->print(*amplitud);
  lcd->print(" V");
  lcd->setCursor(0, 1);
  lcd->print("Freq: ");
  lcd->print(*frecuencia);
  lcd->print(" Hz");

  // Identificar la forma de onda
  identificarFormaOnda();

  delay(500);  // Retardo para evitar sobrecarga
}

void identificarFormaOnda() {
  lcd->clear();
  if (esOndaSenoidal()) {
    lcd->setCursor(0, 0);
    lcd->print("Onda: Senoidal");
  } else if (esOndaCuadrada()) {
    lcd->setCursor(0, 0);
    lcd->print("Onda: Cuadrada");
  } else if (esOndaTriangular()) {
    lcd->setCursor(0, 0);
    lcd->print("Onda: Triangular");
  } else {
    lcd->setCursor(0, 0);
    lcd->print("Onda: Desconocida");
  }
}

bool esOndaSenoidal() {
  int transicionesSuaves = 0;
  for (int i = 1; i < numLecturas - 1; i++) {
    if ((lecturas[i] > lecturas[i - 1] && lecturas[i] < lecturas[i + 1]) || 
        (lecturas[i] < lecturas[i - 1] && lecturas[i] > lecturas[i + 1])) {
      transicionesSuaves++;
    }
  }
  return transicionesSuaves > numLecturas * 0.8;
}

bool esOndaCuadrada() {
  int cambiosBruscos = 0;
  for (int i = 1; i < numLecturas; i++) {
    if (abs(lecturas[i] - lecturas[i - 1]) > 2.0) {
      cambiosBruscos++;
    }
  }
  return (cambiosBruscos >= numLecturas / 4);
}

bool esOndaTriangular() {
  bool subiendo = lecturas[1] > lecturas[0];
  int transicionesLineales = 0;
  for (int i = 1; i < numLecturas - 1; i++) {
    if (subiendo && lecturas[i] > lecturas[i + 1]) {
      subiendo = false;
    } else if (!subiendo && lecturas[i] < lecturas[i + 1]) {
      subiendo = true;
    }
    transicionesLineales++;
  }
  return transicionesLineales > numLecturas * 0.8;
}

void obtenerLecturas() {
  for (int i = 0; i < numLecturas; i++) {
    lecturas[i] = analogRead(pinAnalogico) * (5.0 / 1023.0);  // Leer directamente del pin analógico
  }
}

// Liberar memoria dinámica al final
void liberarMemoria() {
  free(voltaje);
  free(amplitud);
  free(frecuencia);
  free(lecturas);
  delete lcd;
}
