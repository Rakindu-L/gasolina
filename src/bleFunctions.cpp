#include <ArduinoBLE.h>

BLEDevice BLEconnect() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    delay(3000);
    BLE.stopAdvertise();
  }
  return central;
}

char* BLEread(BLEDevice central, BLECharacteristic charactersitic, char* outData) {

  BLE.poll();

  if (charactersitic.written()) {

    int ssid_length = charactersitic.valueLength();
    const uint8_t* data = charactersitic.value();

    outData = new char[ssid_length + 1];
    for (size_t i = 0; i < ssid_length; i++) {
      outData[i] = *(data + i);
    }
    outData[ssid_length] = '\0';
    Serial.println(outData);
  }
      return outData;
}

int BLEreadInt(BLEDevice central, BLEIntCharacteristic charactersitic) {

  BLE.poll();
    int intData = 0;
  if (charactersitic.written()) {
    intData = charactersitic.value();
    Serial.printf("scan : %d", intData);
    Serial.println("");
  }
  return intData;
}

void BLEwriteString(String data, BLECharacteristic charactersitic) {
  uint8_t value[data.length()];
  for (size_t i = 0; i < data.length(); i++) {
    value[i] = data.charAt(i);
  }
  if (charactersitic.writeValue(value, data.length())) {
    delay(1000);
  }
}
