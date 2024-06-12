#include <WiFi.h>

String scanWifi() {
    String networks = "";

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();

  if (n == 0) {
    //Serial.println("no networks found");
  } else {
    // Serial.print(n);
    // Serial.println(" networks found");

    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      String wifi_name = WiFi.SSID(i);
      networks = networks + ">>" + wifi_name;
      Serial.println(wifi_name);
      // Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      delay(10);
    }
    networks += "!!";
  }

  delay(1000);
  return networks;
}

void initWiFi(String ssid, String password) {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  unsigned long int start_time = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("connected: ");
      Serial.println(WiFi.localIP());
      break;
    } else if (millis() - start_time > 7000) {
      Serial.println("connect try failed");
      break;
    }
  }
}