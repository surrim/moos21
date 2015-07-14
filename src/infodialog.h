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
		InfoDialog(wxWindow* parent, wxFileConfig *langIni, const wxFont& font, const wxString& title, const wxString& text, int style=0, int defaultButton=0);
		void OnButton1(wxCommandEvent& event);
		void OnButton2(wxCommandEvent& event);
	private:
		InfoDialog(const InfoDialog& cc);
		InfoDialog& operator=(const InfoDialog& cc);

		wxBoxSizer *mainSizer;
		wxBoxSizer *buttonSizer;
		wxStaticText *ltext;
		wxButton *button1;
		wxButton *button2;
		int style;
	DECLARE_EVENT_TABLE()
};

#endif
