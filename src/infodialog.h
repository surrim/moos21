/* Copyright 2011-2017 surrim
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

#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <wx/dialog.h>

class wxBoxSizer;
class wxStaticText;
class wxButton;
class wxString;
class wxFileConfig;

class InfoDialog: public wxDialog {
	public:
		InfoDialog(wxWindow* parent, wxFileConfig *langIni, const wxFont& font, const wxString& title, const wxString& text, int style=0, int defaultButton=0);
		void OnButton1(wxCommandEvent& event);
		void OnButton2(wxCommandEvent& event);
	private:
		InfoDialog(const InfoDialog& cc);
		InfoDialog& operator=(const InfoDialog& cc);

		wxBoxSizer *mainSizer;
		wxBoxSizer *buttonSizer;
		wxStaticText *ltext;
		wxButton *button1;
		wxButton *button2;
		int style;
	DECLARE_EVENT_TABLE()
};

#endif
