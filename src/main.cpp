#include <Arduino.h>
#include "JVSIO.h"

# include "ProMicroClient.h"
// JVS pins for Arduino Nano/Uno
//  D0  - JVS Data+  => USB Type B Pin 3 (D+ Pin in USB proper use)
//  D2  - JVS Data-  => USB Type B Pin 2 (D- Pin in USB proper use)
//  D3  - JVS Sense  => USB Type B Pin 1 (5V Pin in USB proper use)
//  D13 - LED
ProMicroDataClient dataClient;
ProMicroSenseClient sense;
ProMicroLedClient led;
static const uint8_t buttonPin = 13;
static const char id[] = "TAITO CORP.;RFID CTRL P.C.B.;Ver1.00;";
static const uint8_t cardData[] = { 0x04, 0xC2, 0x3D, 0xDA, 0x6F, 0x52, 0x80, 0x00, 0x37, 0x30, 0x32, 0x30, 0x33, 0x39,
									0x32, 0x30, 0x31, 0x30, 0x32, 0x38, 0x31, 0x35, 0x30, 0x32 };
JVSIO io(&dataClient, &sense, &led);

void setup() {
	// TODO : factor out following Serial initialization code into LogClient or
	// something. See also another TODO in JVSIO.cpp - dump().
	Serial.begin(115200);
	Serial.println(id);
	pinMode(buttonPin, INPUT_PULLUP);
	delayMicroseconds(1000000);
	Serial.println("Starting...");
	io.begin();
}

bool cardIn = false;
void loop() {
	auto pressed = (digitalRead(buttonPin) == LOW);
	if (pressed) {
		cardIn = true;
	}
	uint8_t len;
	uint8_t* data = io.getNextCommand(&len);
	if (!data) {
		// Serial.println("No data");
		return;
	}

	auto cmd = static_cast<JVSIO::Cmd>(*data);
	switch (cmd) {
	case JVSIO::Cmd::kCmdIoId:
		Serial.println("GetID");
		io.pushReport(JVSIO::kReportOk);
		for (size_t i = 0; id[i]; ++i)
			io.pushReport(id[i]);
		io.pushReport(0);
		break;
	case JVSIO::Cmd::kCmdFunctionCheck:
		Serial.println("FunctionCheck");
		io.pushReport(JVSIO::kReportOk);

		io.pushReport(0x01);  // sw
		io.pushReport(0x02);  // players
		io.pushReport(0x0C);  // buttons
		io.pushReport(0x00);

/*		io.pushReport(0x02);  // coin
		io.pushReport(0x02);  // slots
		io.pushReport(0x00);
		io.pushReport(0x00);

		io.pushReport(0x03);  // analog inputs
		io.pushReport(0x08);  // channels
		io.pushReport(0x00);  // bits
		io.pushReport(0x00);*/

		io.pushReport(0x12);  // general purpose driver
		io.pushReport(0x08);  // slots
		io.pushReport(0x00);
		io.pushReport(0x00);

		io.pushReport(0x00);
		break;
	case JVSIO::Cmd::kCmdSwInput:
		Serial.println("kCmdSwInput");
		io.pushReport(JVSIO::kReportOk);
		io.pushReport(0x00);  // TEST, TILT1-3, and undefined x4.
		for (size_t player = 0; player < data[1]; ++player) {
			for (size_t line = 1; line <= data[2]; ++line)
				io.pushReport(0x00);
		}
		break;
	case JVSIO::Cmd::kCmdCoinInput:
		Serial.println("kCmdCoinInput");
		io.pushReport(JVSIO::kReportOk);
		for (size_t slot = 0; slot < data[1]; ++slot) {
			io.pushReport(0x00);
			io.pushReport(0x00);
		}
		break;
	case JVSIO::Cmd::kCmdAnalogInput:
		Serial.println("kCmdAnalogInput");
		io.pushReport(JVSIO::kReportOk);
		for (size_t channel = 0; channel < data[1]; ++channel) {
			io.pushReport(0x80);
			io.pushReport(0x00);
		}
		break;
	case JVSIO::Cmd::kCmdCoinSub:
	case JVSIO::Cmd::kCmdCoinAdd:
		io.pushReport(JVSIO::kReportOk);
		break;
	case JVSIO::Cmd::kCmdDriverInput:
		Serial.println("kCmdDriverInput");
		io.pushReport(JVSIO::kReportOk);
		for (auto i = 0; i < data[1]; ++i)
		{
			io.pushReport(cardIn ? 0x19 : 0);
		}
		break;
	case JVSIO::Cmd::kCmdDriverOutput:
		Serial.println("kCmdDriverOutput");
		io.pushReport(JVSIO::kReportOk);
		for (auto i : cardData)
		{
			io.pushReport(cardIn ? i: 0);
		}
		io.pushReport(JVSIO::kReportOk);
		cardIn = false;
		break;
	case JVSIO::Cmd::kCmdTaito01:
	case JVSIO::Cmd::kCmdTaito03:
		Serial.println("kCmdTaito01/3");
		io.pushReport(JVSIO::kReportOk);
		io.pushReport(1);
		break;
	case JVSIO::Cmd::kCmdTaito04:
	case JVSIO::Cmd::kCmdTaito05:
	case JVSIO::Cmd::kCmdHandlePayout:
		Serial.println("kCmdTaito04/5");
		io.pushReport(JVSIO::kReportOk);
		break;
		
	}
}