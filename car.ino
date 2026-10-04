#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

#define IN1 26
#define IN2 25
#define IN3 14
#define IN4 27

const char* ssid = "YCODE_CAR";
const char* password = "Youssif88me";

const byte DNS_PORT = 53;
DNSServer dnsServer;
WebServer server(80);

unsigned long lastCmd = 0;
bool moving = false;

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void forward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void backward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void right() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void left() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<title>YCODE CAR</title>

<style>
* {
  box-sizing: border-box;
  -webkit-tap-highlight-color: transparent;
}

body {
  margin: 0;
  background: #101010;
  color: white;
  font-family: Arial, sans-serif;
  min-height: 100vh;
  display: flex;
  justify-content: center;
  align-items: center;
  user-select: none;
  -webkit-user-select: none;
  -webkit-touch-callout: none;
}

.container {
  width: 100%;
  max-width: 400px;
  padding: 20px;
  text-align: center;
}

h1 {
  margin-bottom: 5px;
}

.status {
  color: #00ff88;
  margin-bottom: 30px;
}

.controls {
  display: grid;
  grid-template-columns: 1fr 1fr 1fr;
  gap: 15px;
}

button {
  height: 90px;
  border: 0;
  border-radius: 20px;
  background: #222;
  color: white;
  display: flex;
  justify-content: center;
  align-items: center;
  touch-action: none;
  -webkit-touch-callout: none;
  user-select: none;
  -webkit-user-select: none;
}

button:active {
  transform: scale(.94);
  background: #333;
}

button svg {
  width: 46px;
  height: 46px;
  pointer-events: none;
}

.left svg  { transform: rotate(-90deg); }
.right svg { transform: rotate(90deg); }
.down svg  { transform: rotate(180deg); }

.empty {
  visibility: hidden;
}

.stop {
  background: #b91c1c;
}

.stop:active {
  background: #dc2626;
}
</style>
</head>

<body>

<div class="container">

<h1>YCODE CAR</h1>

<div class="status">ESP32 AP CONTROL</div>

<div class="controls">

<div class="empty"></div>

<button data-dir="forward" class="up">
<svg viewBox="0 0 24 24"><path fill="currentColor" d="M12 3l9 10h-5.5v8h-7v-8H3z"/></svg>
</button>

<div class="empty"></div>

<button data-dir="left" class="left">
<svg viewBox="0 0 24 24"><path fill="currentColor" d="M12 3l9 10h-5.5v8h-7v-8H3z"/></svg>
</button>

<button class="stop" id="stopBtn">
<svg viewBox="0 0 24 24"><rect x="5" y="5" width="14" height="14" rx="2" fill="currentColor"/></svg>
</button>

<button data-dir="right" class="right">
<svg viewBox="0 0 24 24"><path fill="currentColor" d="M12 3l9 10h-5.5v8h-7v-8H3z"/></svg>
</button>

<div class="empty"></div>

<button data-dir="backward" class="down">
<svg viewBox="0 0 24 24"><path fill="currentColor" d="M12 3l9 10h-5.5v8h-7v-8H3z"/></svg>
</button>

<div class="empty"></div>

</div>

</div>

<script>
let timer = null;

function send(path) {
  fetch("/" + path).catch(() => {});
}

function start(dir) {
  if (timer) return;
  send(dir);
  timer = setInterval(() => send(dir), 150);
}

function release() {
  if (!timer) return;
  clearInterval(timer);
  timer = null;
  send("stop");
}

document.querySelectorAll("button[data-dir]").forEach(btn => {
  btn.addEventListener("pointerdown", e => {
    e.preventDefault();
    start(btn.dataset.dir);
  });
  ["pointerup", "pointercancel", "pointerleave"].forEach(ev =>
    btn.addEventListener(ev, release)
  );
});

document.getElementById("stopBtn").addEventListener("pointerdown", e => {
  e.preventDefault();
  if (timer) {
    clearInterval(timer);
    timer = null;
  }
  send("stop");
});

document.addEventListener("contextmenu", e => e.preventDefault());
window.addEventListener("blur", release);
</script>

</body>
</html>
)rawliteral";

void redirectToPortal() {
  String url = "http://" + WiFi.softAPIP().toString() + "/";
  server.sendHeader("Location", url, true);
  server.send(302, "text/plain", "");
}

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  stopMotors();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  IPAddress apIP = WiFi.softAPIP();

  Serial.println();
  Serial.println("YCODE CAR");
  Serial.print("IP: ");
  Serial.println(apIP);

  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", []() {
    server.send(200, "text/html; charset=utf-8", webpage);
  });

  server.on("/forward", []() {
    forward();
    moving = true;
    lastCmd = millis();
    server.send(200, "text/plain", "FORWARD");
  });

  server.on("/backward", []() {
    backward();
    moving = true;
    lastCmd = millis();
    server.send(200, "text/plain", "BACKWARD");
  });

  server.on("/right", []() {
    right();
    moving = true;
    lastCmd = millis();
    server.send(200, "text/plain", "RIGHT");
  });

  server.on("/left", []() {
    left();
    moving = true;
    lastCmd = millis();
    server.send(200, "text/plain", "LEFT");
  });

  server.on("/stop", []() {
    stopMotors();
    moving = false;
    server.send(200, "text/plain", "STOP");
  });

  server.onNotFound(redirectToPortal);

  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  if (moving && millis() - lastCmd > 400) {
    stopMotors();
    moving = false;
  }
}