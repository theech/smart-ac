#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Gree.h>

#define sprint Serial.print
#define sprintln Serial.println
#define sprintf Serial.printf

const uint16_t kIrLed = 21;
IRGreeAC ac(kIrLed);

#define sw 22

int state = LOW;
int reading;
int previous = LOW;

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
	Serial.begin(115200);
	pinMode(sw, INPUT);
	delay(200); // Set up what we want to send. See ir_Gree.cpp for all the options.
	// Most things default on to off.
	sprintln("Default state of the remote.");
	printState();
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

	pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
	reading = digitalRead(sw);

	if (reading == HIGH && previous == LOW)
	{
		if (state == HIGH)
		{
			state = LOW;

			ac.on();
			ac.send();
			sprintln("Sending IR On command");
		}
		else
		{
			state = HIGH;

			ac.off();
			ac.send();
			sprintln("Sending IR Off command");
		}
	}

	digitalWrite(LED_BUILTIN, state);

	previous = reading;

	printState();
	delay(100);
}