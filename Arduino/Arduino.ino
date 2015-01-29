
/*
 *	Engineering 122 Freshman Design Project Client
 *	Copyright (C) 2014	Gustave Abel Michel III
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// Libraries
//SPI
	#include "SPI.h"
//I2C
	#include <Wire.h>
//Ethernet
	#include "Ethernet.h"
//Servo
	#include "Servo.h"
// GY-217 Compass
	#include "HMC5883L.h"

// Contant Declarations
#define key "password1"

//char server[] = "wifi.gustavemichel.com"; // Port 80
//char server[] = "gustave.me"; // Port 4500
IPAddress server(10,10,10,1); // Port 4500

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //DEADBEEF because WHY NOT!
IPAddress ip(10,10,10,2); //default

EthernetClient client;
HMC5883L compass;

unsigned long lastConnectionTime = 0;			// last time you connected to the server, in milliseconds
boolean lastConnected = false;					// state of the connection last time through the main loop
const unsigned long postingInterval = 6000;		// delay between updates, in milliseconds

// Variable/Object Declarations



void setup() {
	// start serial port:
	Serial.begin(9600);

	//Setup Compass
	Wire.begin();
	compass = HMC5883L(); 
	compass.SetScale(1.3); 
	compass.SetMeasurementMode(Measurement_Continuous);

	// give the ethernet module time to boot up:
	delay(1000);
	// start the Ethernet connection using a fixed IP address:
	//if (Ethernet.begin(mac) == 0) { //attempt DHCP
		Serial.println("Failed to configure Ethernet using DHCP");
		Ethernet.begin(mac, ip);
	//}
	// print the Ethernet board/shield's IP address:
	Serial.print("My IP address: ");
	Serial.println(Ethernet.localIP());
}

void loop() {
	while (client.available()) { //We have Stuff from Server
		char c = client.read();
		Serial.print(c);
	}

	if(!client.connected()) { //If no Stuff, are we connected?
		if(lastConnected) { // We were connected, no more.
			Serial.println();
			Serial.println("disconnecting.");
			client.stop();
		}
		if(millis() - lastConnectionTime > postingInterval) { //Is it time to reconnect?
			httpRequest();
		}
	}
	lastConnected = client.connected(); //store state for next iteration

	MagnetometerRaw raw = compass.ReadRawAxis(); //Read Data
	MagnetometerScaled scaled = compass.ReadScaledAxis(); 
	float xHeading = atan2(scaled.YAxis, scaled.XAxis); //Calculate Heading
	float yHeading = atan2(scaled.ZAxis, scaled.XAxis); 
	float zHeading = atan2(scaled.ZAxis, scaled.YAxis); 
	if(xHeading < 0) 	//Heading Calibration
		xHeading += 2*PI;
	if(xHeading > 2*PI) 
		xHeading -= 2*PI; 
	if(yHeading < 0) 
		yHeading += 2*PI; 
	if(yHeading > 2*PI) 
		yHeading -= 2*PI; 
	if(zHeading < 0) 
		zHeading += 2*PI; 
	if(zHeading > 2*PI) 
		zHeading -= 2*PI; 
	float xDegrees = xHeading * 180/M_PI; //Convert to degrees
	float yDegrees = yHeading * 180/M_PI; 
	float zDegrees = zHeading * 180/M_PI; 
	Serial.print(xDegrees);  //print degrees
	//Serial.print(","); 
	//Serial.print(yDegrees); 
	//Serial.print(","); 
	//Serial.print(zDegrees); 
	Serial.println(";"); 
	delay(100);
}


// this function makes a HTTP connection to the server:
void httpRequest() {
	if (client.connect(server, 4500)) { // Successful connection?
		Serial.println("connecting...");
		// send the HTTP PUT request:
		client.println("GET /api/dish/password1");
		client.println("Host: 10.10.10.2");
		client.println("User-Agent: arduino-ethernet");
		client.println("Connection: close");
		client.println();

		// note the time that the connection was made:
		lastConnectionTime = millis();
	} 
	else { // No connection... :(
		Serial.println("connection failed");
		Serial.println("disconnecting.");
		client.stop();
	}
}