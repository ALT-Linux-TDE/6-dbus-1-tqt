/* qdbusconnection.cpp
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

#include <tqapplication.h>

#include "tqdbusconnection.h"
#include "tqdbuserror.h"
#include "tqdbusmessage.h"
#include "tqdbusconnection_p.h"

#include "tqdbusmessage_p.h"

const char *TQT_DBusConnection::default_connection_name = "qt_dbus_default_connection";

class TQT_DBusConnectionManager
{
public:
    TQT_DBusConnectionManager(): default_connection(0) {}
    ~TQT_DBusConnectionManager();
    void bindToApplication();
    TQT_DBusConnectionPrivate *connection(const TQString &name) const;
    void removeConnection(const TQString &name);
    void setConnection(const TQString &name, TQT_DBusConnectionPrivate *c);

    static TQT_DBusConnectionManager* instance() {
        if (managerInstance == 0) managerInstance = new TQT_DBusConnectionManager();
        return managerInstance;
    }

private:
    TQT_DBusConnectionPrivate *default_connection;
    // FIXME-QT4 TQHash<TQString, TQT_DBusConnectionPrivate *> connectionHash;
    typedef TQMap<TQString, TQT_DBusConnectionPrivate*> ConnectionHash;
    ConnectionHash connectionHash;

    static TQT_DBusConnectionManager* managerInstance;
};

// FIXME-QT4 TQ_GLOBAL_STATIC(TQT_DBusConnectionManager, manager);
TQT_DBusConnectionManager* TQT_DBusConnectionManager::managerInstance = 0;
TQT_DBusConnectionManager* manager() {
    return TQT_DBusConnectionManager::instance();
}

TQT_DBusConnectionPrivate *TQT_DBusConnectionManager::connection(const TQString &name) const
{
    if (name == TQString::fromLatin1(TQT_DBusConnection::default_connection_name))
        return default_connection;

    ConnectionHash::const_iterator it = connectionHash.find(name);

    return (it != connectionHash.end() ? it.data() : 0);
}

void TQT_DBusConnectionManager::removeConnection(const TQString &name)
{
    TQT_DBusConnectionPrivate *d = 0;
    if (name == TQString::fromLatin1(TQT_DBusConnection::default_connection_name)) {
        d = default_connection;
        default_connection = 0;
    } else {
        ConnectionHash::iterator it = connectionHash.find(name);
        if (it == connectionHash.end())
            return;

        d = it.data();
        connectionHash.erase(it);
    }
    if (!d->ref.deref())
        delete d;
}

TQT_DBusConnectionManager::~TQT_DBusConnectionManager()
{
    if (default_connection) {
        delete default_connection;
        default_connection = 0;
    }
/* FIXME-QT4
    for (TQHash<TQString, TQT_DBusConnectionPrivate *>::const_iterator it = connectionHash.constBegin();
         it != connectionHash.constEnd(); ++it) {
             delete it.value();
    }*/
    for (ConnectionHash::const_iterator it = connectionHash.constBegin();
         it != connectionHash.constEnd(); ++it)
    {
        delete it.data();
    }
    connectionHash.clear();
}

void TQT_DBusConnectionManager::bindToApplication()
{
    if (default_connection) {
        default_connection->bindToApplication();
    }
/* FIXME-QT4
    for (TQHash<TQString, TQT_DBusConnectionPrivate *>::const_iterator it = connectionHash.constBegin();
         it != connectionHash.constEnd(); ++it) {
             (*it)->bindToApplication();
    }*/
    for (ConnectionHash::const_iterator it = connectionHash.constBegin();
         it != connectionHash.constEnd(); ++it)
    {
        it.data()->bindToApplication();
    }
}

void qDBusBindToApplication()
{
    manager()->bindToApplication();
}

void TQT_DBusConnectionManager::setConnection(const TQString &name, TQT_DBusConnectionPrivate *c)
{
    if (name == TQString::fromLatin1(TQT_DBusConnection::default_connection_name))
        default_connection = c;
    else
        connectionHash[name] = c;
}


