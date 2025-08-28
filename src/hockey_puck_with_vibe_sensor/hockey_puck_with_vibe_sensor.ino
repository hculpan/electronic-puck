#include <Adafruit_CircuitPlayground.h>
#include <ArduinoLowPower.h>

const int VIB_PIN = A1;
const bool DISABLE_SLEEP = false;
const uint32_t IDLE_TIMEOUT_MS = 30000; // 30 seconds
const uint32_t DEBOUNCE_MS     = 30;
const uint32_t HOLD_MS         = 700;

volatile bool vibFlag = false;         // set in ISR while awake
volatile bool wokeFromInterrupt = false;

uint32_t lastHitMs = 0;
uint32_t ledHoldUntil = 0;
uint32_t lastActivityMs = 0;
unsigned long hits = 0;

long lastRandom = -1;

// ---------- Interrupts ----------
void vibISR() {        
  vibFlag = true;
}

void wakeISR() {       
  wokeFromInterrupt = true;
}

static inline void attachVibInterrupt() {
  attachInterrupt(digitalPinToInterrupt(VIB_PIN), vibISR, FALLING);
}

static inline void detachVibInterrupt() {
  detachInterrupt(digitalPinToInterrupt(VIB_PIN));
}

// Handle a latched vibration event in main context
void handleVibrationFlag() {
  bool got = false;
  noInterrupts();
  if (vibFlag) { got = true; vibFlag = false; }
  interrupts();

  if (got) {
    uint32_t now = millis();
    if (now - lastHitMs > DEBOUNCE_MS) {
      lastHitMs = now;
      ledHoldUntil = now + HOLD_MS;   // stretch the visible event
      lastActivityMs = now;           // reset idle timer
      hits++;
    }
  }
}

void delayWithSensor(uint32_t ms) {
  uint32_t end = millis() + ms;
  while ((int32_t)(millis() - end) < 0) {
    handleVibrationFlag();

    if (millis() < ledHoldUntil) {
      CircuitPlayground.setPixelColor(0, 0, 255, 0); 
    }
    delay(1);  
  }
}

void enterDeepSleep() {
  for (int i = 0; i < 10; i++) CircuitPlayground.setPixelColor(i, 0, 0, 0);
  CircuitPlayground.strip.show();

  detachVibInterrupt();

  LowPower.attachInterruptWakeup(digitalPinToInterrupt(VIB_PIN), wakeISR, FALLING);

  LowPower.sleep();
}

void setup() {
  CircuitPlayground.begin();
  pinMode(VIB_PIN, INPUT_PULLUP);
  CircuitPlayground.setBrightness(64);

  // Good practice for randomness
  randomSeed(analogRead(A5));

  // Attach interrupt for awake operation
  attachVibInterrupt();

  lastActivityMs = millis();
}

void loop() {
  uint32_t now = millis();

  // Service any pending vibration events immediately
  handleVibrationFlag();

  // Brief visual “hold” (pixel 0 green for HOLD_MS)
  if (now < ledHoldUntil) {
    CircuitPlayground.setPixelColor(0, 0, 255, 0);
  }

  // Auto-sleep after idle timeout
  if (!DISABLE_SLEEP && (now - lastActivityMs) >= IDLE_TIMEOUT_MS) {
    enterDeepSleep();

    // After wake, re-attach awake-mode interrupt
    attachVibInterrupt();

    // Reset timers and give a quick wake blip (blue)
    lastActivityMs = millis();
    ledHoldUntil   = millis() + 300;
    CircuitPlayground.setPixelColor(0, 0, 0, 255);
  }

  // Your animation chooser (avoid repeats)
  long r = random(5);
  while (r == lastRandom) { r = random(5); }
  lastRandom = r;

  switch (r) {
    case 0: circle();         break;
    case 1: reverse_circle(); break;
    case 2: colorScale();     break;
    case 3: randomPixel();    break;
    case 4: fillItUp();       break;
    default: blink();         break;
  }
}

// ---------- Animations (now sensor-friendly via delayWithSensor) ----------
void fillItUp() {
  for (int x = 0; x < 8u; x++) {
    CircuitPlayground.clearPixels();
    bool lit[10] = {false,false,false,false,false,false,false,false,false,false};
    for (int n = 0; n < 10; n++) {
      uint32_t color = numToColor(n);
      int pixel = random(10);
      while (lit[pixel]) { pixel = random(10); }
      lit[pixel] = true;
      CircuitPlayground.setPixelColor(pixel, color);
      delayWithSensor(100);
    }
  }
}

void randomPixel() {
  for (int x = 0; x < 8; x++) {
    for (int n = 0; n < 10; n++) {
      CircuitPlayground.clearPixels();
      uint32_t color = numToColor(n);
      int pixel = random(10);
      CircuitPlayground.setPixelColor(pixel, color);
      delayWithSensor(100);
    }
  }
}

void colorScale() {
  CircuitPlayground.clearPixels();
  for (int x = 0; x < 6; x++) {
    for (int n = 0; n < 10; n++) {
      uint32_t color = numToColor(n);
      for (int i = 0; i < 10; i++) {
        CircuitPlayground.setPixelColor(i, color);
      }
      delayWithSensor(250);
    }
  }
}

void blink() {
  for (int x = 0; x < 3; x++) {
    for (int n = 0; n < 10; n++) {
      uint32_t color = numToColor(n);

      CircuitPlayground.clearPixels();
      for (int i = 0; i < 10; i++) {
        CircuitPlayground.setPixelColor(i, color);
      }
      delayWithSensor(250);

      CircuitPlayground.clearPixels();
      delayWithSensor(250);
    }
  }
}

void circle() {
  for (int x = 0; x < 10; x++) {
    for (int n = 0; n < 10; n++) {
      uint32_t color = numToColor(x);
      CircuitPlayground.clearPixels();
      CircuitPlayground.setPixelColor(n, color);
      delayWithSensor(50);
    }
  }
}

void reverse_circle() {
  for (int x = 9; x > -1; x--) {
    for (int n = 9; n > -1; n--) {
      uint32_t color = numToColor(x);
      CircuitPlayground.clearPixels();
      CircuitPlayground.setPixelColor(n, color);
      delayWithSensor(50);
    }
  }
}

// Map 0–9 to color wheel positions
uint32_t numToColor(int n) {
  float hue = (n % 10) * 36; // 0–324 degrees in 36° steps
  return CircuitPlayground.colorWheel((int)(hue / 360.0 * 255));
}