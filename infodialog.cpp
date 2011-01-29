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
InfoDialog::InfoDialog(wxWindow* parent, wxFileConfig *LangIni, wxFont Font, wxString Title, wxString Text, int Style, int Default):
		wxDialog(parent, -1, Title),
		MainSizer(0),
		ButtonSizer(0),
		ltext(0),
		button1(0),
		button2(0),
		Style(Style) {
	SetFont(Font);
	MainSizer=new wxBoxSizer(wxVERTICAL);
	MainSizer->AddSpacer(6);
	if (Text!=wxEmptyString) {
		ltext=new wxStaticText(this, -1, Text);
		MainSizer->Add(ltext, 1, wxEXPAND|wxLEFT|wxRIGHT, 12);
		MainSizer->AddSpacer(3);
	}
	ButtonSizer=new wxBoxSizer(wxHORIZONTAL);
	ButtonSizer->AddStretchSpacer();
	if (Style==1) {
		button1=new wxButton(this, wxID_OK, LangIni->Read(wxT("translations/yes"), wxT("Yes")));
		button2=new wxButton(this, wxID_CANCEL, LangIni->Read(wxT("translations/no"), wxT("No")));
	} else {
		button1=new wxButton(this, wxID_OK, LangIni->Read(wxT("translations/ok"), wxT("OK")));
	}
	ButtonSizer->Add(button1, 0, wxEXPAND|wxTOP|wxBOTTOM, 12);
	if (Style==1) {
		ButtonSizer->AddSpacer(6);
		ButtonSizer->Add(button2, 0, wxEXPAND|wxTOP|wxBOTTOM, 12);
	}
	ButtonSizer->AddStretchSpacer();

	button1->SetDefault();
	if (Style && Default==1) button2->SetDefault();
	MainSizer->Add(ButtonSizer, 0, wxEXPAND|wxLEFT|wxRIGHT, 12);
	MainSizer->AddSpacer(6);

	SetSizer(MainSizer);
	SetInitialSize();
	SetAutoLayout(true);
}

void InfoDialog::OnButton1(wxCommandEvent &event) {
	if (Style) EndModal(wxID_YES);
	else EndModal(wxID_OK);
}

void InfoDialog::OnButton2(wxCommandEvent &event) {
	EndModal(wxID_NO);
}
