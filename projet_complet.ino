#include <WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
#include <WebServer.h>

// --- Définition des broches ---
#define DHTPIN 4             // Broche connectée au DHT11
#define DHTTYPE DHT11        // Type de DHT
#define RAIN_PIN 34          // Capteur de pluie sur une entrée numérique
#define MQ135_PIN 35         // Entrée analogique pour le capteur MQ135

// --- Objets capteurs ---
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;

// --- Réseau WiFi ---
const char* ssid = "LB_ADSL_ZAQC";
const char* password = "kNHhf764GNsVquHYxS";

// --- Serveur Web ---
WebServer server(80);

// --- Page HTML ---
const char* pageHTML = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <title>Dashboard ESP32</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    * {
      box-sizing: border-box;
    }

    body {
      margin: 0;
      font-family: 'Segoe UI', sans-serif;
      background-color: #0f172a;
      color: #e2e8f0;
      display: flex;
    }

    /* Sidebar */
    .sidebar {
      width: 200px;
      background: #1e293b;
      height: 100vh;
      padding: 20px;
      display: flex;
      flex-direction: column;
      gap: 20px;
      color: #38bdf8;
    }

    .sidebar h2 {
      font-size: 20px;
      margin-bottom: 10px;
    }

    .sidebar a {
      color: #94a3b8;
      text-decoration: none;
      padding: 10px;
      border-radius: 5px;
      transition: 0.3s;
    }

    .sidebar a:hover {
      background-color: #334155;
      color: #ffffff;
    }

    /* Main */
    .main {
      flex-grow: 1;
      padding: 20px;
    }

    .header {
      display: flex;
      justify-content: space-between;
      align-items: center;
    }

    .title {
      font-size: 24px;
    }

    .lang-switch {
      background: #334155;
      color: #f1f5f9;
      padding: 5px 10px;
      border: none;
      border-radius: 5px;
      cursor: pointer;
    }

    .grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
      gap: 20px;
      margin-top: 30px;
    }

    .card {
      background-color: #1e293b;
      padding: 20px;
      border-radius: 12px;
      box-shadow: 0 0 10px rgba(56, 189, 248, 0.2);
      text-align: center;
    }

    .card h3 {
      margin-bottom: 10px;
      color: #38bdf8;
    }

    .value {
      font-size: 2em;
      font-weight: bold;
      color: #f8fafc;
    }
  </style>
</head>
<body>

  <div class="sidebar">
    <h2>🌐 Dashboard</h2>
    <a href="#">Accueil</a>
    <a href="#">Capteurs</a>
    <a href="#">Historique</a>
    <a href="#">Paramètres</a>
  </div>

  <div class="main">
    <div class="header">
      <div class="title" id="dashboard-title">Données Environnementales</div>
      <select id="language" class="lang-switch">
        <option value="fr">Français</option>
        <option value="en">English</option>
      </select>
    </div>

    <div class="grid">
      <div class="card"><h3 id="label-temp">Température</h3><div class="value" id="temp">-- °C</div></div>
      <div class="card"><h3 id="label-hum">Humidité</h3><div class="value" id="hum">-- %</div></div>
      <div class="card"><h3 id="label-pres">Pression</h3><div class="value" id="pres">-- hPa</div></div>
      <div class="card"><h3 id="label-alt">Altitude</h3><div class="value" id="alt">-- m</div></div>
      <div class="card"><h3 id="label-air">Qualité de l'air</h3><div class="value" id="air">--</div></div>
      <div class="card"><h3 id="label-rain">Pluie</h3><div class="value" id="rain">--</div></div>
    </div>
  </div>

  <script>
    const translations = {
      fr: {
        title: "Données Environnementales",
        temp: "Température",
        hum: "Humidité",
        pres: "Pression",
        alt: "Altitude",
        air: "Qualité de l'air",
        rain: "Pluie",
        rainYes: "Pluie détectée",
        rainNo: "Pas de pluie"
      },
      en: {
        title: "Environmental Data",
        temp: "Temperature",
        hum: "Humidity",
        pres: "Pressure",
        alt: "Altitude",
        air: "Air Quality",
        rain: "Rain",
        rainYes: "Rain detected",
        rainNo: "No rain"
      }
    };

    function applyLanguage(lang) {
      const t = translations[lang];
      document.getElementById("dashboard-title").innerText = t.title;
      document.getElementById("label-temp").innerText = t.temp;
      document.getElementById("label-hum").innerText = t.hum;
      document.getElementById("label-pres").innerText = t.pres;
      document.getElementById("label-alt").innerText = t.alt;
      document.getElementById("label-air").innerText = t.air;
      document.getElementById("label-rain").innerText = t.rain;
      currentLang = lang;
    }

    let currentLang = 'fr';

    document.getElementById("language").addEventListener("change", function () {
      applyLanguage(this.value);
    });

    async function fetchData() {
      try {
        const res = await fetch("/data");
        const data = await res.json();

        document.getElementById("temp").innerText = data.temperature + " °C";
        document.getElementById("hum").innerText = data.humidity + " %";
        document.getElementById("pres").innerText = data.pressure + " hPa";
        document.getElementById("alt").innerText = data.altitude + " m";
        document.getElementById("air").innerText = data.air_quality;
        document.getElementById("rain").innerText = data.rain
          ? translations[currentLang].rainYes
          : translations[currentLang].rainNo;

      } catch (err) {
        console.error("Erreur récupération données :", err);
      }
    }

    applyLanguage(currentLang);
    fetchData();
    setInterval(fetchData, 2000);
  </script>

</body>
</html>
)rawliteral";

// --- Fonctions serveur ---
void handleRoot() {
  server.send(200, "text/html", pageHTML);
}

void handleData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    server.send(500, "application/json", "{\"error\":\"Lecture DHT échouée\"}");
    return;
  }

  float pressure = bmp.readPressure(); // en Pascal
  float altitude = bmp.readAltitude(); // en mètres

  int airQuality = analogRead(MQ135_PIN);
  bool isRaining = digitalRead(RAIN_PIN) == LOW;

  String json = "{";
  json += "\"temperature\":" + String(temperature, 2) + ",";
  json += "\"humidity\":" + String(humidity, 2) + ",";
  json += "\"pressure\":" + String(pressure / 100.0, 2) + ","; // en hPa
  json += "\"altitude\":" + String(altitude, 2) + ",";
  json += "\"air_quality\":" + String(airQuality) + ",";
  json += "\"rain\":" + String(isRaining ? "true" : "false");
  json += "}";

  server.send(200, "application/json", json);
}

// --- Initialisation ---
void setup() {
  Serial.begin(115200);
  dht.begin();

  if (!bmp.begin()) {
    Serial.println("Erreur BMP180 !");
    while (1) {}
  }

  pinMode(RAIN_PIN, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connexion WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnecté ! IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
}

// --- Boucle principale ---
void loop() {
  server.handleClient();
}
