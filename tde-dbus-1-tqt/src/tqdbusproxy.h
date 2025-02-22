/* qdbusproxy.h DBUS Object proxy
 *
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

#ifndef TQDBUSPROXY_H
#define TQDBUSPROXY_H

/**
 * @page dbusclient Using D-Bus as a client
 *
 * Contents:
 * - @ref dbusclient-introduction
 *
 * - @ref dbusclient-example
 *
 * - @ref dbusclient-initialization
 *
 * - @ref dbusclient-methodcall
 *     - @ref dbusclient-synccall
 *     - @ref dbusclient-asynccall
 *
 * - @ref dbusclient-signals
 *     - @ref dbusclient-signals-example
 *
 * @section dbusclient-introduction Introduction
 *
 * While it is of course possible to just exchange D-Bus messages with a
 * D-Bus service, it is a lot more comfortable to use TQT_DBusProxy.
 *
 * With TQT_DBusProxy you only need to specify the service object once, i.e.
 * its D-Bus name, path and interface, and just provide the method and its
 * parameters when initiating an invokation.
 *
 * Additionally the proxy transforms D-Bus signals from the proxy's peer
 * (the D-Bus service object's interface it is associated with) to TQObject
 * signal carrying the original signal's content.
 *
 * @section dbusclient-example A simple D-Bus client example
 *
 * @code
 *   #include <dbus/tqdbusconnection.h>
 *   #include <dbus/tqdbusmessage.h>
 *   #include <dbus/tqdbusproxy.h>
 *
 *   int main()
 *   {
 *       // establish a connection to the session bus
 *
 *       TQT_DBusConnection connection = TQT_DBusConnection::sessionBus();
 *       if (!connection.isConnected())
 *           tqFatal("Failed to connect to session bus");
 *
 *       // create a proxy object for method calls
 *
 *       TQT_DBusProxy proxy(connection);
 *       proxy.setService("org.freedesktop.DBus");   // who we work with
 *       proxy.setPath("/org/freedesktop/DBus");     // which object inside the peer work with
 *       proxy.setInterface("org.freedesktop.DBus"); // which of its interfaces we will use
 *
 *       // call the "ListNames" method. It returns an array of string, in TQt3 terms
 *       // a TQStringList, it expects no parameters
 *
 *       TQValueList<TQT_DBusData> params;
 *       TQT_DBusMessage reply = proxy.sendWithReply("ListNames", params);
 *
 *       if (reply.type() != TQT_DBusMessage::ReplyMessage)
 *           tqFatal("Call failed");
 *
 *       if (reply.count() != 1 || reply[0].type() != TQT_DBusData::List)
 *           tqFatal("Unexpected reply");
 *
 *       bool ok = false;
 *       TQStringList names = reply[0].toTQStringList(&ok);
 *
 *       if (!ok) tqFatal("Unexpected reply");
 *
 *       for (TQStringList::iterator it = names.begin(); it != names.end(); ++it)
 *       {
 *           tqDebug("%s", (*it).local8Bit().data());
 *       }
 *
 *       return 0;
 *   }
 * @endcode
 *
 * @section dbusclient-initialization Program initialization
 *
 * A connection to the bus is acquired using TQT_DBusConnection::sessionBus()
 *
 * Next, a proxy is created for the object @c "/org/freedesktop/DBus" with
 * interface @c "org.freedesktop.DBus" on the service @c "org.freedesktop.DBus"
 *
 * This is a proxy for the message bus itself.
 *
 * @section dbusclient-methodcall Method invocation
 *
 * There are two choices for method invocation:
 * - sychronous (blocking) calls
 * - asynchronous (non-blocking) calls
 *
 * @subsection dbusclient-synccall Synchronous method calls
 *
 * As seen in the example code above, a synchronous method call can be achieved
 * by TQT_DBusProxy::sendWithReply(). It sends a method call to the remote object,
 * and blocks until reply is received. The outgoing arguments are specified as
 * a list of TQT_DBusData.
 *
 * @subsection dbusclient-asynccall Asynchronous method calls
 *
 * To invoke a method asynchronously, connect the proxy's signal
 * TQT_DBusProxy::asyncReply(int, const TQT_DBusMessage&) to a suitable slot like
 * with any other TQt Signal-Slot connection.
 *
 * Then call TQT_DBusProxy::sendWithAsyncReply()
 * It returns a numerical identifier of the call, so it can be related in the
 * slot once the result is available.
 *
 * The slot's first argument is the reveived reply's call identifier as
 * returned by the method call. The second parameter is the reply message
 * similar to the one in the synchronous call.
 *
 * @note For asynchronous calls you'll need a running event loop, i.e. a
 * TQApplication object and its exec() having been invoked.
 *
 * @section dbusclient-signals Connecting to D-Bus signals
 *
 * To receive D-BUS signals just connect to the TQT_DBusProxy's signal
 * TQT_DBusProxy::dbusSignal(const TQT_DBusMessage&)
 *
 * It will be emitted whenever a D-Bus signal message is received from the peer
 * object.
 * Filtering of signals is based on the value set for @c service, @c path and
 * @c interface
 *
 * @note Filtering for @c service will only happen if @c service is a unique
 *       D-Bus name, i.e. if it starts with a colon @c ":" since D-Bus signals
 *       carry the sender's unique name and filtering by a requested name
 *       would reject all signals
 *
 * Usually a proxy will be also be used to send to the peer object, thus having
 * them all set. However if a proxy is only needed for signals, any of the
 * three properties can be omitted (e.g. set to @c TQString() ), in which
 * case only the available properties will be checked against the respective
 * message field when deciding about dropping or emitting the message.
 * See TQT_DBusProxy::handleDBusSignal()
 *
 * If you want all signal travelling on the bus, or apply filtering for
 * different criteria, e.g. get all signals coming from interfaces starting
 * with @c "org.", use @c TQT_DBusConnection::connect() instead.
 * The signature of the slot stays the same.
 *
 * @subsection dbusclient-signals-example Signal example
 *
 * First declare a receiver class:
 * @code
 *   class MyReceiver : public TQObject
 *   {
 *       TQ_OBJECT
 *       
 *   public slots:
 *       void handleDBusSignal(const TQT_DBusMessage&);
 *   };
 * @endcode
 * Then somewhere else in a source file:
 * @code
 *   TQT_DBusConnection connection = TQT_DBusConnection::sessionBus();
 *
 *   MyReceiver* receiver1 = new MyReceiver();
 *
 *   connection.connect(receiver1, TQ_SLOT(handleDBusSignal(const TQT_DBusMessage&)));
 * @endcode
 * @c receiver1 will get all signals on this connection
 *
 * @code
 *   TQT_DBusProxy* proxy = new TQT_DBusProxy(connection);
 *   proxy->setService("org.freedesktop.DBus");   // who we work with
 *   proxy->setPath("/org/freedesktop/DBus");     // which object inside the peer work with
 *   proxy->setInterface("org.freedesktop.DBus"); // which of its interfaces we will use
 *
 *   MyReceiver* receiver2 = new MyReceiver();
 *
 *   TQObject::connect(proxy,     TQ_SIGNAL(dbusSignal(const TQT_DBusMessage&)),
 *                    receiver2, TQ_SLOT(handleDBusSignal(const TQT_DBusMessage&)));
 * @endcode
 * @c receiver2 will only get signals coming from the proxy's peer interface
 */

