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

#include "infodialog.h"
#include <wx/fileconf.h>

void MainFrame::OnOnlineReadme(wxCommandEvent& WXUNUSED(event)) {
	wxLaunchDefaultBrowser(wxT("https://surrim.org/moon-project/moos.html"));
}

void MainFrame::OnMailBugs(wxCommandEvent& WXUNUSED(event)) {
	wxLaunchDefaultBrowser(wxT("mailto:moosbugs@surrim.org?subject=moosbugs"));
}

void MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event)) {
	InfoDialog(this, langIni, font.GetChosenFont(), langIni->Read(wxT("translations/menus/help/about"), wxT("About Moos...")),
			   wxT(R"raw(
MoonNet Omnipresent Observation System 2.2.1
moos is an Inside Earth Operations application (www.insideearth.de)
written by mensi (mensi@insideearth.de), modified by surrim.

moos is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

moos is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with moos.  If not, see <http://www.gnu.org/licenses/>.
			   )raw")).ShowModal();
}
