/*
 * MessageHandler.cpp
 *
 * Created: 5/2/2014 11:40:41 AM
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

#include "MessagePump.h"
#include "MessageHandler.h"
#include "LcdImpl.h"

// Base class ctor just initializes OK on bottom line of menu[].
IMessageHandler::IMessageHandler(MessagePump* _pump)
    :
    m_pump(_pump),
    m_caretCol(0),
	m_caretRow(0)
{
    //for(unsigned int line = 0; line < sizeof(menu) / sizeof(menu[0]); line++) menu[line] = NULL;
	memset(menu, 0, sizeof(menu)); // 33076
}

void IMessageHandler::printMenuItem(uint8_t col, uint8_t item)
{
	g_print->setCursor(col, 0);
	g_print->print(menu[item]);
}
// Draws the menu text, indenting all lines but
// the 1st by one column
void IMessageHandler::show()
{
    g_print->clear();
	
	if(menu[0])
	{
		g_print->setCursor(0, 0);
		g_print->print(menu[0]);
	}
	if(menu[1])
	{
		g_print->setCursor(8, 0);
		g_print->print(menu[1]);
	}
	if(menu[2])
	{
		g_print->setCursor(0, 1);
		g_print->print(menu[2]);
	}
	if(menu[3])
	{
		g_print->setCursor(8, 1);
		g_print->print(menu[2]);
	}
	m_caretCol = 0;
    
}

// Draws or hides the caret.

void IMessageHandler::showCaret(bool show)
{
	g_print->setCursor(m_caretCol, m_caretRow);
	if(show) g_print->cursor();
	else g_print->noCursor();
}

// Redraws the caret at the specified location
void IMessageHandler::moveCaret(uint8_t col, uint8_t row)
{
	showCaret(false);
	m_caretCol  = col;	
	m_caretRow = row;
	showCaret(true);
}

