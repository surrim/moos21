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
	SetStatusText(langIni->Read(wxT("translations/statusbar/notconnected"), wxT("Not connected")));
	RemoveAllChannelsAndUsers();
	if (userList->GetCount()) {
		Message(langIni->Read(wxT("translations/channel/events/userleftearthnet"), wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%0> <0xA050FFFF>left channel")), loginName);
	}
	if (socket->IsConnected()) {
		socket->Close();
	}
	if (InfoDialog(this, langIni, font.GetChosenFont(), wxT("moos2.1"), langIni->Read(wxT("translations/dialogtext/reconnect"), wxT("Connection lost. Try to reconnect?")), 1).ShowModal()==wxID_YES) {
		loginAs(loginName, loginPassword, moosIni->Read(wxT("accounts/")+Base64Encode(loginName)+wxT("/server"), wxT("netserver.earth2150.com")),
				moosIni->Read(wxT("accounts/")+Base64Encode(loginName)+wxT("/port"), wxT("17171")));
	}
}

void MainFrame::OnConnect() {
	SetStatusText(langIni->Read(wxT("translations/statusbar/logginin"), wxT("Logging in...")));
	static const std::string identTMP=std::string("\x24\x00\x00\x00\x78\x9c\xeb\x4d\xaa\x7b\xb5\xf3\x5f\x91\xef\xc6\x86\x2d\x7f\x27\xe6\xf6\x5e\x65\x66\x60\x60\x70\x77\x0d\x02\x00\x9c\x04\x0a\xf1", 36);

	std::string loginPacket=generateLoginMessage(loginName, loginPassword);
	socket->Write(identTMP.c_str(), identTMP.size());
	socket->Write(loginPacket.c_str(), loginPacket.length());
}

void MainFrame::OnIncomingData() {
	char inputbuffer[1024];
	unsigned int inputbufferlen=1024, i, j;
	char *buffer=new char[0]; //for delete
	unsigned int bufferlen=0;

	while (inputbufferlen==1024) {
		socket->Read(inputbuffer, 1024);
		inputbufferlen=socket->LastCount();
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
				parseProcessCommand(wxConvLibc.cMB2WC(command.c_str()));
				char *tmp=new char[bufferlen-(i+1)];
				for (j=i+1;j!=bufferlen;j++) {
					tmp[j-(i+1)]=buffer[j];
				}
				bufferlen-=i+1;
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
			std::string content;
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
					std::string identifier;
					std::string welcomemsg;

					for (i=0;i!=stringlen;++i) {
						identifier+=cbuffer[i+12];
					}
					stringlen=*((unsigned int*)(cbuffer+12+identifier.size()));
					for (i=0;i!=stringlen;i++) {
						welcomemsg+=cbuffer[i+16+identifier.size()];
					}
					wxIPV4address tmp;
					socket->GetPeer(tmp);
					SetStatusText(Format(langIni->Read(wxT("translations/statusbar/connected"), wxT("Connected with <%0> (<%1>)")), tmp.Hostname(), tmp.IPAddress()));
					//detect Game Versions
					unsigned int vdx=16+identifier.size()+welcomemsg.size()+44;
					unsigned int versionlen=0;
					gameVersions.clear();
					while (~cbuffer[vdx]) {
						version tmp;
						tmp.id=cbuffer[vdx];
						versionlen=*((unsigned int*)(cbuffer+vdx+1));
						for (unsigned int idv=0;idv!=versionlen;++idv) {
							tmp.name+=cbuffer[vdx+5+idv];
						}
						gameVersions.push_back(tmp);
						vdx+=5+versionlen;
					}

					while (true) {
						unsigned int foundPosition=welcomemsg.find("<*>");
						if (foundPosition!=std::string::npos) {
							welcomemsg=welcomemsg.substr(0, foundPosition)+"<0xFFFFFFFF>"+welcomemsg.substr(foundPosition+3);
						} else {
							break;
						}
					}

					Message(welcomemsg);
					AddUser(loginName);
					RefreshAutocomplete(loginName);
					moosIni->Write(wxT("accounts/")+Base64Encode(loginName)+wxT("/password"), Base64Encode(loginPassword, true));
				}
			} else if (packettype==2) { //Error
				if (content==wxT("translateInvalidCharactersInName")) {
					InfoDialog(this, langIni, font.GetChosenFont(), wxT("moos2.1"), langIni->Read(wxT("translations/dialogtext/illegalusername"),
							   wxT("Error: Your user name contains invalid characters"))).ShowModal();
					moosIni->DeleteGroup(wxT("accounts/")+Base64Encode(loginName));
					loginName=wxEmptyString;
				} else if (content==wxT("translateInvalidUserName")) {
					InfoDialog(this, langIni, font.GetChosenFont(), wxT("moos2.1"),
							   langIni->Read(wxT("translations/dialogtext/userdoesntexist"), wxT("Error: Invalid user name"))).ShowModal();
					moosIni->DeleteGroup(wxT("accounts/")+Base64Encode(loginName));
					loginName=wxEmptyString;
				} else if (content==wxT("translateInvalidPassword")) {
					InfoDialog(this, langIni, font.GetChosenFont(), wxT("moos2.1"),
							   langIni->Read(wxT("translations/dialogtext/wrongpassword"), wxT("Error: Invalid password"))).ShowModal();
					moosIni->DeleteEntry(wxT("accounts/")+Base64Encode(loginName)+wxT("/password"), false);
				} else if (content==wxT("translateAlreadyLogIn")) {
					moosIni->Write(wxT("accounts/")+Base64Encode(loginName)+wxT("/password"), Base64Encode(loginPassword, true));
					InfoDialog(this, langIni, font.GetChosenFont(), wxT("moos2.1"),
							   langIni->Read(wxT("translations/dialogtext/allreadyloggedin"), wxT("Error: The user is already logged in"))).ShowModal();
				} else {
					InfoDialog(this, langIni, font.GetChosenFont(), wxT("moos2.1"), content).ShowModal();
					moosIni->DeleteGroup(wxT("accounts/")+Base64Encode(loginName));
					loginName=wxEmptyString;
				}
				SetStatusText(langIni->Read(wxT("translations/statusbar/notconnected"), wxT("Not connected")));
				if (socket->IsConnected()) {
					socket->Close();
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

void MainFrame::parseProcessCommand(const wxString& strData) {
	wxArrayString parsed;
	size_t i=0, start;
	a:
	if (strData[i]=='\"') {
		start=++i;
		while (strData[i]!='\"' && i!=strData.Len()) {
			i++;
		}
		parsed.Add(strData.Mid(start, i-start));
		i++;
	} else {
		start=i;
		while (strData[i]!=' ' && i!=strData.Len()) {
			i++;
		}
		parsed.Add(strData.Mid(start, i-start));
	}
	while (strData[i]==' ' && i!=strData.Len()) i++;
	if (i!=strData.Len()) {
		goto a;
	}
	//process command
	if (!parsed.GetCount()) return;

	// ----------- Chat
	if (parsed[0]==wxT("/send")) {
		RefreshAutocomplete(parsed[1]);
		parsed[2].Replace(wxT("\n"), wxEmptyString);
		parsed[2].Replace(wxT("\t"), wxEmptyString);
		if (
			!IsIgnored(parsed[1])
			&&
			!(settingsMenu->IsChecked(ID_MAINWIN_DISABLE_SLAPS)
			&&
			parsed[2].Find(wxT("<0xFF0000FF>slaps <0xFF2153E8>"))!=-1
			&&
			parsed[2].Find(wxT("<0xFF0000FF> with "))!=-1)
		) {
			Message(langIni->Read(wxT("translations/channel/message"), wxT("<%0>: <%1>")), parsed[1], parsed[2]);
		}
	} else if (parsed[0]==wxT("/msg")) {
		RefreshAutocomplete(parsed[2]);
		if (IsIgnored(parsed[2])) {
			return;
		}
		parsed[3].Replace(wxT("\n"), wxEmptyString);
		parsed[3].Replace(wxT("\t"), wxEmptyString);
		Message(langIni->Read(wxT("translations/channel/whispermessage"), wxT("<0xFFFFFFFF>[<%1> (<%0>)]: <%3>")), parsed[1], parsed[2], parsed[3], parsed[4]);
		if (IsIconized() && parsed[3][0]!='#' && parsed[3][0]!='$') {
			RequestUserAttention();
		}
	} else if (parsed[0]==wxT("/msgc")) {
		RefreshAutocomplete(parsed[1]);
		Message(langIni->Read(wxT("translations/channel/whispermessageto"), wxT("<0xFFFFFFFF>->[<%0>]: <%1>")), parsed[1], parsed[2]);
	} else if (parsed[0]==wxT("/beep") && !IsIgnored(parsed[1])) {
		RefreshAutocomplete(parsed[2]);
		if (IsIgnored(parsed[2])) {
			return;
		}
		Message(langIni->Read(wxT("translations/channel/events/beepmessage"), wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%1><0xA050FFFF> (<%0>) beeps you")), parsed[1], parsed[2]);
		if (settingsMenu->IsChecked(ID_MAINWIN_SOUND_ON_BEEP)) {
			wxSound::Play(moosIni->Read(wxT("beepsound")));
		}
	} else if (parsed[0]==wxT("/beepc")) {
		RefreshAutocomplete(parsed[1]);
		Message(langIni->Read(wxT("translations/channel/events/beepmessageto")), parsed[1]);
		if (settingsMenu->IsChecked(ID_MAINWIN_SOUND_ON_BEEP)) {
			wxSound::Play(moosIni->Read(wxT("beepsound")));
		}
	}
	// ----------- User
	else if (parsed[0]==wxT("$user")) {
		if (parsed[1][0]=='@') {
			return;
		}
		RefreshAutocomplete(parsed[1]);
		AddUser(parsed[1]);
	} else if (parsed[0]==wxT("/$user")) {
		RefreshAutocomplete(parsed[1]);
		AddUser(parsed[1]);
		if (IsIgnored(parsed[1])) {
			return;
		}
		if (parsed[3]!=wxEmptyString) {
			Message(langIni->Read(wxT("translations/channel/events/userenterchannel"), wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%0><0xA050FFFF> (<%1>) (comming from: <%2>) enter channel")), parsed[1], getVersion(parsed[2]), parsed[3]);
		} else {
			Message(langIni->Read(wxT("translations/channel/events/userenterearthnet"), wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%0><0xA050FFFF> (<%1>) enter channel")), parsed[1], getVersion(parsed[2]));
		}
	} else if (parsed[0]==wxT("/&user")) {
		RefreshAutocomplete(parsed[1], false);
		RemoveUser(parsed[1]);
		if (IsIgnored(parsed[1])) {
			return;
		}
		if (parsed[2][0]=='$') {
			Message(langIni->Read(wxT("translations/channel/events/userjoinedgame"), wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%0><0xA050FFFF> has left channel and joined <0xFF80FFFF>'<%1>'<0xA050FFFF> game")), parsed[1], parsed[2].Mid(1));
		} else if (parsed[2][0]=='#') {
			Message(langIni->Read(wxT("translations/channel/events/userswitchedchannel"), wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%0><0xA050FFFF> has left channel and joined <0xFF80FFFF>'<%1>'<0xA050FFFF> channel")), parsed[1], parsed[2].Mid(1));
		} else {
			Message(langIni->Read(wxT("translations/channel/events/userleftearthnet"), wxT("<0xA050FFFF>[<%T>] User <0xFF80FFFF><%0><0xA050FFFF> left channel")), parsed[1]);
		}
		// ----------- Channel
	} else if (parsed[0]==wxT("$channel")) {
		AddChannel(parsed[1]);
	} else if (parsed[0]==wxT("/$channel")) {
		AddChannel(parsed[1]);
	} else if (parsed[0]==wxT("/&channel")) {
		RemoveChannel(parsed[1]);
	} else if (parsed[0]==wxT("/join")) {
		RefreshAutocomplete(loginName);
		RemoveAllUsers();
		AddUser(loginName);
		Message(langIni->Read(wxT("translations/channel/events/yourselfjoinedchannel"), wxT("<0xA050FFFF>[<%T>] <0xFF80FFFF><%0><0xA050FFFF> enter channel")), loginName, parsed[1]);
		SetChannel(parsed[1]);
		// ----------- Games (only TMP 1.3)
	} else if (parsed[0]==wxT("$play")) {
	} else if (parsed[0]==wxT("/$play")) {
	} else if (parsed[0]==wxT("/&play")) {
		// ----------- Other
	} else if (parsed[0]==wxT("/whois")) {
		RefreshAutocomplete(parsed[1]);
		WhoisDialog *tmp=new WhoisDialog(this, langIni, parsed, loginName==parsed[1], socket, font.GetChosenFont());
		tmp->Show();
	} else if (parsed[0]==wxT("/error")) {
		if (parsed[1]==wxT("notLoggedIn")) {
			Message(langIni->Read(wxT("translations/channel/errors/usernotloggedin"), wxT("<0xFF0000FF>Error for <%0> (not logged in)")), parsed[2]);
		} else if (parsed[1]==wxT("notExist")) {
			Message(langIni->Read(wxT("translations/channel/errors/userdoesntexist"), wxT("<0xFF0000FF>Error for <%0> (does not exist)")), parsed[2]);
		} else if (parsed[1]==wxT("invalidChannelName")) {
			Message(langIni->Read(wxT("translations/channel/errors/invalidchannelname"), wxT("<0xFF0000FF>Error for <%0> (invalid channel name)")), parsed[2]);
		} else if (parsed[1]==wxT("invalidGameName")) {
			Message(langIni->Read(wxT("translations/channel/errors/invalidgamename"), wxT("<0xFF0000FF>Invalid game name <%0>")), parsed[2]);
		} else if (parsed[1]==wxT("notAvailable")) {
			Message(langIni->Read(wxT("translations/channel/errors/gamenotavailable"), wxT("<0xFF0000FF>Error for <%0> (not available)")), parsed[2]);
		} else if (parsed[1]==wxT("Message")) {
			Message(langIni->Read(wxT("translations/channel/errors/messagetoolong"), wxT("<0xFF0000FF>Message too long")));
		} else {
			wxString tmp;
			for (size_t i=1;i!=parsed.GetCount();i++) {
				tmp+=parsed[i]+wxT(" ");
			}
			Message(Format(langIni->Read(wxT("translations/channel/errors/other"), wxT("<0xFF0000FF><%0>")), tmp));
		}
	} else if (parsed[0]==wxT("/admin")) {
		wxString tmp;
		for (size_t i=1;i!=parsed.GetCount();i++) {
			tmp+=parsed[i]+wxT(" ");
		}
		Message(langIni->Read(wxT("translations/channel/events/adminmessage"), wxT("<0xFF00FFFF>[Admin]: <%0>")), tmp);
		if (
			parsed.GetCount()==8
			&&
			parsed[1]==wxT("User")
			&&
			parsed[2]==loginName+wxT("'s")
			&&
			parsed[3]==wxT("is")
			&&
			parsed[4]==wxT("now")
			&&
			parsed[5]==wxT("known")
			&&
			parsed[6]==wxT("as")
		) {
			moosIni->SetPath(wxT("accounts/"));
			moosIni->RenameGroup(Base64Encode(loginName), Base64Encode(parsed[7]));
			moosIni->SetPath(wxEmptyString);
			loginName=parsed[7];
		}
	} else if (parsed[0]==wxT("/syncstats")) {
	} else if (parsed[0]==wxT("/nop")) {
		Message(langIni->Read(wxT("translations/channel/errors/doublelogin"), wxT("<0xFF0000FF>Somebody tried to log in with your name")));
	} else if (parsed[0]==wxT("/info")) {
		Message(parsed[1]);
	} else {
		Message(parsed[0]);
	}
}

wxString MainFrame::getVersion(const wxString& id) {
	long tmp;
	id.ToLong(&tmp);
	for (size_t i=0;i!=gameVersions.size();i++) {
		if (gameVersions[i].id==tmp) {
			return gameVersions[i].name;
		}
	}
	return wxT("Earth 2150");
}

std::string MainFrame::encodeMessage(const std::string& message) {
	unsigned char buffer[1024];
	unsigned long length=1024;
	compress((Bytef*)buffer, &length, (const Bytef*)message.c_str(), message.size());
	std::string _;
	for (unsigned int i=0;i!=length;i++) {
		_+=buffer[i];
	}
	return _;
}

std::string MainFrame::getInvertedLength(unsigned int length) {
	std::string _;
	_+=*(((char*)&length));
	_+=*(((char*)&length)+1);
	_+=*(((char*)&length)+2);
	_+=*(((char*)&length)+3);
	return _;
}

std::string MainFrame::generateLoginMessage(const wxString& loginUser, const wxString& loginPassword) {
	std::string loginData=getInvertedLength(loginUser.Len());
	loginData+=loginUser.To8BitData();
	loginData+=getInvertedLength(loginPassword.Len());
	loginData+=loginPassword.To8BitData();
	loginData+=getInvertedLength(0);
	loginData+=getInvertedLength(0);
	std::string _=encodeMessage(loginData);
	return getInvertedLength(_.length()+4)+_;
}

void MainFrame::write(const wxString& data) {
	std::string dataToSend;
	if (data.IsAscii()) {
		dataToSend=data.ToAscii();
	} else {
		static const wxCSConv csconv(wxT("iso8859-1"));
		size_t outSize;
		const wxCharBuffer cb=csconv.cWC2MB(data.data(), data.size(), &outSize);
		dataToSend=std::string(cb, outSize);
	}
	if (socket->IsConnected() && !dataToSend.empty()) {
		socket->Write(dataToSend.c_str(), dataToSend.size()+1);
	}
}

void MainFrame::loginAs(const wxString& loginName, const wxString& loginPassword, const wxString& server, const wxString& port) {
	RemoveAllChannelsAndUsers();
	if (socket->IsConnected()) {
		socket->Close();
	}
	this->loginName=loginName;
	this->loginPassword=loginPassword;
	settingsMenu->Check(ID_MAINWIN_AUTOLOGIN_ACCOUNT, Base64Encode(loginName)==moosIni->Read(wxT("autologin")));

	wxIPV4address addr;
	addr.Hostname(server);
	addr.Service(port);
	SetStatusText(langIni->Read(wxT("translations/statusbar/connecting"), wxT("Connecting to server...")));
	socket->Connect(addr, false);
}
