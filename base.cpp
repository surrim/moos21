#include "base.h"

#include "enums.h"
#include "inputdialog.h"
#include "tools.h"
#include <wx/file.h>
#include <wx/fileconf.h>
#include <wx/font.h>
#include <wx/image.h>
#include <wx/menu.h>
#include <wx/settings.h>
#include <wx/socket.h>
#include <wx/wfstream.h>

IMPLEMENT_APP(MoosApp)

const wxString MoosApp::CONFIG_FILE(wxT("moos.ini"));

bool MoosApp::OnInit() {
	wxImage::AddHandler(new wxPNGHandler);
	MainFrame *win=new MainFrame(CONFIG_FILE, wxDefaultPosition, wxSize(720, 480));
	win->Show();
	SetTopWindow(win);
	return true;
}

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	//Common
	EVT_CHOICE(wxID_ANY, MainFrame::OnSelectChannel)
	EVT_LISTBOX(wxID_ANY, MainFrame::OnSelectUser)
	EVT_CHECKBOX(wxID_ANY, MainFrame::OnWhisperClick)
	EVT_LISTBOX_DCLICK(wxID_ANY, MainFrame::OnWhoisUser)
	EVT_TEXT_ENTER(wxID_ANY, MainFrame::OnSendMessage)
	EVT_TEXT(wxID_ANY, MainFrame::OnText)
	//socket
	EVT_SOCKET(ID_SOCKET, MainFrame::OnSocketEvent)
	//moos Menu
	EVT_MENU(ID_MAINWIN_LOGIN_AS, MainFrame::OnLoginAs)
	EVT_MENU(ID_MAINWIN_LOGOUT, MainFrame::OnLogout)
	EVT_MENU(ID_MAINWIN_EXIT, MainFrame::OnExit)
	//User Menu
	EVT_MENU(ID_MAINWIN_WHOIS_USER, MainFrame::OnWhoisUser)
	EVT_MENU(ID_MAINWIN_SLAP_USER, MainFrame::OnSlapUser)
	EVT_MENU(ID_MAINWIN_IGNORE_USER, MainFrame::OnIgnoreUser)
	EVT_MENU(ID_MAINWIN_IGNORELIST, MainFrame::OnIgnoreList)
	//View Menu
	EVT_MENU(ID_MAINWIN_SAVE_CHAT, MainFrame::OnSaveChat)
	EVT_MENU(ID_MAINWIN_FREEZE_CHAT, MainFrame::OnFreezeChat)
	EVT_MENU(ID_MAINWIN_DELETE_CHAT, MainFrame::OnDeleteChat)
	EVT_MENU(ID_MAINWIN_COLORSELECT, MainFrame::OnColorSelect)
	EVT_MENU(ID_MAINWIN_MANUAL_COLORCODE, MainFrame::OnManualColorCode)
	//Moos Settings Menu
	EVT_MENU(ID_MAINWIN_AUTOLOGIN_ACCOUNT, MainFrame::OnAutoLoginAccount)
	EVT_MENU(ID_MAINWIN_SOUND_ON_BEEP, MainFrame::OnBeepSound)
	EVT_MENU(ID_MAINWIN_DISABLE_SLAPS, MainFrame::OnDisableSlaps)
	EVT_MENU(ID_MAINWIN_CHANGE_FONT, MainFrame::OnChangeFont)
	EVT_MENU(ID_MAINWIN_SELECT_LANGUAGE, MainFrame::OnSelectLanguage)
	//Hilfe Menu
	EVT_MENU(ID_MAINWIN_ONLINEREADME, MainFrame::OnOnlineReadme)
	EVT_MENU(ID_MAINWIN_MAILBUGS, MainFrame::OnMailBugs)
	EVT_MENU(ID_MAINWIN_ABOUT, MainFrame::OnAbout)
END_EVENT_TABLE()

