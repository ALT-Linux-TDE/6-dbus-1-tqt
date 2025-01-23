/* tqdbusunixfd.cpp DBUS unix file handle data type
 *
 * Copyright (C) 2013 Slávek Banko <slavek.banko@axis.cz>
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

#include <unistd.h> 
#include "tqdbusunixfd.h"

TQT_DBusUnixFd::TQT_DBusUnixFd() : d(new TQT_DBusUnixFdPrivate())
{
    d->ref();
    d->fd = -1;
};

TQT_DBusUnixFd::TQT_DBusUnixFd(const TQT_DBusUnixFd& other) : d(other.d)
{
    d->ref();
}

TQT_DBusUnixFd::TQT_DBusUnixFd(int other) : d(0)
{
    setFileDescriptor(other);
}

TQT_DBusUnixFd::~TQT_DBusUnixFd()
{
    if (d && d->deref() ) {
        if ( isValid() ) {
            close(d->fd);
        }
        delete d;
    }
}

bool TQT_DBusUnixFd::isValid() const
{
    return d ? d->fd != -1 : false;
}

int TQT_DBusUnixFd::fileDescriptor() const
{
    return d ? d->fd : -1;
}

void TQT_DBusUnixFd::setFileDescriptor(int fileDescriptor)
{
    giveFileDescriptor(fileDescriptor != -1 ? dup(fileDescriptor) : -1);
}

void TQT_DBusUnixFd::giveFileDescriptor(int fileDescriptor) 
{
    if ( d && d->deref() ) {
        if ( isValid() ) {
            close(d->fd);
        }
    }
    else {
        d = new TQT_DBusUnixFdPrivate;
    }
    d->ref();
    d->fd = fileDescriptor;
}

TQT_DBusUnixFd &TQT_DBusUnixFd::operator=( const TQT_DBusUnixFd &other )
{
    if (other.d) {
        other.d->ref();
    }
    if ( d && d->deref() ) {
        if ( isValid() ) {
            close(d->fd);
        }
        delete d;
    }
    d = other.d;
    return *this;
}
