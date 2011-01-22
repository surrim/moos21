#include "base.h"

#include "enums.h"
#include "tools.h"
#include <wx/fileconf.h>
#include <wx/filedlg.h>
#include <wx/fontdlg.h>
#include <wx/menu.h>
#include <wx/settings.h>
#include <wx/socket.h>
#include <wx/stdpaths.h>
#include <wx/wfstream.h>

void MainFrame::OnAutoLoginAccount(wxCommandEvent& event) {
	if (LoginName==wxEmptyString) return SettingsMenu->Check(ID_MAINWIN_AUTOLOGIN_ACCOUNT, false);
	if (SettingsMenu->IsChecked(ID_MAINWIN_AUTOLOGIN_ACCOUNT))
		MoosIni->Write(wxT("autologin"), Base64Encode(LoginName));
	else
		MoosIni->DeleteEntry(wxT("autologin"));
}

void MainFrame::OnBeepSound(wxCommandEvent& event) {
	if (SettingsMenu->IsChecked(ID_MAINWIN_SOUND_ON_BEEP)) {
		wxFileDialog tmp(this, LangIni->Read(wxT("translations/menus/settings/soundonbeep"), wxT("Play sound on beep")), wxEmptyString, MoosIni->Read(wxT("beepsound"), wxT("%windir%\\Media\\notify.wav")), LangIni->Read(wxT("translations/other/soundfiles"), wxT("Sound Files"))+wxT(" (*.wav)|*.wav|")+LangIni->Read(wxT("translations/other/allfiles"), wxT("All Files"))+wxT(" (*.*)|*.*"), wxFD_OPEN|wxFD_FILE_MUST_EXIST);
		if (tmp.ShowModal()!=wxID_OK) return SettingsMenu->Check(ID_MAINWIN_SOUND_ON_BEEP, false);
		MoosIni->Write(wxT("beepsound"), tmp.GetPath());
		MoosIni->Write(wxT("enablebeep"), true);
	} else {
		MoosIni->DeleteEntry(wxT("enablebeep"));
	}
}

void MainFrame::OnDisableSlaps(wxCommandEvent& event) {
	MoosIni->DeleteEntry(wxT("disableslaps"));
	if (SettingsMenu->IsChecked(ID_MAINWIN_DISABLE_SLAPS)) MoosIni->Write(wxT("disableslaps"), true);
}

void MainFrame::OnChangeFont(wxCommandEvent & WXUNUSED(event)) {
	wxFontDialog tmp(this, Font);
	if (tmp.ShowModal()!=wxID_OK || !tmp.GetFontData().GetChosenFont().Ok()) return;
	Font=tmp.GetFontData();
	wxFont tmp2=Font.GetChosenFont();
	tmp2.SetWeight(wxFONTWEIGHT_NORMAL);
	tmp2.SetStyle(wxFONTSTYLE_NORMAL);
	Font.SetChosenFont(tmp2);
	ChannelSwitcher->SetFont(Font.GetChosenFont());
	UserList->SetFont(Font.GetChosenFont());
	ChatInput->SetFont(Font.GetChosenFont());
	WhisperCheckbox->SetFont(Font.GetChosenFont());

	if (tmp.GetFontData().GetChosenFont().GetFaceName()!=wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT).GetFaceName())
		MoosIni->Write(wxT("layout/fontface"), tmp.GetFontData().GetChosenFont().GetFaceName());
	else
		MoosIni->DeleteEntry(wxT("layout/fontface"));
	if (tmp.GetFontData().GetChosenFont().GetPointSize()!=wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT).GetPointSize())
		MoosIni->Write(wxT("layout/fontsize"), tmp.GetFontData().GetChosenFont().GetPointSize());
	else
		MoosIni->DeleteEntry(wxT("layout/fontsize"));

	Font.SetInitialFont(Font.GetChosenFont());
	MainSizer->Layout();
	ChatInput->SetFocus();
}

