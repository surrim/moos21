#include "utils.h"

#include "tools.h"

wxString getDefaultServer(const wxFileConfig& moosIni) {
	return moosIni.Read(wxT("server"), wxT("moos.insideearth.info"));
}

wxString getUserServer(const wxFileConfig& moosIni, const wxString& user) {
	auto defaultServer = getDefaultServer(moosIni);
	return moosIni.Read(wxT("accounts/") + Base64Encode(user) + wxT("/server"), defaultServer);
}

wxString getServerPort(const wxFileConfig& moosIni, const wxString& server) {
	return moosIni.Read(wxT("server/") + server + wxT("/port"), wxT("12100"));
}

wxString getUserPort(const wxFileConfig& moosIni, const wxString& user) {
	auto userServer = getUserServer(moosIni, user);
	auto serverPort = getServerPort(moosIni, userServer);
	return moosIni.Read(wxT("accounts/") + Base64Encode(user) + wxT("/port"), serverPort);
}

wxString getUserPassword(const wxFileConfig& moosIni, const wxString& user) {
	auto usernameAsBase64 = Base64Encode(user);
	auto encodedPassword = moosIni.Read(wxT("accounts/") + usernameAsBase64 + wxT("/password"));
	return Base64Decode(encodedPassword, true);
}
