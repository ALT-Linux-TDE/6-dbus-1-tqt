/* qdbusconnection_p.h TQT_DBusConnection private object
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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//
//

#ifndef TQDBUSCONNECTION_P_H
#define TQDBUSCONNECTION_P_H

#include <tqguardedptr.h>
#include <tqmap.h>
#include <tqobject.h>
#include <tqvaluelist.h>

#include <dbus/dbus.h>

#include "tqdbusatomic.h"
#include "tqdbuserror.h"
#include "tqdbusobject.h"
#include "tqdbusmessage.h"

class TQT_DBusMessage;
class TQSocketNotifier;
class TQTimer;
class TQTimerEvent;

struct DBusConnection;
struct DBusServer;

class TQT_DBusResultInfo
{
	public:
		TQT_DBusMessage message;
		TQObject* receiver;
		TQCString method;
};
typedef TQValueList<TQT_DBusResultInfo> TQT_DBusResultInfoList;

class TQT_DBusConnectionPrivate: public TQObject
{
    TQ_OBJECT
    

public:
    TQT_DBusConnectionPrivate(TQObject *parent = 0);
    ~TQT_DBusConnectionPrivate();

    void bindToApplication();

    void setConnection(DBusConnection *connection);
    void setServer(DBusServer *server);
    void closeConnection();
    void timerEvent(TQTimerEvent *e);

    bool handleSignal(DBusMessage *msg);
    bool handleObjectCall(DBusMessage *message);
    bool handleError();
    bool handleUnreadMessages();

    void emitPendingCallReply(const TQT_DBusMessage& message);

signals:
    void dbusSignal(const TQT_DBusMessage& message);

    void dbusPendingCallReply(const TQT_DBusMessage& message);

public slots:
    void socketRead(int);
    void socketWrite(int);

    void objectDestroyed(TQObject* object);

    void purgeRemovedWatches();

    void scheduleDispatch();
    void dispatch();

public:
    DBusError error;
    TQT_DBusError lastError;

    enum ConnectionMode { InvalidMode, ServerMode, ClientMode };

    // FIXME TQAtomic ref;
    Atomic ref;
    ConnectionMode mode;
    DBusConnection *connection;
    DBusServer *server;

    TQTimer* dispatcher;

    static int messageMetaType;
    static int registerMessageMetaType();
    int sendWithReplyAsync(const TQT_DBusMessage &message, TQObject *receiver,
                           const char *method);
    void flush();

    struct Watcher
    {
        Watcher(): watch(0), read(0), write(0) {}
        DBusWatch *watch;
        TQSocketNotifier *read;
        TQSocketNotifier *write;
    };
    // FIXME typedef TQMultiHash<int, Watcher> WatcherHash;
    typedef TQValueList<Watcher> WatcherList;
    WatcherList removedWatches;
    typedef TQMap<int, WatcherList> WatcherHash;
    WatcherHash watchers;

    // FIXME typedef TQHash<int, DBusTimeout *> TimeoutHash;
    typedef TQMap<int, DBusTimeout*> TimeoutHash;
    TimeoutHash timeouts;

    typedef TQMap<TQString, TQT_DBusObjectBase*> ObjectMap;
    ObjectMap registeredObjects;

    TQValueList<DBusTimeout *> pendingTimeouts;

    struct TQT_DBusPendingCall
    {
        TQGuardedPtr<TQObject> receiver;
        TQCString method;
        DBusPendingCall *pending;
    };
    typedef TQMap<DBusPendingCall*, TQT_DBusPendingCall*> PendingCallMap;
    PendingCallMap pendingCalls;

    typedef TQValueList<TQT_DBusMessage> PendingMessagesForEmit;
    PendingMessagesForEmit pendingMessages;

    bool inDispatch;

    TQT_DBusResultInfoList m_resultEmissionQueue;

public:
    void newMethodInResultEmissionQueue();

private slots:
    void transmitResultEmissionQueue();
    void transmitMessageEmissionQueue();

private:
    TQTimer* m_resultEmissionQueueTimer;
    TQTimer* m_messageEmissionQueueTimer;
};

#endif