MainFrame::MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size):
		wxFrame(0, -1, title, pos, size),
		mainSizer(0),
		listSizer(0),
		chatSizer(0),
		inputSizer(0),
		chatHistory(),
		lastInput(),
		chatBuffer(0),
		seenUsers(0),
		userList(0),
		ignoredUserList(0),
		font(),
		channelSwitcher(0),
		chatView(0),
		chatInput(0),
		whisperCheckbox(0),
		moosMenu(0),
		userMenu(0),
		viewMenu(0),
		settingsMenu(0),
		helpMenu(0),
		moosIcon(0),
		menuBar(0),
		moosIni(0),
		langIni(0),
		socket(0),
		disableOnText(false),
		loginName(),
		loginPassword(),
		gameVersions(0) {
	bool firstrun=false;
	if (!wxFile::Exists(MoosApp::CONFIG_FILE)) {
		wxFile(MoosApp::CONFIG_FILE, wxFile::write);
		firstrun=true;
	}
	wxFileInputStream moos(MoosApp::CONFIG_FILE);
	if (moos.CanRead()) moosIni=langIni=new wxFileConfig(moos, wxConvUTF8);
	if (moosIni->Exists(wxT("languagefile"))) {
		if (wxFile::Exists(moosIni->Read(wxT("languagefile")))) {
			wxFileInputStream lang(moosIni->Read(wxT("languagefile"), wxEmptyString));
			if (lang.CanRead()) {
				langIni=new wxFileConfig(lang, wxConvUTF8);
			}
		}
	}

	int x=moosIni->Read(wxT("layout/x"), wxDefaultCoord);
	if ((x<0 && x!=wxDefaultCoord) || x>wxGetDisplaySize().GetWidth()) {
		x=0;
	}
	int y=moosIni->Read(wxT("layout/y"), wxDefaultCoord);
	if ((y<0 && y!=wxDefaultCoord) || y>wxGetDisplaySize().GetHeight()) {
		y=0;
	}
	int width=moosIni->Read(wxT("layout/width"), 720);
	if (x+width>wxGetDisplaySize().GetWidth()) {
		width=720;
	}
	int height=moosIni->Read(wxT("layout/height"), 480);
	if (y+height>wxGetDisplaySize().GetHeight()) {
		height=480;
	}
	SetSize(x, y, width, height);
	if (moosIni->Read(wxT("layout/maximized"), 0L)) {
		Maximize();
	}

	#ifdef WIN32
	SetIcon(wxICON(A));
	#else
	SetIcon(wxIcon(wxT("icons/moos.png"), wxBITMAP_TYPE_PNG));
	#endif
	CreateStatusBar();
	SetStatusText(langIni->Read(wxT("translations/statusbar/notconnected"), wxT("Not connected")));
	wxFont tmp=wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
	if (moosIni->Exists(wxT("layout/fontface"))) tmp.SetFaceName(moosIni->Read(wxT("layout/fontface")));
	if (moosIni->Exists(wxT("layout/fontsize"))) {
		long tmp2;
		moosIni->Read(wxT("layout/fontsize"), &tmp2);
		tmp.SetPointSize(tmp2);
	}
	font.SetInitialFont(tmp);
	font.SetChosenFont(tmp);
	font.EnableEffects(false);
	SetBackgroundColour(wxColour(212, 208, 200));
	chatHistory=wxT("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"><html><head><meta http-equiv=\"content-type\" content=\"text/html;charset=UTF-16LE\"><title>moos2.1</title><style type=\"text/css\">body{background-color:#1F3038; color:#8C9FBB}pre{font-family:\"ms sans serif\", sans-serif;}</style></head><body><pre>");

	moosMenu=new wxMenu;
	moosMenu->Append(ID_MAINWIN_LOGIN_AS, langIni->Read(wxT("translations/menus/moos/loginas"), wxT("Login as...")));
	moosMenu->Append(ID_MAINWIN_LOGOUT, langIni->Read(wxT("translations/menus/moos/logout"), wxT("Logout")));
	moosMenu->AppendSeparator();
	moosMenu->Append(ID_MAINWIN_EXIT, langIni->Read(wxT("translations/menus/moos/exit"), wxT("Exit")));

	userMenu=new wxMenu;
	userMenu->Append(ID_MAINWIN_WHOIS_USER, langIni->Read(wxT("translations/menus/user/profile"), wxT("Userinformation...")));
	userMenu->Append(ID_MAINWIN_SLAP_USER, langIni->Read(wxT("translations/menus/user/slap"), wxT("Slap...")));
	userMenu->Append(ID_MAINWIN_IGNORE_USER, langIni->Read(wxT("translations/menus/user/ignore"), wxT("Ignore")));
	userMenu->AppendSeparator();
	userMenu->Append(ID_MAINWIN_IGNORELIST, langIni->Read(wxT("translations/menus/user/ignorelist"), wxT("Ignorelist...")));

	viewMenu=new wxMenu;
	viewMenu->Append(ID_MAINWIN_SAVE_CHAT, langIni->Read(wxT("translations/menus/view/savechathistory"), wxT("Save Chat...")));
	viewMenu->AppendCheckItem(ID_MAINWIN_FREEZE_CHAT, langIni->Read(wxT("translations/menus/view/freezechathistory"), wxT("Freeze Chat")));
	viewMenu->Append(ID_MAINWIN_DELETE_CHAT, langIni->Read(wxT("translations/menus/view/deletechathistory"), wxT("Delete Chat")));
	viewMenu->AppendSeparator();
	viewMenu->Append(ID_MAINWIN_COLORSELECT, langIni->Read(wxT("translations/menus/view/selectcolor"), wxT("Select Color...")));
	viewMenu->Append(ID_MAINWIN_MANUAL_COLORCODE, langIni->Read(wxT("translations/menus/view/manualcolorcode"), wxT("Manual Color...")));

	settingsMenu=new wxMenu;
	settingsMenu->AppendCheckItem(ID_MAINWIN_AUTOLOGIN_ACCOUNT, langIni->Read(wxT("translations/menus/settings/autologinaccount"), wxT("Activate AutoLogin for this Account")));
	settingsMenu->AppendCheckItem(ID_MAINWIN_SOUND_ON_BEEP, langIni->Read(wxT("translations/menus/settings/soundonbeep"), wxT("Play sound on beep")));
	settingsMenu->Check(ID_MAINWIN_SOUND_ON_BEEP, moosIni->Read(wxT("enablebeep"), 0L));
	settingsMenu->AppendCheckItem(ID_MAINWIN_DISABLE_SLAPS, langIni->Read(wxT("translations/menus/settings/disableslaps"), wxT("Disable Slaps")));
	settingsMenu->Check(ID_MAINWIN_DISABLE_SLAPS, moosIni->Read(wxT("disableslaps"), 0L));
	settingsMenu->AppendSeparator();
	settingsMenu->Append(ID_MAINWIN_CHANGE_FONT, langIni->Read(wxT("translations/menus/settings/changefont"), wxT("Change font...")));
	settingsMenu->Append(ID_MAINWIN_SELECT_LANGUAGE, langIni->Read(wxT("translations/menus/settings/selectlanguage"), wxT("Select Language...")));

	helpMenu=new wxMenu;
	helpMenu->Append(ID_MAINWIN_ONLINEREADME, langIni->Read(wxT("translations/menus/help/onlinereadme"), wxT("Online Readme")));
	helpMenu->Append(ID_MAINWIN_MAILBUGS, langIni->Read(wxT("translations/menus/help/mailbugs"), wxT("Mail Bugs")));
	helpMenu->AppendSeparator();
	helpMenu->Append(ID_MAINWIN_ABOUT, langIni->Read(wxT("translations/menus/help/about"), wxT("About Moos 2.1...")));

	menuBar=new wxMenuBar;
	menuBar->Append(moosMenu, langIni->Read(wxT("translations/menus/moos"), wxT("Moos")));
	menuBar->Append(userMenu, langIni->Read(wxT("translations/menus/user"), wxT("User")));
	menuBar->Append(viewMenu, langIni->Read(wxT("translations/menus/view"), wxT("View")));
	menuBar->Append(settingsMenu, langIni->Read(wxT("translations/menus/settings"), wxT("Settings")));
	menuBar->Append(helpMenu, langIni->Read(wxT("translations/menus/help"), wxT("?")));
	SetMenuBar(menuBar);

	//Sizer
	mainSizer=new wxBoxSizer(wxHORIZONTAL);
	listSizer=new wxBoxSizer(wxVERTICAL);
	chatSizer=new wxBoxSizer(wxVERTICAL);
	inputSizer=new wxBoxSizer(wxHORIZONTAL);

	//Elemente
	SetFont(font.GetChosenFont());
	channelSwitcher=new wxChoice(this, ID_MAINWIN_CHANNELSWITCHER, wxDefaultPosition, wxDefaultSize, 0, 0, wxSTATIC_BORDER);
	channelSwitcher->SetBackgroundColour(wxColour(31, 48, 56));
	channelSwitcher->SetForegroundColour(wxColour(140, 158, 180));
	userList=new wxListBox(this, ID_MAINWIN_USERLIST, wxDefaultPosition, wxSize(200,200), 0, 0, wxLB_EXTENDED|wxLB_NEEDED_SB|wxLB_SORT|wxSTATIC_BORDER);
	userList->SetBackgroundColour(wxColour(31, 48, 56));
	userList->SetForegroundColour(wxColour(140, 158, 180));
	ignoredUserList=new wxListBox(this, ID_MAINWIN_IGNOREDUSERLIST, wxDefaultPosition, wxSize(200,200), 0, 0, wxLB_EXTENDED|wxLB_NEEDED_SB|wxLB_SORT|wxSTATIC_BORDER);
	ignoredUserList->SetBackgroundColour(wxColour(31, 48, 56));
	ignoredUserList->SetForegroundColour(wxColour(140, 158, 180));
	ignoredUserList->Hide();
	chatView=new wxTextCtrl(this, ID_MAINWIN_CHATVIEW, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_RICH|wxTE_RICH2|wxTE_MULTILINE|wxTE_READONLY|wxSTATIC_BORDER);
	chatView->SetBackgroundColour(wxColour(31, 48, 56));
	chatInput=new wxTextCtrl(this, ID_MAINWIN_CHATINPUT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER|wxWANTS_CHARS);
	whisperCheckbox=new wxCheckBox(this, ID_MAINWIN_WHISPERCHECKBOX, langIni->Read(wxT("translations/whisper"), wxT("Whisper")));
	socket=new wxSocketClient();
	socket->SetEventHandler(*this, ID_SOCKET);
	socket->SetNotify(wxSOCKET_INPUT_FLAG|wxSOCKET_LOST_FLAG|wxSOCKET_OUTPUT_FLAG|wxSOCKET_CONNECTION_FLAG);
	socket->Notify(true);
	if (firstrun) {
		wxCommandEvent tmp3;
		OnSelectLanguage(tmp3);
	}

	mainSizer->Add(listSizer, 0, wxEXPAND|wxALL, 3);
	mainSizer->Add(chatSizer, 1, wxEXPAND|wxALL, 3);

	listSizer->Add(channelSwitcher, 0, wxEXPAND|wxALL, 0);
	listSizer->AddSpacer(3);
	listSizer->Add(userList, 1, wxEXPAND|wxALL, 0);
	listSizer->Add(ignoredUserList, 1, wxEXPAND|wxALL, 0);
	chatSizer->Add(chatView, 1, wxEXPAND|wxALL, 0);
	chatSizer->AddSpacer(1);
	chatSizer->Add(inputSizer, 0, wxEXPAND|wxALL, 0);
	inputSizer->Add(chatInput, 1, wxEXPAND|wxALL, 0);
	inputSizer->AddSpacer(3);
	inputSizer->Add(whisperCheckbox, 0, wxEXPAND|wxALL, 0);

	if (moosIni->Exists(wxT("autologin"))) {
		loginAs(Base64Decode(moosIni->Read(wxT("autologin"))),
				Base64Decode(moosIni->Read(wxT("accounts/")+moosIni->Read(wxT("autologin"))+wxT("/password")), true),
				moosIni->Read(wxT("accounts/")+moosIni->Read(wxT("autologin"))+wxT("/server"), wxT("netserver.earth2150.com")),
				moosIni->Read(wxT("accounts/")+moosIni->Read(wxT("autologin"))+wxT("/port"), wxT("17171")));
	}

	SetSizer(mainSizer);
	SetAutoLayout(true);
	chatInput->SetFocus();
}

