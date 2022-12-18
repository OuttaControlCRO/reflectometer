#include <Wire.h>             // include the I2C library
#include <Adafruit_Sensor.h>  // include the Adafruit sensor library
#include <Adafruit_TSL2591.h> // include the Adafruit TSL2591 library
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const int buttonPin = 12; // the pin that the button is attached to
const int relayPin = 16;  // the pin that the relay is attached to
// define a threshold value for the double-click interval
const int doubleClickThreshold = 500; // in milliseconds

int modeState = 0;
int buttonState = 0;         // variable to store the push button state
int previousButtonState = 0; // variable to store the previous push button state
long pressDuration = 0;      // variable to store the duration of the button

// define a variable to track the time of the last click
unsigned long lastClickTime = 0;
// initialize the TSL2591 lux sensor
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);

// initialize the OLED display
Adafruit_SSD1306 display(128, 32, &Wire, 4);

// variables to hold the lux values with the relay off and on
uint16_t luxOff, luxOn;

void setup()
{
    pinMode(buttonPin, INPUT); // set the button pin as an input
    pinMode(relayPin, OUTPUT); // set the relay pin as an input
    Serial.begin(115200);
    Wire.begin(); // initialize the I2C interface

    // initialize the TSL2591 lux sensor
    if (!tsl.begin())
    {
        Serial.println("Error initializing TSL2591 sensor!");
        while (1)
            ;
    }
    tsl.setGain(TSL2591_GAIN_MED);
    tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
    // initialize the OLED display

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println("Error initializing Display!");
    }
    display.clearDisplay();
    display.display();

    // display the configuration message on the OLED display
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.cp437(true);                 // Use full 256 char 'Code Page 437' font

    display.setCursor(0, 0);
    display.println("Configuration:");
    display.println("Click button to start");
    display.println("dark value measuring.");
    display.display();
    Serial.println("Click Button");
    // wait for the button to be pressed
    while (digitalRead(buttonPin) == LOW);

    // read the lux value with the relay off and save it
    digitalWrite(relayPin, LOW); // turn off the relay
    delay(1000);                 // wait for the lux value to stabilize
    //uint16_t lux = tsl.getFullLuminosity() & 0xFFFF;
    uint32_t full = tsl.getFullLuminosity();
    uint16_t lux = full & 0xFFFF;
    uint16_t ir = full >> 16;
    luxOff = lux;
    Serial.println("Raw: " + String(full) + ", Visible Spectrum = " + String(lux) + +", IR: " + String(ir));
    Serial.println("Lux " + String(tsl.calculateLux(lux, ir)));
    Serial.println("Raw visible spectrum while laser is off");
    Serial.println(luxOff);

    // display the lux value on the OLED display
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Visible off: " + String(luxOff));
    display.println("Click button, point");
    display.println("laser to measure max value");
    display.display();

    // wait for the button to be pressed
    while (digitalRead(buttonPin) == LOW);

    // read the lux value with the relay on and save it
    digitalWrite(relayPin, HIGH); // turn on the relay
    delay(2000);                  // wait for the lux value to stabilize
                                  //uint16_t lux = tsl.getFullLuminosity() & 0xFFFF;
    full = tsl.getFullLuminosity();
    lux = full & 0xFFFF;
    ir = full >> 16;
    luxOn = lux;
    Serial.println("Raw: " + String(full) + ", Visible spectrum = " + String(lux) + +", IR: " + String(ir));
    Serial.println("Lux " + String(tsl.calculateLux(lux, ir)));
    Serial.println("Raw visible spectrum value while laser is on");
    Serial.println(luxOn);

    // display the lux value on the OLED display
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Visible on: " + String(luxOn));
    display.println("Config done, press button long or short ");
    display.println("to start measuring");
    display.display();
    digitalWrite(relayPin, LOW);
    Serial.println("Press button to start measuring, press button for long to continous mode");
    // wait for the button to be pressed
    while (digitalRead(buttonPin) == LOW);
}

void loop()
{
    // read the state of the push button
    buttonState = digitalRead(buttonPin);

    // check if the button has just been pressed (i.e. the state has changed from LOW to HIGH)
    if (buttonState == HIGH && previousButtonState == LOW)
    {
        // record the time that the button was pressed
        pressDuration = millis();
          Serial.println(" From Low to high");

    }
    // check if the button has just been released (i.e. the state has changed from HIGH to LOW)
    if ((digitalRead(buttonPin) == LOW && previousButtonState == HIGH)&& (millis() - pressDuration > 10))
    {
     Serial.println(" From High to Low");
        // calculate the duration of the button press
        pressDuration = millis() - pressDuration;
     Serial.println(pressDuration);

        // check if the button press was short or long
        if (pressDuration < 1000)
        {
            // button press was short
            // return to button mode
            if (modeState == 2)
            {
                modeState = 0;
                digitalWrite(relayPin, LOW);
                display.clearDisplay();
                display.setTextSize(1);
                display.setCursor(0, 0);
                display.println("Continous mode deactivated");
                display.display();
                Serial.println(" Continous mode deactivated");
                delay(500);
            }
            else
            {
                modeState = 1;
            }
        }
        else
        {
                modeState = 2;
                display.clearDisplay();
                display.setTextSize(1);
                display.setCursor(0, 0);
                display.println("Continous mode activated");
                display.display();
                Serial.println(" Continous mode activated");
                delay(500);
        }
    }
    // save the current button state for the next loop iteration
    previousButtonState = buttonState;
    if (modeState == 2)
    {
        delay(400);
        continousMode();
    }
    else if (modeState == 1)
    {
        digitalWrite(relayPin, LOW);
        buttonMode();
        modeState = 0;
    }
}
void buttonMode()
{
    digitalWrite(relayPin, HIGH);
    // read the current lux value
    delay(2000);
    //uint16_t lux = tsl.getFullLuminosity() & 0xFFFF;
    uint32_t full = tsl.getFullLuminosity();
    uint16_t lux = full & 0xFFFF;
    uint16_t ir = full >> 16;

    // calculate the percentage between the relay off and on values
    float percentage = 100.00 * (lux - luxOff) / (luxOn - luxOff);
    Serial.println("Lux" + String(tsl.calculateLux(lux, ir)));
    Serial.println("Raw: " + String(full) + ", Full Spectrum = " + String(lux) + +", IR: " + String(ir) + ", Percentage = " + String(percentage));
    // display the percentage on the OLED display
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Visible: " + String(lux));
    display.println("Percentage: " + String(percentage) + "%");
    display.display();

    digitalWrite(relayPin, LOW);
}

void continousMode()
{
    digitalWrite(relayPin, HIGH);
    // read the current lux value
    delay(100);
    //uint16_t lux = tsl.getFullLuminosity() & 0xFFFF;
    uint32_t full = tsl.getFullLuminosity();
    uint16_t lux = full & 0xFFFF;
    uint16_t ir = full >> 16;

    // calculate the percentage between the relay off and on values
    float percentage = 100.00 * (lux - luxOff) / (luxOn - luxOff);
    Serial.println("Lux" + String(tsl.calculateLux(lux, ir)));
    Serial.println("Raw: " + String(full) + ", Full Spectrum = " + String(lux) + +", IR: " + String(ir) + ", Percentage = " + String(percentage));
    // display the percentage on the OLED display
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Visible: " + String(lux));
    display.println("Percentage: " + String(percentage) + "%");
    display.display();
    // check for button press and reset mode
    if (digitalRead(buttonPin) == HIGH)
    {
        modeState = 0;
        digitalWrite(relayPin, LOW);
    }
}
