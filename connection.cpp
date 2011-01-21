#include "base.h"

#include "enums.h"
#include "infodialog.h"
#include "tools.h"
#include "whoisdialog.h"
#include <wx/menu.h>
#include <wx/sound.h>
#include <wx/socket.h>
#include <wx/fileconf.h>
#include <zlib.h>

void MainFrame::OnSocketEvent(wxSocketEvent& event) {
	switch (event.GetSocketEvent()) {
	case wxSOCKET_CONNECTION:
		return OnConnect();
	case wxSOCKET_INPUT:
		return OnIncomingData();
	case wxSOCKET_LOST:
		return OnConnectionLost();
	case wxSOCKET_OUTPUT:
		break;
	}
	return;
}

void MainFrame::OnConnectionLost() {
	SetStatusText(LangIni->Read(wxT("translations/statusbar/notconnected"), wxT("Not connected")));
	RemoveAllChannelsAndUsers();
	if (UserList->GetCount()) {
		Message(LangIni->Read(wxT("translations/channel/events/userleftearthnet"),wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%0> <0xA050FFFF>left channel")), LoginName);
	}
	if (Socket->IsConnected()) {
		Socket->Close();
	}
	if (InfoDialog(this, LangIni, Font.GetChosenFont(), wxT("moos2.1"), LangIni->Read(wxT("translations/dialogtext/reconnect"), wxT("Connection lost. Try to reconnect?")), 1).ShowModal()==wxID_YES) {
		LoginAs(LoginName, LoginPassword, MoosIni->Read(wxT("accounts/")+Base64Encode(LoginName)+wxT("/server"), wxT("netserver.earth2150.com")),
				MoosIni->Read(wxT("accounts/")+Base64Encode(LoginName)+wxT("/port"), wxT("17171")));
	}
}

void MainFrame::OnConnect() {
	SetStatusText(LangIni->Read(wxT("translations/statusbar/logginin"), wxT("Logging in...")));
	static const std::string IdentTMP=std::string("\x24\x00\x00\x00\x78\x9c\xeb\x4d\xaa\x7b\xb5\xf3\x5f\x91\xef\xc6\x86\x2d\x7f\x27\xe6\xf6\x5e\x65\x66\x60\x60\x70\x77\x0d\x02\x00\x9c\x04\x0a\xf1", 36);

	std::string LoginPacket=GenerateLoginMessage(LoginName, LoginPassword);
	Socket->Write(IdentTMP.c_str(), IdentTMP.size());
	Socket->Write(LoginPacket.c_str(), LoginPacket.length());
}

void MainFrame::OnIncomingData() {
	char inputbuffer[1024];
	unsigned int inputbufferlen=1024, i, j;
	char *buffer=new char[0]; //for delete
	unsigned int bufferlen=0;

	while (inputbufferlen==1024) {
		Socket->Read(inputbuffer, 1024);
		inputbufferlen=Socket->LastCount();
		char *tmp=new char[bufferlen+inputbufferlen];
		for (i=0;i!=bufferlen;++i) tmp[i]=buffer[i];
		for (i=0;i!=inputbufferlen;++i) tmp[bufferlen+i]=inputbuffer[i];
		delete[] buffer;
		buffer=tmp;
		bufferlen+=inputbufferlen;
	}

	a:
	if (buffer[0]=='/' || buffer[0]=='$') {
		std::string command="";
		for (i=0;i!=bufferlen;++i) {
			if (!buffer[i]) {
				ParseProcessCommand(wxConvLibc.cMB2WC(command.c_str()));
				char *tmp=new char[bufferlen-(i+1)];
				for (j=i+1;j!=bufferlen;++j) {
					tmp[j-(i+1)]=buffer[j];
				}
				bufferlen-=(i+1);
				delete[] buffer;
				buffer=tmp;
				goto a;
			} else {
				command+=buffer[i];
			}
		}
		delete[] buffer;
		return;
	} else { //compressed data
		unsigned int packetlen=*((unsigned int*)buffer);
		char cbuffer[10240];
		unsigned long cbufferlen=10240;
		if (uncompress((Bytef*)cbuffer, &cbufferlen, (const Bytef*)(buffer+4), packetlen-4)==Z_OK) {
			unsigned int packettype=*((unsigned int*)cbuffer);
			unsigned int contentlen=*((unsigned int*)(cbuffer+4));
			wxString content;
			if (contentlen>packetlen) {
				contentlen=packetlen-8;
			}
			for (i=0;i!=contentlen;++i) {
				content+=cbuffer[i+8];
			}
			if (packettype==0) { //Success
				unsigned int stringlen=*((unsigned int*)(cbuffer+8));
				if (stringlen>contentlen) {
					//Aha das ist also dieser komische Ident
				} else { //Welcome msg
					wxString identifier;
					wxString welcomemsg;

					for (i=0;i!=stringlen;++i) {
						identifier+=cbuffer[i+12];
					}
					stringlen=*((unsigned int*)(cbuffer+12+identifier.Len()));
					for (i=0;i!=stringlen;i++) {
						welcomemsg+=cbuffer[i+16+identifier.Len()];
					}
					wxIPV4address tmp;
					Socket->GetPeer(tmp);
					SetStatusText(Format(LangIni->Read(wxT("translations/statusbar/connected"), wxT("Connected with <%0> (<%1>)")), tmp.Hostname(), tmp.IPAddress()));
					//detect Game Versions
					unsigned int vdx=16+identifier.Len()+welcomemsg.Len()+44;
					unsigned int versionlen=0;
					GameVersion.clear();
					while (~cbuffer[vdx]) {
						version tmp;
						tmp.ID=cbuffer[vdx];
						versionlen=*((unsigned int*)(cbuffer+vdx+1));
						for (unsigned int idv=0;idv!=versionlen;++idv) {
							tmp.Name+=cbuffer[vdx+5+idv];
						}
						GameVersion.push_back(tmp);
						vdx+=5+versionlen;
					}
					if (identifier==wxT("EarthNet Beta Server")) {
						welcomemsg.Replace(wxT("<*>"), wxT("<0xFFFFFFFF>"));
						Message(welcomemsg);
						AddUser(LoginName);
						RefreshAutocomplete(LoginName);
						MoosIni->Write(wxT("accounts/")+Base64Encode(LoginName)+wxT("/password"), Base64Encode(LoginPassword, true));
					}
				}
			} else if (packettype==2) { //Error
				if (content==wxT("translateInvalidCharactersInName")) {
					InfoDialog(this, LangIni, Font.GetChosenFont(), wxT("moos2.1"), LangIni->Read(wxT("translations/dialogtext/illegalusername"),
							   wxT("Error: Your user name contains invalid characters"))).ShowModal();
					MoosIni->DeleteGroup(wxT("accounts/")+Base64Encode(LoginName));
					LoginName=wxEmptyString;
				} else if (content==wxT("translateInvalidUserName")) {
					InfoDialog(this, LangIni, Font.GetChosenFont(), wxT("moos2.1"),
							   LangIni->Read(wxT("translations/dialogtext/userdoesntexist"), wxT("Error: Invalid user name"))).ShowModal();
					MoosIni->DeleteGroup(wxT("accounts/")+Base64Encode(LoginName));
					LoginName=wxEmptyString;
				} else if (content==wxT("translateInvalidPassword")) {
					InfoDialog(this, LangIni, Font.GetChosenFont(), wxT("moos2.1"),
							   LangIni->Read(wxT("translations/dialogtext/wrongpassword"), wxT("Error: Invalid password"))).ShowModal();
					MoosIni->DeleteEntry(wxT("accounts/")+Base64Encode(LoginName)+wxT("/password"), false);
				} else if (content==wxT("translateAlreadyLogIn")) {
					MoosIni->Write(wxT("accounts/")+Base64Encode(LoginName)+wxT("/password"), Base64Encode(LoginPassword, true));
					InfoDialog(this, LangIni, Font.GetChosenFont(), wxT("moos2.1"),
							   LangIni->Read(wxT("translations/dialogtext/allreadyloggedin"), wxT("Error: The user is already logged in"))).ShowModal();
				} else {
					InfoDialog(this, LangIni, Font.GetChosenFont(), wxT("moos2.1"), content).ShowModal();
					MoosIni->DeleteGroup(wxT("accounts/")+Base64Encode(LoginName));
					LoginName=wxEmptyString;
				}
				SetStatusText(LangIni->Read(wxT("translations/statusbar/notconnected"), wxT("Not connected")));
				if (Socket->IsConnected()) {
					Socket->Close();
				}
			}
		}

		if (bufferlen<=packetlen) {
			delete[] buffer;
			bufferlen=0;
			return;
		}
		char *tmp=new char[bufferlen-packetlen];
		for (i=packetlen;i!=bufferlen;++i) tmp[i-packetlen]=buffer[i];
		bufferlen-=packetlen;
		delete[] buffer;
		buffer=tmp;
		goto a;
	}
}

void MainFrame::ParseProcessCommand(wxString strData) {
	wxArrayString parsed;
	size_t i=0, start;
	a:
	if (strData[i]=='\"') {
		start=++i;
		while (strData[i]!='\"' && i!=strData.Len()) i++;
		parsed.Add(strData.Mid(start, i-start));
		i++;
	} else {
		start=i;
		while (strData[i]!=' ' && i!=strData.Len()) i++;
		parsed.Add(strData.Mid(start, i-start));
	}
	while (strData[i]==' ' && i!=strData.Len()) i++;
	if (i!=strData.Len()) goto a;

	//process command
	if (!parsed.GetCount()) return;

	// ----------- Chat
	if (parsed[0]==wxT("/send")) {
		RefreshAutocomplete(parsed[1]);
		parsed[2].Replace(wxT("\n"), wxEmptyString);
		parsed[2].Replace(wxT("\t"), wxEmptyString);
		if (!IsIgnored(parsed[1]) && !(SettingsMenu->IsChecked(ID_MAINWIN_DISABLE_SLAPS)
									   && parsed[2].Find(wxT("<0xFF0000FF>slaps <0xFF2153E8>"))!=-1 && parsed[2].Find(wxT("<0xFF0000FF> with "))!=-1))
			Message(LangIni->Read(wxT("translations/channel/message"), wxT("<%0>: <%1>")), parsed[1], parsed[2]);
	} else if (parsed[0]==wxT("/msg")) {
		RefreshAutocomplete(parsed[2]);
		if (IsIgnored(parsed[2])) return;
		parsed[3].Replace(wxT("\n"), wxEmptyString);
		parsed[3].Replace(wxT("\t"), wxEmptyString);
		Message(LangIni->Read(wxT("translations/channel/whispermessage"), wxT("<0xFFFFFFFF>[<%1> (<%0>)]: <%3>")), parsed[1], parsed[2], parsed[3], parsed[4]);
		if (IsIconized() && parsed[3][0]!='#' && parsed[3][0]!='$') RequestUserAttention();
	} else if (parsed[0]==wxT("/msgc")) {
		RefreshAutocomplete(parsed[1]);
		Message(LangIni->Read(wxT("translations/channel/whispermessageto"), wxT("<0xFFFFFFFF>->[<%0>]: <%1>")), parsed[1], parsed[2]);
	} else if (parsed[0]==wxT("/beep") && !IsIgnored(parsed[1])) {
		RefreshAutocomplete(parsed[2]);
		if (IsIgnored(parsed[2])) return;
		Message(LangIni->Read(wxT("translations/channel/events/beepmessage"), wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%1><0xA050FFFF> (<%0>) beeps you")), parsed[1], parsed[2]);
		if (SettingsMenu->IsChecked(ID_MAINWIN_SOUND_ON_BEEP)) {
			wxSound::Play(MoosIni->Read(wxT("beepsound")));
		}
	} else if (parsed[0]==wxT("/beepc")) {
		RefreshAutocomplete(parsed[1]);
		Message(LangIni->Read(wxT("translations/channel/events/beepmessageto")), parsed[1]);
		if (SettingsMenu->IsChecked(ID_MAINWIN_SOUND_ON_BEEP)) {
			wxSound::Play(MoosIni->Read(wxT("beepsound")));
		}
	}
	// ----------- User
	else if (parsed[0]==wxT("$user")) {
		if (parsed[1][0]=='@') return;
		RefreshAutocomplete(parsed[1]);
		AddUser(parsed[1]);
	} else if (parsed[0]==wxT("/$user")) {
		RefreshAutocomplete(parsed[1]);
		AddUser(parsed[1]);
		if (IsIgnored(parsed[1])) return;
		if (parsed[3]!=wxEmptyString)
			Message(LangIni->Read(wxT("translations/channel/events/userenterchannel"), wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%0><0xA050FFFF> (<%1>) (comming from: <%2>) enter channel")), parsed[1], GetVersion(parsed[2]), parsed[3]);
		else
			Message(LangIni->Read(wxT("translations/channel/events/userenterearthnet"), wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%0><0xA050FFFF> (<%1>) enter channel")), parsed[1], GetVersion(parsed[2]));
	} else if (parsed[0]==wxT("/&user")) {
		RefreshAutocomplete(parsed[1], false);
		RemoveUser(parsed[1]);
		if (IsIgnored(parsed[1])) return;
		if (parsed[2][0]=='$') Message(LangIni->Read(wxT("translations/channel/events/userjoinedgame"), wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%0><0xA050FFFF> has left channel and joined <0xFF80FFFF>'<%1>'<0xA050FFFF> game")), parsed[1], parsed[2].Mid(1));
		else if (parsed[2][0]=='#') Message(LangIni->Read(wxT("translations/channel/events/userswitchedchannel"), wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%0><0xA050FFFF> has left channel and joined <0xFF80FFFF>'<%1>'<0xA050FFFF> channel")), parsed[1], parsed[2].Mid(1));
		else Message(LangIni->Read(wxT("translations/channel/events/userleftearthnet"), wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%0><0xA050FFFF> left channel")), parsed[1]);
		// ----------- Channel
	} else if (parsed[0]==wxT("$channel")) {
		AddChannel(parsed[1]);
	} else if (parsed[0]==wxT("/$channel")) {
		AddChannel(parsed[1]);
	} else if (parsed[0]==wxT("/&channel")) {
		RemoveChannel(parsed[1]);
	} else if (parsed[0]==wxT("/join")) {
		RefreshAutocomplete(LoginName);
		RemoveAllUsers();
		AddUser(LoginName);
		Message(LangIni->Read(wxT("translations/channel/events/yourselfjoinedchannel"), wxT("<0xA050FFFF>[<%T>] <0xFF80FFFF><%0><0xA050FFFF> enter channel")), LoginName, parsed[1]);
		SetChannel(parsed[1]);
		// ----------- Games (only TMP 1.3)
	} else if (parsed[0]==wxT("$play")) {
	} else if (parsed[0]==wxT("/$play")) {
	} else if (parsed[0]==wxT("/&play")) {
		// ----------- Other
	} else if (parsed[0]==wxT("/whois")) {
		RefreshAutocomplete(parsed[1]);
		WhoisDialog *tmp=new WhoisDialog(this, LangIni, parsed, LoginName==parsed[1], Socket, Font.GetChosenFont());
		tmp->Show();
	} else if (parsed[0]==wxT("/error")) {
		if (parsed[1]==wxT("notLoggedIn")) Message(LangIni->Read(wxT("translations/channel/errors/usernotloggedin"), wxT("<0xFF0000FF>Error for <%0> (not logged in)")), parsed[2]);
		else if (parsed[1]==wxT("notExist")) Message(LangIni->Read(wxT("translations/channel/errors/userdoesntexist"), wxT("<0xFF0000FF>Error for <%0> (does not exist)")), parsed[2]);
		else if (parsed[1]==wxT("invalidChannelName")) Message(LangIni->Read(wxT("translations/channel/errors/invalidchannelname"), wxT("<0xFF0000FF>Error for <%0> (invalid channel name)")), parsed[2]);
		else if (parsed[1]==wxT("invalidGameName")) Message(LangIni->Read(wxT("translations/channel/errors/invalidgamename"), wxT("<0xFF0000FF>Invalid game name <%0>")), parsed[2]);
		else if (parsed[1]==wxT("notAvailable")) Message(LangIni->Read(wxT("translations/channel/errors/gamenotavailable"), wxT("<0xFF0000FF>Error for <%0> (not available)")), parsed[2]);
		else if (parsed[1]==wxT("Message")) Message(LangIni->Read(wxT("translations/channel/errors/messagetoolong"), wxT("<0xFF0000FF>Message too long")));
		else {
			wxString tmp;
			for (size_t i=1;i!=parsed.GetCount();++i) tmp+=parsed[i]+wxT(" ");
			Message(Format(LangIni->Read(wxT("translations/channel/errors/other"), wxT("<0xFF0000FF><%0>")), tmp));
		}
	} else if (parsed[0]==wxT("/admin")) {
		wxString tmp;
		for (size_t i=1;i!=parsed.GetCount();++i) {
			tmp+=parsed[i]+wxT(" ");
		}
		Message(LangIni->Read(wxT("translations/channel/events/adminmessage"), wxT("<0xFF00FFFF>[Admin]: <%0>")), tmp);
		if (parsed.GetCount()==8 && parsed[1]==wxT("User") && parsed[2]==LoginName+wxT("'s") && parsed[3]==wxT("is")
				&& parsed[4]==wxT("now") && parsed[5]==wxT("known") && parsed[6]==wxT("as")) {
			MoosIni->SetPath(wxT("accounts/"));
			MoosIni->RenameGroup(Base64Encode(LoginName), Base64Encode(parsed[7]));
			MoosIni->SetPath(wxEmptyString);
			LoginName=parsed[7];
		}
	} else if (parsed[0]==wxT("/syncstats")) {
	} else if (parsed[0]==wxT("/nop")) {
		Message(LangIni->Read(wxT("translations/channel/errors/doublelogin"), wxT("<0xFF0000FF>Somebody tried to log in with your name")));
	} else if (parsed[0]==wxT("/info")) {
		Message(parsed[1]);
	} else {
		Message(parsed[0]);
	}
}

wxString MainFrame::GetVersion(wxString ID) {
	long tmp;
	ID.ToLong(&tmp);
	for (size_t i=0;i!=GameVersion.size();++i) if (GameVersion[i].ID==tmp) return GameVersion[i].Name;
	return wxT("Earth 2150");
}

std::string MainFrame::EncodeMessage(const std::string Message) {
	unsigned char buffer[1024];
	std::string sCompressed;
	unsigned long nLength=1024;
	compress((Bytef*)buffer, &nLength, (const Bytef*)Message.c_str(), Message.length());
	for (unsigned int i=0;i!=nLength;++i) sCompressed+=buffer[i];
	return sCompressed;
}

std::string MainFrame::GetInvertedLength(const unsigned int &Length) {
	std::string sLength;
	sLength+=*(((char*)&Length));
	sLength+=*(((char*)&Length)+1);
	sLength+=*(((char*)&Length)+2);
	sLength+=*(((char*)&Length)+3);
	return sLength;
}

std::string MainFrame::GenerateLoginMessage(wxString User, wxString Password) {
	std::string sLogin=GetInvertedLength(User.Len());
	sLogin+=User.mb_str();
	sLogin+=GetInvertedLength(Password.Len());
	sLogin+=Password.mb_str();
	sLogin+=GetInvertedLength(0);
	sLogin+=GetInvertedLength(0);
	sLogin=EncodeMessage(sLogin);
	sLogin=GetInvertedLength(sLogin.length()+4)+sLogin;
	return sLogin;
}

void MainFrame::Write(wxString Data) {
	if (Socket->IsConnected()) Socket->Write(Data.mb_str(), Data.length()+1);
}

void MainFrame::LoginAs(wxString User, wxString Password, wxString Server, wxString Port) {
	RemoveAllChannelsAndUsers();
	if (Socket->IsConnected()) Socket->Close();
	LoginName=User;
	LoginPassword=Password;
	SettingsMenu->Check(ID_MAINWIN_AUTOLOGIN_ACCOUNT, Base64Encode(LoginName)==MoosIni->Read(wxT("autologin")));

	wxIPV4address Addr;
	Addr.Hostname(Server);
	Addr.Service(Port);
	SetStatusText(LangIni->Read(wxT("translations/statusbar/connecting"), wxT("Connecting to server...")));
	Socket->Connect(Addr, false);
}