MainFrame::~MainFrame() {
	wxRect rect=GetRect();
	if (IsMaximized()) {
		moosIni->Write(wxT("layout/maximized"), 1);
	} else {
		moosIni->Write(wxT("layout/maximized"), 0);
		moosIni->Write(wxT("layout/x"), rect.GetX());
		moosIni->Write(wxT("layout/y"), rect.GetY());
		moosIni->Write(wxT("layout/width"), rect.GetWidth());
		moosIni->Write(wxT("layout/height"), rect.GetHeight());
	}
	if (socket->IsConnected()) {
		socket->Close();
	}
	wxFileOutputStream tmp(MoosApp::CONFIG_FILE);
	moosIni->Save(tmp, wxConvUTF8);
}

void MainFrame::OnSelectChannel(wxCommandEvent &event) {
	write(wxT("/join \"")+channelSwitcher->GetStringSelection()+wxT("\""));
	whisperCheckbox->SetValue(false);
	userList->DeselectAll();
	chatInput->SetFocus();
}

void MainFrame::OnSelectUser(wxCommandEvent& event) {
	wxArrayInt selections;
	if (!userList->GetSelections(selections)) {
		whisperCheckbox->SetValue(false);
	}
	chatInput->SetFocus();
}

void MainFrame::OnWhisperClick(wxCommandEvent& event) {
	wxArrayInt selections;
	if (!userList->GetSelections(selections)) {
		whisperCheckbox->SetValue(false);
	}
	chatInput->SetFocus();
}

