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

#include "whoisdialog.h"

#include "tools.h"
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/fileconf.h>
#include <wx/sizer.h>
#include <wx/socket.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

BEGIN_EVENT_TABLE(WhoisDialog, wxDialog)
	EVT_CHOICE(wxID_ANY, WhoisDialog::OnChoice)
	EVT_BUTTON(wxID_OK, WhoisDialog::OnApply)
	EVT_BUTTON(wxID_CANCEL, WhoisDialog::OnCancel)
END_EVENT_TABLE()

WhoisDialog::WhoisDialog(wxWindow* parent, wxFileConfig *langIni, wxArrayString WhoisData, bool IsYourself, wxSocketClient* socket, wxFont font):
		wxDialog(parent, wxID_ANY, langIni->Read(wxT("translations/menus/user/profile"), wxT("Userinformation..."))),
		MainSizer(nullptr),
		BoxSizer(nullptr),
		ButtonSizer(nullptr),
		GridSizer(nullptr),
		GridSizer2(nullptr),
		GridSizer3(nullptr),
		lplayername(nullptr),
		lplayerstatus(nullptr),
		lemail(nullptr),
		licq(nullptr),
		lplace(nullptr),
		lage(nullptr),
		lgender(nullptr),
		ldescription(nullptr),
		lgamename(nullptr),
		llastplayed(nullptr),
		lstatistics(nullptr),
		lvictories(nullptr),
		ldefeats(nullptr),
		ldisconnects(nullptr),
		lposition(nullptr),
		lrank(nullptr),
		playername(nullptr),
		playerstatus(nullptr),
		email(nullptr),
		icq(nullptr),
		place(nullptr),
		age(nullptr),
		description(nullptr),
		lastplayed(nullptr),
		victories(nullptr),
		defeats(nullptr),
		disconnects(nullptr),
		position(nullptr),
		rank(nullptr),
		gender(nullptr),
		gamename(nullptr),
		statistics(nullptr),
		applybutton(nullptr),
		cancelbutton(nullptr),
		WhoisData(),
		socket(nullptr) {
	WhoisData[5].Replace(wxT("\t"), wxEmptyString);
	WhoisData[5].Replace(wxT("\n"), wxEmptyString);
	WhoisData[5].Replace(wxT("%25"), wxT("%"));
	WhoisData[5].Replace(wxT("%22"), wxT("\""));
	WhoisData[7].Replace(wxT("\t"), wxEmptyString);
	WhoisData[7].Replace(wxT("\n"), wxEmptyString);
	WhoisData[7].Replace(wxT("%25"), wxT("%"));
	WhoisData[7].Replace(wxT("%22"), wxT("\""));
	WhoisData[10].Replace(wxT("\t"), wxEmptyString);
	WhoisData[10].Replace(wxT("%25"), wxT("%"));
	WhoisData[10].Replace(wxT("%22"), wxT("\""));
	SetFont(font);
	WhoisDialog::WhoisData=WhoisData;
	WhoisDialog::socket=socket;
	MainSizer=new wxBoxSizer(wxVERTICAL);
	MainSizer->AddSpacer(6);
	GridSizer=new wxFlexGridSizer(2, 4, 8);
	GridSizer->AddGrowableCol(1);
	lplayername=new wxStaticText(this, wxID_ANY, langIni->Read(wxT("translations/profile/playername"), wxT("Player name")));
	lplayerstatus=new wxStaticText(this, wxID_ANY, langIni->Read(wxT("translations/profile/playerstatus"), wxT("Player status")));
	lemail=new wxStaticText(this, wxID_ANY, langIni->Read(wxT("translations/profile/email"), wxT("E-mail")));
	licq=new wxStaticText(this, wxID_ANY, langIni->Read(wxT("translations/profile/icq"), wxT("ICQ")));
	lplace=new wxStaticText(this, wxID_ANY, langIni->Read(wxT("translations/profile/place"), wxT("Place")));
	playername=new wxTextCtrl(this, wxID_ANY, WhoisData[1], wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	playerstatus=new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	if (WhoisData[4][0]=='#') {
		playerstatus->SetValue(Format(langIni->Read(wxT("translations/profile/onchannel"), wxT("On channel <%0>")), WhoisData[4].Mid(1)));
	} else if (WhoisData[4][0]=='$') {
		playerstatus->SetValue(Format(langIni->Read(wxT("translations/profile/ingame"), wxT("In game <%0>")), WhoisData[4].Mid(1)));
	} else {
		playerstatus->SetValue(langIni->Read(wxT("translations/profile/notloggedin"), wxT("Not logged")));
	}
	email=new wxTextCtrl(this, wxID_ANY, WhoisData[5], wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	icq=new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	if (WhoisData[6]!=wxT("4294967295")) {
		icq->SetValue(WhoisData[6]);
	}
	place=new wxTextCtrl(this, wxID_ANY, WhoisData[7], wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	GridSizer->Add(lplayername, 0, wxALIGN_CENTER_VERTICAL);
	GridSizer->Add(playername, 1, wxEXPAND|wxALL);
	GridSizer->Add(lplayerstatus, 0, wxALIGN_CENTER_VERTICAL);
	GridSizer->Add(playerstatus, 1, wxEXPAND|wxALL);
	GridSizer->Add(lemail, 0, wxALIGN_CENTER_VERTICAL);
	GridSizer->Add(email, 1, wxEXPAND|wxALL);
	GridSizer->Add(licq, 0, wxALIGN_CENTER_VERTICAL);
	GridSizer->Add(icq, 1, wxEXPAND|wxALL);
	GridSizer->Add(lplace, 0, wxALIGN_CENTER_VERTICAL);
	GridSizer->Add(place, 1, wxEXPAND|wxALL);
	MainSizer->Add(GridSizer, 1, wxEXPAND|wxLEFT|wxRIGHT, 12);

	BoxSizer=new wxBoxSizer(wxHORIZONTAL);
	lage=new wxStaticText(this, wxID_ANY, langIni->Read(wxT("translations/profile/age"), wxT("Age")));
	lgender=new wxStaticText(this, wxID_ANY, langIni->Read(wxT("translations/profile/gender"), wxT("Gender")));
	age=new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(2*GetCharHeight(), -1), wxTE_READONLY|wxTE_CENTRE);
	if (WhoisData[8]!=wxT("255")) {
		age->SetValue(WhoisData[8]);
	}
	gender=new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(3*GetCharHeight(), -1));
	if (IsYourself) {
		gender->Append(wxEmptyString);
		gender->Append(langIni->Read(wxT("translations/profile/male"), wxT("M")));
		gender->Append(langIni->Read(wxT("translations/profile/female"), wxT("F")));
		if (WhoisData[9]==wxT("0")) {
			gender->SetSelection(1);
		} else if (WhoisData[9]==wxT("1")) {
			gender->SetSelection(2);
		} else {
			gender->SetSelection(0);
		}
	} else {
		if (WhoisData[9]==wxT("0")) {
			gender->Append(langIni->Read(wxT("translations/profile/male"), wxT("M")));
		} else if (WhoisData[9]==wxT("1")) {
			gender->Append(langIni->Read(wxT("translations/profile/female"), wxT("F")));
		} else {
			gender->Append(wxEmptyString);
		}
		gender->SetSelection(0);
	}
	MainSizer->AddSpacer(6);
	BoxSizer->Add(lage, 0, wxALIGN_CENTER_VERTICAL);
	BoxSizer->AddSpacer(4);
	BoxSizer->Add(age, 0, wxALIGN_CENTER_VERTICAL);
	BoxSizer->AddStretchSpacer();
	BoxSizer->AddSpacer(4);
	BoxSizer->Add(lgender, 0, wxALIGN_CENTER_VERTICAL);
	BoxSizer->AddSpacer(4);
	BoxSizer->Add(gender, 0, wxALIGN_CENTER_VERTICAL);
	MainSizer->Add(BoxSizer, 0, wxEXPAND|wxLEFT|wxRIGHT, 12);

	ldescription=new wxStaticText(this, wxID_ANY, langIni->Read(wxT("translations/profile/description"), wxT("Description")));
	description=new wxTextCtrl(this, wxID_ANY, WhoisData[10], wxDefaultPosition, wxSize(-1, 4*GetCharHeight()), wxTE_MULTILINE|wxTE_READONLY|wxVSCROLL);
	MainSizer->AddSpacer(6);
	MainSizer->Add(ldescription, 0, wxEXPAND|wxLEFT|wxRIGHT, 12);
	MainSizer->AddSpacer(6);
	MainSizer->Add(description, 0, wxEXPAND|wxLEFT|wxRIGHT, 12);

	GridSizer2=new wxFlexGridSizer(2, 4, 8);
	GridSizer2->AddGrowableCol(1);
	lgamename=new wxStaticText(this, wxID_ANY, langIni->Read(wxT("translations/profile/gamename"), wxT("Game name")));
	llastplayed=new wxStaticText(this, wxID_ANY, langIni->Read(wxT("translations/profile/lastplayed"), wxT("Last played")));
	lstatistics=new wxStaticText(this, wxID_ANY, langIni->Read(wxT("translations/profile/statistics"), wxT("Statistic")));
	gamename=new wxChoice(this, wxID_ANY);
	lastplayed=new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	statistics=new wxChoice(this, wxID_ANY);
	GridSizer2->Add(lgamename, 0, wxALIGN_CENTER_VERTICAL);
	GridSizer2->Add(gamename, 1, wxEXPAND|wxALL);
	GridSizer2->Add(llastplayed, 0, wxALIGN_CENTER_VERTICAL);
	GridSizer2->Add(lastplayed, 1, wxEXPAND|wxALL);
	GridSizer2->Add(lstatistics, 0, wxALIGN_CENTER_VERTICAL);
	GridSizer2->Add(statistics, 1, wxEXPAND|wxALL);
	MainSizer->AddSpacer(6);
	MainSizer->Add(GridSizer2, 0, wxEXPAND|wxLEFT|wxRIGHT, 12);

	GridSizer3=new wxFlexGridSizer(4, 4, 8);
	GridSizer3->AddGrowableCol(0);
	GridSizer3->AddGrowableCol(2);
	lvictories=new wxStaticText(this, wxID_ANY, langIni->Read(wxT("translations/profile/victories"), wxT("Victories")));
	ldefeats=new wxStaticText(this, wxID_ANY, langIni->Read(wxT("translations/profile/defeats"), wxT("Defeats")));
	ldisconnects=new wxStaticText(this, wxID_ANY, langIni->Read(wxT("translations/profile/disconnects"), wxT("Disconnects")));
	lposition=new wxStaticText(this, wxID_ANY, langIni->Read(wxT("translations/profile/position"), wxT("Position")));
	lrank=new wxStaticText(this, wxID_ANY, langIni->Read(wxT("translations/profile/rank"), wxT("Rank")));
	victories=new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(3*GetCharHeight(), -1), wxTE_READONLY|wxTE_CENTRE);
	defeats=new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(3*GetCharHeight(), -1), wxTE_READONLY|wxTE_CENTRE);
	disconnects=new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(3*GetCharHeight(), -1), wxTE_READONLY|wxTE_CENTRE);
	position=new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(3*GetCharHeight(), -1), wxTE_READONLY|wxTE_CENTRE);
	rank=new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(3*GetCharHeight(), -1), wxTE_READONLY|wxTE_CENTRE);
	GridSizer3->Add(lvictories, 1, wxALIGN_CENTER_VERTICAL);
	GridSizer3->Add(victories, 1, wxEXPAND|wxALL);
	GridSizer3->Add(lposition, 1, wxALIGN_CENTER_VERTICAL);
	GridSizer3->Add(position, 1, wxEXPAND|wxALL);
	GridSizer3->Add(ldefeats, 1, wxALIGN_CENTER_VERTICAL);
	GridSizer3->Add(defeats, 1, wxEXPAND|wxALL);
	GridSizer3->Add(lrank, 1, wxALIGN_CENTER_VERTICAL);
	GridSizer3->Add(rank, 1, wxEXPAND|wxALL);
	GridSizer3->Add(ldisconnects, 1, wxALIGN_CENTER_VERTICAL);
	GridSizer3->Add(disconnects, 1, wxEXPAND|wxALL);
	MainSizer->AddSpacer(6);
	MainSizer->Add(GridSizer3, 0, wxEXPAND|wxLEFT|wxRIGHT, 12);

	ButtonSizer=new wxBoxSizer(wxHORIZONTAL);
	cancelbutton=new wxButton(this, wxID_CANCEL, langIni->Read(wxT("translations/ok"), wxT("OK")));
	cancelbutton->SetDefault();
	cancelbutton->SetFocus();
	ButtonSizer->Add(cancelbutton, 1, wxEXPAND|wxALL);
	if (IsYourself) {
		cancelbutton->SetLabel(langIni->Read(wxT("translations/cancel"), wxT("Cancel")));
		ButtonSizer->AddSpacer(4);
		applybutton=new wxButton(this, wxID_OK, langIni->Read(wxT("translations/apply"), wxT("Apply")));
		applybutton->SetDefault();
		applybutton->SetFocus();
		ButtonSizer->Add(applybutton, 1, wxEXPAND|wxALL);
		email->SetEditable(true);
		icq->SetEditable(true);
		place->SetEditable(true);
		age->SetEditable(true);
		description->SetEditable(true);
		description->SetMaxLength(255);
	}
	MainSizer->AddSpacer(6);
	MainSizer->Add(ButtonSizer, 0, wxEXPAND|wxLEFT|wxRIGHT, 12);
	MainSizer->AddSpacer(6);

	if (WhoisData.GetCount()>=47) {
		statistics->Append(langIni->Read(wxT("translations/profile/all"), wxT("All")));
		statistics->Append(langIni->Read(wxT("translations/profile/week"), wxT("Week")));
		statistics->Append(langIni->Read(wxT("translations/profile/month"), wxT("Month")));
		statistics->Append(langIni->Read(wxT("translations/profile/previousweek"), wxT("Previous Week")));
		statistics->Append(langIni->Read(wxT("translations/profile/previousmonth"), wxT("Previous Month")));
		statistics->SetSelection(0);
		for (size_t i=11; i!=WhoisData.GetCount();i+=37) {
			if (WhoisData[i]==wxT("0")) {
				gamename->Append(wxT("Earth 2150"));
			} else if (WhoisData[i]==wxT("1")) {
				gamename->Append(wxT("The Moon Project"));
			} else if (WhoisData[i]==wxT("2")) {
				gamename->Append(wxT("Lost Souls"));
			} else {
				gamename->Append(wxT("?"));
			}
		}
		gamename->SetSelection(0);
		wxCommandEvent tmp;
		OnChoice(tmp);
	} else {
		gamename->Disable();
		lastplayed->Disable();
		statistics->Disable();
		victories->Disable();
		defeats->Disable();
		disconnects->Disable();
		position->Disable();
		rank->Disable();
	}

	SetSizer(MainSizer);
	SetInitialSize();
	SetAutoLayout(true);
	SetSize(wxDefaultCoord, wxDefaultCoord, GetCharHeight()*19+52, wxDefaultCoord);
}

