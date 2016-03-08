#define DHTPIN 7     // what pin we're connected to
DHT dht(DHTPIN, DHT11);

void setupSensors() {
    dht.begin();
}

void localSensors() {

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float dht_H = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float dht_T = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if ( isnan(dht_H) || isnan(dht_T) ) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  } else {
    soladinNode.Temp      = (int) dht_T * 100;
    soladinNode.Humidity  = (int) dht_H * 100;

    Serial.print(F("DHT11: "));
    Serial.print(dht_T);
    Serial.print(F(" C, "));
    Serial.print(dht_H);
    Serial.println(F("%, "));
  }
}

