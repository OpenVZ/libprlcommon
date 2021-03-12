Virtuozzo Common Libraries
=============

Virtuozzo Common Libraries is a set of libraries used in other Virtuozzo
components providing common functionality for them:

- CAuth - class for checking user permissions on host;
- HostUtils - a set of common functions for host information gathering/parsing;
- IOService - common transport classes;
- Logging - logging library;
- OpenSSL - a useful wrapper over openssl library;
- PrlCommonUtilsBase - a set of common functions;
- PrlDataSerializer - data serializer/deserializer;
- PrlUuid - a wrapper for Virtuozzo uuids generation;
- Std - set of simple common headers.

How to install
--------------

Project depends on:

- [libprlsdk](https://src.openvz.org/scm/ovz/libprlsdk.git) headers.
  One needs to install them or checkout and define a proper `SDK_HEADERS` variable
  (`export SDK_HEADERS=~/libprlsdk/Sources`).
- qt-devel
- boost-devel

To build the libraries run:

```bash
qmake-qt4
make
sudo make install
```
