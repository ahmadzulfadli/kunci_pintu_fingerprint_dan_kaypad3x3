#include <Adafruit_Fingerprint.h>
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

//------------------------------------lcd keypad------------------------------------
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Password.h>



byte currentLength = 0;

Password password = Password("2003");//Password bisa di ubah ubah
LiquidCrystal_I2C lcd(0x27, 16, 2);// pin lcd yang di gunakan

const byte ROWS= 4; //baris pada keypad
const byte COLS= 3; //Kolom pada keypad
char keys[ROWS][COLS] =
{
{'1', '2', '3'},
{'4', '5', '6'},
{'7', '8', '9'},
{'*', '0', '#'}
};

byte rowPins[ROWS] = {2, 3, 4, 5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {6, 7, 8}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//------------------------------akhir-------------------------------

SoftwareSerial mySerial(9, 10); //Inisialisasi serial RX TX modul fingerprint
int doorLock = 11; //pin untuk sinyal doorlock

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
    Serial.begin(9600);
    while (!Serial);
    delay(100);
    Serial.println("nnAdafruit finger detect test");

    // set the data rate for the sensor serial port
    finger.begin(57600);

    if (finger.verifyPassword()) {
        Serial.println("Found fingerprint sensor!");
    } else {
        Serial.println("Did not find fingerprint sensor :(");
        while (1) {
            delay(1);
        }
    }

    finger.getTemplateCount();
    Serial.print("Sensor contains ");
    Serial.print(finger.templateCount);
    Serial.println(" templates");
    Serial.println("Waiting for valid finger...");

    lcd.begin();
    lcd.clear();

    pinMode(doorLock, OUTPUT);
    digitalWrite(doorLock, HIGH);

    //-----------------------------------------lcd keypad--------------------------
    keypad.addEventListener(keypadEvent);

    lcd.begin();
    lcd.setCursor(1, 0);
    lcd.print("TEKNIK ELEKTRO");

    lcd.setCursor(4, 1);
    lcd.print("RESTEC");
    delay(3000);

    lcd.clear();
}

void loop() {
    getFingerprintID();
    delay(50);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fingerprint");
    lcd.setCursor(0, 1);
    lcd.print("Input PIN: ");
    keypad.getKey();
}
//-----------------------------------------keypad--------------------------------

void checkPassword() {
    if (password.evaluate()) {
        digitalWrite(doorLock, LOW);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("BERHASIL");
        lcd.setCursor(0, 1);
        lcd.print("TERIDENTIFIKASI");
        digitalWrite(doorLock, LOW);
        lcd.setCursor(20, 0);
        lcd.print("AutoLock ");
        for (int i = 5; i > 0; i--) {
            lcd.setCursor(30, 0);
            lcd.print(i);
            delay(1000);
        }
        digitalWrite(doorLock, HIGH);
        password.reset();
    } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("TIDAK ");
        lcd.setCursor(0, 1);
        lcd.print("TERIDENTIFIKASI");
        delay(500);
        lcd.setCursor(20, 0);
        lcd.print("COBA LAGI");
        delay(2000); 
        password.reset();
    }
}


void keypadEvent(KeypadEvent eKey) {

    switch (keypad.getState()) {
        case PRESSED:
            lcd.setCursor(35, 1);
            lcd.print(eKey);
            switch (eKey) {
                case '*':
                    checkPassword();
                    lcd.clear();
                    currentLength = 0;
                    break;
                case '#':
                    password.reset();
                    lcd.clear();
                    currentLength = 0;
                    break;
                default: // password.append(eKey);
                    password << eKey;
                    currentLength++;

                    //Print some feedback.
                    lcd.setCursor(12, 1);
                    //lcd.print("Pass: ");
                    for (byte i = 0; i < currentLength; i++) {
                        lcd.print('*');
                    }

            }
    }
}

//------------------------------------------------------------
uint8_t getFingerprintID() {
    uint8_t p = finger.getImage();
    switch (p) {
        case FINGERPRINT_OK:
            Serial.println("Image taken");
            break;
        case FINGERPRINT_NOFINGER:
            Serial.println("No finger detected");
            return p;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            return p;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println("Imaging error");
            return p;
        default:
            Serial.println("Unknown error");
            return p;
    }

    // OK success!

    p = finger.image2Tz();
    switch (p) {
        case FINGERPRINT_OK:
            Serial.println("Image converted");
            break;
        case FINGERPRINT_IMAGEMESS:
            Serial.println("Image too messy");
            return p;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            return p;
        case FINGERPRINT_FEATUREFAIL:
            Serial.println("Could not find fingerprint features");
            return p;
        case FINGERPRINT_INVALIDIMAGE:
            Serial.println("Could not find fingerprint features");
            return p;
        default:
            Serial.println("Unknown error");
            return p;
    }

    // OK converted!---------------------------------------------
    p = finger.fingerFastSearch();
    if (p == FINGERPRINT_OK) { //jika fingerprint terdeteksi
        Serial.println("Found a print match!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("BERHASIL");
        lcd.setCursor(0, 1);
        lcd.print("TERIDENTIFIKASI");
        digitalWrite(doorLock, LOW);
        lcd.setCursor(20, 0);
        lcd.print("AutoLock ");
        for (int i = 5; i > 0; i--) {
            lcd.setCursor(30, 0);
            lcd.print(i);
            delay(1000);
        }
        digitalWrite(doorLock, HIGH);
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
    } else if (p == FINGERPRINT_NOTFOUND) { //jika fingerprint salah
        Serial.println("Did not find a match");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("TIDAK ");
        lcd.setCursor(0, 1);
        lcd.print("TERIDENTIFIKASI");
        delay(500);
        lcd.setCursor(20, 0);
        lcd.print("COBA LAGI");
        delay(2000);
        
        return p;
    } else {
        Serial.println("Unknown error");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("??");
        delay(100);
        return p;
    }

    // found a match!
    Serial.print("Found ID #");
    Serial.print(finger.fingerID);
    Serial.print(" with confidence of ");
    Serial.println(finger.confidence);

    return finger.fingerID;
}
//-----------------------------------------------------------------------------

