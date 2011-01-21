#include "base.h"

#include "enums.h"
#include "inputdialog.h"
#include "tools.h"
#include <wx/fileconf.h>
#include <wx/menu.h>
#include <wx/socket.h>

void MainFrame::OnWhoisUser(wxCommandEvent &event) {
	ChatInput->SetFocus();
	if (!Socket->IsConnected()) return;
	wxArrayInt selections;
	if (!UserList->GetSelections(selections)) { //nobody selected
		wxArrayString tmp2=SeenUsers;
		tmp2.Sort();
		InputDialog *tmp=new InputDialog(this, LangIni, Font.GetChosenFont(), LangIni->Read(wxT("translations/menus/user/profile"), wxT("Userinformation...")),
										 LangIni->Read(wxT("translations/dialogtext/selectusername"), wxT("Please select an username")), 2, wxT(""), tmp2);
		if (tmp->ShowModal()==wxID_OK && tmp->GetValue()!=wxT("")) {
			Write(wxT("/whois \"")+tmp->GetValue()+wxT("\""));
		}
		return;
	}
	for (size_t i=0;i!=selections.GetCount();++i) {
		Write(wxT("/whois \"")+UserList->GetString(selections[i])+wxT("\""));
	}
}

void MainFrame::OnSlapUser(wxCommandEvent &event) {
	if (!UserList->GetCount() || SettingsMenu->IsChecked(ID_MAINWIN_DISABLE_SLAPS)) {
		return;
	}
	wxArrayInt selections;
	wxArrayString users;

	if (!UserList->GetSelections(selections)) { //nobody selected
		wxArrayString tmp3;
		for (size_t i=0;i!=UserList->GetCount();++i) {
			tmp3.Add(UserList->GetString(i));
		}
		InputDialog *tmp=new InputDialog(this, LangIni, Font.GetChosenFont(),
										 LangIni->Read(wxT("translations/menus/user/slap"), wxT("Slap...")),
										 LangIni->Read(wxT("translations/dialogtext/selectusername"), wxT("Please select an username")), 3, wxT(""), tmp3);
		if (tmp->ShowModal()!=wxID_OK || tmp->GetValue()==wxT("")) {
			return;
		}
		users.Add(tmp->GetValue());
	} else {
		for (size_t i=0;i!=selections.GetCount();++i) {
			users.Add(UserList->GetString(selections[i]));
		}
	}
	InputDialog *tmp=new InputDialog(this, LangIni, Font.GetChosenFont(), LangIni->Read(wxT("translations/menus/user/slap"), wxT("Slap...")),
									 LangIni->Read(wxT("translations/dialogtext/enterslap"), wxT("Please enter slap text")));
	if (tmp->ShowModal()!=wxID_OK || tmp->GetValue()==wxT("")) {
		return;
	}
	wxString tmp2=tmp->GetValue();
	tmp2.Replace(wxT("%"), wxT("%25"));
	tmp2.Replace(wxT("\""), wxT("%22"));
	for (size_t i=0;i!=users.GetCount();++i) {
		Write(wxT("/send \"<0xFF0000FF>slaps <0xFF2153E8>")+users[i]+wxT("<0xFF0000FF> with ")+tmp2+wxT("\""));
	}
	ChatInput->SetFocus();
}

void MainFrame::OnIgnoreUser(wxCommandEvent &event) {
	wxArrayInt selections;
	if (!UserList->GetSelections(selections)) { //nobody selected
		wxArrayString tmp2=SeenUsers;
		tmp2.Sort();
		InputDialog *tmp=new InputDialog(this, LangIni, Font.GetChosenFont(),
										 LangIni->Read(wxT("translations/menus/user/ignore"), wxT("Ignore")),
										 LangIni->Read(wxT("translations/dialogtext/selectusername"), wxT("Please select an username")), 2, wxT(""), tmp2);
		if (tmp->ShowModal()==wxID_OK && tmp->GetValue()!=wxT("")) {
			IgnoreUser(tmp->GetValue());
			return;
		}
	}
	while (UserList->GetSelections(selections)) IgnoreUser(UserList->GetString(selections[0]));
}

