/* qdbusdatamap.h DBUS data mapping transport type
 *
 * Copyright (C) 2007 Kevin Krammer <kevin.krammer@gmx.at>
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

#ifndef TQDBUSDATAMAP_H
#define TQDBUSDATAMAP_H

#include "tqdbusmacros.h"
#include <tqmap.h>

class TQT_DBusData;
class TQT_DBusObjectPath;
class TQT_DBusUnixFd;
class TQT_DBusVariant;

/**
 * @brief Class to transport maps of D-Bus data types
 *
 * \note while the D-Bus data type is actually called @c dict this bindings
 *       use the term @c map since TQT_DBusDataMap is essentially a TQMap
 *
 * There are basically two ways to create TQT_DBusDataMap objects:
 * - non-empty from content
 * - empty by specifying the desired element type
 *
 * Example for creating a filled map from content
 * @code
 * TQMap<TQ_INT16, TQString> intToStringMap;
 * map.insert(2, "two");
 * map.insert(3, "three");
 * map.insert(5, "five");
 * map.insert(7, "seven");
 *
 * TQT_DBusDataMap<TQ_INT16> dbusMap(intToStringMap);
 * TQT_DBusData data = TQT_DBusData::fromInt16KeyMap(dbusMap);
 *
 * // or even shorter, using implicit conversion
 * TQT_DBusData other = TQT_DBusData::fromInt16KeyMap(intList);
 * @endcode
 *
 * Example for creating an empty map
 * @code
 * // empty map for a simple type, mapping from TQString to double
 * TQT_DBusDataMap<TQString> list(TQT_DBusData::Double);
 *
 * // empty map for value type string lists
 * TQT_DBusData valueType = TQT_DBusData::fromList(TQT_DBusDataList(TQT_DBusData::String));
 * TQT_DBusDataMap<TQString> map(valueType);
 * @endcode
 *
 * @see TQT_DBusDataList
 */
template <typename T>
class TQDBUS_EXPORT TQT_DBusDataMap : private TQMap<T, TQT_DBusData>
{
    friend class TQT_DBusData;

public:
    /**
     * Constant iterator. A TQMapConstIterator with value type specified
     * as TQT_DBusData
     */
    typedef TQMapConstIterator<T, TQT_DBusData> const_iterator;

