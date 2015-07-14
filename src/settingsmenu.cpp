#include "base.h"

#include "enums.h"
#include "tools.h"
#include <wx/fileconf.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/fontdlg.h>
#include <wx/menu.h>
#include <wx/settings.h>
#include <wx/socket.h>
#include <wx/stdpaths.h>
#include <wx/wfstream.h>

void MainFrame::OnAutoLoginAccount(wxCommandEvent& WXUNUSED(event)) {
	if (loginName==wxEmptyString) {
		return settingsMenu->Check(ID_MAINWIN_AUTOLOGIN_ACCOUNT, false);
	}
	if (settingsMenu->IsChecked(ID_MAINWIN_AUTOLOGIN_ACCOUNT)) {
		moosIni->Write(wxT("autologin"), Base64Encode(loginName));
	} else {
		moosIni->DeleteEntry(wxT("autologin"));
	}
}

void MainFrame::OnBeepSound(wxCommandEvent& WXUNUSED(event)) {
	if (settingsMenu->IsChecked(ID_MAINWIN_SOUND_ON_BEEP)) {
		wxFileDialog tmp(this, langIni->Read(wxT("translations/menus/settings/soundonbeep"), wxT("Play sound on beep")), wxEmptyString, moosIni->Read(wxT("beepsound"), wxT("%windir%\\Media\\notify.wav")), langIni->Read(wxT("translations/other/soundfiles"), wxT("Sound Files"))+wxT(" (*.wav)|*.wav|")+langIni->Read(wxT("translations/other/allfiles"), wxT("All Files"))+wxT(" (*.*)|*.*"), wxFD_OPEN|wxFD_FILE_MUST_EXIST);
		if (tmp.ShowModal()!=wxID_OK) return settingsMenu->Check(ID_MAINWIN_SOUND_ON_BEEP, false);
		moosIni->Write(wxT("beepsound"), tmp.GetPath());
		moosIni->Write(wxT("enablebeep"), true);
	} else {
		moosIni->DeleteEntry(wxT("enablebeep"));
	}
}

void MainFrame::OnDisableSlaps(wxCommandEvent& WXUNUSED(event)) {
	moosIni->DeleteEntry(wxT("disableslaps"));
	if (settingsMenu->IsChecked(ID_MAINWIN_DISABLE_SLAPS)) moosIni->Write(wxT("disableslaps"), true);
}

void MainFrame::OnChangeFont(wxCommandEvent& WXUNUSED(event)) {
	wxFontDialog tmp(this, font);
	if (tmp.ShowModal()!=wxID_OK || !tmp.GetFontData().GetChosenFont().Ok()) return;
	font=tmp.GetFontData();
	wxFont tmp2=font.GetChosenFont();
	tmp2.SetWeight(wxFONTWEIGHT_NORMAL);
	tmp2.SetStyle(wxFONTSTYLE_NORMAL);
	font.SetChosenFont(tmp2);
	channelSwitcher->SetFont(font.GetChosenFont());
	userList->SetFont(font.GetChosenFont());
	chatInput->SetFont(font.GetChosenFont());
	whisperCheckbox->SetFont(font.GetChosenFont());

	if (tmp.GetFontData().GetChosenFont().GetFaceName()!=wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT).GetFaceName()) {
		moosIni->Write(wxT("layout/fontface"), tmp.GetFontData().GetChosenFont().GetFaceName());
	} else {
		moosIni->DeleteEntry(wxT("layout/fontface"));
	}
	if (tmp.GetFontData().GetChosenFont().GetPointSize()!=wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT).GetPointSize()) {
		moosIni->Write(wxT("layout/fontsize"), tmp.GetFontData().GetChosenFont().GetPointSize());
	} else {
		moosIni->DeleteEntry(wxT("layout/fontsize"));
	}

	font.SetInitialFont(font.GetChosenFont());
	mainSizer->Layout();
}

