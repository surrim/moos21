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

bool MoosApp::OnInit() {
    wxImage::AddHandler(new wxPNGHandler);
    MainFrame *win=new MainFrame(wxT("moos2.1"), wxDefaultPosition, wxSize(720, 480));
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
    //Socket
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

MainFrame::MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size)
: wxFrame((wxFrame *) NULL, -1, title, pos, size) {
    bool firstrun=false;
    if (!wxFile::Exists(wxT("moos.ini"))) {
        wxFile(wxT("moos.ini"), wxFile::write);
        firstrun=true;
    }
    wxFileInputStream moos(wxT("moos.ini"));
    if (moos.CanRead()) MoosIni=LangIni=new wxFileConfig(moos, wxConvUTF8);
    if (MoosIni->Exists(wxT("languagefile"))) {
        if (wxFile::Exists(MoosIni->Read(wxT("languagefile")))) {
            wxFileInputStream lang(MoosIni->Read(wxT("languagefile"), wxT("")));
            if (lang.CanRead()) {
                LangIni=new wxFileConfig(lang, wxConvUTF8);
            }
        }
    }

    int x=MoosIni->Read(wxT("layout/x"), wxDefaultCoord);
    if (x<0 && x!=wxDefaultCoord) {
        x=0;
    }
    int y=MoosIni->Read(wxT("layout/y"), wxDefaultCoord);
    if (y<0 && y!=wxDefaultCoord) {
        y=0;
    }
    int width=MoosIni->Read(wxT("layout/width"), 720);
    if (x+width>wxGetDisplaySize().GetWidth()) {
        width=wxDefaultCoord;
    }
    int height=MoosIni->Read(wxT("layout/height"), 512);
    if (y+height>wxGetDisplaySize().GetHeight()) {
        height=wxDefaultCoord;
    }
    SetSize(x, y, width, height);
    if (MoosIni->Read(wxT("layout/maximized"), 0L)) {
        Maximize();
    }

    #ifdef WIN32
    SetIcon(wxICON(A));
    #else
    SetIcon(wxIcon(wxT("icons/moos.png"), wxBITMAP_TYPE_PNG));
    #endif
    CreateStatusBar();
    SetStatusText(LangIni->Read(wxT("translations/statusbar/notconnected"), wxT("Not connected")));
    wxFont tmp=wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    if (MoosIni->Exists(wxT("layout/fontface"))) tmp.SetFaceName(MoosIni->Read(wxT("layout/fontface")));
    if (MoosIni->Exists(wxT("layout/fontsize"))) {
        long tmp2;
        MoosIni->Read(wxT("layout/fontsize"), &tmp2);
        tmp.SetPointSize(tmp2);
    }
    Font.SetInitialFont(tmp);
    Font.SetChosenFont(tmp);
    Font.EnableEffects(false);
    SetBackgroundColour(wxColour(212, 208, 200));
    ChatHistory=wxT("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"><html><head><meta http-equiv=\"content-type\" content=\"text/html;charset=UTF-16LE\"><title>moos2.1</title><style type=\"text/css\">body{background-color:#1F3038; color:#8C9FBB}pre{font-family:\"ms sans serif\", sans-serif;}</style></head><body><pre>");

    MoosMenu=new wxMenu;
    MoosMenu->Append(ID_MAINWIN_LOGIN_AS, LangIni->Read(wxT("translations/menus/moos/loginas"), wxT("Login as...")));
    MoosMenu->Append(ID_MAINWIN_LOGOUT, LangIni->Read(wxT("translations/menus/moos/logout"), wxT("Logout")));
    MoosMenu->AppendSeparator();
    MoosMenu->Append(ID_MAINWIN_EXIT, LangIni->Read(wxT("translations/menus/moos/exit"), wxT("Exit")));

    UserMenu=new wxMenu;
    UserMenu->Append(ID_MAINWIN_WHOIS_USER, LangIni->Read(wxT("translations/menus/user/profile"), wxT("Userinformation...")));
    UserMenu->Append(ID_MAINWIN_SLAP_USER, LangIni->Read(wxT("translations/menus/user/slap"), wxT("Slap...")));
    UserMenu->Append(ID_MAINWIN_IGNORE_USER, LangIni->Read(wxT("translations/menus/user/ignore"), wxT("Ignore")));
    UserMenu->AppendSeparator();
    UserMenu->Append(ID_MAINWIN_IGNORELIST, LangIni->Read(wxT("translations/menus/user/ignorelist"), wxT("Ignorelist...")));

    ViewMenu=new wxMenu;
    ViewMenu->Append(ID_MAINWIN_SAVE_CHAT, LangIni->Read(wxT("translations/menus/view/savechathistory"), wxT("Save Chat...")));
    ViewMenu->AppendCheckItem(ID_MAINWIN_FREEZE_CHAT, LangIni->Read(wxT("translations/menus/view/freezechathistory"), wxT("Freeze Chat")));
    ViewMenu->Append(ID_MAINWIN_DELETE_CHAT, LangIni->Read(wxT("translations/menus/view/deletechathistory"), wxT("Delete Chat")));
    ViewMenu->AppendSeparator();
    ViewMenu->Append(ID_MAINWIN_COLORSELECT, LangIni->Read(wxT("translations/menus/view/selectcolor"), wxT("Select Color...")));
    ViewMenu->Append(ID_MAINWIN_MANUAL_COLORCODE, LangIni->Read(wxT("translations/menus/view/manualcolorcode"), wxT("Manual Color...")));

    SettingsMenu=new wxMenu;
    SettingsMenu->AppendCheckItem(ID_MAINWIN_AUTOLOGIN_ACCOUNT, LangIni->Read(wxT("translations/menus/settings/autologinaccount"), wxT("Activate AutoLogin for this Account")));
    SettingsMenu->AppendCheckItem(ID_MAINWIN_SOUND_ON_BEEP, LangIni->Read(wxT("translations/menus/settings/soundonbeep"), wxT("Play sound on beep")));
    SettingsMenu->Check(ID_MAINWIN_SOUND_ON_BEEP, MoosIni->Read(wxT("enablebeep"), 0L));
    SettingsMenu->AppendCheckItem(ID_MAINWIN_DISABLE_SLAPS, LangIni->Read(wxT("translations/menus/settings/disableslaps"), wxT("Disable Slaps")));
    SettingsMenu->Check(ID_MAINWIN_DISABLE_SLAPS, MoosIni->Read(wxT("disableslaps"), 0L));
    SettingsMenu->AppendSeparator();
    SettingsMenu->Append(ID_MAINWIN_CHANGE_FONT, LangIni->Read(wxT("translations/menus/settings/changefont"), wxT("Change Font...")));
    SettingsMenu->Append(ID_MAINWIN_SELECT_LANGUAGE, LangIni->Read(wxT("translations/menus/settings/selectlanguage"), wxT("Select Language...")));

    HelpMenu=new wxMenu;
    HelpMenu->Append(ID_MAINWIN_ONLINEREADME, LangIni->Read(wxT("translations/menus/help/onlinereadme"), wxT("Online Readme")));
    HelpMenu->Append(ID_MAINWIN_MAILBUGS, LangIni->Read(wxT("translations/menus/help/mailbugs"), wxT("Mail Bugs")));
    HelpMenu->AppendSeparator();
    HelpMenu->Append(ID_MAINWIN_ABOUT, LangIni->Read(wxT("translations/menus/help/about"), wxT("About Moos 2.1...")));

    MenuBar=new wxMenuBar;
    MenuBar->Append(MoosMenu, LangIni->Read(wxT("translations/menus/moos"), wxT("Moos")));
    MenuBar->Append(UserMenu, LangIni->Read(wxT("translations/menus/user"), wxT("User")));
    MenuBar->Append(ViewMenu, LangIni->Read(wxT("translations/menus/view"), wxT("View")));
    MenuBar->Append(SettingsMenu, LangIni->Read(wxT("translations/menus/settings"), wxT("Settings")));
    MenuBar->Append(HelpMenu, LangIni->Read(wxT("translations/menus/help"), wxT("?")));
    SetMenuBar(MenuBar);

    //Sizer
    MainSizer=new wxBoxSizer(wxHORIZONTAL);
    ListSizer=new wxBoxSizer(wxVERTICAL);
    ChatSizer=new wxBoxSizer(wxVERTICAL);
    InputSizer=new wxBoxSizer(wxHORIZONTAL);

    //Elemente
    SetFont(Font.GetChosenFont());
    ChannelSwitcher=new wxChoice(this, ID_MAINWIN_CHANNELSWITCHER, wxDefaultPosition, wxDefaultSize, 0, 0, wxSTATIC_BORDER);
    ChannelSwitcher->SetBackgroundColour(wxColour(31, 48, 56));
    ChannelSwitcher->SetForegroundColour(wxColour(140, 158, 180));
    UserList=new wxListBox(this, ID_MAINWIN_USERLIST, wxDefaultPosition, wxSize(200,200), 0, 0, wxLB_EXTENDED|wxLB_NEEDED_SB|wxLB_SORT|wxSTATIC_BORDER);
    UserList->SetBackgroundColour(wxColour(31, 48, 56));
    UserList->SetForegroundColour(wxColour(140, 158, 180));
    IgnoredUserList=new wxListBox(this, ID_MAINWIN_IGNOREDUSERLIST, wxDefaultPosition, wxSize(200,200), 0, 0, wxLB_EXTENDED|wxLB_NEEDED_SB|wxLB_SORT|wxSTATIC_BORDER);
    IgnoredUserList->SetBackgroundColour(wxColour(31, 48, 56));
    IgnoredUserList->SetForegroundColour(wxColour(140, 158, 180));
    IgnoredUserList->Hide();
    ChatView=new wxTextCtrl(this, ID_MAINWIN_CHATVIEW, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_RICH|wxTE_RICH2|wxTE_MULTILINE|wxTE_READONLY|wxSTATIC_BORDER);
    ChatView->SetBackgroundColour(wxColour(31, 48, 56));
    ChatInput=new wxTextCtrl(this, ID_MAINWIN_CHATINPUT, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER|wxWANTS_CHARS);
    WhisperCheckbox=new wxCheckBox(this, ID_MAINWIN_WHISPERCHECKBOX, LangIni->Read(wxT("translations/whisper"), wxT("Whisper")));
    Socket=new wxSocketClient();
    Socket->SetEventHandler(*this, ID_SOCKET);
    Socket->SetNotify(wxSOCKET_INPUT_FLAG|wxSOCKET_LOST_FLAG|wxSOCKET_OUTPUT_FLAG|wxSOCKET_CONNECTION_FLAG);
    Socket->Notify(true);
    if (firstrun) {
        wxCommandEvent tmp3;
        OnSelectLanguage(tmp3);
    }

    MainSizer->Add(ListSizer, 0, wxEXPAND|wxALL, 3);
    MainSizer->Add(ChatSizer, 1, wxEXPAND|wxALL, 3);

    ListSizer->Add(ChannelSwitcher, 0, wxEXPAND|wxALL, 0);
    ListSizer->AddSpacer(3);
    ListSizer->Add(UserList, 1, wxEXPAND|wxALL, 0);
    ListSizer->Add(IgnoredUserList, 1, wxEXPAND|wxALL, 0);
    ChatSizer->Add(ChatView, 1, wxEXPAND|wxALL, 0);
    ChatSizer->AddSpacer(1);
    ChatSizer->Add(InputSizer, 0, wxEXPAND|wxALL, 0);
    InputSizer->Add(ChatInput, 1, wxEXPAND|wxALL, 0);
    InputSizer->AddSpacer(3);
    InputSizer->Add(WhisperCheckbox, 0, wxEXPAND|wxALL, 0);

    if (MoosIni->Exists(wxT("autologin"))) {
        LoginAs(Base64Decode(MoosIni->Read(wxT("autologin"))),
                Base64Decode(MoosIni->Read(wxT("accounts/")+MoosIni->Read(wxT("autologin"))+wxT("/password")), true),
                MoosIni->Read(wxT("accounts/")+MoosIni->Read(wxT("autologin"))+wxT("/server"), wxT("netserver.earth2150.com")),
                MoosIni->Read(wxT("accounts/")+MoosIni->Read(wxT("autologin"))+wxT("/port"), wxT("17171")));
    }

    SetSizer(MainSizer);
    SetAutoLayout(true);
    ChatInput->SetFocus();
}

