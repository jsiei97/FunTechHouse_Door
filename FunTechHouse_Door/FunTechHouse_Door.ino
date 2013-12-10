/**
 * @file FunTechHouse_Door.ino
 * @author Johan Simonsson
 * @brief A iButton door function
 *
 * This is a modified version based on the idea on this page:
 * http://tushev.org/articles/arduino/item/50-reading-ibutton-with-arduino
 */

/*
 * Copyright (C) 2013 Johan Simonsson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <EEPROM.h>
#include "FT_EDS.h"
#include "FT_EDS_Door.h"
#include "OneWire.h"

FT_EDS_Door eds;

OneWire  ds(A0);
byte addr[8];

char str[18];
int blink = 0;


void setup(void)
{
    Serial.begin(9600);
    pinMode(3, OUTPUT); //Indicatior LED
    pinMode(4, OUTPUT); //Door lock relay

    eds.init();

    //Check what keys is in the EEPROM!
    uint8_t key[8];
    unsigned int parts = eds.getParts(EDS_ONEWIRE_LIST);
    for( int i=0 ; i<parts ; i++ )
    {
        eds.readPart(EDS_ONEWIRE_LIST, EDS_BYTE_ARRAY, i, key, 8);
        snprintf(str, 17, "%02X%02X%02X%02X%02X%02X%02X%02X",
                key[0], key[1], key[2], key[3],
                key[4], key[5], key[6], key[7]);
        Serial.print(" - ");
        Serial.println(str);
    }
}

void loop(void)
{
    if(getKeyCode())
    {
        snprintf(str, 17, "%02X%02X%02X%02X%02X%02X%02X%02X",
                addr[0], addr[1], addr[2], addr[3],
                addr[4], addr[5], addr[6], addr[7]);
        Serial.println(str);

        if(eds.checkKey(EDS_ONEWIRE_LIST, addr, 8))
        {
            //This is a valid key, open the door...
            digitalWrite(3, HIGH);
            digitalWrite(4, HIGH);
            delay(5000);
            digitalWrite(4, LOW);
        }
        else
        {
            //This is a NOT valid key...
            digitalWrite(3, LOW);
            delay(3000);
        }
    }
    //else
    //{
    //    Serial.println("no 1w...");
    //}

    blink++;
    if(20 == blink)
    {
        digitalWrite(3, HIGH);
    }
    else if(40 == blink)
    {
        digitalWrite(3, LOW);
        blink = 0;
    }
    delay(50);
}


bool getKeyCode()
{
    byte present = 0;
    byte data[12];
    bool res = false;

    if ( !ds.search(addr) )
    {
        ds.reset_search();
        return false;
    }

    if ( OneWire::crc8( addr, 7) != addr[7] )
    {
        Serial.println("CRC invalid");
        return false;
    }

    if ( addr[0] != 0x01 )
    {
        Serial.println("not DS1990A");
        return false;
    }

    //Serial.println("ok");
    ds.reset();

    //Since I like the address the other way, let's fix it
    int i,j;
    byte tmp;
    for( i=0,j=7; i<4 ;i++,j-- )
    {
        tmp = addr[i];
        addr[i]=addr[j];
        addr[j]=tmp;
    }

    return true;
}
