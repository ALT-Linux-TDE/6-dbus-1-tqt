/* qdbusmessage.cpp
 *
 * Copyright (C) 2005 Harald Fernengel <harry@kdevelop.org>
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

#include "tqdbusmessage.h"

#include <tqstringlist.h>

#include <dbus/dbus.h>

#include "tqdbusmarshall.h"
#include "tqdbusmessage_p.h"

TQT_DBusMessagePrivate::TQT_DBusMessagePrivate(TQT_DBusMessage *qq)
    : msg(0), reply(0), q(qq), type(DBUS_MESSAGE_TYPE_INVALID), timeout(-1), ref(1)
{
}

TQT_DBusMessagePrivate::~TQT_DBusMessagePrivate()
{
    if (msg)
        dbus_message_unref(msg);
    if (reply)
        dbus_message_unref(reply);
}

///////////////


TQT_DBusMessage TQT_DBusMessage::signal(const TQString &path, const TQString &interface,
                                  const TQString &member)
{
    TQT_DBusMessage message;
    message.d->type = DBUS_MESSAGE_TYPE_SIGNAL;
    message.d->path = path;
    message.d->interface = interface;
    message.d->member = member;

    return message;
}

TQT_DBusMessage TQT_DBusMessage::methodCall(const TQString &service, const TQString &path,
                                      const TQString &interface, const TQString &method)
{
    TQT_DBusMessage message;
    message.d->type = DBUS_MESSAGE_TYPE_METHOD_CALL;
    message.d->service = service;
    message.d->path = path;
    message.d->interface = interface;
    message.d->member = method;

    return message;
}

TQT_DBusMessage TQT_DBusMessage::methodReply(const TQT_DBusMessage &other)
{
    Q_ASSERT(other.d->msg);

    TQT_DBusMessage message;
    message.d->type = DBUS_MESSAGE_TYPE_METHOD_RETURN;
    message.d->reply = dbus_message_ref(other.d->msg);

    return message;
}

TQT_DBusMessage TQT_DBusMessage::methodError(const TQT_DBusMessage &other, const TQT_DBusError& error)
{
    Q_ASSERT(other.d->msg);

    TQT_DBusMessage message;
    if (!error.isValid())
    {
        tqWarning("TQT_DBusMessage: error passed to methodError() is not valid!");
        return message;
    }

    message.d->type = DBUS_MESSAGE_TYPE_ERROR;
    message.d->reply = dbus_message_ref(other.d->msg);
    message.d->error = error;

    return message;
}

TQT_DBusMessage::TQT_DBusMessage()
{
    d = new TQT_DBusMessagePrivate(this);
}

TQT_DBusMessage::TQT_DBusMessage(const TQT_DBusMessage &other)
    : TQValueList<TQT_DBusData>(other)
{
    d = other.d;
    d->ref.ref();
}

TQT_DBusMessage::~TQT_DBusMessage()
{
    if (!d->ref.deref())
        delete d;
}

TQT_DBusMessage &TQT_DBusMessage::operator=(const TQT_DBusMessage &other)
{
    TQValueList<TQT_DBusData>::operator=(other);
    // FIXME-QT4 qAtomicAssign(d, other.d);
    if (other.d) other.d->ref.ref();
    TQT_DBusMessagePrivate* old = d;
    d = other.d;
    if (old && !old->ref.deref())
        delete old;
    return *this;
}

DBusMessage *TQT_DBusMessage::toDBusMessage() const
{
    DBusMessage *msg = 0;
    switch (d->type) {
    case DBUS_MESSAGE_TYPE_METHOD_CALL:
        msg = dbus_message_new_method_call(d->service.utf8().data(),
                d->path.utf8().data(), d->interface.utf8().data(),
                d->member.utf8().data());
        break;
    case DBUS_MESSAGE_TYPE_SIGNAL:
        msg = dbus_message_new_signal(d->path.utf8().data(),
                d->interface.utf8().data(), d->member.utf8().data());
        break;
    case DBUS_MESSAGE_TYPE_METHOD_RETURN:
        msg = dbus_message_new_method_return(d->reply);
        break;
    case DBUS_MESSAGE_TYPE_ERROR:
        msg = dbus_message_new_error(d->reply, d->error.name().utf8().data(),
                d->error.message().utf8().data());
        break;
    }
    if (!msg)
        return 0;

    TQT_DBusMarshall::listToMessage(*this, msg);
    return msg;
}

TQT_DBusMessage TQT_DBusMessage::fromDBusMessage(DBusMessage *dmsg)
{
    TQT_DBusMessage message;
    if (!dmsg)
        return message;

    message.d->type = dbus_message_get_type(dmsg);
    message.d->path = TQString::fromUtf8(dbus_message_get_path(dmsg));
    message.d->interface = TQString::fromUtf8(dbus_message_get_interface(dmsg));
    message.d->member = TQString::fromUtf8(dbus_message_get_member(dmsg));
    message.d->sender = TQString::fromUtf8(dbus_message_get_sender(dmsg));
    message.d->msg = dbus_message_ref(dmsg);

    DBusError dbusError;
    dbus_error_init(&dbusError);
    if (dbus_set_error_from_message(&dbusError, dmsg))
    {
        message.d->error = TQT_DBusError(&dbusError);
    }

    TQT_DBusMarshall::messageToList(message, dmsg);

    return message;
}

TQString TQT_DBusMessage::path() const
{
    return d->path;
}

TQString TQT_DBusMessage::interface() const
{
    return d->interface;
}

TQString TQT_DBusMessage::member() const
{
    return d->member;
}

TQString TQT_DBusMessage::sender() const
{
    return d->sender;
}

TQT_DBusError TQT_DBusMessage::error() const
{
    return d->error;
}

int TQT_DBusMessage::timeout() const
{
    return d->timeout;
}

void TQT_DBusMessage::setTimeout(int ms)
{
    d->timeout = ms;
}

/*!
    Returns the unique serial number assigned to this message
    or 0 if the message was not sent yet.
 */
int TQT_DBusMessage::serialNumber() const
{
    if (!d->msg)
        return 0;
    return dbus_message_get_serial(d->msg);
}

/*!
    Returns the unique serial number assigned to the message
    that triggered this reply message.

    If this message is not a reply to another message, 0
    is returned.

 */
int TQT_DBusMessage::replySerialNumber() const
{
    if (!d->msg)
        return 0;
    return dbus_message_get_reply_serial(d->msg);
}

TQT_DBusMessage::MessageType TQT_DBusMessage::type() const
{
    switch (d->type) {
    case DBUS_MESSAGE_TYPE_METHOD_CALL:
        return MethodCallMessage;
    case DBUS_MESSAGE_TYPE_METHOD_RETURN:
        return ReplyMessage;
    case DBUS_MESSAGE_TYPE_ERROR:
        return ErrorMessage;
    case DBUS_MESSAGE_TYPE_SIGNAL:
        return SignalMessage;
    default:
        return InvalidMessage;
    }
}

