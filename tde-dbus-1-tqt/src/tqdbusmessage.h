/* qdbusmessage.h TQT_DBusMessage object
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

#ifndef TQDBUSMESSAGE_H
#define TQDBUSMESSAGE_H

#include "tqdbusmacros.h"
#include "tqdbusdata.h"

#include <tqvaluelist.h>

#include <limits.h>

class TQT_DBusError;
class TQT_DBusMessagePrivate;
struct DBusMessage;

/**
 * @brief A message converts and transports data over D-Bus
 *
 * A TQT_DBusMessage is implicitly shared, similar to a TQString, i.e. copying
 * a message creates just a shallow copy.
 *
 * The TQT_DBusMessage is the TQt3 bindings means of encapsulating data for a
 * method call, a method reply or an error.
 *
 * Data specifying the sender and receipient is directly accessible through
 * getter methods, while data, e.g. method parameters or return values, are
 * managed as a list of TQT_DBusData.
 *
 * To create a message suitable for sending use one of the static factory
 * methods:
 * - signal() for creating a D-Bus signal message
 *
 * - methodCall() for creating a D-Bus method calls to a service object
 *
 * - methodReply() for creating a method reply on success
 *
 * - methodError() for creating a method reply on error
 *
 * @note for applications that just want to perform method calls and/or receive
 *       signals, it is usually more convenient to use TQT_DBusProxy instead.
 *
 * Message sending is achieved through TQT_DBusConnection
 *
 * Example:
 * @code
 *   TQT_DBusConnection con = TQT_DBusConnection::sessionBus();
 *
 *   // receipient service is the bus' main interface
 *
 *   TQString service   = "org.freedesktop.DBus";
 *   TQString path      = "/org/freedesktop/DBus";
 *   TQString interface = "org.freedesktop.DBus";
 *
 *   TQT_DBusMessage msg = TQBusMessage::methodCall(service, path, interface, "ListNames");
 *
 *   TQT_DBusMessage reply = con.sendWithReply(msg);
 *
 *   // awaiting for a message list
 *
 *   if (reply.type() != TQT_DBusMessage::ReplyMessage || reply.count() != 2 ||
 *       reply[0].type() != TQT_DBusData::List)
 *   {
 *       // error handling here
 *   }
 *   else
 *   {
 *       TQStringList list = reply[0].toTQStringList();
 *
 *       // reply handling here
 *   }
 * @endcode
 *
 * A service returning such a reply would do something like this
 * @code
 *   bool Service::handleMethodCall(const TQT_DBusMessage& call)
 *   {
 *       // checks for correctness, i.e. correct interface, member,
 *       // would usually haven been placed here
 *
 *       TQStringList result;
 *       result << "Foo" << "Bar";
 *
 *       TQT_DBusMessage reply = TQT_DBusMessage::methodReply(call);
 *       reply << TQT_DBusData::fromList(result);
 *
 *       connection.send(reply);
 *
 *       return true;
 *   }
 * @endcode
 */
class TQDBUS_EXPORT TQT_DBusMessage: public TQValueList<TQT_DBusData>
{
    friend class TQT_DBusConnection;
public:
    /**
     * @brief Anonymous enum for timeout constants
     *
     * @see timeout()
     * @see setTimeout()
     */
    enum
    {
        /**
         * Use whatever D-Bus has as default timeout
         */
        DefaultTimeout = -1,

        /**
         * Use no timeout at all, i.e. wait as long as necessary
         */
        NoTimeout = INT_MAX
    };

    /**
     * @brief D-Bus message types
     *
     * A message of a specific type can be created using the respective factory
     * method. A message created by the default constructor becomes an
     * InvalidMessage
     *
     * @see type()
     * @see signal()
     * @see methodCall()
     * @see methodReply()
     * @see methodError()
     */
    enum MessageType
    {
        /**
         * An invalid message cannot be sent over D-Bus. This type serves for
         * initializing message variables without requiring a "real" message
         */
        InvalidMessage,

        /**
         * A message for doing method calls on remote service objects
         *
         * @see methodCall()
         */
        MethodCallMessage,

        /**
         * A message for replying to a method call in case of success
         *
         * @see methodReply()
         */
        ReplyMessage,

        /**
         * A message for replying to a method call in case of failure
         *
         * @see methodError()
         */
        ErrorMessage,

        /**
         * A message for emitting D-Bus signals
         *
         * @see signal()
         */
        SignalMessage
    };

    /**
     * @brief Creates an empty and invalid message
     *
     * To create a message suitable for sending through D-Bus see the factory
     * methods signal(), methodCall(), methodReply() and methodError()
     *
     * @see #InvalidMessage
     */
    TQT_DBusMessage();


