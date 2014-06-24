/*
 * MainMenu.h
 *
 * Created: 4/30/2014 9:49:17 AM
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
#include "MessageHandler.h"

// Main "menu" handler for the focus stacking app.
// This just allows the user to navigate between
// setting up the focus stack parameters and kicking
// off a focus stack operation.
class MainMenuHandler : public IMessageHandler
{
public:
    MainMenuHandler(MessagePump *_pump);
    ~MainMenuHandler();
    MsgResp processMessage(Msg& msg);
    virtual void show();
};
