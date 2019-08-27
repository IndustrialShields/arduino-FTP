#ifndef __FTP_H__
#define __FTP_H__

#include <Arduino.h>
#include <Client.h>

class FTP {
	public:
		explicit FTP(Client &cClient, Client &dClient);

	public:
		int connect(IPAddress ip, uint16_t port, const char *user, const char *password = nullptr);
		inline int connect(IPAddress ip, const char *user, const char *password = nullptr) {
			return connect(ip, 21, user, password);
		}
		size_t retrieve(const char *fileName, void *buff, size_t size);
		size_t store(const char *fileName, const void *buff, size_t size);

	private:
		int beginTransaction();
		int waitServerCode(const __FlashStringHelper *cmd, const char *arg, uint16_t response);
		uint16_t waitServerCode(const __FlashStringHelper *cmd, const char *arg = nullptr, char *desc = nullptr);
		uint16_t waitServerCode(char *desc = nullptr);

	private:
		Client &cClient;
		Client &dClient;
};

#endif // __FTP_H__
