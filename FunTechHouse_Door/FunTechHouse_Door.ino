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
#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>
#include <avr/wdt.h>

#include "FT_EDS.h"
#include "FT_EDS_Door.h"
#include "OneWire.h"
#include "PubSubClient.h"

#include "QuickDate.h"
#include "DateTime.h"
#include "RTC_DS1307.h"

FT_EDS_Door eds;

OneWire  ds(A0);
byte addr[8];

char str[50];
uint8_t blink = 0;
uint8_t blinkstate = 0;

DateTime now;
DateTime last;
RTC_DS1307 rtc;
QuickDate qd;

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0x10 };

// The MQTT device name, this must be unique
char project_name[]  = "FunTechHouse_Door";


EthernetClient ethClient;
PubSubClient client("mosqhub", 1883, callback, ethClient);

void callback(char* topic, byte* payload, unsigned int length)
{
    // handle message arrived
    //client.publish(topic_out, "echo...");
}

void setup(void)
{
    wdt_enable(WDTO_8S);

    Serial.begin(9600);
    pinMode(2, OUTPUT); //Indicatior LED (on the Door)
    pinMode(4, OUTPUT); //Door lock relay

    pinMode(3, OUTPUT); //Status LED 1
    pinMode(5, OUTPUT); //Status LED 2
    pinMode(6, OUTPUT); //Status LED 3

    eds.init();

    Wire.begin();

    //Feed the dog.
    wdt_reset();

    //Get MAC from EDS and start Ethernet
    eds.readDE(EDS_ETH_MAC, EDS_BYTE_ARRAY, mac, 6);
    //Shall I ignore dhcp for stability?
    //eds.readDE(EDS_ETH_IP,  EDS_BYTE_ARRAY, ip, 6);
    Ethernet.begin(mac);

    if (client.connect(project_name))
    {
        //Check what keys is in the EEPROM!
        uint8_t key[8];
        unsigned int parts = eds.getParts(EDS_ONEWIRE_LIST);
        for( int i=0 ; i<parts ; i++ )
        {
            eds.readPart(EDS_ONEWIRE_LIST, EDS_BYTE_ARRAY, i, key, 8);
            snprintf(str, 17, "%d/%d %02X%02X%02X%02X%02X%02X%02X%02X",
                    (i+1), parts,
                    key[0], key[1], key[2], key[3],
                    key[4], key[5], key[6], key[7]);
            Serial.print(" - ");
            Serial.println(str);
            client.publish( "FunTechHouse/Door1", str );
        }
    }
}

void loop(void)
{
    //Feed the dog.
    wdt_reset();

    //Talk with the server so he dont forget us.
    if(client.loop() == false)
    {
        client.connect(project_name);
    }

    if(false == client.connected())
    {
        client.connect(project_name);
    }

    if(getKeyCode())
    {
        if(eds.checkKey(EDS_ONEWIRE_LIST, addr, 8))
        {
            //This is a valid key, open the door...
            //Keep all led:s high
            digitalWrite(2, HIGH);
            digitalWrite(3, HIGH);
            digitalWrite(4, HIGH);

            unsigned int time = millis();
            //Here we have 5s to talk tell the server that we had a OK key
            rtc.getTime(&now, &last);

            snprintf(str, 49, "20%02x-%02x-%02x %02x:%02x:%02x OPEN %02X%02X%02X%02X%02X%02X%02X%02X",
                    now.year, now.month, now.day,
                    now.hour, now.min, now.sec,
                    addr[0], addr[1], addr[2], addr[3],
                    addr[4], addr[5], addr[6], addr[7]);
            Serial.println(str);

            /// @todo What to do if we fail?
            client.publish( "FunTechHouse/Door1", str );

            time = millis()-time;

            Serial.print("Worktime: ");
            Serial.println(time);
            if(time < 5000)
            {
                delay(5000-time);
            }

            digitalWrite(2, LOW);
            digitalWrite(4, LOW);
        }
        else
        {
            //This is a NOT valid key...
            //Keep the status led off
            digitalWrite(3, LOW);

            unsigned int time = millis();
            //Here we have 3s to talk tell the server that we had a non valid key
            rtc.getTime(&now, &last);

            snprintf(str, 49, "20%02x-%02x-%02x %02x:%02x:%02x STOP %02X%02X%02X%02X%02X%02X%02X%02X",
                    now.year, now.month, now.day,
                    now.hour, now.min, now.sec,
                    addr[0], addr[1], addr[2], addr[3],
                    addr[4], addr[5], addr[6], addr[7]);
            Serial.println(str);

            /// @todo What to do if we fail?
            client.publish( "FunTechHouse/Door1", str );

            time = millis()-time;
            Serial.print("Worktime: ");
            Serial.println(time);
            if(time < 3000)
            {
                delay(3000-time);
            }
        }
    }
    //else
    //{
    //    Serial.println("no 1w...");
    //}


    if(0==blink)
    {
        //Serial.println("Time to check time...");
        //Check time

        rtc.getTime(&now, &last);
        if(!rtc.isrunning() || (now.secSince2000()-last.secSince2000()>(1*60*60)) )
        {
            Serial.println("Time to sync time...");
            int qdStatus = qd.doTimeSync(str);
            if(qdStatus > 0)
            {
                Serial.print("ok: ");
                Serial.print(str);
                Serial.print(" : ");
                now.setTime(str);
                rtc.adjust(&now);
            }
            else
            {
                Serial.print("fail: ");
            }

            Serial.println(qdStatus);
        }
    }

    if(blink%20==0)
    {
        if(0 != blinkstate)
        {
            digitalWrite(3, LOW);
            blinkstate = 0;
        }
        else
        {
            digitalWrite(3, HIGH);
            blinkstate++;
        }
    }

    //0xFF*50ms=12,75s per loop (since blink is uint8_t!)
    blink++;
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