/**
 * @include example-client.h
 * @example example-client.cpp
 */

#include <tqobject.h>

#include "tqdbusmacros.h"

class TQT_DBusConnection;
class TQT_DBusData;
class TQT_DBusError;
class TQT_DBusMessage;

template <class T> class TQValueList;

/**
 * @brief Client interface to a remote service object
 *
 * TQT_DBusProxy provides a convenience interface for working with D-Bus services,
 * or more precisely, interfaces of D-Bus service objects.
 *
 * A D-Bus service object is identified through the name of its host application
 * on the bus and its path (logical location) within the host application.
 * Such a service object can implement any number of interfaces, i.e. groups
 * methods and signals, and can create a TQT_DBusProxy instance for every one
 * your application needs to work with.
 *
 * See section @ref dbusclient for documentation on how to use TQT_DBusProxy
 */
class TQDBUS_EXPORT TQT_DBusProxy : public TQObject
{
  TQ_OBJECT
  
public:
    /**
     * @brief Creates a proxy without binding it to a service or connection
     *
     * This basic constructor allows to create a proxy and specify the peer
     * object and interface later on.
     *
     * @param parent TQObject parent
     * @param name TQObject name
     */
    TQT_DBusProxy(TQObject* parent = 0, const char* name = 0);

    /**
     * @brief Creates a proxy on a given connection without binding it to a
     *        service
     *
     * Similar to the above constructor, it does not yet specify and details
     * about the proxy's peer, but already specifies which connection to work
     * on.
     *
     * This can be useful to monitor all signal on a connection without
     * filtering for a specific peer.
     *
     * @param connection the D-Bus connection to work on
     * @param parent TQObject parent
     * @param name TQObject name
     */
    TQT_DBusProxy(const TQT_DBusConnection& connection, TQObject* parent = 0,
               const char* name = 0);

