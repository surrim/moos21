/* Copyright 2011-2023 surrim
 *
 * This file is part of moos.
 *
 * moos is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * moos is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with moos.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ENUMS_H
#define ENUMS_H

enum {
	ID_MAINWIN_LOGIN_AS=wxID_HIGHEST+1,
	ID_MAINWIN_LOGOUT,
	ID_MAINWIN_EXIT,

	ID_MAINWIN_WHOIS_USER,
	ID_MAINWIN_SLAP_USER,
	ID_MAINWIN_IGNORE_USER,
	ID_MAINWIN_IGNORELIST,

	ID_MAINWIN_SAVE_CHAT,
	ID_MAINWIN_FREEZE_CHAT,
	ID_MAINWIN_DELETE_CHAT,
	ID_MAINWIN_COLORSELECT,
	ID_MAINWIN_MANUAL_COLORCODE,

	ID_MAINWIN_AUTOLOGIN_ACCOUNT,
	ID_MAINWIN_SOUND_ON_BEEP,
	ID_MAINWIN_DISABLE_SLAPS,
	ID_MAINWIN_CHANGE_FONT,
	ID_MAINWIN_SELECT_LANGUAGE,

	ID_MAINWIN_ONLINEREADME,
	ID_MAINWIN_MAILBUGS,
	ID_MAINWIN_ABOUT,

	ID_MAINWIN_USERLIST,
	ID_MAINWIN_IGNOREDUSERLIST,
	ID_MAINWIN_CHANNELSWITCHER,
	ID_MAINWIN_CHATVIEW,
	ID_MAINWIN_CHATINPUT,
	ID_MAINWIN_WHISPERCHECKBOX,
	ID_SOCKET
};

/*
todo:
versionen 1.3/2.2
unselect button
translateAccountBanned
*/

#endif
