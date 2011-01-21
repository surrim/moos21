#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <wx/dialog.h>

class wxBoxSizer;
class wxStaticText;
class wxButton;
class wxString;
class wxFileConfig;

class InfoDialog: public wxDialog {
	public:
		InfoDialog(wxWindow* parent, wxFileConfig *LangIni, wxFont Font, wxString Title, wxString Text, int Style=0, int Default=0);
		void OnButton1(wxCommandEvent &event);
		void OnButton2(wxCommandEvent &event);
	private:
		InfoDialog(const InfoDialog& cc);
		InfoDialog& operator=(const InfoDialog& cc);

		wxBoxSizer *MainSizer, *ButtonSizer;
		wxStaticText *ltext;
		wxButton *button1, *button2;
		int Style;
	DECLARE_EVENT_TABLE()
};

#endif