    /**
     * @brief Creates a shallow copy of the given message
     *
     * This instance will become a handle to the same message data the other
     * message is using, including #MessageType
     *
     * @param other the message to copy
     */
    TQT_DBusMessage(const TQT_DBusMessage &other);

    /**
     * @brief Destroys a message
     *
     * If this message handle is the last one using this respective message
     * content, the message content will be deleted as well
     */
    ~TQT_DBusMessage();

    /**
     * @brief Creates a shallow copy of the given message
     *
     * This instance will become a handle to the same message data the other
     * message is usingm including #MessageType
     *
     * Any content used in this instance will be deleted if this instance was
     * the last handle using that content
     *
     * @param other the message to copy
     *
     * @return a reference to this instance as required by assignment operator
     *         semantics
     */
    TQT_DBusMessage &operator=(const TQT_DBusMessage &other);

    /**
     * @brief Creates a message for sending a D-Bus signal
     *
     * Sending/emitting a signal over D-Bus requires a message of type
     * #SignalMessage as well as the information where it is coming from, i.e.
     * which interface of which object is sending it.
     * See @ref dbusconventions for recommendations on those parameters.
     *
     * @param path the object path of the service object
     * @param interface the object's interface to which the signal belongs
     * @param member the signal's name
     *
     * @return a message suitable for appending arguments and for sending
     *
     * @see TQT_DBusConnection::send()
     */
    static TQT_DBusMessage signal(const TQString &path, const TQString &interface,
                               const TQString &member);

    /**
     * @brief Creates a message for sending a D-Bus method call
     *
     * Invoking a method over D-Bus requires a message of type
     * #MethodCallMessage as well as the information where it should be sent
     * to, e.g which interface of which object in which service.
     * See @ref dbusconventions for recommendations on those parameters.
     *
     * @param service the D-Bus name of the application hosting the service
     *                object
     * @param path the object path of the service object
     * @param interface the object's interface to which the method belongs
     * @param method the method's name
     *
     * @return a message suitable for appending arguments and for sending
     *
     * @see methodReply()
     * @see methodError()
     * @see TQT_DBusConnection::send()
     */
    static TQT_DBusMessage methodCall(const TQString &service, const TQString &path,
                                   const TQString &interface, const TQString &method);

    /**
     * @brief Creates a message for replying to a D-Bus method call
     *
     * Replying to a D-Bus method call in the case of success requires a message
     * of type #ReplyMessage as well as the information to which method call it
     * is replying to.
     *
     * @param other the method call message it is replying to
     *
     * @return a message suitable for appending arguments and for sending
     *
     * @see methodCall()
     * @see methodError()
     * @see TQT_DBusConnection::send()
     */
    static TQT_DBusMessage methodReply(const TQT_DBusMessage &other);

    /**
     * @brief Creates a message for replying to a D-Bus method call
     *
     * Replying to a D-Bus method call in the case of failure requires a message
     * of type #ErrorMessage as well as the information to which method call it
     * is replying to and which error occured.
     *
     * @param other the method call message it is replying to
     * @param error the error which occured during during the method call
     *
     * @return a message suitable for appending arguments and for sending
     *
     * @see methodCall()
     * @see methodReply()
     * @see TQT_DBusConnection::send()
     */
    static TQT_DBusMessage methodError(const TQT_DBusMessage &other, const TQT_DBusError& error);

    /**
     * @brief Returns the message's object path
     *
     * See section @ref dbusconventions-objectpath for details.
     *
     * The context of the object path depends on the message type:
     * - #SignalMessage: the path of the service object which emits the signal
     * - #MethodCallMessage: the path of the service object the call is sent to
     * - #ReplyMessage: the path of the object which is replying
     * - #ErrorMessage: the path of the object which is replying
     *
     * @return a non-empty object path or @c TQString()
     *
     * @see interface()
     * @see member()
     * @see sender()
     */
    TQString path() const;

    /**
     * @brief Returns the message's interface name
     *
     * See section @ref dbusconventions-interfacename for details.
     *
     * The context of the interface name depends on the message type:
     * - #SignalMessage: the name of the interface which emits the signal
     * - #MethodCallMessage: the name of the interface the call is sent to
     * - #ReplyMessage: the name of the interface to which the method belonged
     * - #ErrorMessage: the name of the interface to which the method belonged
     *
     * @return a non-empty interface name or @c TQString()
     *
     * @see path()
     * @see member()
     * @see sender()
     */
    TQString interface() const;