//---------- Chat --------------
void MainFrame::OnSendMessage(wxCommandEvent& WXUNUSED(event)) {
	if (viewMenu->IsChecked(ID_MAINWIN_FREEZE_CHAT)) {
		return;
	}
	if (!chatInput->GetValue().Len() || !socket->IsConnected()) {
		chatInput->Clear();
		return;
	}
	wxString tmp=chatInput->GetLineText(0);
	tmp.Replace(wxT("\n"), wxEmptyString);
	tmp.Replace(wxT("\t"), wxEmptyString);
	if (tmp[0]=='/') {
		if (tmp.Mid(1, 5)==wxT("slap ")) { // /ignore "xxx"
			if (settingsMenu->IsChecked(ID_MAINWIN_DISABLE_SLAPS)) {
				return;
			}
			InputDialog tmp2(this, langIni, font.GetChosenFont(), langIni->Read(wxT("translations/menus/user/slap"), wxT("Slap...")),
							 langIni->Read(wxT("translations/dialogtext/enterslap"), wxT("Please enter slap text")));
			if (tmp2.ShowModal()==wxID_OK && tmp2.GetValue()!=wxEmptyString) {
				if (tmp[6]=='\"' && tmp[tmp.Len()-1]=='\"') {
					write(wxT("/send \"<0xFF0000FF>slaps <0xFF2153E8>")+tmp.Mid(7, tmp.Len()-8)+wxT("<0xFF0000FF> with ")+tmp2.GetValue()+wxT("\""));
				}				  else {
					write(wxT("/send \"<0xFF0000FF>slaps <0xFF2153E8>")+tmp.Mid(6, tmp.Len()-6)+wxT("<0xFF0000FF> with ")+tmp2.GetValue()+wxT("\""));
				}
			}
		} else if (tmp.Mid(1, 7)==wxT("ignore ")) { // /ignore "xxx"
			if (tmp[8]=='\"' && tmp[tmp.Len()-1]=='\"')
				IgnoreUser(tmp.Mid(9, tmp.Len()-10));
			else
				IgnoreUser(tmp.Mid(8));
		} else if (tmp.Mid(1, 9)==wxT("unignore ")) { // /unignore "xxx"
			if (tmp[10]=='\"' && tmp[tmp.Len()-1]=='\"')
				UnignoreUser(tmp.Mid(11, tmp.Len()-12));
			else
				UnignoreUser(tmp.Mid(10));
		} else {
			write(tmp);
		}
		return chatInput->Clear();
	}
	tmp.Replace(wxT("%"), wxT("%25"));
	tmp.Replace(wxT("\""), wxT("%22"));
	if (whisperCheckbox->IsChecked()) { //whisper
		wxArrayInt selections;
		userList->GetSelections(selections);
		for (size_t i=0;i!=selections.GetCount();i++) {
			write(wxT("/msg \"")+userList->GetString(selections[i])+wxT("\" \"")+tmp+wxT("\""));
		}
	} else {
		write(wxT("/send \"")+Long2MoonCode(moosIni->Read(wxT("accounts/")+Base64Encode(loginName)+wxT("/chatcolor"),
											langIni->Read(wxT("translations/channel/defaultcolor"), 0xFFBB9F8C)))+tmp+wxT("\""));
	}
	chatInput->Clear();
}