    /**
     * @brief Creates a proxy for a given peer on a given connection
     *
     * This creates a proxy for a specific peer object-interface combination
     * It is equvalent to creating an "empty" proxy and calling setConnection(),
     * setService(), setPath() and setInterface() manually.
     *
     * @param service the name the peer's host application uses on the bus
     * @param path the peer object's path within its host application
     * @param interface the interface to work with
     * @param connection the D-Bus connection to work on
     * @param parent TQObject parent
     * @param name TQObject name
     */
    TQT_DBusProxy(const TQString& service, const TQString& path,
               const TQString& interface, const TQT_DBusConnection& connection,
               TQObject* parent = 0, const char* name = 0);

    /**
     * @brief Destroys the proxy instance
     */
    virtual ~TQT_DBusProxy();

    /**
     * @brief Sets the D-Bus connection to work on
     *
     * Disconnects from any previously used connection and connects
     * to the new connection's signal distribution.
     * If no peer information has been provided at creation time or through
     * the other set methods, the instance's signal dbusSignal() will emit
     * all signals received on the given connection.
     *
     * @param connection the D-Bus connection to work on
     *
     * @return @c true if connecting to the new connection's signal succeeded,
     *         @c false if it failed, e.g. if the connection is a "null"
     *         connection
     *
     * @see connection()
     * @see setService()
     * @see setPath()
     * @see setInterface()
     */
    bool setConnection(const TQT_DBusConnection& connection);

    /**
     * @brief Returns the currently used D-Bus connection
     *
     * @see setConnection()
     */
    const TQT_DBusConnection& connection() const;

    /**
     * @brief Sets the peer's service name
     *
     * A non-empty service name is required if the proxy is to be used for
     * method calls. See section @ref dbusconventions-servicename for details.
     *
     * If a string other than @c TQString() is set, it will be used to
     * filter signals, i.e. a signal received by the proxy will only be emitted
     * if the service name matches.
     *
     * @param service the peer's name on the bus
     *
     * @see service()
     * @see setPath()
     * @see setInterface()
     */
    void setService(const TQString& service);

    /**
     * @brief Returns the peer's service name
     *
     * @return the peer object's service name
     *
     * @see setService()
     */
    TQString service() const;

    /**
     * @brief Sets the peer's object path
     *
     * A non-empty object path is required if the proxy is to be used for
     * method calls. See section @ref dbusconventions-objectpath for details.
     *
     * If a string other than @c TQString() is set, it will be used to
     * filter signals, i.e. a signal received by the proxy will only be emitted
     * if the object path matches.
     *
     * @param path the peer's object path inside its host application
     *            (logical address)
     *
     * @see path()
     * @see setService()
     * @see setInterface()
     */
    void setPath(const TQString& path);

    /**
     * @brief Returns the peer's object path
     *
     * @return the peer object's path
     *
     * @see setPath()
     */
    TQString path() const;

    /**
     * @brief Sets the name of the peer interface
     *
     * A non-empty interface name is required if the proxy is to be used for
     * method calls. See section @ref dbusconventions-interfacename for
     * details.
     *
     * If a string other than @c TQString() is set, it will be used to
     * filter signals, i.e. a signal received by the proxy will only be emitted
     * if the interface name matches.
     *
     * @param interface the peer's interface to work with
     *
     * @see interface()
     * @see setService()
     * @see setPath()
     */
    void setInterface(const TQString& interface);

    /**
     * @brief Returns the name of the peer interface
     *
     * @return the peer object's interface
     *
     * @see setInterface()
     */
    TQString interface() const;

    /**
     * @brief Returns whether the proxy can be used to send method calls
     *
     * The capabilitly to send method calls depends on having all necessary
     * base information:
     * - Service name, see setService()
     * - Object path, see setPath()
     * - Interface, see setInterface()
     *
     * and a working connection, see setConnection()
     *
     * @return @c true if method calls can be sent, @c false if any of the three
     *         base information is missing or if the connection is not connected
     *
     * @see send()
     * @see sendWithReply()
     * @see sendWithAsyncReply()
     */
    bool canSend() const;

    /**
     * @brief Sends a method call to the peer object
     *
     * This is roughly equivalent to calling a C++ method with no return value
     * or like ignoring the it.
     *
     * @param method the name of the method to invoke
     * @param params the method parameters. Use an empty list if the method
     *               does not require parameters
     *
     * @return @c true if sending succeeded, @c false if sending failed,
     *         the method name was empty or any of the conditions for
     *         successfull sending as described for canSend() are not met
     *
     * @see lastError()
     * @see sendWithReply()
     * @see sendWithAsyncReply()
     * @see @ref dbusconventions-membername
     */
    bool send(const TQString& method, const TQValueList<TQT_DBusData>& params) const;

