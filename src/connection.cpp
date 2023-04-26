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

#include "base.h"

#include "enums.h"
#include "infodialog.h"
#include "tools.h"
#include "utils.h"
#include "whoisdialog.h"
#include <cinttypes>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/sound.h>
#include <wx/socket.h>
#include <wx/fileconf.h>
#include <zlib.h>

static std::string compressMessage(const std::string& message) {
	uLongf compressedMessageSize = 1000;
	std::string compressedMessage(compressedMessageSize, '\0');
	if (compress((Bytef*)compressedMessage.data(), &compressedMessageSize, (const Bytef*)message.data(), message.size()) != Z_OK) {
		wxMessageDialog(nullptr, wxT("compression error"), wxT("moos2.2"), wxOK | wxCENTRE | wxICON_ERROR).ShowModal();
	}
	return compressedMessage.substr(0, compressedMessageSize);
}

static std::string invertLength(uint32_t length) {
	std::string invertedLength(4, '\0');
	invertedLength[0] = length;
	invertedLength[1] = length << 8;
	invertedLength[2] = length << 16;
	invertedLength[3] = length << 24;
	return invertedLength;
}

static std::string generateLoginMessage(const wxString& loginUser, const wxString& loginPassword) {
	std::string loginData = invertLength(loginUser.Len());
	loginData += loginUser.ToUTF8();
	loginData += invertLength(loginPassword.Len());
	loginData += loginPassword.ToUTF8();
	loginData += invertLength(0);
	loginData += invertLength(0);
	auto compressedMessage = compressMessage(loginData);
	return invertLength(compressedMessage.length() + 4) + compressedMessage;
}

void MainFrame::OnSocketEvent(wxSocketEvent& event) {
	switch (event.GetSocketEvent()) {
	case wxSOCKET_CONNECTION:
		return OnConnect();
	case wxSOCKET_INPUT:
		return OnIncomingData();
	case wxSOCKET_LOST:
		return OnConnectionLost();
	case wxSOCKET_OUTPUT:
	default:
		return;
	}
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
	if (InfoDialog(this, langIni, font.GetChosenFont(), wxT("moos2.2"), langIni->Read(wxT("translations/dialogtext/reconnect"), wxT("Connection lost. Try to reconnect?")), 1).ShowModal()==wxID_YES) {
		loginAs(
				loginName,
				loginPassword,
				getUserServer(*moosIni, loginName),
				getUserPort(*moosIni, loginName)
		);
	}
}

void MainFrame::OnConnect() {
	SetStatusText(langIni->Read(wxT("translations/statusbar/logginin"), wxT("Logging in...")));
	static const std::string identTMP=std::string("\x24\x00\x00\x00\x78\x9c\xeb\x4d\xaa\x7b\xb5\xf3\x5f\x91\xef\xc6\x86\x2d\x7f\x27\xe6\xf6\x5e\x65\x66\x60\x60\x70\x77\x0d\x02\x00\x9c\x04\x0a\xf1", 36);

	std::string loginPacket=generateLoginMessage(loginName, loginPassword);
	socket->Write(identTMP.data(), identTMP.size());
	socket->Write(loginPacket.data(), loginPacket.size());
}

static uint32_t readU32(const uint8_t *data) {
    return
        ((uint32_t)data[3] << 24)
        |
        ((uint32_t)data[2] << 16)
        |
        ((uint32_t)data[1] << 8)
        |
        ((uint32_t)data[0] << 0);
}

static std::string readString(const uint8_t *data, size_t size) {
    return std::string(data, data + size);
}

