/* qdbuserror.cpp TQT_DBusError object
 *
 * Copyright (C) 2005 Harald Fernengel <harry@kdevelop.org>
 * Copyright (C) 2005-2007 Kevin Krammer <kevin.krammer@gmx.at>
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

#include "tqdbuserror.h"

#include <dbus/dbus.h>

#include <tqmap.h>

typedef TQMap<TQString, TQT_DBusError::ErrorType> ErrorNameMap;
static ErrorNameMap errorTypesByName;

static TQString qDBusErrorNameForType(TQT_DBusError::ErrorType type)
{
    switch (type)
    {
        case TQT_DBusError::InvalidError:
            Q_ASSERT(false);
            return TQString();

        case TQT_DBusError::Failed:
            return TQString::fromUtf8(DBUS_ERROR_FAILED);
        case TQT_DBusError:: NoMemory:
            return TQString::fromUtf8(DBUS_ERROR_NO_MEMORY);
        case TQT_DBusError:: ServiceUnknown:
            return TQString::fromUtf8(DBUS_ERROR_SERVICE_UNKNOWN);
        case TQT_DBusError:: NameHasNoOwner:
            return TQString::fromUtf8(DBUS_ERROR_NAME_HAS_NO_OWNER);
        case TQT_DBusError:: NoReply:
            return TQString::fromUtf8(DBUS_ERROR_NO_REPLY);
        case TQT_DBusError:: IOError:
            return TQString::fromUtf8(DBUS_ERROR_IO_ERROR);
        case TQT_DBusError:: BadAddress:
            return TQString::fromUtf8(DBUS_ERROR_BAD_ADDRESS);
        case TQT_DBusError:: NotSupported:
            return TQString::fromUtf8(DBUS_ERROR_NOT_SUPPORTED);
        case TQT_DBusError:: LimitsExceeded:
            return TQString::fromUtf8(DBUS_ERROR_LIMITS_EXCEEDED);
        case TQT_DBusError:: AccessDenied:
            return TQString::fromUtf8(DBUS_ERROR_ACCESS_DENIED);
        case TQT_DBusError:: AuthFailed:
            return TQString::fromUtf8(DBUS_ERROR_AUTH_FAILED);
        case TQT_DBusError:: NoServer:
            return TQString::fromUtf8(DBUS_ERROR_NO_SERVER);
        case TQT_DBusError:: Timeout:
            return TQString::fromUtf8(DBUS_ERROR_TIMEOUT);
        case TQT_DBusError:: NoNetwork:
            return TQString::fromUtf8(DBUS_ERROR_NO_NETWORK);
        case TQT_DBusError:: Disconnected:
            return TQString::fromUtf8(DBUS_ERROR_DISCONNECTED);
        case TQT_DBusError:: InvalidArgs:
            return TQString::fromUtf8(DBUS_ERROR_INVALID_ARGS);
        case TQT_DBusError:: FileNotFound:
            return TQString::fromUtf8(DBUS_ERROR_FILE_NOT_FOUND);
        case TQT_DBusError:: FileExists:
            return TQString::fromUtf8(DBUS_ERROR_FILE_EXISTS);
        case TQT_DBusError:: UnknownMethod:
            return TQString::fromUtf8(DBUS_ERROR_UNKNOWN_METHOD);
        case TQT_DBusError:: TimedOut:
            return TQString::fromUtf8(DBUS_ERROR_TIMED_OUT);
        case TQT_DBusError:: InvalidSignature:
            return TQString::fromUtf8(DBUS_ERROR_INVALID_SIGNATURE);

        case TQT_DBusError::UserDefined:
            Q_ASSERT(false);
            return TQString();
    }

    Q_ASSERT(false);
    return TQString();
}

static void qDBusErrorSetupNameMapping()
{
    for (int i = TQT_DBusError::InvalidError + 1; i < TQT_DBusError::UserDefined; ++i)
    {
        TQT_DBusError::ErrorType type = static_cast<TQT_DBusError::ErrorType>(i);
        errorTypesByName[qDBusErrorNameForType(type)] = type;
    }
}

static TQT_DBusError::ErrorType qDBusErrorTypeForName(const TQString& name)
{
    if (name.isEmpty()) return TQT_DBusError::InvalidError;

    if (errorTypesByName.isEmpty())
        qDBusErrorSetupNameMapping();

    ErrorNameMap::const_iterator it = errorTypesByName.find(name);
    if (it != errorTypesByName.end()) return it.data();

    return TQT_DBusError::UserDefined;
}

TQT_DBusError::TQT_DBusError() : errorType(InvalidError), m_dbusErrorSet(false)
{
}

TQT_DBusError::TQT_DBusError(const DBusError *error) : errorType(InvalidError), m_dbusErrorSet(false)
{
    if (!error || !dbus_error_is_set(error))
        return;

    nm = TQString::fromUtf8(error->name);
    msg = TQString::fromUtf8(error->message);

    errorType = qDBusErrorTypeForName(nm);
}

TQT_DBusError::TQT_DBusError(const TQString& error, const TQString& message)
    : errorType(UserDefined), m_dbusErrorSet(false), nm(error), msg(message)
{
    errorType = qDBusErrorTypeForName(nm);
}

bool TQT_DBusError::isValid() const
{
    return errorType != InvalidError && !nm.isEmpty() && !msg.isEmpty();
}

TQT_DBusError::TQT_DBusError(ErrorType type, const TQString& message)
    : errorType(type), m_dbusErrorSet(false), msg(message)
{
    nm = qDBusErrorNameForType(type);
}

TQT_DBusError TQT_DBusError::stdFailed(const TQString& message)
{
    return TQT_DBusError(TQT_DBusError::Failed, message);
}

TQT_DBusError TQT_DBusError::stdNoMemory(const TQString& message)
{
    return TQT_DBusError(TQT_DBusError::NoMemory, message);
}

TQT_DBusError TQT_DBusError::stdNoReply(const TQString& message)
{
    return TQT_DBusError(TQT_DBusError::NoReply, message);
}

TQT_DBusError TQT_DBusError::stdIOError(const TQString& message)
{
    return TQT_DBusError(TQT_DBusError::IOError, message);
}

TQT_DBusError TQT_DBusError::stdNotSupported(const TQString& message)
{
    return TQT_DBusError(TQT_DBusError::NotSupported, message);
}

TQT_DBusError TQT_DBusError::stdLimitsExceeded(const TQString& message)
{
    return TQT_DBusError(TQT_DBusError::LimitsExceeded, message);
}

TQT_DBusError TQT_DBusError::stdAccessDenied(const TQString& message)
{
    return TQT_DBusError(TQT_DBusError::AccessDenied, message);
}

TQT_DBusError TQT_DBusError::stdAuthFailed(const TQString& message)
{
    return TQT_DBusError(TQT_DBusError::AuthFailed, message);
}

TQT_DBusError TQT_DBusError::stdTimeout(const TQString& message)
{
    return TQT_DBusError(TQT_DBusError::Timeout, message);
}

TQT_DBusError TQT_DBusError::stdInvalidArgs(const TQString& message)
{
    return TQT_DBusError(TQT_DBusError::InvalidArgs, message);
}

TQT_DBusError TQT_DBusError::stdFileNotFound(const TQString& message)
{
    return TQT_DBusError(TQT_DBusError::FileNotFound, message);
}

TQT_DBusError TQT_DBusError::stdFileExists(const TQString& message)
{
    return TQT_DBusError(TQT_DBusError::FileExists, message);
}

TQT_DBusError TQT_DBusError::stdUnknownMethod(const TQString& message)
{
    return TQT_DBusError(TQT_DBusError::UnknownMethod, message);
}

TQT_DBusError TQT_DBusError::stdInvalidSignature(const TQString& message)
{
    return TQT_DBusError(TQT_DBusError::InvalidSignature, message);
}
