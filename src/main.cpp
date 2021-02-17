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
#define SW3 25
#define RL1 19
#define RL2 18
#define RL3 4

const uint16_t kIrLed = 2;
IRGreeAC ac(kIrLed);

int control_state = 0;

int state = HIGH;	// the current state of the output pin
int reading;		// the current reading from the input pin
int previous = LOW; // the previous reading from the input pin

int pir_reading;

int state_sw1 = HIGH;
int reading_sw1;		// the current reading from the input pin
int previous_sw1 = LOW; // the previous reading from the input pin

int state_sw2 = HIGH;
int reading_sw2;		// the current reading from the input pin
int previous_sw2 = LOW; // the previous reading from the input pin

int state_sw3 = HIGH;
int reading_sw3;		// the current reading from the input pin
int previous_sw3 = LOW; // the previous reading from the input pin
// the follow variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long now;			  // the last time the output pin was toggled
unsigned long now_sw1;		  // the last time the output pin was toggled
unsigned long now_sw2;		  // the last time the output pin was toggled
unsigned long now_sw3;		  // the last time the output pin was toggled
unsigned long debounce = 200; // the debounce time, increase if the output flickers

// change mode time controlling
unsigned long now_feature;
unsigned long debounce_feature = 50000;

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

void IRAM_ATTR acOn()
{
	ac.on();
	ac.send();
	sprintln("Sending IR On command");
}

void IRAM_ATTR acOff()
{
	ac.off();
	ac.send();
	sprintln("Sending IR Off command");
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

	// attachInterrupt(digitalPinToInterrupt(PIR1), acOn, RISING);
	// delay(500);
	// attachInterrupt(digitalPinToInterrupt(PIR1), acOff, FALLING);
	// delay(500);
}

int isSence()
{
	// TODO: more than one PIR you need to do in the OR condition
	// uncomment below to complete the statment
	// (pir_reading1 == 1 || pir_reading2 == 1) ? pir_state = 1 : pir_state = 0;
	pir_reading = digitalRead(PIR1);
	sprintln("Check PIR =" + String(pir_reading));
	// turn one builin led when the sensor is detected
	// return pir_state;
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

bool switch2()
{
	reading_sw2 = digitalRead(SW2);
	if (reading_sw2 == HIGH && previous_sw2 == LOW && millis() - now_sw2 > debounce)
	{
		state_sw2 == HIGH ? state_sw2 = LOW : state_sw2 = HIGH;
		now_sw2 = millis();
		sprintln("sw 2 active now");
	}
	previous_sw2 = reading_sw2;

	return state_sw2;
}

bool switch3()
{
	reading_sw3 = digitalRead(SW3);
	if (reading_sw3 == HIGH && previous_sw3 == LOW && millis() - now_sw3 > debounce)
	{
		state_sw3 == HIGH ? state_sw3 = LOW : state_sw3 = HIGH;
		now_sw3 = millis();
		sprintln("sw 3 active now");
	}
	previous_sw3 = reading_sw3;

	return state_sw3;
}

void autoMode()
{
	if (control_state != isSence())
	{
		if (control_state == 1)
		{
			ac.off();
			ac.send();
			sprintln("Sending IR Off command");
			digitalWrite(RL1, LOW);
			digitalWrite(RL2, LOW);
			digitalWrite(RL3, LOW);
		}
		else
		{
			ac.on();
			ac.send();
			sprintln("Sending IR On command");
			digitalWrite(RL1, HIGH);
			digitalWrite(RL2, HIGH);
			digitalWrite(RL3, HIGH);
		}
		control_state = isSence();
	}
}

void manualMode()
{
	digitalWrite(RL1, switch1());
	digitalWrite(RL2, switch2());
	digitalWrite(RL3, switch3());
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
		autoMode();
		break;
	case 1:
		manualMode();
		if (millis() - now_feature > debounce_feature)
		{
			if (!isSence())
			{
				state = 0;
				control_state = 1;
				sprintln("change mode now");
			}
			now_feature = millis();
		}
		break;
	default:
		sprintln("something wrong");
		break;
	}

	digitalWrite(BUILTIN_LED, state);

	previous = reading;
}
