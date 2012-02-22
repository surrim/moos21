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
		InputDialog(wxWindow* parent, wxFileConfig *langIni, wxFont font, wxString Title, wxString Text=L"", int Style=0, wxString Default=L"", wxArrayString ChoiceList=wxArrayString());
		void OnText(wxCommandEvent& Event);
		void OnChoice(wxCommandEvent &event);
		void OnOK(wxCommandEvent &event);
		void OnCancel(wxCommandEvent &event);
		wxString GetValue();
		wxArrayString GetSelections();
		void SetSelections(wxArrayInt& selections);
	private:
		InputDialog(const InputDialog& cc);
		InputDialog& operator=(const InputDialog& cc);

		wxBoxSizer *MainSizer, *ButtonSizer;
		wxStaticText *ltext;
		wxListBox *choice;
		wxTextCtrl *input;
		wxButton *okbutton, *cancelbutton;
		wxString saved;
		int Style;
	DECLARE_EVENT_TABLE()
};

#endif
