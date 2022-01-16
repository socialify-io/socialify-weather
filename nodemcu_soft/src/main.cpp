#include <DHT.h>
#include <Adafruit_Sensor.h>

#include <Arduino.h>
#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>
#include <Hash.h>

const char *ssid = "The_Most_Dangerous";
const char *pass = "TMDno100758492";
const char *server_host = "192.168.8.151";
const uint16_t server_port = 83;

#define DHTPIN 4      // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22 // there are multiple kinds of DHT sensors

DHT dht(DHTPIN, DHTTYPE);
websockets::WebsocketsClient socket;

void connectToWiFi() {
  WiFi.hostname("ESP8266-do-pogody");
  WiFi.begin(ssid, pass);

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

}

void onMessageCallback(websockets::WebsocketsMessage message) {
    Serial.print("Got Message: ");
    Serial.println(message.data());
}

void onEventsCallback(websockets::WebsocketsEvent event, String data) {
    if(event == websockets::WebsocketsEvent::ConnectionOpened) {
        Serial.println("Connnection Opened");
    } else if(event == websockets::WebsocketsEvent::ConnectionClosed) {
        Serial.println("Connnection Closed");
    } else if(event == websockets::WebsocketsEvent::GotPing) {
        Serial.println("Got a Ping!");
    } else if(event == websockets::WebsocketsEvent::GotPong) {
        Serial.println("Got a Pong!");
    }
}

void setup()
{
  delay(1000);
  Serial.begin(9600);
  Serial.setTimeout(2000);

  connectToWiFi();

  // Wait for serial to initialize.
  while (!Serial)
  {
  }

  // Initialize the DHT sensor.
  dht.begin();

  bool connected = socket.connect(server_host, server_port, "/receiveWsData/");
  if(connected) {
      Serial.println("Connecetd!");
  } else {
      Serial.println("Not Connected!");
  }
  
  // run callback when messages are received
  socket.onMessage([&](websockets::WebsocketsMessage message) {
      Serial.print("Got Message: ");
      Serial.println(message.data());
  });
}

int timeSinceLastRead = 0;
void loop()
{
  // Report every 2 seconds.
  if (timeSinceLastRead > 2000)
  {
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t))
    {
      Serial.println("Failed to read from DHT sensor!");
      timeSinceLastRead = 0;
      return;
    }

    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print("C");
    Serial.print("\n");

    if(WiFi.status() == WL_CONNECTED) {
      if(socket.available()) {
        socket.send("{\"humidity\": " + String(h) + ", \"temperature\": " + String(t) + "}");
        socket.poll();
      } else {
        bool connected = socket.connect(server_host, server_port, "/receiveWsData/");
        if(connected) {
            Serial.println("Connecetd!");
        } else {
            Serial.println("Not Connected!");
        }
      }
    } else {
      connectToWiFi();
    }

    timeSinceLastRead = 0;
  }
  delay(100);
  timeSinceLastRead += 100;
}