void MainFrame::OnSelectLanguage(wxCommandEvent & WXUNUSED(event)) {
	wxFileDialog tmp(this, LangIni->Read(wxT("translations/menus/settings/selectlanguage"), wxT("Select Language...")), wxEmptyString, MoosIni->Read(wxT("languagefile"), wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath()+wxFileName::GetPathSeparator()+wxT("*.ini")), LangIni->Read(wxT("translations/other/inifiles"), wxT("Ini Files"))+wxT(" (*.ini)|*.ini|")+LangIni->Read(wxT("translations/other/allfiles"), wxT("All Files"))+wxT(" (*.*)|*.*"), wxFD_OPEN|wxFD_FILE_MUST_EXIST);
	if (tmp.ShowModal()!=wxID_OK) return;
	MoosIni->Write(wxT("languagefile"), tmp.GetPath());
	wxFileInputStream lang(tmp.GetPath());
	if (lang.CanRead()) {
		if (MoosIni!=LangIni) delete LangIni;
		LangIni=new wxFileConfig(lang, wxConvUTF8);
	}
	MoosMenu->SetLabel(ID_MAINWIN_LOGIN_AS, LangIni->Read(wxT("translations/menus/moos/loginas"), wxT("Login as...")));
	MoosMenu->SetLabel(ID_MAINWIN_LOGOUT, LangIni->Read(wxT("translations/menus/moos/logout"), wxT("Logout")));
	MoosMenu->SetLabel(ID_MAINWIN_EXIT, LangIni->Read(wxT("translations/menus/moos/exit"), wxT("Exit")));
	UserMenu->SetLabel(ID_MAINWIN_WHOIS_USER, LangIni->Read(wxT("translations/menus/user/profile"), wxT("Userinformation...")));
	UserMenu->SetLabel(ID_MAINWIN_SLAP_USER, LangIni->Read(wxT("translations/menus/user/slap"), wxT("Slap...")));
	UserMenu->SetLabel(ID_MAINWIN_IGNORE_USER, LangIni->Read(wxT("translations/menus/user/ignore"), wxT("Ignore")));
	UserMenu->SetLabel(ID_MAINWIN_IGNORELIST, LangIni->Read(wxT("translations/menus/user/ignorelist"), wxT("Ignorelist...")));
	ViewMenu->SetLabel(ID_MAINWIN_SAVE_CHAT, LangIni->Read(wxT("translations/menus/view/savechathistory"), wxT("Save Chat...")));
	ViewMenu->SetLabel(ID_MAINWIN_FREEZE_CHAT, LangIni->Read(wxT("translations/menus/view/freezechathistory"), wxT("Freeze Chat")));
	ViewMenu->SetLabel(ID_MAINWIN_DELETE_CHAT, LangIni->Read(wxT("translations/menus/view/deletechathistory"), wxT("Delete Chat")));
	ViewMenu->SetLabel(ID_MAINWIN_COLORSELECT, LangIni->Read(wxT("translations/menus/view/selectcolor"), wxT("Select Color...")));
	ViewMenu->SetLabel(ID_MAINWIN_MANUAL_COLORCODE, LangIni->Read(wxT("translations/menus/view/manualcolorcode"), wxT("Manual Color...")));
	SettingsMenu->SetLabel(ID_MAINWIN_AUTOLOGIN_ACCOUNT, LangIni->Read(wxT("translations/menus/settings/autologinaccount"), wxT("Activate AutoLogin for this Account")));
	SettingsMenu->SetLabel(ID_MAINWIN_SOUND_ON_BEEP, LangIni->Read(wxT("translations/menus/settings/soundonbeep"), wxT("Play sound on beep")));
	SettingsMenu->SetLabel(ID_MAINWIN_DISABLE_SLAPS, LangIni->Read(wxT("translations/menus/settings/disableslaps"), wxT("Disable Slaps")));
	SettingsMenu->SetLabel(ID_MAINWIN_CHANGE_FONT, LangIni->Read(wxT("translations/menus/settings/changefont"), wxT("Change Font...")));
	SettingsMenu->SetLabel(ID_MAINWIN_SELECT_LANGUAGE, LangIni->Read(wxT("translations/menus/settings/selectlanguage"), wxT("Select Language...")));
	HelpMenu->SetLabel(ID_MAINWIN_ONLINEREADME, LangIni->Read(wxT("translations/menus/help/onlinereadme"), wxT("Online Readme")));
	HelpMenu->SetLabel(ID_MAINWIN_MAILBUGS, LangIni->Read(wxT("translations/menus/help/mailbugs"), wxT("Mail Bugs")));
	HelpMenu->SetLabel(ID_MAINWIN_ABOUT, LangIni->Read(wxT("translations/menus/help/about"), wxT("About Moos 2.1...")));
	MenuBar->SetMenuLabel(0, LangIni->Read(wxT("translations/menus/moos"), wxT("Moos")));
	MenuBar->SetMenuLabel(1, LangIni->Read(wxT("translations/menus/user"), wxT("User")));
	MenuBar->SetMenuLabel(2, LangIni->Read(wxT("translations/menus/view"), wxT("View")));
	MenuBar->SetMenuLabel(3, LangIni->Read(wxT("translations/menus/settings"), wxT("Settings")));
	MenuBar->SetMenuLabel(4, LangIni->Read(wxT("translations/menus/help"), wxT("?")));
	WhisperCheckbox->SetLabel(LangIni->Read(wxT("translations/whisper"), wxT("Whisper")));
	SetStatusText(wxEmptyString);
	if (!Socket->IsConnected()) {
		SetStatusText(LangIni->Read(wxT("translations/statusbar/notconnected"), wxT("Not connected")));
	}
	MainSizer->Layout();
}
