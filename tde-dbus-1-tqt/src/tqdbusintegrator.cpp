/* qdbusintegrator.cpp TQT_DBusConnection private implementation
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

#include <tqapplication.h>
#include <tqevent.h>
#include <tqmetaobject.h>
#include <tqsocketnotifier.h>
#include <tqtimer.h>

#include "tqdbusconnection_p.h"
#include "tqdbusmessage.h"

Atomic::Atomic(int value) : m_value(value)
{
}

void Atomic::ref()
{
    m_value++;
}

bool Atomic::deref()
{
    m_value--;
    return m_value > 0;
}

int TQT_DBusConnectionPrivate::messageMetaType = 0;

static dbus_bool_t qDBusAddTimeout(DBusTimeout *timeout, void *data)
{
    Q_ASSERT(timeout);
    Q_ASSERT(data);

  //  tqDebug("addTimeout %d", dbus_timeout_get_interval(timeout));

    TQT_DBusConnectionPrivate *d = static_cast<TQT_DBusConnectionPrivate *>(data);

    if (!dbus_timeout_get_enabled(timeout))
        return true;

    if (!tqApp) {
        d->pendingTimeouts.append(timeout);
        return true;
    }
    int timerId = d->startTimer(dbus_timeout_get_interval(timeout));
    if (!timerId)
        return false;

    d->timeouts[timerId] = timeout;
    return true;
}

static void qDBusRemoveTimeout(DBusTimeout *timeout, void *data)
{
    Q_ASSERT(timeout);
    Q_ASSERT(data);

  //  tqDebug("removeTimeout");

    TQT_DBusConnectionPrivate *d = static_cast<TQT_DBusConnectionPrivate *>(data);
    for (TQValueList<DBusTimeout*>::iterator it = d->pendingTimeouts.begin();
         it != d->pendingTimeouts.end();) {
      if ((*it) == timeout) {
        it = d->pendingTimeouts.erase(it);
      }
      else
        ++it;
    }

    TQT_DBusConnectionPrivate::TimeoutHash::iterator it = d->timeouts.begin();
    while (it != d->timeouts.end()) {
        if (it.data() == timeout) {
            d->killTimer(it.key());
            TQT_DBusConnectionPrivate::TimeoutHash::iterator copyIt = it;
            ++it;
            d->timeouts.erase(copyIt);
        } else {
            ++it;
        }
    }
}

static void qDBusToggleTimeout(DBusTimeout *timeout, void *data)
{
    Q_ASSERT(timeout);
    Q_ASSERT(data);

    //tqDebug("ToggleTimeout");

    qDBusRemoveTimeout(timeout, data);
    qDBusAddTimeout(timeout, data);
}

static dbus_bool_t qDBusAddWatch(DBusWatch *watch, void *data)
{
    Q_ASSERT(watch);
    Q_ASSERT(data);

    TQT_DBusConnectionPrivate *d = static_cast<TQT_DBusConnectionPrivate *>(data);

    int flags = dbus_watch_get_flags(watch);
    int fd = dbus_watch_get_unix_fd(watch);

    TQT_DBusConnectionPrivate::Watcher watcher;
    if (flags & DBUS_WATCH_READABLE) {
        bool enabled = dbus_watch_get_enabled(watch);
        //tqDebug("addReadWatch %d %s", fd, (enabled ? "enabled" : "disabled"));
        watcher.watch = watch;
        if (tqApp) {
            watcher.read = new TQSocketNotifier(fd, TQSocketNotifier::Read, d);
            if (!enabled) watcher.read->setEnabled(false);
            d->connect(watcher.read, TQ_SIGNAL(activated(int)), TQ_SLOT(socketRead(int)));
        }
    }
    if (flags & DBUS_WATCH_WRITABLE) {
        bool enabled = dbus_watch_get_enabled(watch);
        //tqDebug("addWriteWatch %d %s", fd, (enabled ? "enabled" : "disabled"));
        watcher.watch = watch;
        if (tqApp) {
            watcher.write = new TQSocketNotifier(fd, TQSocketNotifier::Write, d);
            if (!enabled) watcher.write->setEnabled(false);
            d->connect(watcher.write, TQ_SIGNAL(activated(int)), TQ_SLOT(socketWrite(int)));
        }
    }
    // FIXME-QT4 d->watchers.insertMulti(fd, watcher);
    TQT_DBusConnectionPrivate::WatcherHash::iterator it = d->watchers.find(fd);
    if (it == d->watchers.end())
    {
        it = d->watchers.insert(fd, TQT_DBusConnectionPrivate::WatcherList());
    }
    it.data().append(watcher);

    return true;
}

static void qDBusRemoveWatch(DBusWatch *watch, void *data)
{
    Q_ASSERT(watch);
    Q_ASSERT(data);

    //tqDebug("remove watch");

    TQT_DBusConnectionPrivate *d = static_cast<TQT_DBusConnectionPrivate *>(data);
    int fd = dbus_watch_get_unix_fd(watch);

    TQT_DBusConnectionPrivate::WatcherHash::iterator it = d->watchers.find(fd);
    if (it != d->watchers.end())
    {
        TQT_DBusConnectionPrivate::WatcherList& list = *it;
        for (TQT_DBusConnectionPrivate::WatcherList::iterator wit = list.begin();
             wit != list.end(); ++wit)
        {
            if ((*wit).watch == watch)
            {
                // migth be called from a function triggered by a socket listener
                // so just disconnect them and schedule their delayed deletion.

                d->removedWatches.append(*wit);
                if ((*wit).read)
                {
                    (*wit).read->disconnect(d);
                    (*wit).read = 0;
                }
                if ((*wit).write)
                {
                    (*wit).write->disconnect(d);
                    (*wit).write = 0;
                }
                (*wit).watch = 0;
            }
        }
    }

    if (d->removedWatches.count() > 0)
        TQTimer::singleShot(0, d, TQ_SLOT(purgeRemovedWatches()));
}

static void qDBusToggleWatch(DBusWatch *watch, void *data)
{
    Q_ASSERT(watch);
    Q_ASSERT(data);

    //tqDebug("toggle watch");

    TQT_DBusConnectionPrivate *d = static_cast<TQT_DBusConnectionPrivate *>(data);
    int fd = dbus_watch_get_unix_fd(watch);

    TQT_DBusConnectionPrivate::WatcherHash::iterator it = d->watchers.find(fd);
    if (it != d->watchers.end()) {
        TQT_DBusConnectionPrivate::WatcherList& list = *it;
        for (TQT_DBusConnectionPrivate::WatcherList::iterator wit = list.begin(); wit != list.end();
             ++wit)
        {
            if ((*wit).watch == watch) {
                bool enabled = dbus_watch_get_enabled(watch);
                int flags = dbus_watch_get_flags(watch);

//                 tqDebug("toggle watch %d to %d (write: %d, read: %d)",
//                         dbus_watch_get_unix_fd(watch), enabled,
//                         flags & DBUS_WATCH_WRITABLE, flags & DBUS_WATCH_READABLE);

                if (flags & DBUS_WATCH_READABLE && (*wit).read)
                    (*wit).read->setEnabled(enabled);
                if (flags & DBUS_WATCH_WRITABLE && (*wit).write)
                    (*wit).write->setEnabled(enabled);
                return;
            }
        }
    }
}

static void qDBusNewConnection(DBusServer *server, DBusConnection *c, void *data)
{
    Q_ASSERT(data); Q_ASSERT(server); Q_ASSERT(c);

    tqDebug("SERVER: GOT A NEW CONNECTION"); // TODO
}

static DBusHandlerResult qDBusSignalFilter(DBusConnection *connection,
                                           DBusMessage *message, void *data)
{
    Q_ASSERT(data);
    Q_UNUSED(connection);

    TQT_DBusConnectionPrivate *d = static_cast<TQT_DBusConnectionPrivate *>(data);
    if (d->mode == TQT_DBusConnectionPrivate::InvalidMode)
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    int msgType = dbus_message_get_type(message);
    bool handled = false;

    //TQT_DBusMessage amsg = TQT_DBusMessage::fromDBusMessage(message);
    //tqDebug() << "got message: " << dbus_message_get_type(message) << amsg;

    if (msgType == DBUS_MESSAGE_TYPE_SIGNAL) {
        handled = d->handleSignal(message);
    } else if (msgType == DBUS_MESSAGE_TYPE_METHOD_CALL) {
        handled = d->handleObjectCall(message);
    }

    return handled ? DBUS_HANDLER_RESULT_HANDLED :
            DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

int TQT_DBusConnectionPrivate::registerMessageMetaType()
{
    // FIXME-QT4 int tp = messageMetaType = qRegisterMetaType<TQT_DBusMessage>("TQT_DBusMessage");
    int tp = 0;
    return tp;
}

TQT_DBusConnectionPrivate::TQT_DBusConnectionPrivate(TQObject *parent)
    : TQObject(parent), ref(1), mode(InvalidMode), connection(0), server(0),
      dispatcher(0), inDispatch(false)
{
    static const int msgType = registerMessageMetaType();
    Q_UNUSED(msgType);

    dbus_error_init(&error);

    dispatcher = new TQTimer(this);
    TQObject::connect(dispatcher, TQ_SIGNAL(timeout()), this, TQ_SLOT(dispatch()));

    m_resultEmissionQueueTimer = new TQTimer(this);
    TQObject::connect(m_resultEmissionQueueTimer, TQ_SIGNAL(timeout()), this, TQ_SLOT(transmitResultEmissionQueue()));
    m_messageEmissionQueueTimer = new TQTimer(this);
    TQObject::connect(m_messageEmissionQueueTimer, TQ_SIGNAL(timeout()), this, TQ_SLOT(transmitMessageEmissionQueue()));
}

TQT_DBusConnectionPrivate::~TQT_DBusConnectionPrivate()
{
    for (PendingCallMap::iterator it = pendingCalls.begin(); it != pendingCalls.end();)
    {
        PendingCallMap::iterator copyIt = it;
        ++it;
        dbus_pending_call_cancel(copyIt.key());
        dbus_pending_call_unref(copyIt.key());
        delete copyIt.data();
        pendingCalls.erase(copyIt);
    }

    if (dbus_error_is_set(&error))
        dbus_error_free(&error);

    closeConnection();
}

void TQT_DBusConnectionPrivate::closeConnection()
{
    ConnectionMode oldMode = mode;
    mode = InvalidMode; // prevent reentrancy
    if (oldMode == ServerMode) {
        if (server) {
            dbus_server_disconnect(server);
            dbus_server_unref(server);
            server = 0;
        }
    } else if (oldMode == ClientMode) {
        if (connection) {
            // closing shared connections is forbidden
#if 0
            dbus_connection_close(connection);
            // send the "close" message
            while (dbus_connection_dispatch(connection) == DBUS_DISPATCH_DATA_REMAINS);
#endif
            dbus_connection_unref(connection);
            connection = 0;
        }
    }
}

bool TQT_DBusConnectionPrivate::handleError()
{
    lastError = TQT_DBusError(&error);
    if (dbus_error_is_set(&error))
        dbus_error_free(&error);
    return lastError.isValid();
}

bool TQT_DBusConnectionPrivate::handleUnreadMessages()
{
  bool res = true;
  WatcherHash::iterator it = watchers.begin();
  while (it != watchers.end())
  {
    WatcherList &list = *it;
    WatcherList::iterator listIt = list.begin();
    while (listIt != list.end())
    {
      Watcher watcher = *listIt;
      if (watcher.read)
      {
        socketRead(watcher.read->socket());
        res &= (!handleError());
      }
      ++listIt;
    }
    ++it;
  }
  return res;
}

void TQT_DBusConnectionPrivate::emitPendingCallReply(const TQT_DBusMessage& message)
{
    emit dbusPendingCallReply(message);
}

void TQT_DBusConnectionPrivate::bindToApplication()
{
    // Yay, now that we have an application we are in business
    // Re-add all watchers
    WatcherHash oldWatchers = watchers;
    watchers.clear();
    // FIXME-QT4 TQHashIterator<int, TQT_DBusConnectionPrivate::Watcher> it(oldWatchers);
    for (WatcherHash::const_iterator it = oldWatchers.begin(); it != oldWatchers.end(); ++it)
    {
        const WatcherList& list = *it;
        for (WatcherList::const_iterator wit = list.begin(); wit != list.end(); ++wit)
        {
            if (!(*wit).read && !(*wit).write) {
                qDBusAddWatch((*wit).watch, this);
            }
        }
    }

    // Re-add all timeouts
    while (!pendingTimeouts.isEmpty()) {
       qDBusAddTimeout(pendingTimeouts.first(), this);
       pendingTimeouts.pop_front();
    }
}

void TQT_DBusConnectionPrivate::socketRead(int fd)
{
    // FIXME-QT4 TQHashIterator<int, TQT_DBusConnectionPrivate::Watcher> it(watchers);
    WatcherHash::const_iterator it = watchers.find(fd);
    if (it != watchers.end()) {
        const WatcherList& list = *it;
        for (WatcherList::const_iterator wit = list.begin(); wit != list.end(); ++wit) {
            if ((*wit).read && (*wit).read->isEnabled()) {
                if (!dbus_watch_handle((*wit).watch, DBUS_WATCH_READABLE))
                    tqDebug("OUT OF MEM");
            }
        }
    }
    if (mode == ClientMode)
        scheduleDispatch();
}

void TQT_DBusConnectionPrivate::socketWrite(int fd)
{
    // FIXME-QT4 TQHashIterator<int, TQT_DBusConnectionPrivate::Watcher> it(watchers);
    WatcherHash::const_iterator it = watchers.find(fd);
    if (it != watchers.end()) {
        const WatcherList& list = *it;
        for (WatcherList::const_iterator wit = list.begin(); wit != list.end(); ++wit) {
            if ((*wit).write && (*wit).write->isEnabled()) {
                if (!dbus_watch_handle((*wit).watch, DBUS_WATCH_WRITABLE))
                    tqDebug("OUT OF MEM");
            }
        }
    }
}

void TQT_DBusConnectionPrivate::objectDestroyed(TQObject* object)
{
    //tqDebug("Object destroyed");
    for (PendingCallMap::iterator it = pendingCalls.begin(); it != pendingCalls.end();)
    {
        TQObject* receiver = (TQObject*) it.data()->receiver;
        if (receiver == object || receiver == 0)
        {
            PendingCallMap::iterator copyIt = it;
            ++it;

            dbus_pending_call_cancel(copyIt.key());
            dbus_pending_call_unref(copyIt.key());
            delete copyIt.data();
            pendingCalls.erase(copyIt);
        }
        else
            ++it;
    }
}

void TQT_DBusConnectionPrivate::purgeRemovedWatches()
{
    if (removedWatches.isEmpty()) return;

    WatcherList::iterator listIt = removedWatches.begin();
    for (; listIt != removedWatches.end(); ++listIt)
    {
        delete (*listIt).read;
        delete (*listIt).write;
    }
    removedWatches.clear();

    uint count = 0;
    WatcherHash::iterator it = watchers.begin();
    while (it != watchers.end())
    {
        WatcherList& list = *it;
        listIt = list.begin();
        while (listIt != list.end())
        {
            if (!((*listIt).read) && !((*listIt).write))
            {
                listIt = list.erase(listIt);
                ++count;
            }
        }

        if (list.isEmpty())
        {
            WatcherHash::iterator copyIt = it;
            ++it;
            watchers.erase(copyIt);
        }
        else
            ++it;
    }
}

void TQT_DBusConnectionPrivate::scheduleDispatch()
{
    dispatcher->start(0);
}

void TQT_DBusConnectionPrivate::dispatch()
{
    // dbus_connection_dispatch will hang if called recursively
    if (inDispatch) {
        printf("[dbus-1-tqt] WARNING: Attempt to call dispatch() recursively was silently ignored to prevent lockup!\n\r"); fflush(stdout);
        return;
    }
    inDispatch = true;

    if (mode == ClientMode)
    {
        if (dbus_connection_dispatch(connection) != DBUS_DISPATCH_DATA_REMAINS)
        {
            // stop dispatch timer
            dispatcher->stop();
        }
    }

    inDispatch = false;
}

void TQT_DBusConnectionPrivate::transmitMessageEmissionQueue()
{
    TQT_DBusConnectionPrivate::PendingMessagesForEmit::iterator pmfe;
    pmfe = pendingMessages.begin();
    while (pmfe != pendingMessages.end()) {
        TQT_DBusMessage msg = *pmfe;
        pmfe = pendingMessages.remove(pmfe);
        dbusSignal(msg);
    }
}

bool TQT_DBusConnectionPrivate::handleObjectCall(DBusMessage *message)
{
    TQT_DBusMessage msg = TQT_DBusMessage::fromDBusMessage(message);

    ObjectMap::iterator it = registeredObjects.find(msg.path());
    if (it == registeredObjects.end())
        return false;

    return it.data()->handleMethodCall(msg);
}

bool TQT_DBusConnectionPrivate::handleSignal(DBusMessage *message)
{
    TQT_DBusMessage msg = TQT_DBusMessage::fromDBusMessage(message);

    // yes, it is a single "|" below...
    // FIXME-QT4
    //return handleSignal(TQString(), msg) | handleSignal(msg.path(), msg);

    // If dbusSignal(msg) were called here, it could easily cause a lockup as it would enter the TQt3 event loop,
    // which could result in arbitrary methods being called while still inside dbus_connection_dispatch.
    // Instead, I enqueue the messages here for TQt3 event loop transmission after dbus_connection_dispatch is finished.
    pendingMessages.append(msg);
    if (!m_messageEmissionQueueTimer->isActive()) m_messageEmissionQueueTimer->start(0, TRUE);

    return true;
}

static dbus_int32_t server_slot = -1;

void TQT_DBusConnectionPrivate::setServer(DBusServer *s)
{
    if (!server) {
        handleError();
        return;
    }

    server = s;
    mode = ServerMode;

    dbus_server_allocate_data_slot(&server_slot);
    if (server_slot < 0)
        return;

    dbus_server_set_watch_functions(server, qDBusAddWatch, qDBusRemoveWatch,
                                    qDBusToggleWatch, this, 0); // ### check return type?
    dbus_server_set_timeout_functions(server, qDBusAddTimeout, qDBusRemoveTimeout,
                                      qDBusToggleTimeout, this, 0);
    dbus_server_set_new_connection_function(server, qDBusNewConnection, this, 0);

    dbus_server_set_data(server, server_slot, this, 0);
}

void TQT_DBusConnectionPrivate::setConnection(DBusConnection *dbc)
{
    if (!dbc) {
        handleError();
        return;
    }

    connection = dbc;
    mode = ClientMode;

    dbus_connection_set_exit_on_disconnect(connection, false);
    dbus_connection_set_watch_functions(connection, qDBusAddWatch, qDBusRemoveWatch,
                                        qDBusToggleWatch, this, 0);
    dbus_connection_set_timeout_functions(connection, qDBusAddTimeout, qDBusRemoveTimeout,
                                          qDBusToggleTimeout, this, 0);

    dbus_bus_add_match(connection, "type='signal'", &error);
    if (handleError()) {
        closeConnection();
        return;
    }

    const char *service = dbus_bus_get_unique_name(connection);
    if (service) {
        TQCString filter;
        filter += "destination='";
        filter += service;
        filter += "'";

        dbus_bus_add_match(connection, filter.data(), &error);
        if (handleError()) {
            closeConnection();
            return;
        }
    } else {
        tqWarning("TQT_DBusConnectionPrivate::SetConnection: Unable to get unique name");
    }

    dbus_connection_add_filter(connection, qDBusSignalFilter, this, 0);

    //tqDebug("unique name: %s", service);
}

static void qDBusResultReceived(DBusPendingCall *pending, void *user_data)
{
    //tqDebug("Pending Call Result received");
    TQT_DBusConnectionPrivate* d = reinterpret_cast<TQT_DBusConnectionPrivate*>(user_data);
    TQT_DBusConnectionPrivate::PendingCallMap::iterator it = d->pendingCalls.find(pending);

    DBusMessage *dbusReply = dbus_pending_call_steal_reply(pending);

    dbus_set_error_from_message(&d->error, dbusReply);
    d->handleError();

    if (it != d->pendingCalls.end())
    {
        TQT_DBusMessage reply = TQT_DBusMessage::fromDBusMessage(dbusReply);

        TQT_DBusResultInfo dbusResult;
        dbusResult.message = reply;
        dbusResult.receiver = it.data()->receiver;
        dbusResult.method = it.data()->method.data();
        d->m_resultEmissionQueue.append(dbusResult);
        d->newMethodInResultEmissionQueue();
    }

    dbus_message_unref(dbusReply);
    dbus_pending_call_unref(pending);
    delete it.data();

    d->pendingCalls.erase(it);
}

int TQT_DBusConnectionPrivate::sendWithReplyAsync(const TQT_DBusMessage &message, TQObject *receiver,
        const char *method)
{
    if (!receiver || !method)
        return 0;

    if (!TQObject::connect(receiver, TQ_SIGNAL(destroyed(TQObject*)),
                          this, TQ_SLOT(objectDestroyed(TQObject*))))
        return false;

    DBusMessage *msg = message.toDBusMessage();
    if (!msg)
        return 0;

    int msg_serial = 0;
    DBusPendingCall *pending = 0;
    if (dbus_connection_send_with_reply(connection, msg, &pending, message.timeout())) {
        TQT_DBusPendingCall *pcall = new TQT_DBusPendingCall;
        pcall->receiver = receiver;
        pcall->method = method;
        pcall->pending = pending;
        pendingCalls.insert(pcall->pending, pcall);

        dbus_pending_call_set_notify(pending, qDBusResultReceived, this, 0);

        msg_serial = dbus_message_get_serial(msg);
    }

    dbus_message_unref(msg);
    return msg_serial;
}

void TQT_DBusConnectionPrivate::flush()
{
    if (!connection) return;

    dbus_connection_flush(connection);
}

void TQT_DBusConnectionPrivate::newMethodInResultEmissionQueue()
{
    if (!m_resultEmissionQueueTimer->isActive()) m_resultEmissionQueueTimer->start(0, TRUE);
}

void TQT_DBusConnectionPrivate::transmitResultEmissionQueue()
{
    if (!m_resultEmissionQueue.isEmpty()) {
        TQT_DBusResultInfoList::Iterator it;
        it = m_resultEmissionQueue.begin();
        while (it != m_resultEmissionQueue.end()) {
            TQT_DBusResultInfo dbusResult = (*it);
            m_resultEmissionQueue.remove(it);
            it = m_resultEmissionQueue.begin();

            TQObject::connect(this, TQ_SIGNAL(dbusPendingCallReply(const TQT_DBusMessage&)), dbusResult.receiver, dbusResult.method.data());
            emitPendingCallReply(dbusResult.message);
            TQObject::disconnect(this, TQ_SIGNAL(dbusPendingCallReply(const TQT_DBusMessage&)), dbusResult.receiver, dbusResult.method.data());
        }
    }
}

#include "tqdbusconnection_p.moc"
