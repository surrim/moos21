#include "base.h"

#include "tools.h"
#include "inputdialog.h"
#include <wx/fileconf.h>
#include <wx/socket.h>

void MainFrame::OnLoginAs(wxCommandEvent & WXUNUSED(event)) {
	wxArrayString tmp;
	long i;
	wxString tmp3;
	moosIni->SetPath(wxT("accounts/"));
	if (moosIni->GetFirstGroup(tmp3, i)) {
		tmp.Add(Base64Decode(tmp3));
	}
	while (moosIni->GetNextGroup(tmp3, i)) {
		tmp.Add(Base64Decode(tmp3));
	}
	moosIni->SetPath(wxEmptyString);
	tmp.Sort();
	InputDialog *tmp2=new InputDialog(this, langIni, font.GetChosenFont(), langIni->Read(wxT("translations/menus/moos/loginas"), wxT("Login as...")),
									  langIni->Read(wxT("translations/dialogtext/selectusername"), wxT("Please select an username")), 2,
									  loginName, tmp);
	if (tmp2->ShowModal()==wxID_OK) {
		wxString User=tmp2->GetValue();
		wxString Password=Base64Decode(moosIni->Read(wxT("accounts/")+Base64Encode(User)+wxT("/password")), true);
		if (!moosIni->Exists(wxT("accounts/")+Base64Encode(User)+wxT("/password"))) {
			InputDialog *tmp=new InputDialog(this, langIni, font.GetChosenFont(),
											 langIni->Read(wxT("translations/menus/moos/loginas"), wxT("Login as...")),
											 langIni->Read(wxT("translations/dialogtext/enterpassword"), wxT("Please enter password")), 1);
			if (tmp->ShowModal()!=wxID_OK) {
				return;
			}
			Password=tmp->GetValue();
		}
		if (socket->IsConnected()) {
			Message(langIni->Read(wxT("translations/channel/events/userleftearthnet"),
								  wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%0><0xA050FFFF> left channel")), loginName);
		}
		loginAs(User, Password, moosIni->Read(wxT("accounts/")+Base64Encode(User)+wxT("/server"), wxT("netserver.earth2150.com")),
				moosIni->Read(wxT("server/")+moosIni->Read(wxT("accounts/")+Base64Encode(User)+wxT("/server"),
							  wxT("netserver.earth2150.com"))+wxT("/port"), wxT("17171")));
		chatInput->SetFocus();
	}
}

void MainFrame::OnLogout(wxCommandEvent & WXUNUSED(event)) {
	RemoveAllChannelsAndUsers();
	if (socket->IsConnected()) {
		Message(langIni->Read(wxT("translations/channel/events/userleftearthnet"),
							  wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%0><0xA050FFFF> left channel")), loginName);
		socket->Close();
	}
	SetStatusText(langIni->Read(wxT("translations/statusbar/notconnected"), wxT("Not connected")));
	chatInput->SetFocus();
}

void MainFrame::OnExit(wxCommandEvent & WXUNUSED(event)) {
	Close(true);
}
