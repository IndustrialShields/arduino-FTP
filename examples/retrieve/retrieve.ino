// FTP library example
// by Industrial Shields

#include <FTP.h>

#if defined(MDUINO_PLUS)
#include <Ethernet2.h>
#else
#include <Ethernet.h>
#endif

uint8_t mac[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };
IPAddress ip(10, 10, 10, 6);
IPAddress namesServer(8, 8, 8, 8);
IPAddress gateway(10, 10, 10, 1);
IPAddress netmask(255, 255, 255, 0);

IPAddress server(192, 168, 1, 220);
const char *user = "YOUR-USER";
const char *pass = "YOUR-PASSWORD";
const char *fileName = "YOUR-FILE";

EthernetClient ftpControl;
EthernetClient ftpData;

FTP ftp(ftpControl, ftpData);
char data[512];

////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600UL);

  Ethernet.begin(mac, ip, namesServer, gateway, netmask);
  Serial.print("IP address: ");
  Serial.println(Ethernet.localIP());

  if (!ftp.connect(server, user, pass)) {
    Serial.println("Error connecting to FTP server");
    while (true);
  }

  size_t len = ftp.retrieve(fileName, data, 512);
  data[len] = '\0';

  Serial.print("File content: ");
  Serial.println(data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  // Nothing to do
}
