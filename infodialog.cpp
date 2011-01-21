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
	MainSizer->AddSpacer(4);
	if (Text!=wxT("")) {
		MainSizer->AddSpacer(-12);
		ltext=new wxStaticText(this, -1, Text);
		MainSizer->Add(ltext, 1, wxEXPAND|wxALL, 16);
		MainSizer->AddSpacer(-16);
	}
	MainSizer->AddSpacer(-12);
	ButtonSizer=new wxBoxSizer(wxHORIZONTAL);
	ButtonSizer->AddStretchSpacer();
	if (Style) {
		button1=new wxButton(this, wxID_OK, LangIni->Read(wxT("translations/yes"), wxT("Yes")));
		button2=new wxButton(this, wxID_CANCEL, LangIni->Read(wxT("translations/no"), wxT("No")));
	} else {
		button1=new wxButton(this, wxID_OK, LangIni->Read(wxT("translations/ok"), wxT("OK")));
	}
	ButtonSizer->Add(button1, 0, wxEXPAND|wxALL, 0);
	if (Style==1) {
		ButtonSizer->AddSpacer(4);
		ButtonSizer->Add(button2, 0, wxEXPAND|wxALL, 0);
	}
	ButtonSizer->AddStretchSpacer();

	button1->SetDefault();
	if (Style && Default==1) button2->SetDefault();
	MainSizer->Add(ButtonSizer, 0, wxEXPAND|wxALL, 16);
	MainSizer->AddSpacer(-8);

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
