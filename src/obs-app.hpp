/******************************************************************************
    Copyright (C) 2013 by Hugh Bailey <obs.jim@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#pragma once

#include <QApplication>
#include <util/util.hpp>

class OBSApp : public QApplication {
	Q_OBJECT

private:
	std::string locale;
	std::string theme;
	bool themeDarkMode = true;
	ConfigFile globalConfig;
	TextLookup textLookup;

public:
	inline const char *GetString(const char *lookupVal) const
	{
		return textLookup.GetString(lookupVal);
	}
};

inline OBSApp *App()
{
	return static_cast<OBSApp *>(qApp);
}

inline const char *Str(const char *lookup)
{
	return App()->GetString(lookup);
}
#define QTStr(lookupVal) QString::fromUtf8(Str(lookupVal))
