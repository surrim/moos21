#ifndef TOOLS_H
#define TOOLS_H

#include <wx/colour.h>
#include <wx/string.h>

wxString Format(wxString Text, const wxString Input0, const wxString Input1=L"", const wxString Input2=L"", const wxString Input3=L"");
bool IsHex(const wxString Text);
unsigned char Hex2Number(const unsigned char Hex); //F --> 15
wxString Number2Hex(const unsigned char Number); //255 --> FF
wxString Long2MoonCode(const long Color); //AABBGGRR --> <0xAABBGGRR>
long Color2Long(const wxColour Color); //Color --> FFBBGGRR
long MoonCode2Long(const wxString MoonCode); //<0xAABBGGRR> --> FFBBGGRR
wxColour MoonCode2Color(const wxString MoonCode); //<0xAABBGGRR> --> FFBBGGRR
wxColour Long2Color(const long Long); //<0xAABBGGRR> --> FFBBGGRR
wxString MoonCode2HTML(const wxString MoonCode); //<0xAABBGGRR> --> RRGGBB
wxString Base64Encode(const wxString &data, const bool encrypt=false);
wxString Base64Decode(const wxString &data, const bool encrypt=false);

#endif
