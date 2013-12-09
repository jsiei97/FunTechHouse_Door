/**
 * @file FT_EDS_Door.cpp
 * @author Johan Simonsson
 * @brief FT_EDS Storage class with Fun Tech Door extensions
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

#include <inttypes.h>
#include <EEPROM.h>

#include "FT_EDS.h"
#include "FT_EDS_Door.h"

//#include <QDebug>

bool FT_EDS_Door::appendDE(edsId id, edsType type, uint8_t* data, uint16_t len)
{
    unsigned int pos = getPos(id);
    if(0==pos)
        return false;

	uint32_t offsetOld = read32(pos+6);
	if(0==offsetOld)
	{
		return updateDE(id, type, data, len);
	}

	if(type != EDS_BYTE_ARRAY || read16(pos+2) != EDS_BYTE_ARRAY)
	{
		return false;
	}

	if(checkKey(id, data, len))
	{
		//Only add data once!
		return true;
	}

	uint16_t lenOld = read16(pos+4);
	uint32_t point  = read32(pos+6);

	if(point != posFreeData)
	{
		//Only allow the last data to grow,
		//the others has data before or after!
		return false;
	}

	lenOld += len;
    write16(pos+4, (uint16_t)lenOld);  //deLen

	point -= len;
	write32(pos+6, point); //deData

	for( unsigned int i=0 ; i<len ; i++ )
	{
		EEPROM.write(point+i, data[i]);
	}
	
	posFreeData = point;
    //write16(pos+2, (uint16_t)type); //deType
	
	return true;
}

bool FT_EDS_Door::removeDE(edsId id, edsType type, uint8_t* data, uint16_t len)
{
    unsigned int pos = getPos(id);
    if(0==pos)
        return false;

	uint32_t offsetOld = read32(pos+6);
	if(0==offsetOld)
	{
		//Remove from nothing? ok I guess..?
		return true;
	}

	if(type != EDS_BYTE_ARRAY || read16(pos+2) != EDS_BYTE_ARRAY)
	{
		return false;
	}

	uint32_t rmKeyPos = checkKeyPos(id, data, len);
	if(0 == rmKeyPos)
	{
		//It's not there! so I guess its ok!
		return true;
	}

	uint16_t size  = read16(pos+4);
	uint32_t point = read32(pos+6);

	//The top byte in the last ok chunk
	rmKeyPos--;
	if(rmKeyPos <= point)
	{
		//this is just wrong!
		return false;
	}

	while(rmKeyPos>=point)
	{
		EEPROM.write(rmKeyPos+8, EEPROM.read(rmKeyPos));
		rmKeyPos--;
	}
	
	point+=8;
	write32(pos+6, point);
	posFreeData = point;

	size-=8;
	write16(pos+4, size);
	
	return true;
}

bool FT_EDS_Door::checkKey(edsId id, uint8_t* data, uint16_t len)
{
	if(checkKeyPos(id, data, len) != 0)
		return true;
	
	return false;
}

/**
 * @return 0 is error! all other is ok!
 */
uint32_t FT_EDS_Door::checkKeyPos(edsId id, uint8_t* data, uint16_t len)
{
	//qDebug() << __func__ << __LINE__;

    unsigned int pos = getPos(id);
    if(0==pos)
        return 0;


	uint16_t size  = read16(pos+4);
	uint32_t point = read32(pos+6);
	uint32_t end = point+size;

	bool found = false;

	for( ;point<end ;point+=8 )
	{
		found = true;
		int i;
		for( i=0; i<8 && found; i++ )
		{
			//qDebug() << EEPROM.read(point+i);
			if( EEPROM.read(point+i) != data[i] )
			{
				found = false;
			}
		}

		if(found)
			return point;
	}
	
	return 0;
}

uint16_t FT_EDS_Door::getSize(edsId id)
{
    unsigned int pos = getPos(id);
    if(0==pos)
        return 0;

	return read16(pos+4);
}

