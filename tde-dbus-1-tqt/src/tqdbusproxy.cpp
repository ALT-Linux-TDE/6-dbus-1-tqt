/* qdbusproxy.cpp DBUS Object proxy
 *
 * Copyright (C) 2005 Kevin Krammer <kevin.krammer@gmx.at>
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
#include "tqdbusconnection.h"
#include "tqdbusmessage.h"
#include "tqdbusproxy.h"

class TQT_DBusProxy::Private
{
public:
    Private() : canSend(false) {}
    ~Private() {}

    void checkCanSend()
    {
        canSend = !path.isEmpty() && !service.isEmpty() && !interface.isEmpty();
    }

public:
    TQT_DBusConnection connection;

    TQString service;
    TQString path;
    TQString interface;
    bool canSend;

    TQT_DBusError error;
};

TQT_DBusProxy::TQT_DBusProxy(TQObject* parent, const char* name)
    : TQObject(parent, (name ? name : "TQT_DBusProxy")),
      d(new Private())
{
}

TQT_DBusProxy::TQT_DBusProxy(const TQT_DBusConnection& connection,
                       TQObject* parent, const char* name)
    : TQObject(parent, (name ? name : "TQT_DBusProxy")),
      d(new Private())
{
    setConnection(connection);
}

TQT_DBusProxy::TQT_DBusProxy(const TQString& service, const TQString& path,
                       const TQString& interface, const TQT_DBusConnection& connection,
                       TQObject* parent, const char* name)
    : TQObject(parent, (name ? name : "TQT_DBusProxy")),
      d(new Private())
{
    setConnection(connection);

    d->service = service;
    d->path = path;
    d->interface = interface;
    d->checkCanSend();
}

TQT_DBusProxy::~TQT_DBusProxy()
{
    delete d;
}

bool TQT_DBusProxy::setConnection(const TQT_DBusConnection& connection)
{
    d->connection.disconnect(this, TQ_SLOT(handleDBusSignal(const TQT_DBusMessage&)));

    d->connection = connection;

    return d->connection.connect(this, TQ_SLOT(handleDBusSignal(const TQT_DBusMessage&)));
}

const TQT_DBusConnection& TQT_DBusProxy::connection() const
{
    return d->connection;
}

void TQT_DBusProxy::setService(const TQString& service)
{
    d->service = service;
    d->checkCanSend();
}

TQString TQT_DBusProxy::service() const
{
    return d->service;
}

void TQT_DBusProxy::setPath(const TQString& path)
{
    d->path = path;
    d->checkCanSend();
}

TQString TQT_DBusProxy::path() const
{
    return d->path;
}

void TQT_DBusProxy::setInterface(const TQString& interface)
{
    d->interface = interface;
    d->checkCanSend();
}

TQString TQT_DBusProxy::interface() const
{
    return d->interface;
}

bool TQT_DBusProxy::canSend() const
{
    return d->canSend && d->connection.isConnected();
}

bool TQT_DBusProxy::send(const TQString& method, const TQValueList<TQT_DBusData>& params) const
{
    if (!d->canSend || method.isEmpty() || !d->connection.isConnected())
        return false;

    TQT_DBusMessage message = TQT_DBusMessage::methodCall(d->service, d->path,
                                                    d->interface, method);
    message += params;

    return d->connection.send(message);
}

TQT_DBusMessage TQT_DBusProxy::sendWithReply(const TQString& method,
                                       const TQValueList<TQT_DBusData>& params,
                                       TQT_DBusError* error) const
{
    if (!d->canSend || method.isEmpty() || !d->connection.isConnected())
        return TQT_DBusMessage();

    TQT_DBusMessage message = TQT_DBusMessage::methodCall(d->service, d->path,
                                                    d->interface, method);
    message += params;

    TQT_DBusMessage reply = d->connection.sendWithReply(message, &d->error);

    if (error)
        *error = d->error;

    return reply;
}

int TQT_DBusProxy::sendWithAsyncReply(const TQString& method, const TQValueList<TQT_DBusData>& params)
{
    if (!d->canSend || method.isEmpty() || !d->connection.isConnected())
        return 0;

    TQT_DBusMessage message = TQT_DBusMessage::methodCall(d->service, d->path,
                                                    d->interface, method);
    message += params;

    return d->connection.sendWithAsyncReply(message, this,
                   TQ_SLOT(handleAsyncReply(const TQT_DBusMessage&)));
}

TQT_DBusError TQT_DBusProxy::lastError() const
{
    return d->error;
}

void TQT_DBusProxy::handleDBusSignal(const TQT_DBusMessage& message)
{
    if (!d->path.isEmpty() && d->path != message.path())
        return;

    // only filter by service name if the name is a unique name
    // because signals are always coming from a connection's unique name
    // and filtering by a generic name would reject all signals
    if (d->service.startsWith(":") && d->service != message.sender())
        return;

    if (!d->interface.isEmpty() && d->interface != message.interface())
        return;

    emit dbusSignal(message);
}

void TQT_DBusProxy::handleAsyncReply(const TQT_DBusMessage& message)
{
    d->error = message.error();

    emit asyncReply(message.replySerialNumber(), message);
}

#include "tqdbusproxy.moc"
