Virtuozzo Common Libraries
=============

Virtuozzo Common Libraries is a set of libraries used in other Virtuozzo
components providing common functionality for them:

- HostUtils - a set of common functions for host information gathering/parsing;
- IOService - common transport classes;
- Logging - logging library;
- OpenSSL - a useful wrapper over openssl library;
- PrlCommonUtilsBase - a set of common functions;
- PrlDataSerializer - data serializer/deserializer;
- PrlUuid - a wrapper for Virtuozzo uuids generation;
- Std - set of simple common headers.
- TestsUtils - Common utils used at all dispatcher API tests suites classes.

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

How to run tests
----------------

Before running tests, create users `prl_unit_test_user` and `prl_unit_test_user2`.
Both users shall have password `test`.

```bash
useradd prl_unit_test_user
useradd prl_unit_test_user2
echo test | passwd prl_unit_test_user --stdin
echo test | passwd prl_unit_test_user2 --stdin
```

To run tests:

```bash
cd Tests
qmake-qt4
make check
```
