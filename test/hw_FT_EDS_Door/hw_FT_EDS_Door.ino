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
#include "CmdParse.h"
#include "IntegerExtra.h"

FT_EDS_Door eds;
CmdParse cmdp;

int currentChar = 0;

#define BUFFER_SIZE 128
uint8_t buffer[BUFFER_SIZE];
int bufferPos;

char printStr[18];

void info(char* str)
{
    //Print all info on the EEPROM.
    Serial.println( "Info function" );
    //Serial.println( str );

    Serial.print  ( "Free: " );
    Serial.println( eds.getFree() );

    Serial.print  ( "DEC: " );
    Serial.println( eds.getDEC() );

    edsId id;
    edsType type;
    unsigned int size;
    for( unsigned int de=0 ; de<eds.getDEC() ; de++ )
    {
        if(eds.getDEInfo(de, &id, &type, &size))
        {
            Serial.print  ( "DE: " );
            Serial.print  ( de, DEC );
            Serial.print  ( ", id=" );
            Serial.print  ( id, HEX );
            Serial.print  ( ", type=" );
            Serial.print  ( type, HEX );
            Serial.print  ( ", size=" );
            Serial.println( size, DEC );

            switch ( id )
            {
                case EDS_ONEWIRE_LIST:
                    {
                        uint8_t key[8];
                        unsigned int parts = eds.getParts(EDS_ONEWIRE_LIST);
                        for( int i=0 ; i<parts ; i++ )
                        {
                            eds.readPart(EDS_ONEWIRE_LIST, EDS_BYTE_ARRAY, i, key, 8);
                            snprintf(printStr, 17, "%02X%02X%02X%02X%02X%02X%02X%02X",
                                    key[0], key[1], key[2], key[3],
                                    key[4], key[5], key[6], key[7]);
                            Serial.print(" - ");
                            Serial.println(printStr);
                        }
                    }
                    break;
            }
        }
    }
}

void format(char* str)
{
    Serial.println( "Format EEPROM" );
    //Serial.println( str );
    eds.format();
    Serial.println( "Done..." );
}

/**
 * put data into the EEPROM
 *
 * To save a ethernet mac:
 * put=0x1=BADEBADEBABE
 *
 * @param str command data
 */
void put(char* str)
{
    if(str == NULL)
        return;

    size_t len = strlen(str);
    if(len <= 2)
        return;

    char* data = &str[0];

    //Split string into 2 strings before and after the = sign
    size_t max = strlen(str);
    size_t pos = 0;
    for( pos=0; pos<max; pos++ )
    {
        if('=' == str[pos])
        {
            str[pos] = '\0';
            if(pos+1 < max)
            {
                data = &str[pos+1];
            }
        }
    }

    if(str == data)
    {
        //printf("Fail: no =\n");
        return;
    }

    edsId id = (edsId)IntegerExtra::hex2uint(str);
    Serial.print("Update DE: ");
    Serial.println( id, HEX );
    switch ( id )
    {
        case EDS_ETH_MAC:
            if( (6*2) == strlen(data) )
            {
                uint8_t mac[6];
                char str[5];
                str[0]='0';
                str[1]='x';
                str[4]='\0';
                int i=0;
                int pos=0;
                for( i=0 ; i<12 ; i+=2, pos++ )
                {
                    str[2] = data[i];
                    str[3] = data[i+1];
                    mac[pos] =  IntegerExtra::hex2uint((char*)str);

                    Serial.print( i );
                    Serial.print( ":" );
                    Serial.print( pos );
                    Serial.print( " - " );
                    Serial.print( str );
                    Serial.print( " - " );
                    Serial.print( mac[pos], HEX );
                    Serial.println( "" );
                }
                eds.updateDE(id, EDS_BYTE_ARRAY , mac, 6);

                for( pos=0 ; pos<6 ; pos++ )
                {
                    mac[pos]=0;
                }

                if(!eds.readDE(id, EDS_BYTE_ARRAY, mac, 6))
                {
                    Serial.println( "readDE failed..." );
                }

                Serial.print( "readDE: MAC: " );
                for( pos=0 ; pos<6 ; pos++ )
                {
                    Serial.print( mac[pos], HEX );
                }
                Serial.println( "" );


            }
            else
            {
                Serial.println( "" );
                Serial.println( "Error: Wrong MAC size..." );
            }
            break;
        case EDS_REGUL_P:
        case EDS_REGUL_I:
        case EDS_REGUL_D:
            {
                double valIn = atof(data);
                eds.updateDE(id, EDS_FIXED_32_8, valIn);
                double valOut;
                eds.readDE(id, &valOut);
                Serial.print(" = ");
                Serial.print( valOut );
                Serial.print(" / ");
                Serial.println( valIn );
            }
            break;

        default:
            Serial.print("Error id: ");
            Serial.println(id);
            return;
            break;
    }
}

