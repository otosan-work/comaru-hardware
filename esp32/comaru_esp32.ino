//------------------------------------------------------------------------------
// COMmon Api for Remote Unit sample code
// for ESP32 Devkit-c / ESP-WROOM-32
//
// Library : Arduino core for ESP32 WiFi chip
//           https://github.com/espressif/arduino-esp32
//           WiFiClientSecure Library (Version 1.0.0)
//           ArduinoJson Library      (Version 5.13.1)
//           IRremote Arduino Library (Version 2.2.3)

#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <IRremote.h>

// Wifi Settings
const char* ssid     = ""; // your network SSID (name of wifi network)
const char* password = ""; // your network password

// CA Settings
const char*  server = "comaru.herokuapp.com";  // Server URL
const char* root_ca= \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n" \
"ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\n" \
"MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n" \
"LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\n" \
"RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\n" \
"+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\n" \
"PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\n" \
"xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\n" \
"Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\n" \
"hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\n" \
"EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\n" \
"MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\n" \
"FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\n" \
"nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\n" \
"eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\n" \
"hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\n" \
"Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n" \
"vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\n" \
"+OkuE6N36B9K\n" \
"-----END CERTIFICATE-----\n" ;

WiFiClientSecure client;

//------------------------------------------------------------------------------
// Tell IRremote which Arduino pin is connected to the IR Receiver (TSOP4838)
//
// Signal Variable
unsigned int* IR_ROW_DATA;
int           IR_Herz = 0;
int           IR_Signal_Length = 0;

// GPIO Setting
int recvPin = 25;
int sendPin = 26;
IRrecv irrecv(recvPin);
IRsend irsend(sendPin);

// Allocate JsonBuffer
DynamicJsonBuffer jsonBuffer(4096);

//+=============================================================================
// Configure the Arduino
//
void  setup ( )
{

  // Wifi Flow
  WiFi.begin(ssid, password);

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }

  // HTTPS connecting 
  client.setCACert(root_ca);
  
  if (!client.connect(server, 443))
  {
    
  } else {
      // Make a HTTP request:
      client.println("GET /api/v1/?ProductNo=RMF-TX200J&Command=PowerOn HTTP/1.1");
      client.println("Host: comaru.herokuapp.com");
      client.println("Connection: close");
      client.println();

      if (client.println() == 0) {
        return;
      }

      while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
          break;
        }
      }

    // Parse JSON object
    String c = "";
    while (client.available()) {
        c.concat((char)client.read());  
    }
    JsonObject& root = jsonBuffer.parseObject(c);
    if (!root.success()) {
      return;
    }
    
    // IR Signal set
    IR_Signal_Length = root["Response"][0]["RawData"].size();
    IR_ROW_DATA = new unsigned int [IR_Signal_Length];
    for (int i = 0; i < IR_Signal_Length; i++) {
      IR_ROW_DATA[i] = root["Response"][0]["RawData"][i].as<unsigned int>();
    }
    IR_Herz     = root["Response"][0]["Herz"].as<int>();

    // Client release
    client.stop();
  }

  // Send IR Signal
  for (int i = 0; i < 3; i++) {
    irsend.sendRaw(IR_ROW_DATA, IR_Signal_Length, IR_Herz ); 
    delay(40); 
  }

}

//+=============================================================================
// The repeating section of the code
//
void  loop ( )
{
  // No use
}