MainFrame::~MainFrame() {
    wxRect rect=GetRect();
    if (IsMaximized()) {
        MoosIni->Write(wxT("layout/maximized"), 1);
    } else {
        MoosIni->Write(wxT("layout/maximized"), 0);
        MoosIni->Write(wxT("layout/x"), rect.GetX());
        MoosIni->Write(wxT("layout/y"), rect.GetY());
        MoosIni->Write(wxT("layout/width"), rect.GetWidth());
        MoosIni->Write(wxT("layout/height"), rect.GetHeight());
    }
    if (Socket->IsConnected()) {
        Socket->Close();
    }
    wxFileOutputStream tmp(wxT("moos.ini"));
    MoosIni->Save(tmp, wxConvUTF8);
}

void MainFrame::OnSelectChannel(wxCommandEvent &event) {
    Write(wxT("/join \"")+ChannelSwitcher->GetStringSelection()+wxT("\""));
    WhisperCheckbox->SetValue(false);
    UserList->DeselectAll();
    ChatInput->SetFocus();
}

void MainFrame::OnSelectUser(wxCommandEvent& event) {
    wxArrayInt selections;
    if (!UserList->GetSelections(selections)) WhisperCheckbox->SetValue(false);
    ChatInput->SetFocus();
}

void MainFrame::OnWhisperClick(wxCommandEvent& event) {
    wxArrayInt selections;
    if (!UserList->GetSelections(selections)) WhisperCheckbox->SetValue(false);
    ChatInput->SetFocus();
}