void MainFrame::OnSelectLanguage(wxCommandEvent& WXUNUSED(event)) {
	wxFileDialog tmp(this, langIni->Read(wxT("translations/menus/settings/selectlanguage"), wxT("Select Language...")), wxEmptyString, moosIni->Read(wxT("languagefile"), wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath()+wxFileName::GetPathSeparator()+wxT("*.ini")), langIni->Read(wxT("translations/other/inifiles"), wxT("Ini Files"))+wxT(" (*.ini)|*.ini|")+langIni->Read(wxT("translations/other/allfiles"), wxT("All Files"))+wxT(" (*.*)|*.*"), wxFD_OPEN|wxFD_FILE_MUST_EXIST);
	if (tmp.ShowModal()!=wxID_OK) return;
	moosIni->Write(wxT("languagefile"), tmp.GetPath());
	wxFileInputStream lang(tmp.GetPath());
	if (lang.CanRead()) {
		if (moosIni!=langIni) delete langIni;
		langIni=new wxFileConfig(lang, wxConvUTF8);
	}
	moosMenu->SetLabel(ID_MAINWIN_LOGIN_AS, langIni->Read(wxT("translations/menus/moos/loginas"), wxT("Login as...")));
	moosMenu->SetLabel(ID_MAINWIN_LOGOUT, langIni->Read(wxT("translations/menus/moos/logout"), wxT("Logout")));
	moosMenu->SetLabel(ID_MAINWIN_EXIT, langIni->Read(wxT("translations/menus/moos/exit"), wxT("Exit")));
	userMenu->SetLabel(ID_MAINWIN_WHOIS_USER, langIni->Read(wxT("translations/menus/user/profile"), wxT("Userinformation...")));
	userMenu->SetLabel(ID_MAINWIN_SLAP_USER, langIni->Read(wxT("translations/menus/user/slap"), wxT("Slap...")));
	userMenu->SetLabel(ID_MAINWIN_IGNORE_USER, langIni->Read(wxT("translations/menus/user/ignore"), wxT("Ignore")));
	userMenu->SetLabel(ID_MAINWIN_IGNORELIST, langIni->Read(wxT("translations/menus/user/ignorelist"), wxT("Ignorelist...")));
	viewMenu->SetLabel(ID_MAINWIN_SAVE_CHAT, langIni->Read(wxT("translations/menus/view/savechathistory"), wxT("Save Chat...")));
	viewMenu->SetLabel(ID_MAINWIN_FREEZE_CHAT, langIni->Read(wxT("translations/menus/view/freezechathistory"), wxT("Freeze Chat")));
	viewMenu->SetLabel(ID_MAINWIN_DELETE_CHAT, langIni->Read(wxT("translations/menus/view/deletechathistory"), wxT("Delete Chat")));
	viewMenu->SetLabel(ID_MAINWIN_COLORSELECT, langIni->Read(wxT("translations/menus/view/selectcolor"), wxT("Select Color...")));
	viewMenu->SetLabel(ID_MAINWIN_MANUAL_COLORCODE, langIni->Read(wxT("translations/menus/view/manualcolorcode"), wxT("Manual Color...")));
	settingsMenu->SetLabel(ID_MAINWIN_AUTOLOGIN_ACCOUNT, langIni->Read(wxT("translations/menus/settings/autologinaccount"), wxT("Activate AutoLogin for this Account")));
	settingsMenu->SetLabel(ID_MAINWIN_SOUND_ON_BEEP, langIni->Read(wxT("translations/menus/settings/soundonbeep"), wxT("Play sound on beep")));
	settingsMenu->SetLabel(ID_MAINWIN_DISABLE_SLAPS, langIni->Read(wxT("translations/menus/settings/disableslaps"), wxT("Disable Slaps")));
	settingsMenu->SetLabel(ID_MAINWIN_CHANGE_FONT, langIni->Read(wxT("translations/menus/settings/changefont"), wxT("Change font...")));
	settingsMenu->SetLabel(ID_MAINWIN_SELECT_LANGUAGE, langIni->Read(wxT("translations/menus/settings/selectlanguage"), wxT("Select Language...")));
	helpMenu->SetLabel(ID_MAINWIN_ONLINEREADME, langIni->Read(wxT("translations/menus/help/onlinereadme"), wxT("Online Readme")));
	helpMenu->SetLabel(ID_MAINWIN_MAILBUGS, langIni->Read(wxT("translations/menus/help/mailbugs"), wxT("Mail Bugs")));
	helpMenu->SetLabel(ID_MAINWIN_ABOUT, langIni->Read(wxT("translations/menus/help/about"), wxT("About Moos 2.1...")));
	menuBar->SetMenuLabel(0, langIni->Read(wxT("translations/menus/moos"), wxT("Moos")));
	menuBar->SetMenuLabel(1, langIni->Read(wxT("translations/menus/user"), wxT("User")));
	menuBar->SetMenuLabel(2, langIni->Read(wxT("translations/menus/view"), wxT("View")));
	menuBar->SetMenuLabel(3, langIni->Read(wxT("translations/menus/settings"), wxT("Settings")));
	menuBar->SetMenuLabel(4, langIni->Read(wxT("translations/menus/help"), wxT("?")));
	whisperCheckbox->SetLabel(langIni->Read(wxT("translations/whisper"), wxT("Whisper")));
	SetStatusText(wxEmptyString);
	if (!socket->IsConnected()) {
		SetStatusText(langIni->Read(wxT("translations/statusbar/notconnected"), wxT("Not connected")));
	}
	mainSizer->Layout();
}