void MainFrame::OnIgnoreList(wxCommandEvent & WXUNUSED(event)) {
	long i;
	wxArrayString tmp;
	wxString tmp2;
	wxArrayInt tmp4;
	MoosIni->SetPath(wxT("ignored/"));
	if (!MoosIni->GetFirstEntry(tmp2, i)) {
		MoosIni->SetPath(wxT(""));
		return;
	}
	tmp.Add(Base64Decode(tmp2));
	if (IgnoredUserList->FindString(Base64Decode(tmp2))!=wxNOT_FOUND) tmp4.Add(0);
	while (MoosIni->GetNextEntry(tmp2, i)) {
		tmp.Add(Base64Decode(tmp2));
		if (IgnoredUserList->FindString(Base64Decode(tmp2))!=wxNOT_FOUND) {
			tmp4.Add(tmp.GetCount()-1);
		}
	}
	MoosIni->SetPath(wxT(""));
	InputDialog *tmp3=new InputDialog(this, LangIni, Font.GetChosenFont(),
									  LangIni->Read(wxT("translations/menus/user/ignorelist"), wxT("Ignorelist...")),
									  LangIni->Read(wxT("translations/dialogtext/selectuserstounignore"),
													wxT("Please select all users you want to remove from your Ignore List")), 4, wxT(""), tmp);
	tmp3->SetSelections(tmp4);
	if (tmp3->ShowModal()==wxID_OK) {
		for (size_t i=0;i!=tmp3->GetSelections().GetCount();++i) {
			UnignoreUser(tmp3->GetSelections()[i]);
		}
		ChatInput->SetFocus();
	}
}

void MainFrame::AddUser(wxString User) {
	if (!IsIgnored(User)) {
		UserList->Append(User);
	} else {
		IgnoredUserList->Append(User);
	}
}

void MainFrame::IgnoreUser(wxString User) {
	if (!IsIgnored(User)) {
		MoosIni->Write(wxT("ignored/")+Base64Encode(User), true);
		if (UserList->FindString(User)!=wxNOT_FOUND) {
			RemoveUser(User);
			AddUser(User); //this tima as ignored
		}
		Message(LangIni->Read(wxT("translations/channel/events/ignoreuser"), wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%0><0xA050FFFF> is now on your Ignore List")), User);
	}
}

void MainFrame::UnignoreUser(wxString User) {
	if (IsIgnored(User)) {
		MoosIni->DeleteEntry(wxT("ignored/")+Base64Encode(User));
		if (IgnoredUserList->FindString(User)!=wxNOT_FOUND) {
			RemoveUser(User);
			AddUser(User); //this tima as ignored
		}
		Message(LangIni->Read(wxT("translations/channel/events/unignoreuser"), wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%0><0xA050FFFF> has been removed from your Ignore List")), User);
	}
}

bool MainFrame::IsIgnored(wxString User) {
	if (User[0]=='^') {
		User=User.Mid(1);
	}
	return MoosIni->HasEntry(wxT("ignored/")+Base64Encode(User));
}

void MainFrame::RemoveUser(wxString User) {
	if (UserList->FindString(User)!=wxNOT_FOUND) {
		UserList->Delete(UserList->FindString(User));
	}
	if (IgnoredUserList->FindString(User)!=wxNOT_FOUND) {
		IgnoredUserList->Delete(IgnoredUserList->FindString(User));
	}
	wxArrayInt selections;
	if (!UserList->GetSelections(selections)) {
		WhisperCheckbox->SetValue(false);
	}
}

void MainFrame::RemoveAllUsers() {
	UserList->Clear();
	IgnoredUserList->Clear();
	WhisperCheckbox->SetValue(false);
}
