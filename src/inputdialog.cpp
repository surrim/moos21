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

#include "inputdialog.h"

#include "enums.h"
#include <wx/button.h>
#include <wx/fileconf.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

BEGIN_EVENT_TABLE(InputDialog, wxDialog)
	EVT_TEXT(wxID_ANY, InputDialog::OnText)
	EVT_LISTBOX(wxID_ANY, InputDialog::OnChoice)
	EVT_LISTBOX_DCLICK(wxID_ANY, InputDialog::OnOK)
	EVT_BUTTON(wxID_OK, InputDialog::OnOK)
	EVT_BUTTON(wxID_CANCEL, InputDialog::OnCancel)
END_EVENT_TABLE()

//for user choice, password, slap and ignore list
//0=input, 1=passwdinput, 2=singlechoice+input, 3=singlechoice, 4=multichoice
InputDialog::InputDialog(wxWindow* parent, wxFileConfig *langIni, const wxFont& font, const wxString& title, const wxString& text, int style, const wxString& defaultButton, const wxArrayString& choiceList):
		wxDialog(parent, wxID_ANY, title),
		mainSizer(nullptr),
		buttonSizer(nullptr),
		ltext(nullptr),
		choice(nullptr),
		input(nullptr),
		okButton(nullptr),
		cancelButton(nullptr),
		saved(),
		style(style) {
	SetFont(font);
	mainSizer=new wxBoxSizer(wxVERTICAL);
	mainSizer->AddSpacer(6);
	if (text!=wxEmptyString) {
		ltext=new wxStaticText(this, wxID_ANY, text);
		mainSizer->Add(ltext, 0, wxEXPAND|wxLEFT|wxRIGHT, 12);
		mainSizer->AddSpacer(3);
	}
	if (style>=2) {
		if (style!=4)
			choice=new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(120, wxDefaultCoord), choiceList);
		else
			choice=new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(120, wxDefaultCoord), choiceList, wxLB_EXTENDED);
		if (choice->FindString(defaultButton)!=wxNOT_FOUND) {
			choice->SetSelection(choice->FindString(defaultButton));
		}
		mainSizer->Add(choice, 1, wxEXPAND|wxLEFT|wxRIGHT, 12);
		mainSizer->AddSpacer(3);
	}
	if (style<=2) {
		//MainSizer->AddSpacer(-12);
		//if (Style==2) MainSizer->AddSpacer(-5);
		if (style!=1)
			input=new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(120, -1));
		else
			input=new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(120, -1), wxTE_PASSWORD);
		input->SetValue(defaultButton);
		mainSizer->Add(input, 0, wxEXPAND|wxLEFT|wxRIGHT, 12);
		mainSizer->AddSpacer(3);
	}
	//MainSizer->AddSpacer(-12);
	okButton=new wxButton(this, wxID_OK, langIni->Read(wxT("translations/ok"), wxT("OK")));
	cancelButton=new wxButton(this, wxID_CANCEL, langIni->Read(wxT("translations/cancel"), wxT("Cancel")));
	buttonSizer=new wxBoxSizer(wxHORIZONTAL);
	//ButtonSizer->AddStretchSpacer();
	buttonSizer->Add(cancelButton, 1, wxEXPAND|wxALL, 0);
	buttonSizer->AddSpacer(6);
	buttonSizer->Add(okButton, 1, wxEXPAND|wxALL, 0);
	okButton->SetDefault();
	mainSizer->Add(buttonSizer, 0, wxEXPAND|wxLEFT|wxRIGHT, 12);
	mainSizer->AddSpacer(6);
	if (style<=1) {
		input->SetFocus();
	} else {
		choice->SetFocus();
	}

	SetSizer(mainSizer);
	SetAutoLayout(true);
	SetInitialSize();
}

void InputDialog::OnText(wxCommandEvent& WXUNUSED(event)) {
	saved=input->GetValue();
	if (style>1) {
		if (choice->FindString(saved)!=wxNOT_FOUND) {
			choice->SetSelection(choice->FindString(saved));
		} else {
			choice->DeselectAll();
		}
	}
}

void InputDialog::OnChoice(wxCommandEvent& WXUNUSED(event)) {
	saved=choice->GetStringSelection();
	if (style<=2) {
		input->SetValue(saved);
	}
}

void InputDialog::OnOK(wxCommandEvent& WXUNUSED(event)) {
	if (style<=1 || style==4 || saved!=wxEmptyString) {
		EndModal(wxID_OK);
	}
}

void InputDialog::OnCancel(wxCommandEvent& WXUNUSED(event)) {
	EndModal(wxID_CANCEL);
}

wxString InputDialog::GetValue() {
	return saved;
}

wxArrayString InputDialog::GetSelections() {
	wxArrayString _;
	if (style<=1) {
		return _;
	}
	for (size_t i=0;i!=choice->GetCount();i++) {
		if (choice->IsSelected(i)) {
			_.Add(choice->GetString(i));
		}
	}
	return _;
}

void InputDialog::SetSelections(wxArrayInt& selections) {
	for (size_t i=0;i!=selections.GetCount();i++) {
		choice->SetSelection(selections[i]);
	}
}