    /**
     * @brief Creates an empty and invalid map
     *
     * @see TQT_DBusData::Invalid
     */
    TQT_DBusDataMap<T>()
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::Invalid) {}

    /**
     * @brief Creates an empty map with the given simple type for values
     *
     * The given type has be one of the non-container types, i.e. any other than
     * TQT_DBusData::Map, TQT_DBusData::List or TQT_DBusData::Struct
     *
     * For creating a map with elements which are containers themselves, use
     * TQT_DBusDataMap<T>(const TQT_DBusData&);
     *
     * @param simpleValueType the type of the values in the new map
     */
    explicit TQT_DBusDataMap<T>(TQT_DBusData::Type simpleValueType)
        : TQMap<T, TQT_DBusData>(), m_valueType(simpleValueType) {}

    /**
     * @brief Creates an empty map with the given container type for values
     *
     * For creating a map with simple values you can also use
     * TQT_DBusDataMap<T>(TQT_DBusData::Type);
     *
     * @param containerValueType the type of the values in the new map
     *
     * @see hasContainerValueType()
     */
    explicit TQT_DBusDataMap<T>(const TQT_DBusData& containerValueType)
        : TQMap<T, TQT_DBusData>(), m_valueType(containerValueType.type())
    {
        if (hasContainerValueType()) m_containerValueType = containerValueType;
    }

    /**
     * @brief Creates a map from the given @p other map
     *
     * This behaves basically like copying a TQMap through its copy
     * constructor, i.e. no value are actually copied at this time.
     *
     * @param other the other map object to copy from
     */
    TQT_DBusDataMap<T>(const TQT_DBusDataMap<T>& other)
        : TQMap<T, TQT_DBusData>(other), m_valueType(other.m_valueType),
          m_containerValueType(other.m_containerValueType) {}

    /**
     * @brief Creates a map from the given TQMap of TQT_DBusData objects
     *
     * If the @p other map is empty, this will behave like TQT_DBusDataMap<T>(),
     * i.e. create an empty and invalid map object.
     *
     * Type information for the map object, i.e. value type and, if applicable,
     * container value type, will be derived from the @p other map's elements.
     *
     * \warning if the values of the @p other map do not all have the same
     *          type, the map object will also be empty and invalid
     *
     * @param other the TQMap of TQT_DBusData objects to copy from
     *
     * @see toTQMap()
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQT_DBusData>& other)
        : TQMap<T, TQT_DBusData>(other), m_valueType(TQT_DBusData::Invalid)
    {
        const_iterator it = begin();
        if (it == end()) return;

        m_valueType = (*it).type();

        TQCString containerSignature;
        if (hasContainerValueType())
        {
            m_containerValueType = it.data();
            containerSignature = m_containerValueType.buildDBusSignature();
        }

        for (++it; it != end(); ++it)
        {
            if ((*it).type() != m_valueType)
            {
                m_valueType = TQT_DBusData::Invalid;
                m_containerValueType = TQT_DBusData();

                clear();
                return;
            }
            else if (hasContainerValueType())
            {
                if (it.data().buildDBusSignature() != containerSignature)
                {
                    m_valueType = TQT_DBusData::Invalid;
                    m_containerValueType = TQT_DBusData();

                    clear();
                    return;
                }
            }
        }
    }

    /**
     * @brief Creates a list from the given TQMap of boolean values
     *
     * Type information for the map object will be set to TQT_DBusData::Bool
     * also when the @p other map is empty, i.e. this allows to create an
     * empty but valid map object, comparable to using
     * TQT_DBusDataMap<T>(TQT_DBusData::Type) with TQT_DBusData::Bool
     *
     * @param other the TQMap of boolean values to copy from
     *
     * @see toBoolMap()
     */
    TQT_DBusDataMap<T>(const TQMap<T, bool>& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::Bool)
    {
        typename TQMap<T, bool>::const_iterator it    = other.begin();
        typename TQMap<T, bool>::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromBool(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of byte (unsigned char) values
     *
     * Type information for the map object will be set to TQT_DBusData::Byte
     * also when the @p other map is empty, i.e. this allows to create an
     * empty but valid map object, comparable to using
     * TQT_DBusDataMap<T>(TQT_DBusData::Type) with TQT_DBusData::Byte
     *
     * @param other the TQMap of byte (unsigned char) values to copy from
     *
     * @see toByteMap()
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQ_UINT8>& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::Byte)
    {
        typename TQMap<T, TQ_UINT8>::const_iterator it    = other.begin();
        typename TQMap<T, TQ_UINT8>::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromByte(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of signed 16-bit integer values
     *
     * Type information for the map object will be set to TQT_DBusData::Int16
     * also when the @p other map is empty, i.e. this allows to create an
     * empty but valid map object, comparable to using
     * TQT_DBusDataMap<T>(TQT_DBusData::Type) with TQT_DBusData::Int16
     *
     * @param other the TQMap of signed 16-bit integer values to copy from
     *
     * @see toInt16Map()
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQ_INT16>& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::Int16)
    {
        typename TQMap<T, TQ_INT16>::const_iterator it    = other.begin();
        typename TQMap<T, TQ_INT16>::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromInt16(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of unsigned 16-bit integer values
     *
     * Type information for the map object will be set to TQT_DBusData::UInt16
     * also when the @p other map is empty, i.e. this allows to create an
     * empty but valid map object, comparable to using
     * TQT_DBusDataMap<T>(TQT_DBusData::Type) with TQT_DBusData::UInt16
     *
     * @param other the TQMap of unsigned 16-bit integer values to copy from
     *
     * @see toUInt16Map()
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQ_UINT16>& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::UInt16)
    {
        typename TQMap<T, TQ_UINT16>::const_iterator it    = other.begin();
        typename TQMap<T, TQ_UINT16>::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromUInt16(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of signed 32-bit integer values
     *
     * Type information for the map object will be set to TQT_DBusData::Int32
     * also when the @p other map is empty, i.e. this allows to create an
     * empty but valid map object, comparable to using
     * TQT_DBusDataMap<T>(TQT_DBusData::Type) with TQT_DBusData::Int32
     *
     * @param other the TQMap of signed 32-bit integer values to copy from
     *
     * @see toInt32Map()
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQ_INT32>& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::Int32)
    {
        typename TQMap<T, TQ_INT32>::const_iterator it    = other.begin();
        typename TQMap<T, TQ_INT32>::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromInt32(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of unsigned 32-bit integer values
     *
     * Type information for the map object will be set to TQT_DBusData::UInt16
     * also when the @p other map is empty, i.e. this allows to create an
     * empty but valid map object, comparable to using
     * TQT_DBusDataMap<T>(TQT_DBusData::Type) with TQT_DBusData::UInt32
     *
     * @param other the TQMap of unsigned 32-bit integer values to copy from
     *
     * @see toUInt32Map()
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQ_UINT32>& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::UInt32)
    {
        typename TQMap<T, TQ_UINT32>::const_iterator it    = other.begin();
        typename TQMap<T, TQ_UINT32>::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromUInt32(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of signed 64-bit integer values
     *
     * Type information for the map object will be set to TQT_DBusData::Int64
     * also when the @p other map is empty, i.e. this allows to create an
     * empty but valid map object, comparable to using
     * TQT_DBusDataMap<T>(TQT_DBusData::Type) with TQT_DBusData::Int64
     *
     * @param other the TQMap of signed 64-bit integer values to copy from
     *
     * @see toInt64Map()
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQ_INT64>& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::Int64)
    {
        typename TQMap<T, TQ_INT64>::const_iterator it    = other.begin();
        typename TQMap<T, TQ_INT64>::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromInt64(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of unsigned 64-bit integer values
     *
     * Type information for the map object will be set to TQT_DBusData::UInt64
     * also when the @p other map is empty, i.e. this allows to create an
     * empty but valid map object, comparable to using
     * TQT_DBusDataMap<T>(TQT_DBusData::Type) with TQT_DBusData::UInt64
     *
     * @param other the TQMap of unsigned 64-bit integer values to copy from
     *
     * @see toUInt64Map()
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQ_UINT64>& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::UInt64)
    {
        typename TQMap<T, TQ_UINT64>::const_iterator it    = other.begin();
        typename TQMap<T, TQ_UINT64>::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromUInt64(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of double values
     *
     * Type information for the map object will be set to TQT_DBusData::Double
     * also when the @p other map is empty, i.e. this allows to create an
     * empty but valid map object, comparable to using
     * TQT_DBusDataMap<T>(TQT_DBusData::Type) with TQT_DBusData::Double
     *
     * @param other the TQMap of double values to copy from
     *
     * @see toDoubleMap()
     */
    TQT_DBusDataMap<T>(const TQMap<T, double>& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::Double)
    {
        typename TQMap<T, double>::const_iterator it    = other.begin();
        typename TQMap<T, double>::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromDouble(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of TQString values
     *
     * Type information for the map object will be set to TQT_DBusData::String
     * also when the @p other map is empty, i.e. this allows to create an
     * empty but valid map object, comparable to using
     * TQT_DBusDataMap<T>(TQT_DBusData::Type) with TQT_DBusData::String
     *
     * @param other the TQMap of TQString values to copy from
     *
     * @see toStringMap()
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQString>& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::String)
    {
        typename TQMap<T, TQString>::const_iterator it    = other.begin();
        typename TQMap<T, TQString>::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromString(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of object path values
     *
     * Type information for the map object will be set to TQT_DBusData::ObjectPath
     * also when the @p other map is empty, i.e. this allows to create an
     * empty but valid map object, comparable to using
     * TQT_DBusDataMap<T>(TQT_DBusData::Type) with TQT_DBusData::ObjectPath
     *
     * @param other the TQMap of object path values to copy from
     *
     * @see toObjectPathMap()
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQT_DBusObjectPath>& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::ObjectPath)
    {
        typename TQMap<T, TQT_DBusObjectPath>::const_iterator it    = other.begin();
        typename TQMap<T, TQT_DBusObjectPath>::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromObjectPath(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of TQT_DBusUnixFd values
     *
     * Type information for the map object will be set to TQT_DBusData::UnixFd
     * also when the @p other map is empty, i.e. this allows to create an
     * empty but valid map object, comparable to using
     * TQT_DBusDataMap<T>(TQT_DBusData::Type) with TQT_DBusData::UnixFd
     *
     * @param other the TQMap of TQT_DBusUnixFd values to copy from
     *
     * @see toUnixFdMap()
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQT_DBusUnixFd>& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::UnixFd)
    {
        typename TQMap<T, TQT_DBusUnixFd>::const_iterator it    = other.begin();
        typename TQMap<T, TQT_DBusUnixFd>::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromUnixFd(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of TQT_DBusVariant values
     *
     * Type information for the map object will be set to TQT_DBusData::Variant
     * also when the @p other map is empty, i.e. this allows to create an
     * empty but valid map object, comparable to using
     * TQT_DBusDataMap<T>(TQT_DBusData::Type) with TQT_DBusData::Variant
     *
     * @param other the TQMap of variant values to copy from
     *
     * @see toVariantMap()
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQT_DBusVariant>& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::Variant)
    {
        typename TQMap<T, TQT_DBusVariant>::const_iterator it    = other.begin();
        typename TQMap<T, TQT_DBusVariant>::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromVariant(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of TQT_DBusDataMap<TQ_UINT8> values
     *
     * @param other the TQMap of TQT_DBusDataMap<TQ_UINT8> values to copy from
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQT_DBusDataMap<TQ_UINT8> >& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::Map)
    {
        typename TQMap<T, TQT_DBusDataMap<TQ_UINT8> >::const_iterator it    = other.begin();
        typename TQMap<T, TQT_DBusDataMap<TQ_UINT8> >::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromByteKeyMap(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of TQT_DBusDataMap<TQ_INT16> values
     *
     * @param other the TQMap of TQT_DBusDataMap<TQ_INT16> values to copy from
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQT_DBusDataMap<TQ_INT16> >& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::Map)
    {
        typename TQMap<T, TQT_DBusDataMap<TQ_INT16> >::const_iterator it    = other.begin();
        typename TQMap<T, TQT_DBusDataMap<TQ_INT16> >::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromInt16KeyMap(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of TQT_DBusDataMap<TQ_UINT16> values
     *
     * @param other the TQMap of TQT_DBusDataMap<TQ_UINT16> values to copy from
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQT_DBusDataMap<TQ_UINT16> >& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::Map)
    {
        typename TQMap<T, TQT_DBusDataMap<TQ_UINT16> >::const_iterator it    = other.begin();
        typename TQMap<T, TQT_DBusDataMap<TQ_UINT16> >::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromUInt16KeyMap(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of TQT_DBusDataMap<TQ_INT32> values
     *
     * @param other the TQMap of TQT_DBusDataMap<TQ_INT32> values to copy from
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQT_DBusDataMap<TQ_INT32> >& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::Map)
    {
        typename TQMap<T, TQT_DBusDataMap<TQ_INT32> >::const_iterator it    = other.begin();
        typename TQMap<T, TQT_DBusDataMap<TQ_INT32> >::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromInt32KeyMap(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of TQT_DBusDataMap<TQ_UINT32> values
     *
     * @param other the TQMap of TQT_DBusDataMap<TQ_UINT32> values to copy from
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQT_DBusDataMap<TQ_UINT32> >& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::Map)
    {
        typename TQMap<T, TQT_DBusDataMap<TQ_UINT32> >::const_iterator it    = other.begin();
        typename TQMap<T, TQT_DBusDataMap<TQ_UINT32> >::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromUInt32KeyMap(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of TQT_DBusDataMap<TQ_INT64> values
     *
     * @param other the TQMap of TQT_DBusDataMap<TQ_INT64> values to copy from
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQT_DBusDataMap<TQ_INT64> >& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::Map)
    {
        typename TQMap<T, TQT_DBusDataMap<TQ_INT64> >::const_iterator it    = other.begin();
        typename TQMap<T, TQT_DBusDataMap<TQ_INT64> >::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromInt64KeyMap(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of TQT_DBusDataMap<TQ_UINT64> values
     *
     * @param other the TQMap of TQT_DBusDataMap<TQ_UINT64> values to copy from
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQT_DBusDataMap<TQ_UINT64> >& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::Map)
    {
        typename TQMap<T, TQT_DBusDataMap<TQ_UINT64> >::const_iterator it    = other.begin();
        typename TQMap<T, TQT_DBusDataMap<TQ_UINT64> >::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromUInt64KeyMap(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of TQT_DBusDataMap<TQString> values
     *
     * @param other the TQMap of TQT_DBusDataMap<TQString> values to copy from
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQT_DBusDataMap<TQString> >& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::Map)
    {
        typename TQMap<T, TQT_DBusDataMap<TQString> >::const_iterator it    = other.begin();
        typename TQMap<T, TQT_DBusDataMap<TQString> >::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromStringKeyMap(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of TQT_DBusDataMap<TQT_DBusObjectPath> values
     *
     * @param other the TQMap of TQT_DBusDataMap<TQT_DBusObjectPath> values to copy from
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQT_DBusDataMap<TQT_DBusObjectPath> >& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::Map)
    {
        typename TQMap<T, TQT_DBusDataMap<TQT_DBusObjectPath> >::const_iterator it    = other.begin();
        typename TQMap<T, TQT_DBusDataMap<TQT_DBusObjectPath> >::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromObjectPathKeyMap(it.data()));
        }
    }

    /**
     * @brief Creates a map from the given TQMap of TQT_DBusDataMap<TQT_DBusUnixFd > values
     *
     * @param other the TQMap of TQT_DBusDataMap<TQT_DBusUnixFd > values to copy from
     */
    TQT_DBusDataMap<T>(const TQMap<T, TQT_DBusDataMap<TQT_DBusUnixFd > >& other)
        : TQMap<T, TQT_DBusData>(), m_valueType(TQT_DBusData::Map)
    {
        typename TQMap<T, TQT_DBusDataMap<TQT_DBusUnixFd > >::const_iterator it    = other.begin();
        typename TQMap<T, TQT_DBusDataMap<TQT_DBusUnixFd > >::const_iterator endIt = other.end();
        for (; it != endIt; ++it)
        {
            insert(it.key(), TQT_DBusData::fromUnixFdKeyMap(it.data()));
        }
    }

    /**
     * @brief Copies from the given @p other map
     *
     * This behaves basically like copying a TQMap through its assignment
     * operator, i.e. no value are actually copied at this time.
     *
     * @param other the other map object to copy from
     *
     * @return a reference to this map object
     */
    TQT_DBusDataMap<T>& operator=(const TQT_DBusDataMap<T>& other)
    {
        TQMap<T, TQT_DBusData>::operator=(other);

        m_valueType = other.m_valueType;
        m_containerValueType = other.m_containerValueType;

        return *this;
    }

    /**
     * @brief Copies from the given @p other map
     *
     * This behaves basically like copying a TQMap through its assignment
     * operator, i.e. no value are actually copied at this time.
     *
     * \warning the value of the given @p other map have to be of the same
     *          type. If they aren't this maps's content will cleared and the
     *          value type will be set to TQT_DBusData::Invalid
     *
     * @param other the other map object to copy from
     *
     * @return a reference to this map object
     */
    TQT_DBusDataMap<T>& operator=(const TQMap<T, TQT_DBusData>& other)
    {
        TQMap<T, TQT_DBusData>::operator=(other);

        m_valueType = TQT_DBusData::Invalid;
        m_containerValueType = TQT_DBusData();

        const_iterator it = begin();
        if (it == end()) return *this;

        m_valueType = (*it).type();

        TQCString containerSignature;
        if (hasContainerValueType())
        {
            m_containerValueType = it.data();
            containerSignature = m_containerValueType.buildDBusSignature();
        }

        for (++it; it != end(); ++it)
        {
            if ((*it).type() != m_valueType)
            {
                m_valueType = TQT_DBusData::Invalid;
                m_containerValueType = TQT_DBusData();

                clear();
                return *this;
            }
            else if (hasContainerValueType())
            {
                if (it.data()->buildSignature() != containerSignature)
                {
                    m_valueType = TQT_DBusData::Invalid;
                    m_containerValueType = TQT_DBusData();

                    clear();
                    return *this;
                }
            }
        }

        return *this;
    }

    /**
     * @brief Returns the key type of the map object
     *
     * @return one of the values of the TQT_DBusData#Type enum suitable for
     *         map keys. See TQT_DBusData::Map for details
     *
     * @see valueType()
     */
    TQT_DBusData::Type keyType() const { return m_keyType; }

    /**
     * @brief Returns the value type of the map object
     *
     * @return one of the values of the TQT_DBusData#Type enum
     *
     * @see hasContainerValueType()
     * @see containerValueType()
     * @see keyType()
     */
    TQT_DBusData::Type valueType() const { return m_valueType; }

    /**
     * @brief Checks whether the value type is a data container itself
     *
     * If the value of the map are containers as well, this will return
     * @c true
     * In this case containerValueType() will return a prototype for such a
     * container.
     *
     * @return @c true if the value type is either TQT_DBusData::Map,
     *         TQT_DBusData::List or TQT_DBusData::Struct, otherwise @c false
     *
     * @see TQT_DBusDataMap<T>(const TQT_DBusData&)
     */
    bool hasContainerValueType() const
    {
        return m_valueType == TQT_DBusData::List || m_valueType == TQT_DBusData::Struct
                                              || m_valueType == TQT_DBusData::Map;
    }

    /**
     * @brief Returns a container prototype for the map's value type
     *
     * Lists which have containers as their elements, i.e. hasContainerValueType()
     * returns @c true this will actually specify the details for the use
     * container, i.e. the returned data object can be queried for type and
     * possible further subtypes.
     *
     * @return a data object detailing the value type or an invalid data object
     *         if the map does not have a container as its element type
     *
     * @see TQT_DBusDataMap<T>(const TQT_DBusData&);
     * @see valueType()
     * @see TQT_DBusData::Invalid
     */
    TQT_DBusData containerValueType() const { return m_containerValueType; }

    /**
     * @brief Checks whether this map object has a valid value type
     *
     * This is equal to checking valueType() for not being TQT_DBusData::Invalid
     *
     * @return @c true if the map object is valid, otherwise @c false
     */
    inline bool isValid() const { return valueType() != TQT_DBusData::Invalid; }

    /**
     * @brief Checks whether this map object has any key/value pairs
     *
     * @return @c true if there are no key/values in this map, otherwise @c false
     *
     * @see count()
     */
    bool isEmpty() const { return TQMap<T, TQT_DBusData>::empty(); }

    /**
     * @brief Returns the number of key/value pairs of this map object
     *
     * @return the number of key/value pairs
     *
     * @see isEmpty()
     */
    uint count() const { return TQMap<T, TQT_DBusData>::count(); }

    /**
     * @brief Checks whether the given @p other map is equal to this one
     *
     * Two maps are considered equal when they have the same value type (and same
     * container value type if the have one) and the key/value pairs are equal
     * as well.
     *
     * @param other the other map object to compare with
     *
     * @return @c true if the maps are equal, otherwise @c false
     *
     * @see TQT_DBusData::operator==()
     */
    bool operator==(const TQT_DBusDataMap<T>& other) const
    {
        if (m_valueType != other.m_valueType) return false;

        if (count() != other.count()) return false;

        if (hasContainerValueType() != other.hasContainerValueType()) return false;

        if (hasContainerValueType())
        {
            if (m_containerValueType.buildDBusSignature() !=
                other.m_containerValueType.buildDBusSignature()) return false;
        }

        const_iterator it = begin();
        const_iterator otherIt = other.begin();
        for (; it != end() && otherIt != other.end(); ++it, ++otherIt)
        {
            if (it.key() != otherIt.key()) return false;

            if (!(it.data() == otherIt.data())) return false;
        }

        return true;
    }

    /**
     * @brief Clears the map
     *
     * Value type and, if applicable, container value type will stay untouched.
     */
    void clear() { TQMap<T, TQT_DBusData>::clear(); }

    /**
     * @brief Returns an iterator to the first item according to the key sort order
     *
     * @see TQMap::begin()
     */
    const_iterator begin() const
    {
        return TQMap<T, TQT_DBusData>::begin();
    }

    /**
     * @brief Returns an iterator to an invalid position
     *
     * @see TQMap::end()
     */
    const_iterator end() const
    {
        return TQMap<T, TQT_DBusData>::end();
    }

    /**
     * @brief Inserts a given value for a given key
     *
     * Basically works like the respective TQMap method, but checks if
     * type of the new value matches the value type of the list.
     * Maps that are invalid will accept any new type and will then be
     * typed accordingly.
     *
     * If @p data is invalid itself, it will not be inserted at any time.
     *
     * \note the more common use case is to work with a TQMap and then
     *       use the respective constructor to create the TQT_DBusDataMap object
     *
     * @param key the key were to insert into the map
     * @param data the data item to insert into the map
     *
     * @return @c true on successfull insert, otherwise @c false
     */
    bool insert(const T& key, const TQT_DBusData& data)
    {
        if (data.type() == TQT_DBusData::Invalid) return false;

        if (m_valueType == TQT_DBusData::Invalid)
        {
            m_valueType = data.type();

            // TODO: create empty copy of container
            if (hasContainerValueType()) m_containerValueType = data;

            TQMap<T, TQT_DBusData>::insert(key, data);
        }
        else if (data.type() != m_valueType)
        {
            tqWarning("TQT_DBusDataMap: trying to add data of type %s to map of type %s",
                     data.typeName(), TQT_DBusData::typeName(m_valueType));
        }
        else if (hasContainerValueType())
        {
            TQCString ourSignature  = m_containerValueType.buildDBusSignature();
            TQCString dataSignature = data.buildDBusSignature();

            if (ourSignature != dataSignature)
            {
                tqWarning("TQT_DBusDataMap: trying to add data with signature %s "
                        "to map with value signature %s",
                        dataSignature.data(), ourSignature.data());
            }
            else
                TQMap<T, TQT_DBusData>::insert(key, data);
        }
        else
            TQMap<T, TQT_DBusData>::insert(key, data);

        return true;
    }

    /**
     * @brief Converts the map object into a TQMap with TQT_DBusData elements
     *
     * @return the key/value pairs of the map object as a TQMap
     */
    TQMap<T, TQT_DBusData> toTQMap() const { return *this; }

    /**
     * @brief Tries to get the map object's pairs as a TQMap of bool
     *
     * This is a convenience overload for the case when the map is of
     * value type TQT_DBusData::Bool.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of value type
     *        TQT_DBusData::Bool)
     *
     * @return a TQMap of bool containing the list object's boolean
     *         values or an empty map when converting fails
     *
     * @see TQT_DBusData::toBool()
     */
    TQMap<T, bool> toBoolMap(bool* ok = 0) const
    {
        if (m_valueType != TQT_DBusData::Bool)
        {
            if (ok != 0) *ok = false;
            return TQMap<T, bool>();
        }

        TQMap<T, bool> result;

        const_iterator it    = begin();
        const_iterator endIt = end();
        for (; it != endIt; ++it)
        {
            result.insert(it.key(), (*it).toBool());
        }

        if (ok != 0) *ok = true;

        return result;
    }

    /**
     * @brief Tries to get the map object's pairs as a TQMap of TQ_UINT8
     *
     * This is a convenience overload for the case when the map is of
     * value type TQT_DBusData::Byte.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of value type
     *        TQT_DBusData::Byte)
     *
     * @return a TQMap of TQ_UINT8 containing the list object's byte
     *         values or an empty map when converting fails
     *
     * @see TQT_DBusData::toBool()
     */
    TQMap<T, TQ_UINT8> toByteMap(bool* ok = 0) const
    {
        if (m_valueType != TQT_DBusData::Byte)
        {
            if (ok != 0) *ok = false;
            return TQMap<T, TQ_UINT8>();
        }

        TQMap<T, TQ_UINT8> result;

        const_iterator it    = begin();
        const_iterator endIt = end();
        for (; it != endIt; ++it)
        {
            result.insert(it.key(), (*it).toByte());
        }

        if (ok != 0) *ok = true;

        return result;
    }

    /**
     * @brief Tries to get the map object's pairs as a TQMap of TQ_INT16
     *
     * This is a convenience overload for the case when the map is of
     * value type TQT_DBusData::Int16.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of value type
     *        TQT_DBusData::Int16)
     *
     * @return a TQMap of TQ_INT16 containing the map object's
     *         signed 16-bit integer values or an empty map when converting
     *         fails
     *
     * @see TQT_DBusData::toInt16()
     */
    TQMap<T, TQ_INT16> toInt16Map(bool* ok = 0) const
    {
        if (m_valueType != TQT_DBusData::Int16)
        {
            if (ok != 0) *ok = false;
            return TQMap<T, TQ_INT16>();
        }

        TQMap<T, TQ_INT16> result;

        const_iterator it    = begin();
        const_iterator endIt = end();
        for (; it != endIt; ++it)
        {
            result.insert(it.key(), (*it).toInt16());
        }

        if (ok != 0) *ok = true;

        return result;
    }

    /**
     * @brief Tries to get the map object's pairs as a TQMap of TQ_UINT16
     *
     * This is a convenience overload for the case when the map is of
     * value type TQT_DBusData::UInt16.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of value type
     *        TQT_DBusData::UInt16)
     *
     * @return a TQMap of TQ_UINT16 containing the map object's
     *         unsigned 16-bit integer values or an empty map when converting
     *         fails
     *
     * @see TQT_DBusData::toUInt16()
     */
    TQMap<T, TQ_UINT16> toUInt16Map(bool* ok = 0) const
    {
        if (m_valueType != TQT_DBusData::UInt16)
        {
            if (ok != 0) *ok = false;
            return TQMap<T, TQ_UINT16>();
        }

        TQMap<T, TQ_UINT16> result;

        const_iterator it    = begin();
        const_iterator endIt = end();
        for (; it != endIt; ++it)
        {
            result.insert(it.key(), (*it).toUInt16());
        }

        if (ok != 0) *ok = true;

        return result;
    }

    /**
     * @brief Tries to get the map object's pairs as a TQMap of TQ_INT32
     *
     * This is a convenience overload for the case when the map is of
     * value type TQT_DBusData::Int32.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of value type
     *        TQT_DBusData::Int32)
     *
     * @return a TQMap of TQ_INT32 containing the map object's
     *         signed 32-bit integer values or an empty map when converting
     *         fails
     *
     * @see TQT_DBusData::toInt32()
     */
    TQMap<T, TQ_INT32> toInt32Map(bool* ok = 0) const
    {
        if (m_valueType != TQT_DBusData::Int32)
        {
            if (ok != 0) *ok = false;
            return TQMap<T, TQ_INT32>();
        }

        TQMap<T, TQ_INT32> result;

        const_iterator it    = begin();
        const_iterator endIt = end();
        for (; it != endIt; ++it)
        {
            result.insert(it.key(), (*it).toInt32());
        }

        if (ok != 0) *ok = true;

        return result;
    }

    /**
     * @brief Tries to get the map object's pairs as a TQMap of TQ_UINT32
     *
     * This is a convenience overload for the case when the map is of
     * value type TQT_DBusData::UInt32.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of value type
     *        TQT_DBusData::UInt32)
     *
     * @return a TQMap of TQ_UINT32 containing the map object's
     *         unsigned 32-bit integer values or an empty map when converting
     *         fails
     *
     * @see TQT_DBusData::toUInt32()
     */
    TQMap<T, TQ_UINT32> toUInt32Map(bool* ok = 0) const
    {
        if (m_valueType != TQT_DBusData::UInt32)
        {
            if (ok != 0) *ok = false;
            return TQMap<T, TQ_UINT32>();
        }

        TQMap<T, TQ_UINT32> result;

        const_iterator it    = begin();
        const_iterator endIt = end();
        for (; it != endIt; ++it)
        {
            result.insert(it.key(), (*it).toUInt32());
        }

        if (ok != 0) *ok = true;

        return result;
    }

    /**
     * @brief Tries to get the map object's pairs as a TQMap of TQ_INT64
     *
     * This is a convenience overload for the case when the map is of
     * value type TQT_DBusData::Int64.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of value type
     *        TQT_DBusData::Int64)
     *
     * @return a TQMap of TQ_INT64 containing the map object's
     *         signed 64-bit integer values or an empty map when converting
     *         fails
     *
     * @see TQT_DBusData::toInt64()
     */
    TQMap<T, TQ_INT64> toInt64Map(bool* ok = 0) const
    {
        if (m_valueType != TQT_DBusData::Int64)
        {
            if (ok != 0) *ok = false;
            return TQMap<T, TQ_INT64>();
        }

        TQMap<T, TQ_INT64> result;

        const_iterator it    = begin();
        const_iterator endIt = end();
        for (; it != endIt; ++it)
        {
            result.insert(it.key(), (*it).toInt64());
        }

        if (ok != 0) *ok = true;

        return result;
    }

    /**
     * @brief Tries to get the map object's pairs as a TQMap of TQ_UINT64
     *
     * This is a convenience overload for the case when the map is of
     * value type TQT_DBusData::UInt64.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of value type
     *        TQT_DBusData::UInt64)
     *
     * @return a TQMap of TQ_UINT64 containing the map object's
     *         unsigned 64-bit integer values or an empty map when converting
     *         fails
     *
     * @see TQT_DBusData::toUInt64()
     */
    TQMap<T, TQ_UINT64> toUInt64Map(bool* ok = 0) const
    {
        if (m_valueType != TQT_DBusData::UInt64)
        {
            if (ok != 0) *ok = false;
            return TQMap<T, TQ_UINT64>();
        }

        TQMap<T, TQ_UINT64> result;

        const_iterator it    = begin();
        const_iterator endIt = end();
        for (; it != endIt; ++it)
        {
            result.insert(it.key(), (*it).toUInt64());
        }

        if (ok != 0) *ok = true;

        return result;
    }

    /**
     * @brief Tries to get the map object's pairs as a TQMap of double
     *
     * This is a convenience overload for the case when the map is of
     * value type TQT_DBusData::Double.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of value type
     *        TQT_DBusData::Double)
     *
     * @return a TQMap of double containing the map object's double
     *         values or an empty map when converting fails
     *
     * @see TQT_DBusData::toDouble()
     */
    TQMap<T, double> toDoubleMap(bool* ok = 0) const
    {
        if (m_valueType != TQT_DBusData::Double)
        {
            if (ok != 0) *ok = false;
            return TQMap<T, double>();
        }

        TQMap<T, double> result;

        const_iterator it    = begin();
        const_iterator endIt = end();
        for (; it != endIt; ++it)
        {
            result.insert(it.key(), (*it).toDouble());
        }

        if (ok != 0) *ok = true;

        return result;
    }

    /**
     * @brief Tries to get the map object's pairs as a TQMap of TQString
     *
     * This is a convenience overload for the case when the map is of
     * value type TQT_DBusData::String, see also toTQStringList().
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of value type
     *        TQT_DBusData::String)
     *
     * @return a TQMap of TQString containing the map object's string
     *         values or an empty map when converting fails
     *
     * @see TQT_DBusData::toString()
     */
    TQMap<T, TQString> toStringMap(bool* ok = 0) const
    {
        if (m_valueType != TQT_DBusData::String)
        {
            if (ok != 0) *ok = false;
            return TQMap<T, TQString>();
        }

        TQMap<T, TQString> result;

        const_iterator it    = begin();
        const_iterator endIt = end();
        for (; it != endIt; ++it)
        {
            result.insert(it.key(), (*it).toString());
        }

        if (ok != 0) *ok = true;

        return result;
    }

    /**
     * @brief Tries to get the map object's pairs as a TQMap of object paths
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of value type
     *        TQT_DBusData::ObjectPath)
     *
     * @return a TQMap of object paths containing the map object's object path
     *         values or an empty map when converting fails
     *
     * @see TQT_DBusData::toObjectPath()
     */
    TQMap<T, TQT_DBusObjectPath> toObjectPathMap(bool* ok = 0) const
    {
        if (m_valueType != TQT_DBusData::ObjectPath)
        {
            if (ok != 0) *ok = false;
            return TQMap<T, TQT_DBusObjectPath>();
        }

        TQMap<T, TQT_DBusObjectPath> result;

        const_iterator it    = begin();
        const_iterator endIt = end();
        for (; it != endIt; ++it)
        {
            result.insert(it.key(), (*it).toObjectPath());
        }

        if (ok != 0) *ok = true;

        return result;
    }

    /**
     * @brief Tries to get the map object's pairs as a TQMap of TQT_DBusUnixFd
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of value type
     *        TQT_DBusData::UnixFd)
     *
     * @return a TQMap of TQT_DBusUnixFd containing the map object's TQT_DBusUnixFd
     *         values or an empty map when converting fails
     *
     * @see TQT_DBusData::toUnixFd()
     */
    TQMap<T, TQT_DBusObjectPath> toUnixFdMap(bool* ok = 0) const
    {
        if (m_valueType != TQT_DBusData::UnixFd)
        {
            if (ok != 0) *ok = false;
            return TQMap<T, TQT_DBusUnixFd>();
        }

        TQMap<T, TQT_DBusUnixFd> result;

        const_iterator it    = begin();
        const_iterator endIt = end();
        for (; it != endIt; ++it)
        {
            result.insert(it.key(), (*it).toUnixFd());
        }

        if (ok != 0) *ok = true;

        return result;
    }

    /**
     * @brief Tries to get the map object's pairs as a TQMap of TQT_DBusVariant
     *
     * This is a convenience overload for the case when the map is of
     * value type TQT_DBusData::Variant.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of value type
     *        TQT_DBusData::Variant)
     *
     * @return a TQMap of TQT_DBusVariant containing the map object's
     *         TQT_DBusVariant values or an empty map when converting fails
     *
     * @see TQT_DBusData::toVariant()
     */
    TQMap<T, TQT_DBusVariant> toVariantMap(bool* ok = 0) const
    {
        if (m_valueType != TQT_DBusData::Variant)
        {
            if (ok != 0) *ok = false;
            return TQMap<T, TQT_DBusVariant>();
        }

        TQMap<T, TQT_DBusVariant> result;

        const_iterator it    = begin();
        const_iterator endIt = end();
        for (; it != endIt; ++it)
        {
            result.insert(it.key(), (*it).toVariant());
        }

        if (ok != 0) *ok = true;

        return result;
    }

private:
    TQT_DBusData::Type m_valueType;
    TQT_DBusData m_containerValueType;

    static const TQT_DBusData::Type m_keyType;
};

#endif
