/*
   Copyright (c) 2019 Boot&Work Corp., S.L. All rights reserved

   This library is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "FTP.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
FTP::FTP(Client &cClient, Client &dClient) : cClient(cClient), dClient(dClient) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int FTP::connect(IPAddress ip, uint16_t port, const char *user, const char *password) {
	if (!user) {
		return 0;
	}

	cClient.connect(ip, port);
	if (!cClient.connected()) {
		return 0;
	}

	if (waitServerCode() != 220) {
		cClient.stop();
		return 0;
	}

	switch (waitServerCode(F("USER"), user)) {
		case 331:
			if (!password) {
				cClient.stop();
				return 0;
			}

			if (waitServerCode(F("PASS"), password) != 230) {
				cClient.stop();
				return 0;
			}
			break;

		case 230:
			// Logged in
			break;

		default:
			cClient.stop();
			return 0;
	}

	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int FTP::beginTransaction() {
	if (!cClient.connected()) {
		return 0;
	}

	if (!waitServerCode(F("TYPE I"), nullptr, 200)) {
		return 0;
	}

	cClient.println(F("PASV"));
#if DEBUG
	Serial.println(F("PASV"));
#endif

	char pasvResponse[70];
	if (waitServerCode(pasvResponse) != 227) {
		return 0;
	}

	uint8_t params[6];
	char *ptr = strtok(pasvResponse, "(");
	for (int i = 0; i < 6; ++i) {
		ptr = strtok(nullptr, ",");
		if (!ptr) {
			return 0;
		}
		params[i] = atoi(ptr);
	}

	IPAddress dataAddress(params[0], params[1], params[2], params[3]);
	uint16_t dataPort = (uint16_t(params[4]) << 8) | (params[5] & 0xff);

	dClient.connect(dataAddress, dataPort);
	if (!dClient.connected()) {
		return 0;
	}

	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
size_t FTP::retrieve(const char *fileName, void *buff, size_t size) {
	if ((buff == nullptr) || (size == 0)) {
		return 0;
	}

	if (!beginTransaction()) {
		return 0;
	}

	if (waitServerCode(F("RETR"), fileName) != 150) {
		return 0;
	}

	uint32_t startTime = millis();
	uint8_t *ptr = (uint8_t*) buff;
	while (dClient.connected() && (millis() - startTime < 15000) && (size > 0)) {
		size_t len = dClient.available();
		if (len > size) {
			len = size;
		}
		if (len > 0) {
			startTime = millis();
			dClient.read(ptr, len);
			ptr += len;
			size -= len;
		} else {
			delay(1);
		}
	}

	dClient.stop();

	if (waitServerCode() != 226) {
		return 0;
	}

	return ptr - (uint8_t *) buff;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
size_t FTP::store(const char *fileName, const void *buff, size_t size) {
	if ((buff == nullptr) || (size == 0)) {
		return 0;
	}

	if (!beginTransaction()) {
		return 0;
	}

	if (waitServerCode(F("STOR"), fileName) != 150) {
		return 0;
	}

	size_t ret = dClient.write((uint8_t*) buff, size);

	dClient.stop();

	if (waitServerCode() != 226) {
		return 0;
	}

	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int FTP::waitServerCode(const __FlashStringHelper *cmd, const char *arg, uint16_t response) {
	return waitServerCode(cmd, arg, nullptr) == response;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t FTP::waitServerCode(const __FlashStringHelper *cmd, const char *arg, char *desc) {
	cClient.print(cmd);
#if DEBUG
	Serial.print("> ");
	Serial.print(cmd);
#endif

	if (arg) {
		cClient.print(' ');
		cClient.print(arg);
#if DEBUG
		Serial.print(' ');
		Serial.print(arg);
#endif
	}

	cClient.println();
#if DEBUG
	Serial.println();
#endif

	return waitServerCode(desc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t FTP::waitServerCode(char *desc) {
	uint32_t startTime = millis();
	size_t len;
	do {
		len = cClient.available();
		delay(1);
	} while ((len < 4) && (millis() - startTime <= 15000UL));

#if DEBUG
	Serial.print("< ");
#endif

	uint16_t code = 0;
	bool ready = false;
	char c = '\0';
	do {
		len = cClient.available();
		while (len > 0) {
			c = cClient.read();
#if DEBUG
			if (c == '\n') {
				Serial.print("<NL>");
				Serial.println();
			} else if (c == '\r') {
				Serial.print("<CR>");
			} else {
				Serial.write(c);
			}
#endif
			if (c == '\r') {
				// Nothing to do
			} else if (c == '\n') {
				break;
			} else if (!ready) {
				if (c == ' ') {
					ready = true;
				} else {
					code *= 10;
					code += c - '0';
				}
			} else if (desc) {
				*desc++ = c;
			}
			--len;
		}
	} while ((millis() - startTime <= 15000UL) && (c != '\n'));

	if (desc) {
		*desc = '\0';
	}

	return code;
}
