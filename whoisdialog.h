#ifndef WHOISDIALOG_H
#define WHOISDIALOG_H

#include <wx/dialog.h>

class wxFileConfig;
class wxSocketClient;
class wxBoxSizer;
class wxFlexGridSizer;
class wxStaticText;
class wxTextCtrl;
class wxChoice;
class wxButton;

class WhoisDialog: public wxDialog {
	public:
		WhoisDialog(wxWindow* parent, wxFileConfig *LangIni, wxArrayString WhoisData, bool IsYourself, wxSocketClient* Socket, wxFont Font);
		void OnChoice(wxCommandEvent &event);
		void OnApply(wxCommandEvent &event);
		void OnCancel(wxCommandEvent &event);
	private:
		WhoisDialog(const WhoisDialog& cc);
		WhoisDialog& operator=(const WhoisDialog& cc);

		wxBoxSizer *MainSizer, *BoxSizer, *ButtonSizer;
		wxFlexGridSizer *GridSizer, *GridSizer2, *GridSizer3;
		wxStaticText *lplayername, *lplayerstatus, *lemail, *licq, *lplace, *lage, *lgender, *ldescription,
		*lgamename, *llastplayed, *lstatistics, *lvictories, *ldefeats, *ldisconnects, *lposition, *lrank;
		wxTextCtrl *playername, *playerstatus, *email, *icq, *place, *age, *description, *lastplayed, *victories,
		*defeats, *disconnects, *position, *rank;
		wxChoice *gender, *gamename, *statistics;
		wxButton *applybutton, *cancelbutton;
		wxArrayString WhoisData;
		wxSocketClient *Socket;
	DECLARE_EVENT_TABLE()
};

#endif