void WhoisDialog::OnChoice(wxCommandEvent&WXUNUSED(event)) {
	if (WhoisData.GetCount()!=11) { //hasplayed
		double tmp;
		WhoisData[gamename->GetSelection()*37+12].ToDouble(&tmp);
		wxDateTime tmp2(tmp+2415018.58334369);
		lastplayed->SetValue(wxString(wxEmptyString).Format(wxT("%.2i.%.2i.%.4i %.2i:%.2i:%.2i"), tmp2.GetDay(), tmp2.GetMonth()+1, tmp2.GetYear(), tmp2.GetHour(), tmp2.GetMinute(), tmp2.GetSecond()));

		int i=gamename->GetSelection()*37+statistics->GetSelection()*7+13;
		victories->SetValue(WhoisData[i]);
		defeats->SetValue(WhoisData[i+1]);
		disconnects->SetValue(WhoisData[i+2]);
		position->SetValue(WhoisData[i+4]);
		rank->SetValue(WhoisData[i+5]);
	}
}

void WhoisDialog::OnApply(wxCommandEvent& WXUNUSED(event)) {
	if (socket->IsConnected()) {
		wxString tmp=email->GetValue();
		tmp.Replace(wxT("\t"), wxEmptyString);
		tmp.Replace(wxT("\n"), wxEmptyString);
		tmp.Replace(wxT("%"), wxT("%25"));
		tmp.Replace(wxT("\""), wxT("%22"));
		email->SetValue(tmp);
		tmp=place->GetValue();
		tmp.Replace(wxT("\t"), wxEmptyString);
		tmp.Replace(wxT("\n"), wxEmptyString);
		tmp.Replace(wxT("%"), wxT("%25"));
		tmp.Replace(wxT("\""), wxT("%22"));
		place->SetValue(tmp);
		tmp=description->GetValue();
		tmp.Replace(wxT("\t"), wxEmptyString);
		tmp.Replace(wxT("%"), wxT("%25"));
		tmp.Replace(wxT("\""), wxT("%22"));
		description->SetValue(tmp);

		tmp=wxT("/update \"")+playername->GetValue()+wxT("\" \"")+email->GetValue()+wxT("\" \"");
		long int tmp2=0xFFFFFFFF;
		if (icq->GetValue()!=wxEmptyString) {
			icq->GetValue().ToLong(&tmp2);
		}
		tmp+=tmp.Format(wxT("%i"), tmp2)+wxT("\" \"")+place->GetValue()+wxT("\" \"");
		if (!age->GetValue().ToLong(&tmp2)) {
			tmp2=255;
		}
		if (tmp2>=100) {
			tmp+=wxT("-1");
		} else {
			tmp+=tmp.Format(wxT("%i"), tmp2/10)+tmp.Format(wxT("%i"), tmp2%10);
		}
		tmp+=tmp.Format(wxT("\" \"%i\" \""), gender->GetSelection()-1)+description->GetValue()+wxT("\"");
		socket->Write(tmp.mb_str(), tmp.Len()+1);
	}
	Destroy();
}

void WhoisDialog::OnCancel(wxCommandEvent& WXUNUSED(event)) {
	Destroy();
}
