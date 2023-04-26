#include "utils.h"

#include "tools.h"

wxString getDefaultServer(const wxFileConfig& moosIni) {
	return moosIni.Read(wxT("server"), wxT("moos.insideearth.info"));
}

wxString getUserServer(const wxFileConfig& moosIni, const wxString& User) {
	wxString defaultServer = getDefaultServer(moosIni);
	return moosIni.Read(wxT("accounts/")+Base64Encode(User)+wxT("/server"), defaultServer);
}

wxString getServerPort(const wxFileConfig& moosIni, const wxString& Server) {
	return moosIni.Read(wxT("server/")+Server+wxT("/port"), wxT("12100"));
}

wxString getUserPort(const wxFileConfig& moosIni, const wxString& User) {
	wxString userServer = getUserServer(moosIni, User);
	wxString serverPort = getServerPort(moosIni, userServer);
	return moosIni.Read(wxT("accounts/")+Base64Encode(User)+wxT("/port"), serverPort);
}

wxString getUserPassword(const wxFileConfig& moosIni, const wxString& User) {
	wxString usernameAsBase64 = Base64Encode(User);
	wxString encodedPassword = moosIni.Read(wxT("accounts/")+usernameAsBase64+wxT("/password"));
	return Base64Decode(encodedPassword, true);
}