//---------- Chat --------------
void MainFrame::OnSendMessage(wxCommandEvent& WXUNUSED(event)) {
    if (ViewMenu->IsChecked(ID_MAINWIN_FREEZE_CHAT)) {
        return;
    }
    if (!ChatInput->GetValue().Len() || !Socket->IsConnected()) {
        ChatInput->Clear();
        return;
    }
    wxString tmp=ChatInput->GetLineText(0);
    tmp.Replace(wxT("\n"), wxT(""));
    tmp.Replace(wxT("\t"), wxT(""));
    if (tmp[0]=='/') {
        if (tmp.Mid(1, 5)==wxT("slap ")) { // /ignore "xxx"
            if (SettingsMenu->IsChecked(ID_MAINWIN_DISABLE_SLAPS)) {
                return;
            }
            InputDialog tmp2(this, LangIni, Font.GetChosenFont(), LangIni->Read(wxT("translations/menus/user/slap"), wxT("Slap...")),
                             LangIni->Read(wxT("translations/dialogtext/enterslap"), wxT("Please enter slap text")));
            if (tmp2.ShowModal()==wxID_OK && tmp2.GetValue()!=wxT("")) {
                if (tmp[6]=='\"' && tmp[tmp.Len()-1]=='\"') {
                    Write(wxT("/send \"<0xFF0000FF>slaps <0xFF2153E8>")+tmp.Mid(7, tmp.Len()-8)+wxT("<0xFF0000FF> with ")+tmp2.GetValue()+wxT("\""));
                }                  else {
                    Write(wxT("/send \"<0xFF0000FF>slaps <0xFF2153E8>")+tmp.Mid(6, tmp.Len()-6)+wxT("<0xFF0000FF> with ")+tmp2.GetValue()+wxT("\""));
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
            Write(tmp);
        }
        return ChatInput->Clear();
    }
    tmp.Replace(wxT("%"), wxT("%25"));
    tmp.Replace(wxT("\""), wxT("%22"));
    if (WhisperCheckbox->IsChecked()) { //whisper
        wxArrayInt selections;
        UserList->GetSelections(selections);
        for (size_t i=0;i!=selections.GetCount();++i) Write(wxT("/msg \"")+UserList->GetString(selections[i])+wxT("\" \"")+tmp+wxT("\""));
    } else {
        Write(wxT("/send \"")+Long2MoonCode(MoosIni->Read(wxT("accounts/")+Base64Encode(LoginName)+wxT("/chatcolor"),
                                            LangIni->Read(wxT("translations/channel/defaultcolor"), 0xFFBB9F8C)))+tmp+wxT("\""));
    }
    ChatInput->Clear();
}

void MainFrame::OnText(wxCommandEvent& event) {
    if (ChatInput->GetInsertionPoint()==ChatInput->GetLastPosition() && !ChatInput->GetStringSelection().Len()
            && !DisableOnText && LastInput.Len()<ChatInput->GetValue().Len()) {
        LastInput=ChatInput->GetValue();
        int oldpos=ChatInput->GetInsertionPoint();
        DisableOnText=1;
        wxArrayString Cmd;
        Cmd.Add(wxT("/msg "));
        Cmd.Add(wxT("/beep "));
        Cmd.Add(wxT("/whois "));
        Cmd.Add(wxT("/slap "));
        Cmd.Add(wxT("/ignore "));
        Cmd.Add(wxT("/unignore "));
        for (size_t i=0;i!=Cmd.GetCount();++i) {
            if (ChatInput->GetValue().Mid(0, Cmd[i].Len()).Lower()==Cmd[i]) {
                if (ChatInput->GetValue()[Cmd[i].Len()]=='\"') {
                    if (Cmd[i]!=wxT("/msg ")) {
                        ChatInput->SetValue(ChatInput->GetValue().Mid(0, Cmd[i].Len()+1)+AutoComplete(ChatInput->GetValue().Mid(Cmd[i].Len()+1), wxT("\"")));
                    } else {
                        ChatInput->SetValue(ChatInput->GetValue().Mid(0, Cmd[i].Len()+1)+AutoComplete(ChatInput->GetValue().Mid(Cmd[i].Len()+1), wxT("\" ")));
                    }
                    ChatInput->SetSelection(oldpos, ChatInput->GetLastPosition());
                } else {
                    if (Cmd[i]!=wxT("/msg "))
                        ChatInput->SetValue(ChatInput->GetValue().Mid(0, Cmd[i].Len())+wxT("\"")+AutoComplete(ChatInput->GetValue().Mid(Cmd[i].Len()), wxT("\"")));
                    else
                        ChatInput->SetValue(ChatInput->GetValue().Mid(0, Cmd[i].Len())+wxT("\"")+AutoComplete(ChatInput->GetValue().Mid(Cmd[i].Len()), wxT("\" ")));
                    ChatInput->SetSelection(oldpos+1, ChatInput->GetLastPosition());
                }
            }
        }
    } else {
        if (!DisableOnText) LastInput=ChatInput->GetValue();
    }
    DisableOnText=0;
}

wxString MainFrame::AutoComplete(wxString Beginning, wxString Ending, int Ignored) {
    if (Beginning==wxT("")) {
        wxArrayInt selections;
        if (UserList->GetSelections(selections)==1) return UserList->GetString(selections[0])+Ending;
    }
    for (int i=SeenUsers.GetCount()-1;i!=-1;--i) {
        if (SeenUsers[i].Mid(0, Beginning.Len()).Lower()==Beginning.Lower() && ((!Ignored && !IsIgnored(SeenUsers[i])) || Ignored==1
                || (Ignored==2 && IsIgnored(SeenUsers[i])))) return SeenUsers[i]+Ending;
    }
    if (Beginning.Find(wxT("\""))!=-1) return Beginning;
    return Beginning+Ending;
}

//--------- Messages ------------
void MainFrame::Message(wxString Text, const wxString Input0, const wxString Input1, const wxString Input2, const wxString Input3) {
    if (Text==wxT("")) return;
    Text=Format(Text, Input0, Input1, Input2, Input3);
    if (ViewMenu->IsChecked(ID_MAINWIN_FREEZE_CHAT)) {
        ChatBuffer.Add(Text);
        return;
    }
    Text.Replace(wxT("<*>"),
                 Long2MoonCode(LangIni->Read(wxT("translations/channel/defaultcolor"), 0xFFBB9F8C)));
    Text.Replace(wxT("\\n"),wxT(""));
    Text.Replace(wxT("%22"), wxT("\""));
    Text.Replace(wxT("%25"), wxT("%"));

    wxString tmp;
    ChatView->SetDefaultStyle(wxTextAttr(Long2Color(LangIni->Read(wxT("translations/channel/defaultcolor"), 0xFFBB9F8C)), wxNullColour, Font.GetChosenFont()));
    size_t i=0, opentag=0;
    while (i!=Text.Len()) {
        if (Text[i]!='<') {
            tmp+=Text[i++];
        } else { //<...
            if (Text[i+1]=='0' && (Text[i+2]=='x' || Text[i+2]=='X') && IsHex(Text.Mid(i+3, 8)) && Text[i+11]=='>') { //<0xAABBGGRR>
                ChatView->AppendText(tmp);
                ChatView->SetDefaultStyle(wxTextAttr(MoonCode2Color(Text.Mid(i, 12))));
                ChatHistory+=tmp;
                if (opentag || MoonCode2Long(Text.Mid(i, 12))!=LangIni->Read(wxT("translations/channel/defaultcolor"), 0xFFBB9F8C)) {
                    if (opentag) {
                        ChatHistory+=wxT("</span>");
                    }
                    ChatHistory+=MoonCode2HTML(Text.Mid(i, 12));
                    opentag=1;
                }
                tmp=wxT("");
                i+=12;
            } else if (Text[i+1]=='<' && Text[i+2]=='>') { //<<>
                ChatHistory+=wxT("&lt;");
                tmp+=wxT("<");
                i+=3;
            } else {
                while (i!=Text.Len() && Text[i]!='>') i++;
                if (Text[i]=='>') i++;
            }
        }
    }
    ChatView->AppendText(tmp+wxT("\n"));
    #ifdef WIN32
    int h;
    ChatView->GetClientSize(0, &h);
    ChatView->ShowPosition(ChatView->XYToPosition(0, ChatView->GetNumberOfLines()-(h/ChatView->GetCharHeight())));
    #endif
    tmp.Replace(wxT("&"), wxT("&amp;"));
    ChatHistory+=tmp;
    if (opentag) {
        ChatHistory+=wxT("</span>");
    }
    ChatHistory+=wxT("\n");
    return;
}

//------ Channel -------
void MainFrame::AddChannel(wxString Channel) {
    ChannelSwitcher->Append(Channel);
    if (Channel==wxT("MoonNet")) {
        SetChannel(wxT("MoonNet"));
    }
}

void MainFrame::SetChannel(wxString Channel) {
    if (ChannelSwitcher->FindString(Channel)!=wxNOT_FOUND) {
        ChannelSwitcher->SetSelection(ChannelSwitcher->FindString(Channel));
    }
}

void MainFrame::RemoveChannel(wxString Channel) {
    if (ChannelSwitcher->FindString(Channel)!=wxNOT_FOUND) {
        ChannelSwitcher->Delete(ChannelSwitcher->FindString(Channel));
    }
}

void MainFrame::RemoveAllChannelsAndUsers() {
    ChannelSwitcher->Clear();
    RemoveAllUsers();
}

void MainFrame::RefreshAutocomplete(wxString User, bool Event) {
    if (User[0]!='#') {
        if (User[0]=='^') {
            User=User.Mid(1);
        }
        for (size_t i=0;i!=SeenUsers.GetCount();++i) {
            if (SeenUsers[i]==User) {
                if (Event || IsIgnored(User) || User==LoginName) {
                    SeenUsers.RemoveAt(i);
                    SeenUsers.Add(User);
                }
                return;
            }
        }
        SeenUsers.Add(User);
    }
}