void MainFrame::OnIncomingData() {
	uint8_t inputbuffer[1024];
	uint32_t inputbufferlen = 1024;
	uint8_t *buffer = new uint8_t[0]; //for delete
	uint32_t bufferlen = 0;

	while (inputbufferlen == 1024) {
		socket->Read(inputbuffer, 1024);
		inputbufferlen = socket->LastCount();
		uint8_t *tmp = new uint8_t[bufferlen + inputbufferlen];
		for (auto i = 0u; i < bufferlen; i++) {
			tmp[i] = buffer[i];
		}
		for (auto i = 0u; i < inputbufferlen; i++) {
			tmp[bufferlen + i] = inputbuffer[i];
		}
		delete[] buffer;
		buffer = tmp;
		bufferlen += inputbufferlen;
	}

	a:
	if (buffer[0] == '/' || buffer[0] == '$') {
		std::string command = "";
		for (auto i = 0u; i < bufferlen; i++) {
			if (!buffer[i]) {
				parseProcessCommand(wxConvLibc.cMB2WC(command.c_str()));
				uint8_t *tmp = new uint8_t[bufferlen - (i + 1)];
				for (auto j = i + 1; j != bufferlen; j++) {
					tmp[j - (i + 1)] = buffer[j];
				}
				bufferlen -= i + 1;
				delete[] buffer;
				buffer = tmp;
				goto a;
			} else {
				command += buffer[i];
			}
		}
		delete[] buffer;
		return;
	} else { //compressed data
		uint32_t packetlen = readU32(buffer);
		uLongf cbufferlen = 10240;
		uint8_t cbuffer[10240];
		if (uncompress((Bytef*)cbuffer, &cbufferlen, (const Bytef*)(buffer + 4), packetlen - 4) == Z_OK) {
			uint8_t *cbufferpos = cbuffer;
			auto packettype = readU32(cbufferpos); cbufferpos += 4;
			if (packettype == 0) { //Success
				//cbufferpos += 4; // ignore the packet length
				auto ourIp = readU32(cbufferpos); cbufferpos += 4;
				if (ourIp > packetlen) {
					ourIp = packetlen - 8;
					/* I know this code is crap and makes no sense, but it works.
					 * Usually you should use the client connection states
					 * - not connected
					 * - identifying
					 * - loggin in
					 * - logged in
					 * ... and handle the incomming data depending on that state instead
					 * of the first char ("/" or "$"), and "guessing the state".
					 * This is implemented in moos3 but needs classes; Rewriting
					 * many parts of moos2.x would end up in another moos3...
					 */
				}
				uint32_t serverIdentifierSize = readU32(cbufferpos); cbufferpos += 4;
				if (serverIdentifierSize > ourIp) {
					//Aha das ist also dieser komische Ident
					goto b;
				}
				auto serverIdentifier = readString(cbufferpos, serverIdentifierSize); cbufferpos += serverIdentifierSize;
				auto serverWelcomeMessageSize = readU32(cbufferpos); cbufferpos += 4;
				auto serverWelcomeMessage = readString(cbufferpos, serverWelcomeMessageSize); cbufferpos += serverWelcomeMessageSize;
				cbufferpos += 4 * 11; // skip stats and other data

				//detect Game Versions
				gameVersions.clear();
				while (true) {
					uint8_t id = *cbufferpos; cbufferpos++;
					if (id == 0xFF) {
						break;
					}
					auto userAgentNameSize = readU32(cbufferpos); cbufferpos += 4;
					auto userAgentName = readString(cbufferpos, userAgentNameSize); cbufferpos += userAgentNameSize;
					gameVersions[id] = wxString(userAgentName.c_str(), wxConvISO8859_1);
				}

				while (true) {
					auto foundPosition = serverWelcomeMessage.find("<*>");
					if (foundPosition != std::string::npos) {
						serverWelcomeMessage = serverWelcomeMessage.substr(0, foundPosition)+"<0xFFFFFFFF>" + serverWelcomeMessage.substr(foundPosition + 3);
					} else {
						break;
					}
				}

				wxIPV4address tmp;
				socket->GetPeer(tmp);
				SetStatusText(Format(langIni->Read(wxT("translations/statusbar/connected"), wxT("Connected with <%0> (<%1>)")), tmp.Hostname(), tmp.IPAddress()));

				Message(wxString(serverWelcomeMessage.data(), wxConvISO8859_1, serverWelcomeMessage.size()));
				AddUser(loginName);
				RefreshAutocomplete(loginName);
				moosIni->Write(wxT("accounts/") + Base64Encode(loginName) + wxT("/password"), Base64Encode(loginPassword, true));
			} else if (packettype == 2) { //Error
				cbufferpos += 4; // ignore the packet length
				auto contentlen = readU32(cbufferpos); cbufferpos += 4;
				auto content = readString(cbufferpos, contentlen); cbufferpos += contentlen;
				if (content == "translateInvalidCharactersInName") {
					InfoDialog(this, langIni, font.GetChosenFont(), wxT("moos2.2"), langIni->Read(wxT("translations/dialogtext/illegalusername"),
							   wxT("Error: Your user name contains invalid characters"))).ShowModal();
					moosIni->DeleteGroup(wxT("accounts/") + Base64Encode(loginName));
					loginName = wxEmptyString;
				} else if (content == "translateInvalidUserName") {
					InfoDialog(this, langIni, font.GetChosenFont(), wxT("moos2.2"),
							   langIni->Read(wxT("translations/dialogtext/userdoesntexist"), wxT("Error: Invalid user name"))).ShowModal();
					moosIni->DeleteGroup(wxT("accounts/") + Base64Encode(loginName));
					loginName = wxEmptyString;
				} else if (content == "translateInvalidPassword") {
					InfoDialog(this, langIni, font.GetChosenFont(), wxT("moos2.2"),
							   langIni->Read(wxT("translations/dialogtext/wrongpassword"), wxT("Error: Invalid password"))).ShowModal();
					moosIni->DeleteEntry(wxT("accounts/") + Base64Encode(loginName) + wxT("/password"), false);
				} else if (content == "translateAlreadyLogIn") {
					moosIni->Write(wxT("accounts/") + Base64Encode(loginName) + wxT("/password"), Base64Encode(loginPassword, true));
					InfoDialog(this, langIni, font.GetChosenFont(), wxT("moos2.2"),
							   langIni->Read(wxT("translations/dialogtext/allreadyloggedin"), wxT("Error: The user is already logged in"))).ShowModal();
				} else {
					InfoDialog(this, langIni, font.GetChosenFont(), wxT("moos2.2"), wxString(content.data(), wxConvISO8859_1, content.size())).ShowModal();
					moosIni->DeleteGroup(wxT("accounts/") + Base64Encode(loginName));
					loginName = wxEmptyString;
				}
				SetStatusText(langIni->Read(wxT("translations/statusbar/notconnected"), wxT("Not connected")));
				if (socket->IsConnected()) {
					socket->Close();
				}
			}
		}

		b:
		if (bufferlen <= packetlen) {
			delete[] buffer;
			bufferlen = 0;
			return;
		}
		uint8_t *tmp = new uint8_t[bufferlen - packetlen];
		for (uint32_t i = packetlen; i != bufferlen; i++) {
			tmp[i - packetlen] = buffer[i];
		}
		bufferlen -= packetlen;
		delete[] buffer;
		buffer = tmp;
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
	while (strData[i]==' ' && i!=strData.Len()) {
		i++;
	}
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
	std::map<int32_t, wxString>::iterator found=gameVersions.find(tmp);
	return found!=gameVersions.end()?found->second:wxT("Earth 2150");
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
