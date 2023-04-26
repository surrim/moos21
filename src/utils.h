#ifndef UTILS_H
#define UTILS_H
#endif

#include <wx/fileconf.h>

wxString getDefaultServer(const wxFileConfig& moosIni);
wxString getUserServer(const wxFileConfig& moosIni, const wxString& user);
wxString getServerPort(const wxFileConfig& moosIni, const wxString& server);
wxString getUserPort(const wxFileConfig& moosIni, const wxString& user);
wxString getUserPassword(const wxFileConfig& moosIni, const wxString& user);
