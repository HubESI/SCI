#include <ESP8266WiFi.h>
#include <LoRa.h>
#include <PubSubClient.h>
#include <SPI.h>

#define NSS 15
#define RST 16
#define DIO0 7

const char* ssid = "rm8p";
const char* password = "azerty12345";

const char* broker = "broker.mqttdashboard.com";
const char* topic = "geoLocation";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (100)
char msg[MSG_BUFFER_SIZE];

void setup_wifi() {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection to ");
        Serial.print(broker);
        Serial.print(":1883 ... ");
        String clientId = "NodeMCUESP8266-";
        clientId += String(WiFi.macAddress());
        if (client.connect(clientId.c_str())) {
            Serial.println("connected");
            client.publish(topic, "hello");
            client.subscribe("TestSub");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(1000);
        }
    }
}

void setup() {
    Serial.begin(9600);
    Serial.println("LoRa Receiver");
    LoRa.setPins(NSS, RST, DIO0);
    if (!LoRa.begin(433E6)) {
        Serial.println("Starting LoRa failed!");
        while (1);
    }
    setup_wifi();
    client.setServer(broker, 1883);
    client.setCallback(callback);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        Serial.print("Received packet '");
        int i = 0;
        while (LoRa.available()) {
            msg[i] = (char)LoRa.read();
            Serial.print(msg[i]);
            i++;
        }
        Serial.print("' with RSSI ");
        Serial.println(LoRa.packetRssi());
        Serial.print("Publish to ");
        Serial.print(topic);
        Serial.print(" message: ");
        Serial.println(msg);
        client.publish(topic, msg);
    }
    delay(1000);
}
