CONFIG += qtestlib debug
TEMPLATE = app
TARGET =
DEFINES += private=public
DEFINES += protected=public
DEFINES += EEPROM_MAX_SIZE=100

# Test code
DEPENDPATH += .
INCLUDEPATH += .
SOURCES += Test_FT_EDS_Door.cpp

# Code to test
DEPENDPATH += ../../FunTechHouse_Door/
INCLUDEPATH += ../../FunTechHouse_Door/
SOURCES += FT_EDS.cpp FT_EDS_Door.cpp HexDump.cpp IntegerExtra.cpp

# Stubs
DEPENDPATH += stub/
INCLUDEPATH += stub/
SOURCES += EEPROM.cpp

