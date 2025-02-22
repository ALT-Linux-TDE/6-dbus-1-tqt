/* qdbusvariant.h DBUS variant struct
 *
 * Copyright (C) 2005 Harald Fernengel <harry@kdevelop.org>
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

#ifndef TQDBUSVARIANT_H
#define TQDBUSVARIANT_H

#include "tqdbusmacros.h"
#include "tqdbusdata.h"

#include <tqstring.h>

/**
 * @brief Data type for representing a D-Bus variant
 *
 * When D-Bus methods or signal require that a paramater can have any of the
 * D-Bus data types, a D-Bus variant can be used.
 *
 * Basically a D-Bus variant includes the actual data and a D-Bus data signature
 * to allow a receiver to determine the contents.
 *
 * Since the TQT_DBusVariant's #value member will already be fully de-marshalled,
 * a receiver using this bindings can savely ignore the signature if it doesn't
 * need it for a different purpose (e.g. logging).
 *
 * However, when creating a TQT_DBusVariant object for sending, make sure the
 * #signature member is correctly setup, for example by using the #value
 * member's buildDBusSignature() method.
 *
 * @code
 * TQT_DBusVariant variant;
 *
 * variant.value     = TQT_DBusData::fromInt32(131719);
 * variant.signature = variant.value.buildDBusSignature();
 * @endcode
 */
class TQDBUS_EXPORT TQT_DBusVariant
{
public:
    /**
     * @brief Creates an empty variant object
     */
    TQT_DBusVariant() {}

    /**
     * @brief Copies the given @p other variant object
     *
     * @param other the variant object to copy from
     */
    TQT_DBusVariant(const TQT_DBusVariant& other)
    {
        signature = other.signature;
        value = other.value;
    }

    /**
     * @brief Checks if the given @p other variant is equal to this one
     *
     * @param other the variant object to compare with
     *
     * @return @c true if both #signature and #value are equal, otherwise
     *         @c false
     */
    inline bool operator==(const TQT_DBusVariant& other) const
    {
        if (&other == this) return true;

        return signature == other.signature && value == other.value;
    }

    /**
     * @brief Checks if the given @p other variant is not equal to this one
     *
     * @param other the variant object to compare with
     *
     * @return @c true if either #signature or #value is different, otherwise
     *         @c false
     */
    inline bool operator!=(const TQT_DBusVariant& other) const
    {
        if (&other == this) return false;

        return signature != other.signature || value != other.value;
    }

public:
    /**
     * @brief The D-Bus data signature of the data contained in #value
     *
     * @see TQT_DBusData::buildDBusSignature()
     */
    TQString signature;

    /**
     * @brief The D-Bus data type to transport as a variant
     */
    TQT_DBusData value;
};

#endif

