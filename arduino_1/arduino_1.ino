#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte ARD2_RX_PIN = 3;
const byte ARD2_TX_PIN = 4;

SoftwareSerial arduino2Serial(ARD2_RX_PIN, ARD2_TX_PIN);

const byte coinPin = 2;

const byte buttonA = 8;
const byte buttonB = 9;
const byte buttonC = 10;
const byte buttonD = 11;

const byte buttonCancel = 12;
const byte buttonConfirm = 13;

volatile bool coinInserted = false;

int totalMoney = 0;

int priceA = 10;
int priceB = 10;
int priceC = 20;
int priceD = 30;

int stockA = 0;
int stockB = 0;
int stockC = 0;
int stockD = 0;

char selectedProduct = 0;
int selectedPrice = 0;

bool busy = false;

const unsigned long SLEEP_TIMEOUT_MS = 30000;

unsigned long lastActivityTime = 0;
bool sleeping = false;

void coinInterrupt() {
  coinInserted = true;
}

void wakeUp() {
  lastActivityTime = millis();

  if (sleeping) {
    sleeping = false;
    lcd.backlight();
  }
}

void showMoney() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("MONEY: ");
  lcd.print(totalMoney);
  lcd.print(" BAHT");

  lcd.setCursor(0, 1);

  if (selectedProduct == 0) {
    lcd.print("SELECT A-D");
  }
  else {
    lcd.print("SEL ");
    lcd.print(selectedProduct);
    lcd.print(" ");
    lcd.print(selectedPrice);
    lcd.print(" BAHT");
  }
}

void showSelectProduct() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("MONEY: ");
  lcd.print(totalMoney);
  lcd.print(" BAHT");

  lcd.setCursor(0, 1);

  if (selectedProduct == 0) {
    lcd.print("SELECT A-D");
  }
  else {
    lcd.print("SEL ");
    lcd.print(selectedProduct);
    lcd.print(" ");
    lcd.print(selectedPrice);
  }
}

int getPrice(char product) {
  if (product == 'A') return priceA;
  if (product == 'B') return priceB;
  if (product == 'C') return priceC;
  if (product == 'D') return priceD;

  return 0;
}

int getStock(char product) {
  if (product == 'A') return stockA;
  if (product == 'B') return stockB;
  if (product == 'C') return stockC;
  if (product == 'D') return stockD;

  return 0;
}

void selectProduct(char product) {
  if (busy) {
    return;
  }

  if (getStock(product) <= 0) {
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("PRODUCT ");
    lcd.print(product);

    lcd.setCursor(0, 1);
    lcd.print("OUT OF STOCK");

    delay(1200);

    showSelectProduct();

    return;
  }

  selectedProduct = product;
  selectedPrice = getPrice(product);

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("PRODUCT ");
  lcd.print(product);

  lcd.setCursor(0, 1);
  lcd.print("PRICE: ");
  lcd.print(selectedPrice);
  lcd.print(" BAHT");

  delay(700);

  showSelectProduct();
}

void cancelPurchase() {
  if (busy) {
    return;
  }

  selectedProduct = 0;
  selectedPrice = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CANCELLED");

  lcd.setCursor(0, 1);
  lcd.print("MONEY: ");
  lcd.print(totalMoney);

  delay(1000);

  showSelectProduct();
}

void confirmPurchase() {
  if (busy) {
    return;
  }

  if (selectedProduct == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SELECT PRODUCT");

    delay(1000);

    showSelectProduct();

    return;
  }

  if (getStock(selectedProduct) <= 0) {
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("OUT OF STOCK");

    delay(1500);

    selectedProduct = 0;
    selectedPrice = 0;

    showSelectProduct();

    return;
  }

  if (totalMoney < selectedPrice) {
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("NOT ENOUGH");

    lcd.setCursor(0, 1);
    lcd.print("NEED ");
    lcd.print(selectedPrice);
    lcd.print(" BAHT");

    delay(2000);

    showSelectProduct();

    return;
  }

  busy = true;

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("DISPENSING ");

  lcd.print(selectedProduct);

  lcd.setCursor(0, 1);
  lcd.print("PLEASE WAIT");

  arduino2Serial.println(selectedProduct);
}

