/* Copyright 2011-2023 surrim
 *
 * This file is part of moos.
 *
 * moos is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * moos is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with moos.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TOOLS_H
#define TOOLS_H

#include <wx/colour.h>
#include <wx/string.h>

wxString Format(wxString Text, const wxString Input0, const wxString Input1=wxEmptyString, const wxString Input2=wxT(""), const wxString Input3=wxEmptyString);
bool IsHex(const wxString Text);
unsigned char Hex2Number(const unsigned char Hex); //F --> 15
wxString Number2Hex(const unsigned char Number); //255 --> FF
wxString Long2MoonCode(const long Color); //AABBGGRR --> <0xAABBGGRR>
long Color2Long(const wxColour Color); //Color --> FFBBGGRR
long MoonCode2Long(const wxString MoonCode); //<0xAABBGGRR> --> FFBBGGRR
wxColour MoonCode2Color(const wxString MoonCode); //<0xAABBGGRR> --> FFBBGGRR
wxColour Long2Color(const long Long); //<0xAABBGGRR> --> FFBBGGRR
wxString MoonCode2HTML(const wxString MoonCode); //<0xAABBGGRR> --> RRGGBB
wxString Base64Encode(const wxString& data, bool encrypt=false);
wxString Base64Decode(const wxString& data, bool encrypt=false);

#endif