    /**
     * @brief Sends a method call to the peer object and waits for the reply
     *
     * This is roughly equivalent to calling a C++ method on a local object.
     *
     * @param method the name of the method to invoke
     * @param params the method parameters. Use an empty list if the method
     *               does not require parameters
     * @param error optional parameter to get any error directly
     *
     * @return a TQT_DBusMessage containing any return values of the invoked method.
     *         Will be an invalid message if an error occurs. The error can be
     *         accessed through the optional paramater @p error or through
     *         lastError()
     *
     * @see canSend()
     * @see send()
     * @see sendWithAsyncReply()
     * @see @ref dbusconventions-membername
     */
    TQT_DBusMessage sendWithReply(const TQString& method,
                               const TQValueList<TQT_DBusData>& params, TQT_DBusError* error = 0) const;

    /**
     * @brief Sends a method call to the peer object but does not wait for an
     *        answer
     *
     * This is roughly equivalent to calling a C++ method on a local TQt
     * event loop driven object, where the result of the method call is
     * delivered later through a signal.
     *
     * @note as with TQt's asychronous classes this needs a running event loop
     *
     * @param method the name of the method to invoke
     * @param params the method parameters. Use an empty list if the method
     *               does not require parameters
     *
     * @return a serial number to easily identify the reply once it is received
     *         or 0 if the call is not possible, i.e. the method name is empty
     *         or any of the conditions for canSend() are not met
     *
     * @warning if a asynchronous call is followed by a synchronous call, e.g.
     *          using sendWithReply(), without returning to the event loop,
     *          is recommended to call TQT_DBusConnection::scheduleDispatch()
     *          after the synchronous call to make sure any reply received
     *          during the blocking call is delivered
     *
     * @see asyncReply()
     * @see send()
     * @see sendWithReply()
     * @see @ref dbusconventions-membername
     */
    int sendWithAsyncReply(const TQString& method, const TQValueList<TQT_DBusData>& params);

    /**
     * @brief Returns the last error seen by the proxy
     *
     * The last error can a connection error, e.g. sending a message failed due
     * connection being lost, or the error of the last call to sendWithReply or
     * the error of the last received asyncReply()
     *
     * @return the last dbus error seen by this proxy
     */
    TQT_DBusError lastError() const;

signals:
    /**
     * @brief Signal emitted for D-Bus signals from the peer
     *
     * Signals received on the proxy's connection are filtered by
     * handleDBusSignal() for all proxy properties that are not empty.
     *
     * @param message the signal's content
     *
     * @see TQT_DBusMessage::SignalMessage
     */
    void dbusSignal(const TQT_DBusMessage& message);

    /**
     * @brief Signal emitted for received replies to asynchronous method calls
     *
     * If a method invoked by using sendWithAsyncReply() send a response, e.g.
     * method return value or errors, this signal is emitted to notify the
     * proxy's user.
     *
     * @param callID the method call's serial number as returned by
     *               sendWithAsyncReply()
     * @param message the reply's content
     *
     * @see handleAsyncReply()
     * @see TQT_DBusMessage::replySerialNumber()
     */
    void asyncReply(int callID, const TQT_DBusMessage& message);

protected slots:
    /**
     * @brief Handles D-Bus signals received on the proxy's connection
     *
     * The base implementation checks each non-empty property, i.e. service
     * name, object path and interface, with the respective field of the
     * signal's D-Bus message.
     *
     * If all available matches succeed, the message is emitted by
     * dbusSignal(), otherwise it is discarded.
     *
     * @note Filtering for @c service will only happen if @c service is a
     *       unique D-Bus name, i.e. if it starts with a colon @c ":" since
     *       D-Bus signals carry the sender's unique name and filtering by a
     *       requested name would reject all signals.
     *
     * @param message the D-Bus signal message as received
     *
     * @see TQT_DBusMessage::SignalMessage
     */
    virtual void handleDBusSignal(const TQT_DBusMessage& message);

    /**
     * @brief Handles replies to asynchronous method calls
     *
     * The base implementation simply extracts the reply's error and makes
     * it available for lastError(). It then emits asyncReply()
     *
     * @param message the D-Bus reply message as received
     *
     * @see TQT_DBusMessage::replySerialNumber()
     */
    virtual void handleAsyncReply(const TQT_DBusMessage& message);

private:
  class Private;
  Private* d;

private:
  TQT_DBusProxy(const TQT_DBusProxy&);
  TQT_DBusProxy& operator=(const TQT_DBusProxy&);
};

#endif

