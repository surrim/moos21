#include "base.h"

#include "tools.h"
#include "inputdialog.h"
#include <wx/fileconf.h>
#include <wx/socket.h>

void MainFrame::OnLoginAs(wxCommandEvent & WXUNUSED(event)) {
	wxArrayString tmp;
	long i;
	wxString tmp3;
	MoosIni->SetPath(wxT("accounts/"));
	if (MoosIni->GetFirstGroup(tmp3, i)) {
		tmp.Add(Base64Decode(tmp3));
	}
	while (MoosIni->GetNextGroup(tmp3, i)) {
		tmp.Add(Base64Decode(tmp3));
	}
	MoosIni->SetPath(wxEmptyString);
	tmp.Sort();
	InputDialog *tmp2=new InputDialog(this, LangIni, Font.GetChosenFont(), LangIni->Read(wxT("translations/menus/moos/loginas"), wxT("Login as...")),
									  LangIni->Read(wxT("translations/dialogtext/selectusername"), wxT("Please select an username")), 2,
									  LoginName, tmp);
	if (tmp2->ShowModal()==wxID_OK) {
		wxString User=tmp2->GetValue(), Password=Base64Decode(MoosIni->Read(wxT("accounts/")+Base64Encode(User)+wxT("/password")), true);
		if (!MoosIni->Exists(wxT("accounts/")+Base64Encode(User)+wxT("/password"))) {
			InputDialog *tmp=new InputDialog(this, LangIni, Font.GetChosenFont(),
											 LangIni->Read(wxT("translations/menus/moos/loginas"), wxT("Login as...")),
											 LangIni->Read(wxT("translations/dialogtext/enterpassword"), wxT("Please enter password")), 1);
			if (tmp->ShowModal()!=wxID_OK) {
				return;
			}
			Password=tmp->GetValue();
		}
		if (Socket->IsConnected()) {
			Message(LangIni->Read(wxT("translations/channel/events/userleftearthnet"),
								  wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%0><0xA050FFFF> left channel")), LoginName);
		}
		LoginAs(User, Password, MoosIni->Read(wxT("accounts/")+Base64Encode(User)+wxT("/server"), wxT("netserver.earth2150.com")),
				MoosIni->Read(wxT("server/")+MoosIni->Read(wxT("accounts/")+Base64Encode(User)+wxT("/server"),
							  wxT("netserver.earth2150.com"))+wxT("/port"), wxT("17171")));
		ChatInput->SetFocus();
	}
}

void MainFrame::OnLogout(wxCommandEvent & WXUNUSED(event)) {
	RemoveAllChannelsAndUsers();
	if (Socket->IsConnected()) {
		Message(LangIni->Read(wxT("translations/channel/events/userleftearthnet"),
							  wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%0><0xA050FFFF> left channel")), LoginName);
		Socket->Close();
	}
	SetStatusText(LangIni->Read(wxT("translations/statusbar/notconnected"), wxT("Not connected")));
	ChatInput->SetFocus();
}

void MainFrame::OnExit(wxCommandEvent & WXUNUSED(event)) {
	Close(true);
}
