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

#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <wx/dialog.h>

class wxBoxSizer;
class wxStaticText;
class wxListBox;
class wxTextCtrl;
class wxButton;
class wxString;
class wxFileConfig;

class InputDialog: public wxDialog {
	public:
		InputDialog(wxWindow* parent, wxFileConfig *langIni, const wxFont& font, const wxString& title, const wxString& text=wxEmptyString, int style=0, const wxString& defaultButton=wxEmptyString, const wxArrayString& choiceList=wxArrayString());
		void OnText(wxCommandEvent& Event);
		void OnChoice(wxCommandEvent& event);
		void OnOK(wxCommandEvent& event);
		void OnCancel(wxCommandEvent& event);
		wxString GetValue();
		wxArrayString GetSelections();
		void SetSelections(wxArrayInt& selections);
	private:
		InputDialog(const InputDialog& cc);
		InputDialog& operator=(const InputDialog& cc);

		wxBoxSizer *mainSizer;
		wxBoxSizer *buttonSizer;
		wxStaticText *ltext;
		wxListBox *choice;
		wxTextCtrl *input;
		wxButton *okButton;
		wxButton *cancelButton;
		wxString saved;
		int style;
	DECLARE_EVENT_TABLE()
};

#endif