TQT_DBusConnection::TQT_DBusConnection() : d(0)
{
}

TQT_DBusConnection::TQT_DBusConnection(const TQString &name)
{
    d = manager()->connection(name);
    if (d)
        d->ref.ref();
}

TQT_DBusConnection::TQT_DBusConnection(const TQT_DBusConnection &other)
{
    d = other.d;
    if (d)
        d->ref.ref();
}

TQT_DBusConnection::~TQT_DBusConnection()
{
    if (d && !d->ref.deref())
        delete d;
}

TQT_DBusConnection &TQT_DBusConnection::operator=(const TQT_DBusConnection &other)
{
    if (other.d)
        other.d->ref.ref();
/* FIXME-QT4
    TQT_DBusConnectionPrivate *old = static_cast<TQT_DBusConnectionPrivate *>(
            q_atomic_set_ptr(&d, other.d));*/
    TQT_DBusConnectionPrivate* old = d;
    d = other.d;
    if (old && !old->ref.deref())
        delete old;

    return *this;
}

TQT_DBusConnection TQT_DBusConnection::sessionBus()
{
    return addConnection(TQT_DBusConnection::SessionBus);
}

TQT_DBusConnection TQT_DBusConnection::systemBus()
{
    return addConnection(TQT_DBusConnection::SystemBus);
}

TQT_DBusConnection TQT_DBusConnection::addConnection(BusType type, const TQString &name)
{
//    Q_ASSERT_X(TQCoreApplication::instance(), "TQT_DBusConnection::addConnection",
//               "Cannot create connection without a Q[Core]Application instance");

    TQT_DBusConnectionPrivate *d = manager()->connection(name);
    if (d)
        return TQT_DBusConnection(name);

    d = new TQT_DBusConnectionPrivate;
    DBusConnection *c = 0;
    switch (type) {
        case SystemBus:
            c = dbus_bus_get(DBUS_BUS_SYSTEM, &d->error);
            break;
        case SessionBus:
            c = dbus_bus_get(DBUS_BUS_SESSION, &d->error);
            break;
        case ActivationBus:
            c = dbus_bus_get(DBUS_BUS_STARTER, &d->error);
            break;
    }
    d->setConnection(c); //setConnection does the error handling for us

    manager()->setConnection(name, d);

    return TQT_DBusConnection(name);
}

TQT_DBusConnection TQT_DBusConnection::addConnection(const TQString &address,
                    const TQString &name)
{
//    Q_ASSERT_X(TQCoreApplication::instance(), "TQT_DBusConnection::addConnection",
//               "Cannot create connection without a Q[Core]Application instance");

    TQT_DBusConnectionPrivate *d = manager()->connection(name);
    if (d)
        return TQT_DBusConnection(name);

    d = new TQT_DBusConnectionPrivate;
    // setConnection does the error handling for us
    d->setConnection(dbus_connection_open(address.utf8().data(), &d->error));

    manager()->setConnection(name, d);

    return TQT_DBusConnection(name);
}

void TQT_DBusConnection::closeConnection(const TQString &name)
{
    manager()->removeConnection(name);
}

void TQT_DBusConnectionPrivate::timerEvent(TQTimerEvent *e)
{
    DBusTimeout *timeout = timeouts[e->timerId()];
    dbus_timeout_handle(timeout);
}

bool TQT_DBusConnection::send(const TQT_DBusMessage &message) const
{
    if (!d || !d->connection)
        return false;

    DBusMessage *msg = message.toDBusMessage();
    if (!msg)
        return false;

    bool isOk = dbus_connection_send(d->connection, msg, 0);
    dbus_message_unref(msg);
    return isOk;
}

int TQT_DBusConnection::sendWithAsyncReply(const TQT_DBusMessage &message, TQObject *receiver,
        const char *method) const
{
    if (!d || !d->connection)
        return 0;

    return d->sendWithReplyAsync(message, receiver, method);
}