    /**
     * @brief Returns the message's member name
     *
     * See section @ref dbusconventions-membername for details.
     *
     * The context of the member name depends on the message type:
     * - #SignalMessage: the name of the emitted signal
     * - #MethodCallMessage: the name of the method to call
     * - #ReplyMessage: the name of the method which replies
     * - #ErrorMessage: the name of the method which replies
     *
     * @return a non-empty member name or @c TQString()
     *
     * @see path()
     * @see interface()
     * @see sender()
     */
    TQString member() const;

    /**
     * @brief Returns the name of the message sender
     *
     * The message sender name or address used on the D-Bus message bus
     * to refer to the application which sent this message.
     *
     * See section @ref dbusconventions-servicename for details.
     *
     * This can either be a unique name as handed out by the bus, see
     * TQT_DBusConnection::uniqueName() or a name registered with
     * TQT_DBusConnection::requestName()
     *
     * @return a non-empty D-Bus sender name or @c TQString()
     *
     * @see path()
     * @see interface()
     * @see member()
     */
    TQString sender() const;

    /**
     * @brief Returns the error of an error message
     *
     * If this message is of type #ErrorMessage, this method can be used
     * to retrieve the respective error object
     *
     * @return the transported error object. Will be empty if this is not an
     *         error message
     *
     * @see type()
     */
    TQT_DBusError error() const;

    /**
     * @brief Returns which kind of message this is
     *
     * @return the message's type
     */
    MessageType type() const;

    /**
     * @brief Returns the message's timeout
     *
     * @return the asynchronous wait timeout in milliseconds
     *
     * @see setTimeout()
     */
    int timeout() const;

    /**
     * @brief Sets the message's timeout
     *
     * The timeout is the number of milliseconds the D-Bus connection will
     * wait for the reply of an asynchronous call.
     *
     * If no reply is received in time, an error message will be delivered to
     * the asynchronous reply receiver.
     *
     * If no timeout is set explicitly, #DefaultTimeout is assumed, which is
     * usually the best option
     *
     * @return the asynchronous wait timeout in milliseconds
     *
     * @param ms timeout in milliseconds
     *
     * @see timeout()
     * @see #DefaultTimeout
     * @see #NoTimeout
     */
    void setTimeout(int ms);

    /**
     * @brief Returns the message's serial number
     *
     * The serial number is some kind of short term identifier for messages
     * travelling the same connection.
     *
     * It can be used to associate a reply or error message with a method
     * call message.
     *
     * @return the message's serial number or @c 0 if the message hasn't been
     *         send yets
     *
     * @see replySerialNumber()
     */
    int serialNumber() const;

    /**
     * @brief Returns the message's reply serial number
     *
     * The reply serial number is the serial number of the method call message
     * this message is a reply to.
     *
     * If this is neither a message of type #ReplyMessage or #ErrorMessage, the
     * returned value will be @c 0
     *
     * It can be used to associate a reply or error message with a method
     * call message.
     *
     * @return the serial number of the associated method call message or @c 0
     *         if this message is not a reply message
     *
     * @see serialNumber()
     * @see methodReply()
     * @see methodError()
     * @see TQT_DBusConnection::sendWithAsyncReply()
     * @see TQT_DBusProxy::sendWithAsyncReply()
     */
    int replySerialNumber() const;

//protected:
    /**
     * @brief Creates a raw D-Bus message from this TQt3-bindings message
     *
     * Marshalls data contained in the message's value list into D-Bus data
     * format and creates a low level API D-Bus message for it.
     *
     * @note ownership of the returned message is transferred to the caller,
     *       i.e. it has to be deleted using dbus_message_unref()
     *
     * @return a C API D-Bus message or @c 0 if this is an #InvalidMessage
     *         or marshalling failed
     */
    DBusMessage *toDBusMessage() const;

    /**
     * @brief Creates a TQt3-bindings message from the given raw D-Bus message
     *
     * De-marshalls data contained in the message to a list of TQT_DBusData.
     *
     * @note ownership of the given message is shared between the caller and
     *       the returned message, i.e. the message as increased the reference
     *       counter and will still have access to the raw message even if the
     *       caller "deleted" it using dbus_message_unref()
     *
     * @param dmsg a C API D-Bus message
     *
     * @return a TQt3 bindings message. Can be an #InvalidMessage if the given
     *         message was @c 0 or if de-marshalling failed
     */
    static TQT_DBusMessage fromDBusMessage(DBusMessage *dmsg);

private:
    TQT_DBusMessagePrivate *d;
};

#endif

