# Arduino FTP library

You can use any Client to send and receive files from an FTP server. First of all you have to include the library and create an FTP object:
```c++
#include <FTP.h>
FTP ftp(ftpControl, ftpData);
```

Both `ftpControl` and `ftpData` are `Client` objects, like `EthernetClient` or other:
```c++
EthernetClient ftpControl;
EthernetClient ftpData;
```

To connect to the FTP server you only have to set the server IP address, your user and your password. Optionally you can set the server TCP port, but the default one is 21:
```c++
ftp.connect(serverIp, user, password);
```
or in case of another TCP port (i.e. 6758):
```c++
ftp.connect(serverIp, 6758, user, password);
```

When the connection is ready, it is possible to download files content from the FTP server:
```c++
char fileContent[512];
ftp.retrieve(fileName, fileContent, 512);
```
or to upload files to the FTP server:
```c++
const char *fileContent = "This is the new file content";
ftp.store(fileName, fileContent, strlen(fileContent));
```

You can see complete examples in the [examples directory](/examples).