void MainFrame::OnText(wxCommandEvent& event) {
	if (chatInput->GetInsertionPoint()==chatInput->GetLastPosition() && !chatInput->GetStringSelection().Len()
			&& !disableOnText && lastInput.Len()<chatInput->GetValue().Len()) {
		lastInput=chatInput->GetValue();
		int oldpos=chatInput->GetInsertionPoint();
		disableOnText=true;
		wxArrayString Cmd;
		Cmd.Add(wxT("/msg "));
		Cmd.Add(wxT("/beep "));
		Cmd.Add(wxT("/whois "));
		Cmd.Add(wxT("/slap "));
		Cmd.Add(wxT("/ignore "));
		Cmd.Add(wxT("/unignore "));
		for (size_t i=0;i!=Cmd.GetCount();i++) {
			if (chatInput->GetValue().Mid(0, Cmd[i].Len()).Lower()==Cmd[i]) {
				if (chatInput->GetValue()[Cmd[i].Len()]=='\"') {
					if (Cmd[i]!=wxT("/msg ")) {
						chatInput->SetValue(chatInput->GetValue().Mid(0, Cmd[i].Len()+1)+AutoComplete(chatInput->GetValue().Mid(Cmd[i].Len()+1), wxT("\"")));
					} else {
						chatInput->SetValue(chatInput->GetValue().Mid(0, Cmd[i].Len()+1)+AutoComplete(chatInput->GetValue().Mid(Cmd[i].Len()+1), wxT("\" ")));
					}
					chatInput->SetSelection(oldpos, chatInput->GetLastPosition());
				} else {
					if (Cmd[i]!=wxT("/msg ")) {
						chatInput->SetValue(chatInput->GetValue().Mid(0, Cmd[i].Len())+wxT("\"")+AutoComplete(chatInput->GetValue().Mid(Cmd[i].Len()), wxT("\"")));
					} else {
						chatInput->SetValue(chatInput->GetValue().Mid(0, Cmd[i].Len())+wxT("\"")+AutoComplete(chatInput->GetValue().Mid(Cmd[i].Len()), wxT("\" ")));
					}
					chatInput->SetSelection(oldpos+1, chatInput->GetLastPosition());
				}
			}
		}
	} else {
		if (!disableOnText) {
			lastInput=chatInput->GetValue();
		}
	}
	disableOnText=false;
}

