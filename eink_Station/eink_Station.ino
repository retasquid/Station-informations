#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>

#include <WiFi.h>
#include "time.h"
#include <Wire.h>
#include <HTTPClient.h>
#include <NTPClient.h> 
#include <WiFiUdp.h>
#include <ArduinoJson.h>

const char* ssid = "wifi_id";                                               //identifiant et mot de passe WIFI
const char* password = "WIFI_mdp";
float solprice=0;
float cacprice=0;
float usdprice=0.91;
float xagprice=0;

const int httpsPort = 443;
float tmp=0;
WiFiClient client;                                                            //Connection WiFi
HTTPClient http;

GxEPD2_3C<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT> display(GxEPD2_290_C90c(/*CS=5*/ 5, /*DC=*/ 4, /*RST=*/ 2, /*BUSY=*/ 15));
struct tm timeinfo;
void setup()
{
  Serial.begin(115200);
  display.init(115200, true, 2, false);
  display.hibernate();
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)                                        //Connection au WIFI
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected.");
  configTime(3660,3600,"pool.ntp.org");
  http.begin("https://api.coingecko.com/api/v3/simple/price?ids=usd-coin&vs_currencies=EUR");
  int httpCode = http.GET();                                                            //Obtention du cours EUR/USD
  if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
    StaticJsonDocument<2000> doc;
    DeserializationError error = deserializeJson(doc, http.getString());
    String USDPrice = doc["usd-coin"]["eur"].as<String>();            
    usdprice = USDPrice.toDouble();
  }
  http.end();
}

void refreshdata(const int btcprice,const float solprice,const float cacprice,const float xagprice){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  display.setRotation(1);                                 //Mise en page
  display.setFont(&FreeMonoBold9pt7b);
  display.setFullWindow();
  display.firstPage();
  display.setTextColor(GxEPD_BLACK);
  display.fillRoundRect(5, 50, 120, 40, 12, GxEPD_BLACK);
  display.drawRoundRect(130, 5, 3, 118, 3, GxEPD_BLACK);
  display.setCursor(140,25);
  display.print("BTC");
  display.setCursor(140,55);
  display.print("SOL");
  display.setCursor(140,85);
  display.print("CAC40");
  display.setCursor(140,115);
  display.print("XAG");

  display.fillRect(200, 0, 96, 128, GxEPD_WHITE);
  display.fillRect(0, 0, 120, 39, GxEPD_WHITE);
  display.fillRoundRect(10, 55, 110, 30, 8, GxEPD_WHITE);

  display.drawRoundRect(136, 9, 154, 24, 6, GxEPD_BLACK);
  display.drawRoundRect(136, 39, 154, 24, 6, GxEPD_BLACK);
  display.drawRoundRect(136, 69, 154, 24, 6, GxEPD_BLACK);
  display.drawRoundRect(136, 99, 154, 24, 6, GxEPD_BLACK);

  display.setTextColor(GxEPD_RED);
  display.setCursor(220,25);
  display.setTextSize(1);
  display.print(btcprice);
  display.setCursor(220,55);
  display.print(solprice);
  display.setCursor(220,85);
  display.print(cacprice);
  display.setCursor(220,115);
  display.print(xagprice);
  display.setCursor(10,80);
  display.setTextSize(2);
  display.print(&timeinfo, "%H:%M");
  display.setCursor(5,15);
  display.setTextSize(1);
  display.print(&timeinfo, "%A %d");
  display.setCursor(5,30);
  display.print(&timeinfo, "%B");
  display.display();
}

void loop() {
  http.begin("https://api.coingecko.com/api/v3/simple/price?ids=silver-token-xagx&vs_currencies=USD");
  int httpCode = http.GET();                                                            //Obtention du cours Argent/USD
  if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
    StaticJsonDocument<2000> doc;
    DeserializationError error = deserializeJson(doc, http.getString());
    String XAGUSDPrice = doc["silver-token-xagx"]["usd"].as<String>();
    xagprice = XAGUSDPrice.toDouble();
  }
  http.end();

  http.begin("https://finnhub.io/api/v1/quote?symbol=CAC&token=crebsvpr01qnd5cuvff0crebsvpr01qnd5cuvffg");
httpCode = http.GET();                                                            //Obtention du cours CAC40/USD
  if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
    StaticJsonDocument<2000> doc;
    DeserializationError error = deserializeJson(doc, http.getString());

    String CACUSDPrice = doc["c"].as<String>();
    cacprice = CACUSDPrice.toDouble();
  }
  http.end();

  http.begin("https://api.coingecko.com/api/v3/simple/price?ids=solana&vs_currencies=USD&include_24hr_change=true");
  httpCode = http.GET();                                                            //Obtention du cours SOL/USD
  if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
    StaticJsonDocument<2000> doc;
    DeserializationError error = deserializeJson(doc, http.getString());

    String SOLUSDPrice = doc["solana"]["usd"].as<String>();
    solprice = SOLUSDPrice.toDouble();
  }
  http.end();

  http.begin("https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=USD&include_24hr_change=true");
  httpCode = http.GET();                                                            //Obtention du cours BTC/USD
  if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
    StaticJsonDocument<2000> doc;
    DeserializationError error = deserializeJson(doc, http.getString());
    String BTCUSDPrice = doc["bitcoin"]["usd"].as<String>(); 
    int btcprice = BTCUSDPrice.toDouble();
    refreshdata(btcprice, solprice, cacprice*usdprice, xagprice*usdprice);  //Rafraichisement des données
  }
  http.end();
  delay(100000);  //Attente de 2min pour éconnomiser de l'énergie
}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}


}
}
}}}}}
}}}}
}
}}}}}
}}}}}}
}}}}}}}}}
}
}}}}}
}}}}}}
}}}}}}}}}}}}}
}}}}}}}}}}}}}}}}}}
}}}}
}
}}}}}
}}}}}}
}}}}}}}}}
}
}}}}}
}}}}}}
}}}}}}}}}}}}}
}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
}
}}}}}
}}}}}}
}}}}}}}}}}}}}
}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
}
}}}}}
}}}}}}
}}}}}}}}}}}}}
}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}