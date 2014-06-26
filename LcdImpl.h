/*
 * LcdImpl.h
 *
 * Created: 5/8/2014 1:33:18 PM
 *  Author: Ketil Wright
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

#pragma once

// #include for whatever LCD hardware implementation is used.
// Currently using i2c, 16x2 adafruit lcd shield (which also has some buttons)
#include <Wire.h>
#include "Adafruit_MCP23017.h"
#include "Adafruit_RGBLCDShield.h"
class LCDImpl : public Adafruit_RGBLCDShield
{
	uint8_t m_col, m_row;
	uint8_t m_savedCol, m_savedRow;
public:
	LCDImpl() 
		: 
		Adafruit_RGBLCDShield(),
		m_col(0),
		m_row(0),
		m_savedCol(0),
		m_savedRow(0)
	{}		
	
	void setCursor(uint8_t col, uint8_t row)
	{
		m_col = col;
		m_row = row;
		Adafruit_RGBLCDShield::setCursor(m_col, m_row);
	}
	uint8_t getCaretCol() const { return m_col; }
	uint8_t getCaretRow() const { return m_row; }
	void saveCursorLocation()
	{
		m_savedCol = m_col;
		m_savedRow = m_row;
	}		
	void restoreCursorLocation()
	{
		setCursor(m_savedCol, m_savedRow);
	}
};
extern LCDImpl *g_print;