wxString MainFrame::AutoComplete(wxString Beginning, wxString Ending, int Ignored) {
	if (Beginning==wxEmptyString) {
		wxArrayInt selections;
		if (userList->GetSelections(selections)==1) {
			return userList->GetString(selections[0])+Ending;
		}
	}
	for (int i=seenUsers.GetCount()-1;i!=-1;i--) {
		if (
			seenUsers[i].Mid(0, Beginning.Len()).Lower()==Beginning.Lower()
			&&
			(
				(!Ignored && !IsIgnored(seenUsers[i]))
				||
				Ignored==1
				||
				(Ignored==2 && IsIgnored(seenUsers[i]))
			)
		) {
			return seenUsers[i]+Ending;
		}
	}
	if (Beginning.Find(wxT("\""))!=-1) {
		return Beginning;
	}
	return Beginning+Ending;
}

//--------- Messages ------------
void MainFrame::Message(wxString Text, const wxString Input0, const wxString Input1, const wxString Input2, const wxString Input3) {
	if (Text==wxEmptyString) return;
	Text=Format(Text, Input0, Input1, Input2, Input3);
	if (viewMenu->IsChecked(ID_MAINWIN_FREEZE_CHAT)) {
		chatBuffer.Add(Text);
		return;
	}
	Text.Replace(wxT("<*>"), Long2MoonCode(langIni->Read(wxT("translations/channel/defaultcolor"), 0xFFBB9F8C)));
	Text.Replace(wxT("\\n"), wxEmptyString);
	Text.Replace(wxT("%22"), wxT("\""));
	Text.Replace(wxT("%25"), wxT("%"));

	wxString tmp;
	chatView->SetDefaultStyle(wxTextAttr(Long2Color(langIni->Read(wxT("translations/channel/defaultcolor"), 0xFFBB9F8C)), wxColour(31, 48, 56), font.GetChosenFont()));
	size_t i=0, opentag=0;
	while (i!=Text.Len()) {
		if (Text[i]!='<') {
			tmp+=Text[i++];
		} else { //<...
			if (Text[i+1]=='0' && (Text[i+2]=='x' || Text[i+2]=='X') && IsHex(Text.Mid(i+3, 8)) && Text[i+11]=='>') { //<0xAABBGGRR>
				chatView->AppendText(tmp);
				chatView->SetDefaultStyle(wxTextAttr(MoonCode2Color(Text.Mid(i, 12))));
				chatHistory+=tmp;
				if (opentag || MoonCode2Long(Text.Mid(i, 12))!=langIni->Read(wxT("translations/channel/defaultcolor"), 0xFFBB9F8C)) {
					if (opentag) {
						chatHistory+=wxT("</span>");
					}
					chatHistory+=MoonCode2HTML(Text.Mid(i, 12));
					opentag=1;
				}
				tmp=wxEmptyString;
				i+=12;
			} else if (Text[i+1]=='<' && Text[i+2]=='>') { //<<>
				chatHistory+=wxT("&lt;");
				tmp+=wxT("<");
				i+=3;
			} else {
				while (i!=Text.Len() && Text[i]!='>') {
					i++;
				}
				if (Text[i]=='>') {
					i++;
				}
			}
		}
	}
	chatView->AppendText(tmp+wxT("\n"));
	#ifdef WIN32
		int h;
		chatView->GetClientSize(0, &h);
		chatView->ShowPosition(chatView->XYToPosition(0, chatView->GetNumberOfLines()-(h/chatView->GetCharHeight())));
	#endif
	tmp.Replace(wxT("&"), wxT("&amp;"));
	chatHistory+=tmp;
	if (opentag) {
		chatHistory+=wxT("</span>");
	}
	chatHistory+=wxT("\n");
	chatInput->SetFocus();
}

