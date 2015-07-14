#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <wx/dialog.h>

class wxBoxSizer;
class wxStaticText;
class wxListBox;
class wxTextCtrl;
class wxButton;
class wxString;
class wxFileConfig;

class InputDialog: public wxDialog {
	public:
		InputDialog(wxWindow* parent, wxFileConfig *langIni, const wxFont& font, const wxString& title, const wxString& text=wxEmptyString, int style=0, const wxString& defaultButton=wxEmptyString, const wxArrayString& choiceList=wxArrayString());
		void OnText(wxCommandEvent& Event);
		void OnChoice(wxCommandEvent& event);
		void OnOK(wxCommandEvent& event);
		void OnCancel(wxCommandEvent& event);
		wxString GetValue();
		wxArrayString GetSelections();
		void SetSelections(wxArrayInt& selections);
	private:
		InputDialog(const InputDialog& cc);
		InputDialog& operator=(const InputDialog& cc);

		wxBoxSizer *mainSizer;
		wxBoxSizer *buttonSizer;
		wxStaticText *ltext;
		wxListBox *choice;
		wxTextCtrl *input;
		wxButton *okButton;
		wxButton *cancelButton;
		wxString saved;
		int style;
	DECLARE_EVENT_TABLE()
};

#endif
