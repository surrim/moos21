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
InputDialog::InputDialog(wxWindow* parent, wxFileConfig *langIni, wxFont font, wxString Title, wxString Text, int Style, wxString Default, wxArrayString ChoiceList):
		wxDialog(parent, -1, Title),
		MainSizer(0),
		ButtonSizer(0),
		ltext(0),
		choice(0),
		input(0),
		okbutton(0),
		cancelbutton(0),
		saved(),
		Style(Style) {
	SetFont(font);
	MainSizer=new wxBoxSizer(wxVERTICAL);
	MainSizer->AddSpacer(6);
	if (Text!=wxEmptyString) {
		ltext=new wxStaticText(this, -1, Text);
		MainSizer->Add(ltext, 0, wxEXPAND|wxLEFT|wxRIGHT, 12);
		MainSizer->AddSpacer(3);
	}
	if (Style>=2) {
		if (Style!=4)
			choice=new wxListBox(this, -1, wxDefaultPosition, wxSize(120, wxDefaultCoord), ChoiceList);
		else
			choice=new wxListBox(this, -1, wxDefaultPosition, wxSize(120, wxDefaultCoord), ChoiceList, wxLB_EXTENDED);
		if (choice->FindString(Default)!=wxNOT_FOUND) choice->SetSelection(choice->FindString(Default));
		MainSizer->Add(choice, 1, wxEXPAND|wxLEFT|wxRIGHT, 12);
		MainSizer->AddSpacer(3);
	}
	if (Style<=2) {
		//MainSizer->AddSpacer(-12);
		//if (Style==2) MainSizer->AddSpacer(-5);
		if (Style!=1)
			input=new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxSize(120, -1));
		else
			input=new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxSize(120, -1), wxTE_PASSWORD);
		input->SetValue(Default);
		MainSizer->Add(input, 0, wxEXPAND|wxLEFT|wxRIGHT, 12);
		MainSizer->AddSpacer(3);
	}
	//MainSizer->AddSpacer(-12);
	okbutton=new wxButton(this, wxID_OK, langIni->Read(wxT("translations/ok"), wxT("OK")));
	cancelbutton=new wxButton(this, wxID_CANCEL, langIni->Read(wxT("translations/cancel"), wxT("Cancel")));
	ButtonSizer=new wxBoxSizer(wxHORIZONTAL);
	//ButtonSizer->AddStretchSpacer();
	ButtonSizer->Add(cancelbutton, 1, wxEXPAND|wxALL, 0);
	ButtonSizer->AddSpacer(6);
	ButtonSizer->Add(okbutton, 1, wxEXPAND|wxALL, 0);
	okbutton->SetDefault();
	MainSizer->Add(ButtonSizer, 0, wxEXPAND|wxLEFT|wxRIGHT, 12);
	MainSizer->AddSpacer(6);
	if (Style<=1) {
		input->SetFocus();
	} else {
		choice->SetFocus();
	}

	SetSizer(MainSizer);
	SetAutoLayout(true);
	SetInitialSize();
}

void InputDialog::OnText(wxCommandEvent& Event) {
	saved=input->GetValue();
	if (Style>1) {
		if (choice->FindString(saved)!=wxNOT_FOUND) {
			choice->SetSelection(choice->FindString(saved));
		} else {
			choice->DeselectAll();
		}
	}
}

void InputDialog::OnChoice(wxCommandEvent &event) {
	saved=choice->GetStringSelection();
	if (Style<=2) input->SetValue(saved);
}

void InputDialog::OnOK(wxCommandEvent &event) {
	if (Style<=1 || Style==4 || saved!=wxEmptyString) {
		EndModal(wxID_OK);
	}
}

void InputDialog::OnCancel(wxCommandEvent &event) {
	EndModal(wxID_CANCEL);
}

wxString InputDialog::GetValue() {
	return saved;
}

wxArrayString InputDialog::GetSelections() {
	wxArrayString _;
	if (Style<=1) {
		return _;
	}
	for (size_t i=0;i!=choice->GetCount();++i) {
		if (choice->IsSelected(i)) {
			_.Add(choice->GetString(i));
		}
	}
	return _;
}

void InputDialog::SetSelections(wxArrayInt& selections) {
	for (size_t i=0;i!=selections.GetCount();++i) {
		choice->SetSelection(selections[i]);
	}
}
