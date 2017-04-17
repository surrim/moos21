/* Copyright 2011-2017 surrim
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

#include "tools.h"

wxString Format(wxString Text, const wxString Input0, const wxString Input1, const wxString Input2, const wxString Input3) {
	Text.Replace(wxT("<%T>"), wxDateTime::Now().FormatISOTime());
	Text.Replace(wxT("<%0>"), Input0);
	Text.Replace(wxT("<%1>"), Input1);
	Text.Replace(wxT("<%2>"), Input2);
	Text.Replace(wxT("<%3>"), Input3);
	return Text;
}

bool IsHex(const wxString Text) {
	for (size_t i=0;i!=Text.Len();i++) {
		if (!(Text[i]>='0' && Text[i]<='9') && !(Text[i]>='a' && Text[i]<='f') && !(Text[i]>='A' && Text[i]<='F')) {
			return false;
		}
	}
	return true;
}

unsigned char Hex2Number(const unsigned char Hex) { //F --> 15
	if (Hex>='0' && Hex<='9') return Hex-48;
	else if (Hex>='A' && Hex<='F') return Hex-55;
	return Hex-55;
}

wxString Number2Hex(const unsigned char Number) { //255 --> FF
	wxString tmp=wxT("xx");
	wxString hexChars=wxT("0123456789ABCDEF");
	tmp[0]=hexChars[Number>>4];
	tmp[1]=hexChars[Number&0xF];
	return tmp;
}

wxString Long2MoonCode(const long Color) { //AABBGGRR --> <0xAABBGGRR>
	return wxT("<0x")+Number2Hex(Color>>24)+Number2Hex((Color>>16)&0xFF)
		   +Number2Hex((Color>>8)&0xFF)+Number2Hex(Color&0xFF)+wxT(">");
}

long Color2Long(const wxColour Color) { //Color --> FFBBGGRR
	return 0xFF000000|(Color.Blue()<<16)|(Color.Green()<<8)|Color.Red();
}

long MoonCode2Long(const wxString MoonCode) { //<0xAABBGGRR> --> FFBBGGRR
	return (Hex2Number(MoonCode[3])<<28)|(Hex2Number(MoonCode[4])<<24)
		   |(Hex2Number(MoonCode[5])<<20)|(Hex2Number(MoonCode[6])<<16)
		   |(Hex2Number(MoonCode[7])<<12)|(Hex2Number(MoonCode[8])<<8)
		   |(Hex2Number(MoonCode[9])<<4)|Hex2Number(MoonCode[10]);
}

wxColour MoonCode2Color(const wxString MoonCode) { //<0xAABBGGRR> --> FFBBGGRR
	unsigned char a=(Hex2Number(MoonCode[3])<<4)|Hex2Number(MoonCode[4]);
	unsigned char b=a*((Hex2Number(MoonCode[5])<<4)|Hex2Number(MoonCode[6]))/255;
	unsigned char g=a*((Hex2Number(MoonCode[7])<<4)|Hex2Number(MoonCode[8]))/255;
	unsigned char r=a*((Hex2Number(MoonCode[9])<<4)|Hex2Number(MoonCode[10]))/255;
	return wxColour(r, g, b);
}

wxColour Long2Color(const long Long) { //<0xAABBGGRR> --> FFBBGGRR
	unsigned char a=Long>>24;
	unsigned char b=a*((Long>>16)&0xFF)/255;
	unsigned char g=a*((Long>>8)&0xFF)/255;
	unsigned char r=a*(Long&0xFF)/255;
	return wxColour(r, g, b);
}

wxString MoonCode2HTML(const wxString MoonCode) { //<0xAABBGGRR> --> RRGGBB
	unsigned char a=(Hex2Number(MoonCode[3])<<4)|Hex2Number(MoonCode[4]);
	return wxT("<span style=\"color:#")+Number2Hex(a*((Hex2Number(MoonCode[9])<<4)|Hex2Number(MoonCode[10]))/255)
		   +Number2Hex(a*((Hex2Number(MoonCode[7])<<4)|Hex2Number(MoonCode[8]))/255)
		   +Number2Hex(a*((Hex2Number(MoonCode[5])<<4)|Hex2Number(MoonCode[6]))/255)+wxT("\">");
}

wxString Base64Encode(const wxString& data, bool encrypt) {
	std::string cvt="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-";
	if (encrypt) {
		cvt="WS4FtwfL8DTrjzJN9PY0gOohxbC71VMn3Zs6BQm5pd2aHyRe+kIUEvqK-iGlcXAu";
	}
	wxChar c;
	int i, len=data.length();
	std::string ret;
	for (i=0;i<len;i++) {
		c=((char)data[i]>>2)&0x3f;
		ret+=cvt[(int)c];
		c=((char)data[i]<<4)&0x3f;
		if (++i<len) c|=((char)data[i]>>4)&0x0f;
		ret+=cvt[(int)c];
		if (i<len) {
			c=((char)data[i]<<2)&0x3f;
			if (++i<len) c|=((char)data[i]>>6)&0x03;
			ret+=cvt[(int)c];
		} else {
			i++;
			ret+='=';
		}
		if (i<len) {
			c=(char)data[i]&0x3f;
			ret+=cvt[(int)c];
		} else {
			ret+='=';
		}
	}
	return wxString(ret.data(), wxConvISO8859_1, ret.size());
}

wxString Base64Decode(const wxString& data, bool encrypt) {
	std::string cvt="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-";
	if (encrypt) {
		cvt="WS4FtwfL8DTrjzJN9PY0gOohxbC71VMn3Zs6BQm5pd2aHyRe+kIUEvqK-iGlcXAu";
	}
	wxChar c, c1;
	int i, len=data.length();
	std::string ret;
	for (i=0;i<len;i++) {
		c=(wxChar)cvt.find(data[i]);
		i++;
		c1=(wxChar)cvt.find(data[i]);
		c=(c<<2)|((c1>>4)&0x3);
		ret+=c;
		if (++i<len) {
			c=data[i];
			if (c=='=') break;
			c=(wxChar)cvt.find(c);
			c1=((c1<<4)&0xf0)|((c>>2)&0xf);
			ret+=c1;
		}
		if (++i<len) {
			c1=data[i];
			if (c1=='=') break;
			c1=(wxChar)cvt.find(c1);
			c=((c<<6)&0xc0)|c1;
			ret+=c;
		}
	}
	return wxString(ret.data(), wxConvISO8859_1, ret.size());
}
