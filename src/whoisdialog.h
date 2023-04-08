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

#ifndef WHOISDIALOG_H
#define WHOISDIALOG_H

#include <wx/dialog.h>

class wxFileConfig;
class wxSocketClient;
class wxBoxSizer;
class wxFlexGridSizer;
class wxStaticText;
class wxTextCtrl;
class wxChoice;
class wxButton;

class WhoisDialog: public wxDialog {
	public:
		WhoisDialog(wxWindow* parent, wxFileConfig *langIni, wxArrayString WhoisData, bool IsYourself, wxSocketClient* socket, wxFont font);

		void OnChoice(wxCommandEvent& event);
		void OnApply(wxCommandEvent& event);
		void OnCancel(wxCommandEvent& event);
	private:
		WhoisDialog(const WhoisDialog& cc);
		WhoisDialog& operator=(const WhoisDialog& cc);

		wxBoxSizer *MainSizer, *BoxSizer, *ButtonSizer;
		wxFlexGridSizer *GridSizer, *GridSizer2, *GridSizer3;
		wxStaticText *lplayername, *lplayerstatus, *lemail, *licq, *lplace, *lage, *lgender, *ldescription,
		*lgamename, *llastplayed, *lstatistics, *lvictories, *ldefeats, *ldisconnects, *lposition, *lrank;
		wxTextCtrl *playername, *playerstatus, *email, *icq, *place, *age, *description, *lastplayed, *victories,
		*defeats, *disconnects, *position, *rank;
		wxChoice *gender, *gamename, *statistics;
		wxButton *applybutton, *cancelbutton;
		wxArrayString WhoisData;
		wxSocketClient *socket;
	DECLARE_EVENT_TABLE()
};

#endif
