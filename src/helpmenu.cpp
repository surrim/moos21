#include "base.h"

#include "infodialog.h"
#include "moos21_private.h"
#include <wx/fileconf.h>

void MainFrame::OnOnlineReadme(wxCommandEvent& WXUNUSED(event)) {
	wxLaunchDefaultBrowser(wxT("http://surrim.de/?moos"));
}

void MainFrame::OnMailBugs(wxCommandEvent& WXUNUSED(event)) {
	wxLaunchDefaultBrowser(wxT("mailto:moosbugs@arcor.de?subject=moosbugs"));
}

void MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event)) {
	InfoDialog(this, langIni, font.GetChosenFont(), langIni->Read(wxT("translations/menus/help/about"), wxT("About Moos 2.1...")),
			   wxT("MoonNet Omnipresent Observation System ")+wxString(_(VER_STRING))
			   +wxT("\nmoos2.1 is an Inside Earth Operations application (www.insideearth.de)\nwritten by mensi (mensi@insideearth.de), modified by surrim\n\nThis program is freeware WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.")).ShowModal();
}
