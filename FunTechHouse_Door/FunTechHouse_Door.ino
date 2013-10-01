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

#include "OneWire.h"

OneWire  ds(A0);
byte addr[8];

char str[18];
bool blink;

#define LIST_SIZE 2
//Test list with some ok iButtons
int keylist[LIST_SIZE][8] =
{
    { 0x01, 0x5D, 0x79, 0xA7, 0x09, 0x00, 0x00, 0x66},
    { 0x01, 0x12, 0xE8, 0xA5, 0x09, 0x00, 0x00, 0x4A}
};

bool checkKey()
{
    for( int i=0 ; i<LIST_SIZE ; i++ )
    {
        bool hit = true;
        for( int j=0 ; j<=7 ; j++ )
        {
            Serial.print(keylist[i][j], HEX);
            Serial.print(" - ");
            Serial.println(addr[j], HEX);

            if(keylist[i][j] != addr[j])
            {
                hit = false;
                break;
            }
        }

        if(hit)
        {
            Serial.println("OK");
            return true;
        }

    }
    return false;
}

void setup(void)
{
    Serial.begin(115200);
    pinMode(3, OUTPUT);
}

void loop(void)
{
    if(getKeyCode())
    {
        snprintf(str, 17, "%02X%02X%02X%02X%02X%02X%02X%02X",
                addr[7], addr[6], addr[5], addr[4],
                addr[3], addr[2], addr[1], addr[0]);
        Serial.println(str);
        snprintf(str, 17, "%02X%02X%02X%02X%02X%02X%02X%02X",
                addr[0], addr[1], addr[2], addr[3],
                addr[4], addr[5], addr[6], addr[7]);
        Serial.println(str);

        if(checkKey())
        {
            //This is a valid key, open the door...
            digitalWrite(3, HIGH);
            delay(5000);
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

    if(false == blink)
    {
        digitalWrite(3, HIGH);
        blink = true;
    }
    else
    {
        digitalWrite(3, LOW);
        blink = false;
    }
    delay(500);
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
    return true;
}