//------ Channel -------
void MainFrame::AddChannel(wxString Channel) {
	channelSwitcher->Append(Channel);
	if (Channel==wxT("MoonNet")) {
		SetChannel(wxT("MoonNet"));
	}
}

void MainFrame::SetChannel(wxString Channel) {
	if (channelSwitcher->FindString(Channel)!=wxNOT_FOUND) {
		channelSwitcher->SetSelection(channelSwitcher->FindString(Channel));
	}
}

void MainFrame::RemoveChannel(wxString Channel) {
	if (channelSwitcher->FindString(Channel)!=wxNOT_FOUND) {
		channelSwitcher->Delete(channelSwitcher->FindString(Channel));
	}
}

void MainFrame::RemoveAllChannelsAndUsers() {
	channelSwitcher->Clear();
	RemoveAllUsers();
}

void MainFrame::RefreshAutocomplete(wxString User, bool Event) {
	if (User[0]!='#') {
		if (User[0]=='^') {
			User=User.Mid(1);
		}
		for (size_t i=0;i!=seenUsers.GetCount();i++) {
			if (seenUsers[i]==User) {
				if (Event || IsIgnored(User) || User==loginName) {
					seenUsers.RemoveAt(i);
					seenUsers.Add(User);
				}
				return;
			}
		}
		seenUsers.Add(User);
	}
}

