/* tqdbusunixfd.h DBUS unix file handle data type
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

#ifndef TQDBUSUNIXFD_H
#define TQDBUSUNIXFD_H

#include "dbus/dbus.h"
#ifndef QT_H
#include "tqshared.h"
#endif // QT_H
#include "tqdbusmacros.h"


#ifndef DBUS_TYPE_UNIX_FD
#define   DBUS_TYPE_UNIX_FD   ((int) 'h')
#endif

#ifndef DBUS_TYPE_UNIX_FD_AS_STRING
#define   DBUS_TYPE_UNIX_FD_AS_STRING   "h"
#endif

/**
 * @brief Class for representing D-Bus unix file handles
 *
 * This data type is necessary to correctly represent unix file handles in the
 * context of D-Bus messages, since normal strings have a different D-Bus
 * signature than unix file handles.
 *
 * @see @ref dbusconventions-unixfd
 */
class TQDBUS_EXPORT TQT_DBusUnixFd
{
public:
    /**
     * @brief Creates an empty and invalid unix file handle
     */
    TQT_DBusUnixFd();

    /**
     * @brief Creates copy of the given @p other unix file handle
     *
     * @param other the unix file handle to copy
     */
    TQT_DBusUnixFd(const TQT_DBusUnixFd& other);

    /**
     * @brief Creates copy of the given @p other unix file handle
     *
     * @param other the unix file handle to copy
     */
    TQT_DBusUnixFd(int other);

    /**
     * @brief Destroys the unix file handle
     */
    virtual ~TQT_DBusUnixFd();

    /**
     * @brief Returns whether the current content is considered a valid unix file handle
     *
     * @return \c true if the object's content describe a valid unix file handle,
     *         otherwise @c false
     *
     * @see @ref dbusconventions-unixfd
     */
    bool isValid() const;

    /**
     * @brief Get unix file handle
     *
     * @see @ref dbusconventions-unixfd
     */
    int fileDescriptor() const;

    /**
     * @brief Set new unix file handle
     *
     * @see @ref dbusconventions-unixfd
     */
    void setFileDescriptor(int fileDescriptor);

    /**
     * @brief Give unix file handle
     *
     * @see @ref dbusconventions-unixfd
     */
    void giveFileDescriptor(int fileDescriptor);

    /**
     * @brief Copy unix file handle from TQT_DBusUnixFd
     *
     * @see @ref dbusconventions-unixfd
     */
    TQT_DBusUnixFd &operator=( const TQT_DBusUnixFd &other );

    /**
     * @brief Checks if the given @p other variant is equal to this one
     *
     * @param other unix file handle to compare with
     *
     * @return @c true if both use same file handle, otherwise
     *         @c false
     */
    inline bool operator==(const TQT_DBusUnixFd& other) const
    {
        return (&other == this) || (other.d == d);
    }

    /**
     * @brief Checks if the given @p other variant is not equal to this one
     *
     * @param other unix file handle to compare with
     *
     * @return @c true if both use different file handle, otherwise
     *         @c false
     */
    inline bool operator!=(const TQT_DBusUnixFd& other) const
    {
        return (&other != this) && (other.d != d);
    }
 

protected:
    struct TQT_DBusUnixFdPrivate : public TQShared {
        int fd;
    } *d;

};

#endif