void receiveFromPi() {
  if (!Serial.available()) {
    return;
  }

  String command = Serial.readStringUntil('\n');

  command.trim();

  Serial.print("RECEIVED:[");
  Serial.print(command);
  Serial.print("] LEN=");
  Serial.println(command.length());

  if (command.startsWith("PRICE,")) {
    int comma1 = command.indexOf(',');
    int comma2 = command.indexOf(',', comma1 + 1);

    if (comma1 == -1 || comma2 == -1) {
      return;
    }

    char product = command.charAt(comma1 + 1);

    int newPrice = command.substring(comma2 + 1).toInt();

    if (newPrice < 10 || newPrice % 10 != 0) {
      return;
    }

    if (product == 'A') {
      priceA = newPrice;
    }
    else if (product == 'B') {
      priceB = newPrice;
    }
    else if (product == 'C') {
      priceC = newPrice;
    }
    else if (product == 'D') {
      priceD = newPrice;
    }
    else {
      return;
    }

    if (selectedProduct == product) {
      selectedPrice = newPrice;
    }

    Serial.print("PRICE_UPDATED,");
    Serial.print(product);
    Serial.print(",");
    Serial.println(newPrice);

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("PRICE ");
    lcd.print(product);
    lcd.print(" = ");
    lcd.print(newPrice);

    lcd.setCursor(0, 1);
    lcd.print("BAHT");

    delay(1200);

    showSelectProduct();

    return;
  }

  if (command.startsWith("STOCK,")) {
    int comma1 = command.indexOf(',');
    int comma2 = command.indexOf(',', comma1 + 1);

    if (comma1 == -1 || comma2 == -1) {
      return;
    }

    char product = command.charAt(comma1 + 1);

    int newStock = command.substring(comma2 + 1).toInt();

    if (newStock < 0) {
      return;
    }

    if (product == 'A') {
      stockA = newStock;
    }
    else if (product == 'B') {
      stockB = newStock;
    }
    else if (product == 'C') {
      stockC = newStock;
    }
    else if (product == 'D') {
      stockD = newStock;
    }
    else {
      return;
    }

    Serial.print("STOCK_UPDATED,");
    Serial.print(product);
    Serial.print(",");
    Serial.println(newStock);

    if (!busy) {
      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("STOCK ");
      lcd.print(product);
      lcd.print(" = ");
      lcd.print(newStock);

      delay(500);

      showSelectProduct();
    }

    return;
  }
}

void receiveFromArduino2() {
  if (!arduino2Serial.available()) {
    return;
  }

  String command = arduino2Serial.readStringUntil('\n');

  command.trim();

  if (command == "Y") {
    if (!busy) {
      return;
    }

    totalMoney -= selectedPrice;

    Serial.print("SALE,");
    Serial.print(selectedProduct);
    Serial.print(",");
    Serial.print(selectedPrice);
    Serial.println(",SUCCESS");

    selectedProduct = 0;
    selectedPrice = 0;

    busy = false;

    delay(500);

    showSelectProduct();

    return;
  }

  if (command == "N") {
    if (!busy) {
      return;
    }

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("DISPENSE FAILED");

    Serial.print("SALE,");
    Serial.print(selectedProduct);
    Serial.print(",");
    Serial.print(selectedPrice);
    Serial.println(",FAILED");

    selectedProduct = 0;
    selectedPrice = 0;

    busy = false;

    delay(1500);

    showSelectProduct();

    return;
  }
}

void setup() {
  Serial.begin(9600);
  arduino2Serial.begin(9600);

  lcd.begin();
  lcd.backlight();

  pinMode(buttonA, INPUT_PULLUP);
  pinMode(buttonB, INPUT_PULLUP);
  pinMode(buttonC, INPUT_PULLUP);
  pinMode(buttonD, INPUT_PULLUP);

  pinMode(buttonCancel, INPUT_PULLUP);
  pinMode(buttonConfirm, INPUT_PULLUP);

  pinMode(coinPin, INPUT_PULLUP);

  attachInterrupt(
    digitalPinToInterrupt(coinPin),
    coinInterrupt,
    FALLING
  );

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("VENDING MACHINE");

  lcd.setCursor(0, 1);
  lcd.print("READY");

  delay(1500);

  showSelectProduct();

  lastActivityTime = millis();

  Serial.println("STATUS,ONLINE");
}

void loop() {
  receiveFromPi();
  receiveFromArduino2();

  if (busy) {
    return;
  }

  if (!sleeping && (millis() - lastActivityTime > SLEEP_TIMEOUT_MS)) {
    sleeping = true;

    lcd.noBacklight();
  }

  if (coinInserted) {
    noInterrupts();
    coinInserted = false;
    interrupts();

    wakeUp();

    totalMoney += 10;

    showMoney();

    delay(200);
  }

  if (digitalRead(buttonA) == LOW) {
    wakeUp();

    selectProduct('A');

    delay(250);
  }

  if (digitalRead(buttonB) == LOW) {
    wakeUp();

    selectProduct('B');

    delay(250);
  }

  if (digitalRead(buttonC) == LOW) {
    wakeUp();

    selectProduct('C');

    delay(250);
  }

  if (digitalRead(buttonD) == LOW) {
    wakeUp();

    selectProduct('D');

    delay(250);
  }

  if (digitalRead(buttonCancel) == LOW) {
    wakeUp();

    cancelPurchase();

    delay(250);
  }

  if (digitalRead(buttonConfirm) == LOW) {
    wakeUp();

    confirmPurchase();

    delay(250);
  }
}