void get(char* str)
{
    if(str == NULL)
        return;

    size_t len = strlen(str);
    if(len == 0)
        return;

    edsId id = (edsId)IntegerExtra::hex2uint(str);
    switch ( id )
    {
        case EDS_ETH_MAC:
            uint8_t mac[6];
            if(!eds.readDE(id, EDS_BYTE_ARRAY, mac, 6))
            {
                Serial.println( "readDE failed..." );
            }

            Serial.print( "readDE: MAC: " );
            for( int pos=0 ; pos<6 ; pos++ )
            {
                Serial.print( mac[pos], HEX );
            }
            Serial.println( "" );
            break;
        case EDS_REGUL_P:
        case EDS_REGUL_I:
        case EDS_REGUL_D:
            {
                double valOut;
                eds.readDE(id, &valOut);
                Serial.print("Read DE: ");
                Serial.print( id, HEX );
                Serial.print(" = ");
                Serial.println( valOut );
            }
            break;

        default:
            Serial.print("Error id: ");
            Serial.println(id);
            return;
            break;
    }
}

void append(char* str)
{
    if(str == NULL)
        return;

    size_t len = strlen(str);
    if(len <= 2)
        return;

    char* data = &str[0];

    //Split string into 2 strings before and after the = sign
    size_t max = strlen(str);
    size_t pos = 0;
    for( pos=0; pos<max; pos++ )
    {
        if('=' == str[pos])
        {
            str[pos] = '\0';
            if(pos+1 < max)
            {
                data = &str[pos+1];
            }
        }
    }

    if(str == data)
    {
        //printf("Fail: no =\n");
        return;
    }

    edsId id = (edsId)IntegerExtra::hex2uint(str);
    Serial.print("Append to DE: ");
    Serial.println( id, HEX );
    switch ( id )
    {
        case EDS_ONEWIRE_LIST:
            eds.appendStr(EDS_ONEWIRE_LIST, EDS_BYTE_ARRAY, (uint8_t*)data, strlen(data));
            break;

        default:
            Serial.print("Error id: ");
            Serial.println(id);
            return;
            break;
    }
}

void remove(char* str)
{
    if(str == NULL)
        return;

    size_t len = strlen(str);
    if(len <= 2)
        return;

    char* data = &str[0];

    //Split string into 2 strings before and after the = sign
    size_t max = strlen(str);
    size_t pos = 0;
    for( pos=0; pos<max; pos++ )
    {
        if('=' == str[pos])
        {
            str[pos] = '\0';
            if(pos+1 < max)
            {
                data = &str[pos+1];
            }
        }
    }

    if(str == data)
    {
        //printf("Fail: no =\n");
        return;
    }

    edsId id = (edsId)IntegerExtra::hex2uint(str);
    Serial.print("Remove from DE: ");
    Serial.println( id, HEX );
    switch ( id )
    {
        case EDS_ONEWIRE_LIST:
            eds.removeStr(EDS_ONEWIRE_LIST, EDS_BYTE_ARRAY, (uint8_t*)data, strlen(data));
            break;

        default:
            Serial.print("Error id: ");
            Serial.println(id);
            return;
            break;
    }
}

void setup()
{
    Serial.begin(9600);
    Serial.println();
    Serial.println("FT EDS test!");
    eds.init();

    cmdp.cmd_init();
    cmdp.cmd_add_cmd("info", info);
    cmdp.cmd_add_cmd("format", format);
    //cmdp.cmd_add_cmd("put", put);
    //cmdp.cmd_add_cmd("get", get);

    cmdp.cmd_add_cmd("add", append);
    cmdp.cmd_add_cmd("rm",  remove);

    bufferPos = 0;
}

void loop()
{
    if (Serial.available() > 0)
    {
        cmdp.cmd_add_char(Serial.read());

        //Syntax CMD=DATA\n

        //todo add some cmds
        //dump eeprom cmd
        //read data cmd
        //write data cmd (write payload)
    }
}
