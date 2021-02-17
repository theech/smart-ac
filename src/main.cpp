#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Gree.h>

#define sprint Serial.print
#define sprintln Serial.println
#define sprintf Serial.printf

#define FEATURE 12
#define PIR1 32
#define SW1 34
#define SW2 26
#define SW3 27
#define RL1 19
#define RL2 18
#define RL3 4

const uint16_t kIrLed = 17;
IRGreeAC ac(kIrLed);

int state = HIGH;	// the current state of the output pin
int reading;		// the current reading from the input pin
int previous = LOW; // the previous reading from the input pin

int pir_reading;

int state_sw1 = HIGH;
int reading_sw1;		// the current reading from the input pin
int previous_sw1 = LOW; // the previous reading from the input pin
// the follow variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long now;			  // the last time the output pin was toggled
unsigned long now_sw1;		  // the last time the output pin was toggled
unsigned long debounce = 200; // the debounce time, increase if the output flickers

void printState()
{
	// Display the settings.
	sprintln("GREE A/C remote is in the following state:");
	sprintf(" %s\n", ac.toString().c_str());
	// Display the encoded IR sequence.
	unsigned char *ir_code = ac.getRaw();
	sprint("IR Code: 0x");
	for (uint8_t i = 0; i < kGreeStateLength; i++)
		sprintf("%02X", ir_code[i]);
	sprintln();
}

void setup()
{
	pinMode(FEATURE, INPUT);
	pinMode(PIR1, INPUT);
	pinMode(SW1, INPUT);
	pinMode(SW2, INPUT);
	pinMode(SW3, INPUT);
	pinMode(BUILTIN_LED, OUTPUT);
	pinMode(RL1, OUTPUT);
	pinMode(RL2, OUTPUT);
	pinMode(RL3, OUTPUT);
	Serial.begin(115200);

	ac.on();
	delay(200); // Set up what we want to send. See ir_Gree.cpp for all the options.
	// Most things default on to off.
	sprintln("Default state of the remote.");
	sprintln("Setting desired state for A/C.");
	ac.on();
	ac.setFan(3);
	// kGreeAuto, kGreeDry, kGreeCool, kGreeFan, kGreeHeat
	ac.setMode(kGreeCool);
	ac.setTemp(25); // 16-30C
	ac.setSwingVertical(true, kGreeSwingAuto);
	ac.setXFan(true);
	ac.setLight(true);
	ac.setSleep(false);
	ac.setTurbo(true);
}

bool isSence()
{
	pir_reading = digitalRead(PIR1);
	sprintln("Check PIR =" + String(pir_reading));
	// turn one builin led when the sensor is detected
	return pir_reading;
}

bool switch1()
{
	reading_sw1 = digitalRead(SW1);
	if (reading_sw1 == HIGH && previous_sw1 == LOW && millis() - now_sw1 > debounce)
	{
		state_sw1 == HIGH ? state_sw1 = LOW : state_sw1 = HIGH;
		now_sw1 = millis();
		sprintln("sw 1 active now");
	}
	previous_sw1 = reading_sw1;

	return state_sw1;
}

void autoMode()
{
	if (isSence())
	{
		ac.on();
		ac.send();
		sprintln("Sending IR On command");

		digitalWrite(RL1, HIGH);
	}
	else
	{
		ac.off();
		ac.send();
		sprintln("Sending IR Off command");

		digitalWrite(RL1, LOW);
	}
}

void manualMode()
{
	// printState();
	digitalWrite(RL1, switch1());
}

void loop()
{
	reading = digitalRead(FEATURE);

	// if the input just went from LOW and HIGH and we've waited long enough
	// to ignore any noise on the circuit, toggle the output pin and remember
	// the time
	if (reading == HIGH && previous == LOW && millis() - now > debounce)
	{
		state == HIGH ? state = LOW : state = HIGH;
		now = millis();
	}

	switch (state)
	{
	case 0:
		manualMode();
		break;
	case 1:
		autoMode();
		break;
	default:
		sprintln("something wrong");
		break;
	}

	digitalWrite(BUILTIN_LED, state);

	previous = reading;
}
