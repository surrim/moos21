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

#include "infodialog.h"

#include <wx/button.h>
#include <wx/fileconf.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

BEGIN_EVENT_TABLE(InfoDialog, wxDialog)
	EVT_BUTTON(wxID_OK, InfoDialog::OnButton1)
	EVT_BUTTON(wxID_CANCEL, InfoDialog::OnButton2)
END_EVENT_TABLE()

//0=ok, 1=yes/no
InfoDialog::InfoDialog(wxWindow* parent, wxFileConfig *langIni, const wxFont& font, const wxString& title, const wxString& text, int style, int defaultButton):
		wxDialog(parent, wxID_ANY, title),
		mainSizer(nullptr),
		buttonSizer(nullptr),
		ltext(nullptr),
		button1(nullptr),
		button2(nullptr),
		style(style) {
	SetFont(font);
	mainSizer=new wxBoxSizer(wxVERTICAL);
	mainSizer->AddSpacer(6);
	if (text!=wxEmptyString) {
		ltext=new wxStaticText(this, wxID_ANY, text);
		mainSizer->Add(ltext, 1, wxEXPAND|wxLEFT|wxRIGHT, 12);
		mainSizer->AddSpacer(3);
	}
	buttonSizer=new wxBoxSizer(wxHORIZONTAL);
	buttonSizer->AddStretchSpacer();
	if (style==1) {
		button1=new wxButton(this, wxID_OK, langIni->Read(wxT("translations/yes"), wxT("Yes")));
		button2=new wxButton(this, wxID_CANCEL, langIni->Read(wxT("translations/no"), wxT("No")));
	} else {
		button1=new wxButton(this, wxID_OK, langIni->Read(wxT("translations/ok"), wxT("OK")));
	}
	buttonSizer->Add(button1, 0, wxEXPAND|wxTOP|wxBOTTOM, 12);
	if (style==1) {
		buttonSizer->AddSpacer(6);
		buttonSizer->Add(button2, 0, wxEXPAND|wxTOP|wxBOTTOM, 12);
	}
	buttonSizer->AddStretchSpacer();

	button1->SetDefault();
	if (style!=0 && defaultButton==1) {
		button2->SetDefault();
	}
	mainSizer->Add(buttonSizer, 0, wxEXPAND|wxLEFT|wxRIGHT, 12);
	mainSizer->AddSpacer(6);

	SetSizer(mainSizer);
	SetInitialSize();
	SetAutoLayout(true);
}

void InfoDialog::OnButton1(wxCommandEvent& WXUNUSED(event)) {
	if (style) {
		EndModal(wxID_YES);
	}
	else EndModal(wxID_OK);
}

void InfoDialog::OnButton2(wxCommandEvent& WXUNUSED(event)) {
	EndModal(wxID_NO);
}
