#ifndef UTILS_H
#define UTILS_H
#endif

#include <wx/fileconf.h>

wxString getDefaultServer(const wxFileConfig& moosIni);
wxString getUserServer(const wxFileConfig& moosIni, const wxString& User);
wxString getServerPort(const wxFileConfig& moosIni, const wxString& Server);
wxString getUserPort(const wxFileConfig& moosIni, const wxString& User);
wxString getUserPassword(const wxFileConfig& moosIni, const wxString& User);
