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
		wxDialog(parent, -1, title),
		mainSizer(0),
		buttonSizer(0),
		ltext(0),
		choice(0),
		input(0),
		okButton(0),
		cancelButton(0),
		saved(),
		style(style) {
	SetFont(font);
	mainSizer=new wxBoxSizer(wxVERTICAL);
	mainSizer->AddSpacer(6);
	if (text!=wxEmptyString) {
		ltext=new wxStaticText(this, -1, text);
		mainSizer->Add(ltext, 0, wxEXPAND|wxLEFT|wxRIGHT, 12);
		mainSizer->AddSpacer(3);
	}
	if (style>=2) {
		if (style!=4)
			choice=new wxListBox(this, -1, wxDefaultPosition, wxSize(120, wxDefaultCoord), choiceList);
		else
			choice=new wxListBox(this, -1, wxDefaultPosition, wxSize(120, wxDefaultCoord), choiceList, wxLB_EXTENDED);
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
			input=new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxSize(120, -1));
		else
			input=new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxSize(120, -1), wxTE_PASSWORD);
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
