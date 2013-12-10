/**
 * @file FT_EDS_Door.h
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

#ifndef  __FT_EDS_DOOR_H
#define  __FT_EDS_DOOR_H

#include <inttypes.h>
#include "FT_EDS.h"

class FT_EDS_Door : public FT_EDS
{
	private:
	public:
        bool appendDE (edsId id, edsType type, uint8_t* data, uint16_t len);
		bool appendStr(edsId id, edsType type, uint8_t* data, uint16_t len);
        bool removeDE (edsId id, edsType type, uint8_t* data, uint16_t len);
		bool removeStr(edsId id, edsType type, uint8_t* data, uint16_t len);

		bool     checkKey   (edsId id, uint8_t* data, uint16_t len);
		uint32_t checkKeyPos(edsId id, uint8_t* data, uint16_t len);

		uint16_t getSize(edsId id);

		unsigned int getParts(edsId id);
		bool readPart(edsId id, edsType type, unsigned int part, uint8_t* data, uint16_t len);
};

#endif  // __FT_EDS_DOOR_H 
