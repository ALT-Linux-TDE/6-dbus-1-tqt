/* tqdbusobjectpath.cpp DBUS object path data type
 *
 * Copyright (C) 2007 Kevin Krammer <kevin.krammer@gmx.at>
 *
 * Licensed under the Academic Free License version 2.1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 *
 */

#include "tqdbusobjectpath.h"

TQT_DBusObjectPath::TQT_DBusObjectPath() : TQString()
{
}

TQT_DBusObjectPath::TQT_DBusObjectPath(const TQT_DBusObjectPath &other) : TQString(static_cast<const TQString&>(other))
{
}

TQT_DBusObjectPath::TQT_DBusObjectPath(const TQString &other) : TQString(static_cast<const TQString&>(other))
{
}

TQT_DBusObjectPath::TQT_DBusObjectPath(const TQT_DBusObjectPath &parentNode, const TQString &nodeName)
    : TQString(static_cast<const TQString&>(parentNode))
{
    if (parentNode.length() != 1)
    {
      append("/");
    }
    append(nodeName);
}

bool TQT_DBusObjectPath::isValid() const
{
    return (validate(*this) == -1);
}

TQT_DBusObjectPath TQT_DBusObjectPath::parentNode() const
{
    if (length() == 1)
    {
      return TQT_DBusObjectPath();
    }

    int index = findRev('/');
    if (index == -1)
    {
      return TQT_DBusObjectPath();
    }
    else if (index == 0)
    {
      return left(1);
    }

    return left(index);
}

int TQT_DBusObjectPath::validate(const TQString &path)
{
    if (path.isEmpty() || path[0] != '/')
    {
      return 0;
    }

    // only root node allowed to end in slash
    uint len = path.length();
    if (path[len - 1] == '/' && len > 1)
    {
      return (len - 1);
    }

    return -1;
}
