#ifndef BASE_H
#define BASE_H

#include <vector>
#include <wx/app.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/frame.h>
#include <wx/cmndata.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/textctrl.h>

class wxSocketEvent;
class wxTaskBarIcon;
class wxFileConfig;
class wxSocketClient;

struct version {
	version(): id(0), name(){
	}
	int id;
	wxString name;
};

class MoosApp: public wxApp {
	public:
		static const wxString CONFIG_FILE;

		virtual bool OnInit();
};

class MainFrame: public wxFrame {
	public:
		MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size);
		~MainFrame();

		//base.cpp - Common
		void OnSelectChannel(wxCommandEvent &event);
		void OnSelectUser(wxCommandEvent& event);
		void OnWhisperClick(wxCommandEvent& event);
		void OnSendMessage(wxCommandEvent& WXUNUSED(event));
		void OnText(wxCommandEvent& event);
		wxString AutoComplete(wxString Beginning, wxString Ending=wxEmptyString, int Ignored=0);
		void Message(wxString Text, wxString Input0=wxEmptyString, wxString Input1=wxEmptyString, wxString Input2=wxEmptyString, wxString Input3=wxEmptyString);
		void AddChannel(wxString Channel);
		void SetChannel(wxString Channel);
		void RemoveChannel(wxString Channel);
		void RemoveAllChannelsAndUsers();
		void RefreshAutocomplete(wxString User, bool Event=true);

		//connection.cpp - socket
		void OnSocketEvent(wxSocketEvent& event);
		void OnConnectionLost();
		void OnConnect();
		void OnIncomingData();

		void parseProcessCommand(const wxString& strData);
		wxString getVersion(const wxString& id);
		std::string encodeMessage(const std::string& message);
		std::string getInvertedLength(unsigned int length);
		std::string generateLoginMessage(const wxString& loginUser, const wxString& loginPassword);
		void write(const wxString& data);
		void loginAs(const wxString& loginName, const wxString& loginPassword, const wxString& server=wxEmptyString, const wxString& port=wxEmptyString);

		//moosmenu.cpp - Moos Menu
		void OnLoginAs(wxCommandEvent &event);
		void OnLogout(wxCommandEvent &event);
		void OnExit(wxCommandEvent &event);

		//usermenu.cpp - User Menu
		void OnWhoisUser(wxCommandEvent &event);
		void OnSlapUser(wxCommandEvent &event);
		void OnIgnoreUser(wxCommandEvent &event);
		void OnIgnoreList(wxCommandEvent & WXUNUSED(event));
		void AddUser(wxString Username);
		void IgnoreUser(wxString User);
		void UnignoreUser(wxString User);
		bool IsIgnored(wxString User);
		void RemoveUser(wxString User);
		void RemoveAllUsers();

		//viewmenu.cpp - Ansicht Menu
		void OnSaveChat(wxCommandEvent &event);
		void OnFreezeChat(wxCommandEvent & WXUNUSED(event));
		void OnDeleteChat(wxCommandEvent &event);
		void OnColorSelect(wxCommandEvent & WXUNUSED(event));
		void OnManualColorCode(wxCommandEvent & WXUNUSED(event));

		//settingsmenu.cpp - Settings Menu
		void OnAutoLoginAccount(wxCommandEvent& event);
		void OnBeepSound(wxCommandEvent& event);
		void OnDisableSlaps(wxCommandEvent& event);
		void OnChangeFont(wxCommandEvent & WXUNUSED(event));
		void OnSelectLanguage(wxCommandEvent & WXUNUSED(event));

		//helpmenu.cpp - Help Menu
		void OnOnlineReadme(wxCommandEvent &event);
		void OnMailBugs(wxCommandEvent &event);
		void OnAbout(wxCommandEvent &event);
	private:
		MainFrame(const MainFrame& cc);
		MainFrame& operator=(const MainFrame& cc);

		wxBoxSizer *mainSizer;
		wxBoxSizer *listSizer;
		wxBoxSizer *chatSizer;
		wxBoxSizer *inputSizer;
		wxString chatHistory;
		wxString lastInput;
		wxArrayString chatBuffer;
		wxArrayString seenUsers;
		wxListBox *userList;
		wxListBox *ignoredUserList;
		wxFontData font;
		wxChoice *channelSwitcher;
		wxTextCtrl *chatView;
		wxTextCtrl *chatInput;
		wxCheckBox *whisperCheckbox;
		wxMenu *moosMenu;
		wxMenu *userMenu;
		wxMenu *viewMenu;
		wxMenu *settingsMenu;
		wxMenu *helpMenu;
		wxTaskBarIcon *moosIcon;
		wxMenuBar *menuBar;
		wxFileConfig *moosIni;
		wxFileConfig *langIni;
		wxSocketClient *socket;
		bool disableOnText;
		wxString loginName;
		wxString loginPassword;
		std::vector<version> gameVersions;

	DECLARE_EVENT_TABLE()
};

#endif