TQT_DBusMessage TQT_DBusConnection::sendWithReply(const TQT_DBusMessage &message, TQT_DBusError *error) const
{
    if (!d || !d->connection)
        return TQT_DBusMessage::fromDBusMessage(0);

    DBusMessage *msg = message.toDBusMessage();
    if (!msg)
        return TQT_DBusMessage::fromDBusMessage(0);

    DBusMessage *reply = dbus_connection_send_with_reply_and_block(d->connection, msg, -1, &d->error);

    if (d->handleError() && error)
        *error = d->lastError;

    dbus_message_unref(msg);

    TQT_DBusMessage ret = TQT_DBusMessage::fromDBusMessage(reply);
    if (reply) {
        dbus_message_unref(reply);
    }

    bool dbus_error_set = dbus_error_is_set(&d->error);
    ret.d->error.setDBUSError(dbus_error_set);
    if (error) error->setDBUSError(dbus_error_set);

    return ret;
}

void TQT_DBusConnection::flush() const
{
    if (!d || !d->connection) return;

    d->flush();
}

void TQT_DBusConnection::dispatch() const
{
    if (!d || !d->connection) return;

    d->dispatch();
}

void TQT_DBusConnection::scheduleDispatch() const
{
    if (!d || !d->connection) return;

    d->scheduleDispatch();
}

bool TQT_DBusConnection::connect(TQObject* object, const char* slot)
{
    if (!d || !d->connection || !object || !slot)
        return false;

    bool ok = object->connect(d, TQ_SIGNAL(dbusSignal(const TQT_DBusMessage&)), slot);

    return ok;
}

bool TQT_DBusConnection::disconnect(TQObject* object, const char* slot)
{
    if (!d || !d->connection || !object || !slot)
        return false;

    bool ok = d->disconnect(object, slot);

    return ok;
}

bool TQT_DBusConnection::registerObject(const TQString& path, TQT_DBusObjectBase* object)
{
    if (!d || !d->connection || !object || path.isEmpty())
        return false;

    TQT_DBusConnectionPrivate::ObjectMap::const_iterator it = d->registeredObjects.find(path);
    if (it != d->registeredObjects.end())
        return false;

    d->registeredObjects.insert(path, object);

    return true;
}

void TQT_DBusConnection::unregisterObject(const TQString &path)
{
    if (!d || !d->connection || path.isEmpty())
        return;

    TQT_DBusConnectionPrivate::ObjectMap::iterator it = d->registeredObjects.find(path);
    if (it == d->registeredObjects.end())
        return ;

    d->registeredObjects.erase(it);
}

bool TQT_DBusConnection::isConnected( ) const
{
    return d && d->connection && dbus_connection_get_is_connected(d->connection);
}

TQT_DBusError TQT_DBusConnection::lastError() const
{
    return d ? d->lastError : TQT_DBusError();
}

TQString TQT_DBusConnection::uniqueName() const
{
    return d && d->connection ?
            TQString::fromUtf8(dbus_bus_get_unique_name(d->connection))
            : TQString();
}

bool TQT_DBusConnection::requestName(const TQString &name, int modeFlags)
{
    Q_ASSERT(modeFlags >= 0);

    if (!d || !d->connection)
        return false;

    if (modeFlags < 0)
        return false;

    int dbusFlags = 0;
    if (modeFlags & AllowReplace)
        dbusFlags |= DBUS_NAME_FLAG_ALLOW_REPLACEMENT;
    if (modeFlags & ReplaceExisting)
        dbusFlags |= DBUS_NAME_FLAG_REPLACE_EXISTING;

    dbus_bus_request_name(d->connection, name.utf8(), dbusFlags, &d->error);
    bool res = !d->handleError();
    res &= d->handleUnreadMessages();
    return res;
}

#include "tqdbusconnection.moc"
