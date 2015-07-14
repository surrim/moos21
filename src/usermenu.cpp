#include "base.h"

#include "enums.h"
#include "inputdialog.h"
#include "tools.h"
#include <wx/fileconf.h>
#include <wx/menu.h>
#include <wx/socket.h>

void MainFrame::OnWhoisUser(wxCommandEvent& WXUNUSED(event)) {
	if (!socket->IsConnected()) return;
	wxArrayInt selections;
	if (!userList->GetSelections(selections)) { //nobody selected
		wxArrayString tmp2=seenUsers;
		tmp2.Sort();
		InputDialog *tmp=new InputDialog(this, langIni, font.GetChosenFont(), langIni->Read(wxT("translations/menus/user/profile"), wxT("Userinformation...")),
										 langIni->Read(wxT("translations/dialogtext/selectusername"), wxT("Please select an username")), 2, wxEmptyString, tmp2);
		if (tmp->ShowModal()==wxID_OK && tmp->GetValue()!=wxEmptyString) {
			write(wxT("/whois \"")+tmp->GetValue()+wxT("\""));
		}
		return;
	}
	for (size_t i=0;i!=selections.GetCount();i++) {
		write(wxT("/whois \"")+userList->GetString(selections[i])+wxT("\""));
	}
}

void MainFrame::OnSlapUser(wxCommandEvent& WXUNUSED(event)) {
	if (!userList->GetCount() || settingsMenu->IsChecked(ID_MAINWIN_DISABLE_SLAPS)) {
		return;
	}
	wxArrayInt selections;
	wxArrayString users;

	if (!userList->GetSelections(selections)) { //nobody selected
		wxArrayString tmp3;
		for (size_t i=0;i!=userList->GetCount();i++) {
			tmp3.Add(userList->GetString(i));
		}
		InputDialog *tmp=new InputDialog(this, langIni, font.GetChosenFont(),
										 langIni->Read(wxT("translations/menus/user/slap"), wxT("Slap...")),
										 langIni->Read(wxT("translations/dialogtext/selectusername"), wxT("Please select an username")), 3, wxEmptyString, tmp3);
		if (tmp->ShowModal()!=wxID_OK || tmp->GetValue()==wxEmptyString) {
			return;
		}
		users.Add(tmp->GetValue());
	} else {
		for (size_t i=0;i!=selections.GetCount();i++) {
			users.Add(userList->GetString(selections[i]));
		}
	}
	InputDialog *tmp=new InputDialog(this, langIni, font.GetChosenFont(), langIni->Read(wxT("translations/menus/user/slap"), wxT("Slap...")),
									 langIni->Read(wxT("translations/dialogtext/enterslap"), wxT("Please enter slap text")));
	if (tmp->ShowModal()!=wxID_OK || tmp->GetValue()==wxEmptyString) {
		return;
	}
	wxString tmp2=tmp->GetValue();
	tmp2.Replace(wxT("%"), wxT("%25"));
	tmp2.Replace(wxT("\""), wxT("%22"));
	for (size_t i=0;i!=users.GetCount();i++) {
		write(wxT("/send \"<0xFF0000FF>slaps <0xFF2153E8>")+users[i]+wxT("<0xFF0000FF> with ")+tmp2+wxT("\""));
	}
}

void MainFrame::OnIgnoreUser(wxCommandEvent& WXUNUSED(event)) {
	wxArrayInt selections;
	if (!userList->GetSelections(selections)) { //nobody selected
		wxArrayString tmp2=seenUsers;
		tmp2.Sort();
		InputDialog *tmp=new InputDialog(this, langIni, font.GetChosenFont(),
										 langIni->Read(wxT("translations/menus/user/ignore"), wxT("Ignore")),
										 langIni->Read(wxT("translations/dialogtext/selectusername"), wxT("Please select an username")), 2, wxEmptyString, tmp2);
		if (tmp->ShowModal()==wxID_OK && tmp->GetValue()!=wxEmptyString) {
			IgnoreUser(tmp->GetValue());
			return;
		}
	}
	while (userList->GetSelections(selections)) {
		IgnoreUser(userList->GetString(selections[0]));
	}
}

void MainFrame::OnIgnoreList(wxCommandEvent& WXUNUSED(event)) {
	long i;
	wxArrayString tmp;
	wxString tmp2;
	wxArrayInt tmp4;
	moosIni->SetPath(wxT("ignored/"));
	if (!moosIni->GetFirstEntry(tmp2, i)) {
		moosIni->SetPath(wxEmptyString);
		return;
	}
	tmp.Add(Base64Decode(tmp2));
	if (ignoredUserList->FindString(Base64Decode(tmp2))!=wxNOT_FOUND) tmp4.Add(0);
	while (moosIni->GetNextEntry(tmp2, i)) {
		tmp.Add(Base64Decode(tmp2));
		if (ignoredUserList->FindString(Base64Decode(tmp2))!=wxNOT_FOUND) {
			tmp4.Add(tmp.GetCount()-1);
		}
	}
	moosIni->SetPath(wxEmptyString);
	InputDialog *tmp3=new InputDialog(this, langIni, font.GetChosenFont(),
									  langIni->Read(wxT("translations/menus/user/ignorelist"), wxT("Ignorelist...")),
									  langIni->Read(wxT("translations/dialogtext/selectuserstounignore"),
													wxT("Please select all users you want to remove from your Ignore List")), 4, wxEmptyString, tmp);
	tmp3->SetSelections(tmp4);
	if (tmp3->ShowModal()==wxID_OK) {
		for (size_t i=0;i!=tmp3->GetSelections().GetCount();i++) {
			UnignoreUser(tmp3->GetSelections()[i]);
		}
	}
}

void MainFrame::AddUser(wxString User) {
	if (!IsIgnored(User)) {
		userList->Append(User);
	} else {
		ignoredUserList->Append(User);
	}
}

void MainFrame::IgnoreUser(wxString User) {
	if (!IsIgnored(User)) {
		moosIni->Write(wxT("ignored/")+Base64Encode(User), true);
		if (userList->FindString(User)!=wxNOT_FOUND) {
			RemoveUser(User);
			AddUser(User); //this tima as ignored
		}
		Message(langIni->Read(wxT("translations/channel/events/ignoreuser"), wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%0><0xA050FFFF> is now on your Ignore List")), User);
	}
}

void MainFrame::UnignoreUser(wxString User) {
	if (IsIgnored(User)) {
		moosIni->DeleteEntry(wxT("ignored/")+Base64Encode(User));
		if (ignoredUserList->FindString(User)!=wxNOT_FOUND) {
			RemoveUser(User);
			AddUser(User); //this tima as ignored
		}
		Message(langIni->Read(wxT("translations/channel/events/unignoreuser"), wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%0><0xA050FFFF> has been removed from your Ignore List")), User);
	}
}

bool MainFrame::IsIgnored(wxString User) {
	if (User[0]=='^') {
		User=User.Mid(1);
	}
	return moosIni->HasEntry(wxT("ignored/")+Base64Encode(User));
}

void MainFrame::RemoveUser(wxString User) {
	if (userList->FindString(User)!=wxNOT_FOUND) {
		userList->Delete(userList->FindString(User));
	}
	if (ignoredUserList->FindString(User)!=wxNOT_FOUND) {
		ignoredUserList->Delete(ignoredUserList->FindString(User));
	}
	wxArrayInt selections;
	if (!userList->GetSelections(selections)) {
		whisperCheckbox->SetValue(false);
	}
}

void MainFrame::RemoveAllUsers() {
	userList->Clear();
	ignoredUserList->Clear();
	whisperCheckbox->SetValue(false);
}
