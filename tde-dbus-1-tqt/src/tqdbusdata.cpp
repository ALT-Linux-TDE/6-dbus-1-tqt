/* qdbusdata.cpp DBUS data transport type
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

#include "dbus/dbus.h"

#include "tqdbusdata.h"
#include "tqdbusdatalist.h"
#include "tqdbusdatamap.h"
#include "tqdbusobjectpath.h"
#include "tqdbusunixfd.h"
#include "tqdbusvariant.h"

#include <tqshared.h>
#include <tqstring.h>
#include <tqvaluelist.h>

class TQT_DBusData::Private : public TQShared
{
public:
    Private() : TQShared(), type(TQT_DBusData::Invalid), keyType(TQT_DBusData::Invalid) {}

    ~Private()
    {
        switch (type)
        {
            case TQT_DBusData::String:
                delete (TQString*)value.pointer;
                break;

            case TQT_DBusData::ObjectPath:
                delete (TQT_DBusObjectPath*)value.pointer;
                break;

            case TQT_DBusData::UnixFd:
                delete (TQT_DBusUnixFd*)value.pointer;
                break;

            case TQT_DBusData::List:
                delete (TQT_DBusDataList*)value.pointer;
                break;

            case TQT_DBusData::Struct:
                delete (TQValueList<TQT_DBusData>*)value.pointer;
                break;

            case TQT_DBusData::Variant:
                delete (TQT_DBusVariant*)value.pointer;
                break;

            case TQT_DBusData::Map:
                switch (keyType)
                {
                    case TQT_DBusData::Byte:
                        delete (TQT_DBusDataMap<TQ_UINT8>*)value.pointer;
                        break;

                    case TQT_DBusData::Int16:
                        delete (TQT_DBusDataMap<TQ_INT16>*)value.pointer;
                        break;

                    case TQT_DBusData::UInt16:
                        delete (TQT_DBusDataMap<TQ_UINT16>*)value.pointer;
                        break;

                    case TQT_DBusData::Int32:
                        delete (TQT_DBusDataMap<TQ_INT32>*)value.pointer;
                        break;

                    case TQT_DBusData::UInt32:
                        delete (TQT_DBusDataMap<TQ_UINT32>*)value.pointer;
                        break;

                    case TQT_DBusData::Int64:
                        delete (TQT_DBusDataMap<TQ_INT64>*)value.pointer;
                        break;

                    case TQT_DBusData::UInt64:
                        delete (TQT_DBusDataMap<TQ_UINT64>*)value.pointer;
                        break;

                    case TQT_DBusData::String:
                        delete (TQT_DBusDataMap<TQString>*)value.pointer;
                        break;

                    case TQT_DBusData::ObjectPath:
                        delete (TQT_DBusDataMap<TQT_DBusObjectPath>*)value.pointer;
                        break;

                    case TQT_DBusData::UnixFd:
                        delete (TQT_DBusDataMap<TQT_DBusUnixFd>*)value.pointer;
                        break;

                    default:
                        tqFatal("TQT_DBusData::Private: unhandled map key type %d(%s)",
                               keyType, TQT_DBusData::typeName(keyType));
                        break;
                }
                break;

            default:
                break;
        }
    }

public:
    Type type;
    Type keyType;

    union
    {
        bool boolValue;
        TQ_UINT8 byteValue;
        TQ_INT16 int16Value;
        TQ_UINT16 uint16Value;
        TQ_INT32 int32Value;
        TQ_UINT32 uint32Value;
        TQ_INT64 int64Value;
        TQ_UINT64 uint64Value;
        double doubleValue;
        void* pointer;
    } value;
};

// key type definitions for TQT_DBusDataMap
template <>
const TQT_DBusData::Type TQT_DBusDataMap<TQ_UINT8>::m_keyType = TQT_DBusData::Byte;

template <>
const TQT_DBusData::Type TQT_DBusDataMap<TQ_INT16>::m_keyType = TQT_DBusData::Int16;

template <>
const TQT_DBusData::Type TQT_DBusDataMap<TQ_UINT16>::m_keyType = TQT_DBusData::UInt16;

template <>
const TQT_DBusData::Type TQT_DBusDataMap<TQ_INT32>::m_keyType = TQT_DBusData::Int32;

template <>
const TQT_DBusData::Type TQT_DBusDataMap<TQ_UINT32>::m_keyType = TQT_DBusData::UInt32;

template <>
const TQT_DBusData::Type TQT_DBusDataMap<TQ_INT64>::m_keyType = TQT_DBusData::Int64;

template <>
const TQT_DBusData::Type TQT_DBusDataMap<TQ_UINT64>::m_keyType = TQT_DBusData::UInt64;

template <>
const TQT_DBusData::Type TQT_DBusDataMap<TQString>::m_keyType = TQT_DBusData::String;

template <>
const TQT_DBusData::Type TQT_DBusDataMap<TQT_DBusObjectPath>::m_keyType = TQT_DBusData::ObjectPath;

template <>
const TQT_DBusData::Type TQT_DBusDataMap<TQT_DBusUnixFd>::m_keyType = TQT_DBusData::UnixFd;


TQT_DBusData::TQT_DBusData() : d(new Private())
{
}

TQT_DBusData::TQT_DBusData(const TQT_DBusData& other) : d(0)
{
    d = other.d;

    d->ref();
}

TQT_DBusData::~TQT_DBusData()
{
    if (d->deref()) delete d;
}

TQT_DBusData& TQT_DBusData::operator=(const TQT_DBusData& other)
{
    if (&other == this) return *this;

    if (d->deref()) delete d;

    d = other.d;

    d->ref();

    return *this;
}

bool TQT_DBusData::operator==(const TQT_DBusData& other) const
{
    if (&other == this) return true;

    if (d == other.d) return true;

    if (d->type == other.d->type)
    {
        switch (d->type)
        {
            case TQT_DBusData::Invalid:
                return true;

            case TQT_DBusData::Bool:
                return d->value.boolValue == other.d->value.boolValue;

            case TQT_DBusData::Byte:
                return d->value.byteValue == other.d->value.byteValue;

            case TQT_DBusData::Int16:
                return d->value.int16Value == other.d->value.int16Value;

            case TQT_DBusData::UInt16:
                return d->value.uint16Value == other.d->value.uint16Value;

            case TQT_DBusData::Int32:
                return d->value.int32Value == other.d->value.int32Value;

            case TQT_DBusData::UInt32:
                return d->value.uint32Value == other.d->value.uint64Value;

            case TQT_DBusData::Int64:
                return d->value.int64Value == other.d->value.int64Value;

            case TQT_DBusData::UInt64:
                return d->value.uint64Value == other.d->value.uint64Value;

            case TQT_DBusData::Double:
                // FIXME: should not compare doubles for equality like this
                return d->value.doubleValue == other.d->value.doubleValue;

            case TQT_DBusData::String:
                return toString() == other.toString();

            case TQT_DBusData::ObjectPath:
                return toObjectPath() == other.toObjectPath();

            case TQT_DBusData::UnixFd:
                return toUnixFd() == other.toUnixFd();

            case TQT_DBusData::List:
                return toList() == other.toList();

            case TQT_DBusData::Struct:
                return toStruct() == other.toStruct();

            case TQT_DBusData::Variant:
                return toVariant() == other.toVariant();

            case TQT_DBusData::Map:
                if (d->keyType != other.d->keyType) return false;

                switch (d->keyType)
                {
                    case TQT_DBusData::Byte:
                        return toByteKeyMap() == other.toByteKeyMap();

                    case TQT_DBusData::Int16:
                        return toInt16KeyMap() == other.toInt16KeyMap();

                    case TQT_DBusData::UInt16:
                        return toUInt16KeyMap() == other.toUInt16KeyMap();

                    case TQT_DBusData::Int32:
                        return toInt32KeyMap() == other.toInt32KeyMap();

                    case TQT_DBusData::UInt32:
                        return toUInt32KeyMap() == other.toUInt32KeyMap();

                    case TQT_DBusData::Int64:
                        return toInt64KeyMap() == other.toInt64KeyMap();

                    case TQT_DBusData::UInt64:
                        return toUInt64KeyMap() == other.toUInt64KeyMap();

                    case TQT_DBusData::String:
                        return toStringKeyMap() == other.toStringKeyMap();

                    case TQT_DBusData::ObjectPath:
                        return toObjectPathKeyMap() == other.toObjectPathKeyMap();

                    case TQT_DBusData::UnixFd:
                        return toUnixFdKeyMap() == other.toUnixFdKeyMap();

                    default:
                        tqFatal("TQT_DBusData operator== unhandled map key type %d(%s)",
                               d->keyType, TQT_DBusData::typeName(d->keyType));
                        break;
                }

                break;
        }
    }

    return false;
}

bool TQT_DBusData::operator!=(const TQT_DBusData& other) const
{
    return !operator==(other);
}

TQT_DBusData::Type TQT_DBusData::type() const
{
    return d->type;
}

TQT_DBusData::Type TQT_DBusData::keyType() const
{
    if (d->type != TQT_DBusData::Map) return TQT_DBusData::Invalid;

    return d->keyType;
}

const char* TQT_DBusData::typeName(Type type)
{
    switch (type)
    {
        case TQT_DBusData::Invalid:    return "Invalid";
        case TQT_DBusData::Bool:       return "Bool";
        case TQT_DBusData::Byte:       return "Byte";
        case TQT_DBusData::Int16:      return "Int16";
        case TQT_DBusData::UInt16:     return "UInt16";
        case TQT_DBusData::Int32:      return "Int32";
        case TQT_DBusData::UInt32:     return "UInt32";
        case TQT_DBusData::Int64:      return "Int64";
        case TQT_DBusData::UInt64:     return "UInt64";
        case TQT_DBusData::Double:     return "Double";
        case TQT_DBusData::String:     return "String";
        case TQT_DBusData::ObjectPath: return "ObjectPath";
        case TQT_DBusData::UnixFd:     return "UnixFd";
        case TQT_DBusData::List:       return "List";
        case TQT_DBusData::Struct:     return "Struct";
        case TQT_DBusData::Variant:    return "Variant";
        case TQT_DBusData::Map:        return "Map";
    }

    return 0;
}

TQT_DBusData TQT_DBusData::fromBool(bool value)
{
    TQT_DBusData data;

    data.d->type = TQT_DBusData::Bool;
    data.d->value.boolValue = value;

    return data;
}

bool TQT_DBusData::toBool(bool* ok) const
{
    if (d->type != TQT_DBusData::Bool)
    {
        if (ok != 0) *ok = false;
        return false;
    }

    if (ok != 0) *ok = true;

    return d->value.boolValue;
}

TQT_DBusData TQT_DBusData::fromByte(TQ_UINT8 value)
{
    TQT_DBusData data;

    data.d->type = TQT_DBusData::Byte;
    data.d->value.byteValue = value;

    return data;
}

TQ_UINT8 TQT_DBusData::toByte(bool* ok) const
{
    if (d->type != TQT_DBusData::Byte)
    {
        if (ok != 0) *ok = false;
        return 0;
    }

    if (ok != 0) *ok = true;

    return d->value.byteValue;
}

TQT_DBusData TQT_DBusData::fromInt16(TQ_INT16 value)
{
    TQT_DBusData data;

    data.d->type = TQT_DBusData::Int16;
    data.d->value.int16Value = value;

    return data;
}

TQ_INT16 TQT_DBusData::toInt16(bool* ok) const
{
    if (d->type != TQT_DBusData::Int16)
    {
        if (ok != 0) *ok = false;
        return 0;
    }

    if (ok != 0) *ok = true;

    return d->value.int16Value;
}

TQT_DBusData TQT_DBusData::fromUInt16(TQ_UINT16 value)
{
    TQT_DBusData data;

    data.d->type = TQT_DBusData::UInt16;
    data.d->value.uint16Value = value;

    return data;
}

TQ_UINT16 TQT_DBusData::toUInt16(bool* ok) const
{
    if (d->type != TQT_DBusData::UInt16)
    {
        if (ok != 0) *ok = false;
        return 0;
    }

    if (ok != 0) *ok = true;

    return d->value.uint16Value;
}

TQT_DBusData TQT_DBusData::fromInt32(TQ_INT32 value)
{
    TQT_DBusData data;

    data.d->type = TQT_DBusData::Int32;
    data.d->value.int32Value = value;

    return data;
}

TQ_INT32 TQT_DBusData::toInt32(bool* ok) const
{
    if (d->type != TQT_DBusData::Int32)
    {
        if (ok != 0) *ok = false;
        return 0;
    }

    if (ok != 0) *ok = true;

    return d->value.int32Value;
}

TQT_DBusData TQT_DBusData::fromUInt32(TQ_UINT32 value)
{
    TQT_DBusData data;

    data.d->type = TQT_DBusData::UInt32;
    data.d->value.uint32Value = value;

    return data;
}

TQ_UINT32 TQT_DBusData::toUInt32(bool* ok) const
{
    if (d->type != TQT_DBusData::UInt32)
    {
        if (ok != 0) *ok = false;
        return 0;
    }

    if (ok != 0) *ok = true;

    return d->value.uint32Value;
}

TQT_DBusData TQT_DBusData::fromInt64(TQ_INT64 value)
{
    TQT_DBusData data;

    data.d->type = TQT_DBusData::Int64;
    data.d->value.int64Value = value;

    return data;
}

TQ_INT64 TQT_DBusData::toInt64(bool* ok) const
{
    if (d->type != TQT_DBusData::Int64)
    {
        if (ok != 0) *ok = false;
        return 0;
    }

    if (ok != 0) *ok = true;

    return d->value.int64Value;
}

TQT_DBusData TQT_DBusData::fromUInt64(TQ_UINT64 value)
{
    TQT_DBusData data;

    data.d->type = TQT_DBusData::UInt64;
    data.d->value.uint64Value = value;

    return data;
}

TQ_UINT64 TQT_DBusData::toUInt64(bool* ok) const
{
    if (d->type != TQT_DBusData::UInt64)
    {
        if (ok != 0) *ok = false;
        return 0;
    }

    if (ok != 0) *ok = true;

    return d->value.uint64Value;
}

TQT_DBusData TQT_DBusData::fromDouble(double value)
{
    TQT_DBusData data;

    data.d->type = TQT_DBusData::Double;
    data.d->value.doubleValue = value;

    return data;
}

double TQT_DBusData::toDouble(bool* ok) const
{
    if (d->type != TQT_DBusData::Double)
    {
        if (ok != 0) *ok = false;
        return 0.0;
    }

    if (ok != 0) *ok = true;

    return d->value.doubleValue;
}

TQT_DBusData TQT_DBusData::fromString(const TQString& value)
{
    TQT_DBusData data;

    data.d->type = TQT_DBusData::String;
    data.d->value.pointer = new TQString(value);

    return data;
}

TQString TQT_DBusData::toString(bool* ok) const
{
    if (d->type != TQT_DBusData::String)
    {
        if (ok != 0) *ok = false;
        return TQString();
    }

    if (ok != 0) *ok = true;

    return *((TQString*)d->value.pointer);
}

TQT_DBusData TQT_DBusData::fromObjectPath(const TQT_DBusObjectPath& value)
{
    TQT_DBusData data;

    if (value.isValid())
    {
        data.d->type = TQT_DBusData::ObjectPath;
        data.d->value.pointer = new TQT_DBusObjectPath(value);
    }

    return data;
}

TQT_DBusObjectPath TQT_DBusData::toObjectPath(bool* ok) const
{
    if (d->type != TQT_DBusData::ObjectPath)
    {
        if (ok != 0) *ok = false;
        return TQT_DBusObjectPath();
    }

    if (ok != 0) *ok = true;

    return *((TQT_DBusObjectPath*)d->value.pointer);
}

TQT_DBusData TQT_DBusData::fromUnixFd(const TQT_DBusUnixFd& value)
{
    TQT_DBusData data;

    if (value.isValid())
    {
        data.d->type = TQT_DBusData::UnixFd;
        data.d->value.pointer = new TQT_DBusUnixFd(value);
    }

    return data;
}

TQT_DBusUnixFd TQT_DBusData::toUnixFd(bool* ok) const
{
    if (d->type != TQT_DBusData::UnixFd)
    {
        if (ok != 0) *ok = false;
        return TQT_DBusUnixFd();
    }

    if (ok != 0) *ok = true;

    return *((TQT_DBusUnixFd*)d->value.pointer);
}

TQT_DBusData TQT_DBusData::fromList(const TQT_DBusDataList& list)
{
    TQT_DBusData data;

    if (list.type() == TQT_DBusData::Invalid) return data;

    data.d->type = TQT_DBusData::List;
    data.d->value.pointer = new TQT_DBusDataList(list);

    return data;
}

TQT_DBusDataList TQT_DBusData::toList(bool* ok) const
{
    if (d->type != TQT_DBusData::List)
    {
        if (ok != 0) *ok = false;
        return TQT_DBusDataList();
    }

    if (ok != 0) *ok = true;

    return *((TQT_DBusDataList*)d->value.pointer);
}

TQT_DBusData TQT_DBusData::fromTQValueList(const TQValueList<TQT_DBusData>& list)
{
    return fromList(TQT_DBusDataList(list));
}

TQValueList<TQT_DBusData> TQT_DBusData::toTQValueList(bool* ok) const
{
    bool internalOk = false;
    TQT_DBusDataList list = toList(&internalOk);

    if (!internalOk)
    {
        if (ok != 0) *ok = false;
        return TQValueList<TQT_DBusData>();
    }

    return list.toTQValueList();
}

TQT_DBusData TQT_DBusData::fromStruct(const TQValueList<TQT_DBusData>& memberList)
{
    TQT_DBusData data;

    TQValueList<TQT_DBusData>::const_iterator it    = memberList.begin();
    TQValueList<TQT_DBusData>::const_iterator endIt = memberList.end();
    for (; it != endIt; ++it)
    {
        if ((*it).d->type == Invalid) return data;
    }

    data.d->type = TQT_DBusData::Struct;
    data.d->value.pointer = new TQValueList<TQT_DBusData>(memberList);

    return data;
}

TQValueList<TQT_DBusData> TQT_DBusData::toStruct(bool* ok) const
{
    if (d->type != TQT_DBusData::Struct)
    {
        if (ok != 0) *ok = false;
        return TQValueList<TQT_DBusData>();
    }

    if (ok != 0) *ok = true;

    return *((TQValueList<TQT_DBusData>*)d->value.pointer);
}

TQT_DBusData TQT_DBusData::fromVariant(const TQT_DBusVariant& value)
{
    TQT_DBusData data;

    data.d->type = TQT_DBusData::Variant;
    data.d->value.pointer = new TQT_DBusVariant(value);

    return data;
}

TQT_DBusVariant TQT_DBusData::toVariant(bool* ok) const
{
    if (d->type != TQT_DBusData::Variant)
    {
        if (ok != 0) *ok = false;
        return TQT_DBusVariant();
    }

    if (ok != 0) *ok = true;

    return *((TQT_DBusVariant*)d->value.pointer);
}

TQT_DBusData TQT_DBusData::getAsVariantData()
{
    TQT_DBusVariant variant;
    variant.value = *this;
    variant.signature = variant.value.buildDBusSignature();
    return TQT_DBusData::fromVariant(variant);
}

TQT_DBusData TQT_DBusData::fromByteKeyMap(const TQT_DBusDataMap<TQ_UINT8>& map)
{
    TQT_DBusData data;

    data.d->type = TQT_DBusData::Map;
    data.d->keyType = map.keyType();
    data.d->value.pointer = new TQT_DBusDataMap<TQ_UINT8>(map);

    return data;
}

TQT_DBusDataMap<TQ_UINT8> TQT_DBusData::toByteKeyMap(bool* ok) const
{
    if (d->type != TQT_DBusData::Map && d->keyType != TQT_DBusDataMap<TQ_UINT8>::m_keyType)
    {
        if (ok != 0) *ok = false;
        return TQT_DBusDataMap<TQ_UINT8>();
    }

    if (ok != 0) *ok = true;

    return *((TQT_DBusDataMap<TQ_UINT8>*)d->value.pointer);
}

TQT_DBusData TQT_DBusData::fromInt16KeyMap(const TQT_DBusDataMap<TQ_INT16>& map)
{
    TQT_DBusData data;

    data.d->type = TQT_DBusData::Map;
    data.d->keyType = map.keyType();
    data.d->value.pointer = new TQT_DBusDataMap<TQ_INT16>(map);

    return data;
}

TQT_DBusDataMap<TQ_INT16> TQT_DBusData::toInt16KeyMap(bool* ok) const
{
    if (d->type != TQT_DBusData::Map && d->keyType != TQT_DBusDataMap<TQ_INT16>::m_keyType)
    {
        if (ok != 0) *ok = false;
        return TQT_DBusDataMap<TQ_INT16>();
    }

    if (ok != 0) *ok = true;

    return *((TQT_DBusDataMap<TQ_INT16>*)d->value.pointer);
}

TQT_DBusData TQT_DBusData::fromUInt16KeyMap(const TQT_DBusDataMap<TQ_UINT16>& map)
{
    TQT_DBusData data;

    data.d->type = TQT_DBusData::Map;
    data.d->keyType = map.keyType();
    data.d->value.pointer = new TQT_DBusDataMap<TQ_UINT16>(map);

    return data;
}

TQT_DBusDataMap<TQ_UINT16> TQT_DBusData::toUInt16KeyMap(bool* ok) const
{
    if (d->type != TQT_DBusData::Map &&
        d->keyType != TQT_DBusDataMap<TQ_UINT16>::m_keyType)
    {
        if (ok != 0) *ok = false;
        return TQT_DBusDataMap<TQ_UINT16>();
    }

    if (ok != 0) *ok = true;

    return *((TQT_DBusDataMap<TQ_UINT16>*)d->value.pointer);
}

TQT_DBusData TQT_DBusData::fromInt32KeyMap(const TQT_DBusDataMap<TQ_INT32>& map)
{
    TQT_DBusData data;

    data.d->type = TQT_DBusData::Map;
    data.d->keyType = map.keyType();
    data.d->value.pointer = new TQT_DBusDataMap<TQ_INT32>(map);

    return data;
}

TQT_DBusDataMap<TQ_INT32> TQT_DBusData::toInt32KeyMap(bool* ok) const
{
    if (d->type != TQT_DBusData::Map && d->keyType != TQT_DBusDataMap<TQ_INT32>::m_keyType)
    {
        if (ok != 0) *ok = false;
        return TQT_DBusDataMap<TQ_INT32>();
    }

    if (ok != 0) *ok = true;

    return *((TQT_DBusDataMap<TQ_INT32>*)d->value.pointer);
}

TQT_DBusData TQT_DBusData::fromUInt32KeyMap(const TQT_DBusDataMap<TQ_UINT32>& map)
{
    TQT_DBusData data;

    data.d->type = TQT_DBusData::Map;
    data.d->keyType = map.keyType();
    data.d->value.pointer = new TQT_DBusDataMap<TQ_UINT32>(map);

    return data;
}

TQT_DBusDataMap<TQ_UINT32> TQT_DBusData::toUInt32KeyMap(bool* ok) const
{
    if (d->type != TQT_DBusData::Map &&
        d->keyType != TQT_DBusDataMap<TQ_UINT32>::m_keyType)
    {
        if (ok != 0) *ok = false;
        return TQT_DBusDataMap<TQ_UINT32>();
    }

    if (ok != 0) *ok = true;

    return *((TQT_DBusDataMap<TQ_UINT32>*)d->value.pointer);
}

TQT_DBusData TQT_DBusData::fromInt64KeyMap(const TQT_DBusDataMap<TQ_INT64>& map)
{
    TQT_DBusData data;

    data.d->type = TQT_DBusData::Map;
    data.d->keyType = map.keyType();
    data.d->value.pointer = new TQT_DBusDataMap<TQ_INT64>(map);

    return data;
}

TQT_DBusDataMap<TQ_INT64> TQT_DBusData::toInt64KeyMap(bool* ok) const
{
    if (d->type != TQT_DBusData::Map && d->keyType != TQT_DBusDataMap<TQ_INT64>::m_keyType)
    {
        if (ok != 0) *ok = false;
        return TQT_DBusDataMap<TQ_INT64>();
    }

    if (ok != 0) *ok = true;

    return *((TQT_DBusDataMap<TQ_INT64>*)d->value.pointer);
}

TQT_DBusData TQT_DBusData::fromUInt64KeyMap(const TQT_DBusDataMap<TQ_UINT64>& map)
{
    TQT_DBusData data;

    data.d->type = TQT_DBusData::Map;
    data.d->keyType = map.keyType();
    data.d->value.pointer = new TQT_DBusDataMap<TQ_UINT64>(map);

    return data;
}

TQT_DBusDataMap<TQ_UINT64> TQT_DBusData::toUInt64KeyMap(bool* ok) const
{
    if (d->type != TQT_DBusData::Map &&
        d->keyType != TQT_DBusDataMap<TQ_UINT64>::m_keyType)
    {
        if (ok != 0) *ok = false;
        return TQT_DBusDataMap<TQ_UINT64>();
    }

    if (ok != 0) *ok = true;

    return *((TQT_DBusDataMap<TQ_UINT64>*)d->value.pointer);
}

TQT_DBusData TQT_DBusData::fromStringKeyMap(const TQT_DBusDataMap<TQString>& map)
{
    TQT_DBusData data;

    data.d->type = TQT_DBusData::Map;
    data.d->keyType = map.keyType();
    data.d->value.pointer = new TQT_DBusDataMap<TQString>(map);

    return data;
}

TQT_DBusDataMap<TQString> TQT_DBusData::toStringKeyMap(bool* ok) const
{
    if (d->type != TQT_DBusData::Map && d->keyType != TQT_DBusDataMap<TQString>::m_keyType)
    {
        if (ok != 0) *ok = false;
        return TQT_DBusDataMap<TQString>();
    }

    if (ok != 0) *ok = true;

    return *((TQT_DBusDataMap<TQString>*)d->value.pointer);
}

TQT_DBusData TQT_DBusData::fromObjectPathKeyMap(const TQT_DBusDataMap<TQT_DBusObjectPath>& map)
{
    TQT_DBusData data;

    data.d->type = TQT_DBusData::Map;
    data.d->keyType = map.keyType();
    data.d->value.pointer = new TQT_DBusDataMap<TQT_DBusObjectPath>(map);

    return data;
}

TQT_DBusDataMap<TQT_DBusObjectPath> TQT_DBusData::toObjectPathKeyMap(bool* ok) const
{
    if (d->type != TQT_DBusData::Map &&
        d->keyType != TQT_DBusDataMap<TQT_DBusObjectPath>::m_keyType)
    {
        if (ok != 0) *ok = false;
        return TQT_DBusDataMap<TQT_DBusObjectPath>();
    }

    if (ok != 0) *ok = true;

    return *((TQT_DBusDataMap<TQT_DBusObjectPath>*)d->value.pointer);
}

TQT_DBusData TQT_DBusData::fromUnixFdKeyMap(const TQT_DBusDataMap<TQT_DBusUnixFd>& map)
{
    TQT_DBusData data;

    data.d->type = TQT_DBusData::Map;
    data.d->keyType = map.keyType();
    data.d->value.pointer = new TQT_DBusDataMap<TQT_DBusUnixFd>(map);

    return data;
}

TQT_DBusDataMap<TQT_DBusUnixFd> TQT_DBusData::toUnixFdKeyMap(bool* ok) const
{
    if (d->type != TQT_DBusData::Map &&
        d->keyType != TQT_DBusDataMap<TQT_DBusUnixFd>::m_keyType)
    {
        if (ok != 0) *ok = false;
        return TQT_DBusDataMap<TQT_DBusUnixFd>();
    }

    if (ok != 0) *ok = true;

    return *((TQT_DBusDataMap<TQT_DBusUnixFd>*)d->value.pointer);
}

static const char* qDBusTypeForTQT_DBusType(TQT_DBusData::Type type)
{
    switch (type)
    {
        case TQT_DBusData::Invalid:
            return 0;
        case TQT_DBusData::Bool:
            return DBUS_TYPE_BOOLEAN_AS_STRING;
        case TQT_DBusData::Byte:
            return DBUS_TYPE_BYTE_AS_STRING;
        case TQT_DBusData::Int16:
            return DBUS_TYPE_INT16_AS_STRING;
        case TQT_DBusData::UInt16:
            return DBUS_TYPE_UINT16_AS_STRING;
        case TQT_DBusData::Int32:
            return DBUS_TYPE_INT32_AS_STRING;
        case TQT_DBusData::UInt32:
            return DBUS_TYPE_UINT32_AS_STRING;
        case TQT_DBusData::Int64:
            return DBUS_TYPE_INT64_AS_STRING;
        case TQT_DBusData::UInt64:
            return DBUS_TYPE_UINT64_AS_STRING;
        case TQT_DBusData::Double:
            return DBUS_TYPE_DOUBLE_AS_STRING;
        case TQT_DBusData::String:
            return DBUS_TYPE_STRING_AS_STRING;
        case TQT_DBusData::ObjectPath:
            return DBUS_TYPE_OBJECT_PATH_AS_STRING;
        case TQT_DBusData::UnixFd:
            return DBUS_TYPE_UNIX_FD_AS_STRING;
        case TQT_DBusData::Variant:
            return DBUS_TYPE_VARIANT_AS_STRING;
        default:
            break;
    }
    return 0;
}

template <typename T>
TQCString qDBusSignatureForMapValue(const TQT_DBusDataMap<T>& map)
{
    if (map.hasContainerValueType())
        return map.containerValueType().buildDBusSignature();
    else
        return qDBusTypeForTQT_DBusType(map.valueType());
}

TQCString TQT_DBusData::buildDBusSignature() const
{
    TQCString signature;

    switch (d->type)
    {
        case TQT_DBusData::List:
        {
            TQT_DBusDataList* list = (TQT_DBusDataList*) d->value.pointer;
            signature = DBUS_TYPE_ARRAY_AS_STRING;
            if (list->hasContainerItemType())
                signature += list->containerItemType().buildDBusSignature();
            else
                signature += qDBusTypeForTQT_DBusType(list->type());
            break;
        }

        case TQT_DBusData::Struct:
        {
            signature += DBUS_STRUCT_BEGIN_CHAR;

            TQValueList<TQT_DBusData>* memberList =
                (TQValueList<TQT_DBusData>*) d->value.pointer;

            TQValueList<TQT_DBusData>::const_iterator it    = (*memberList).begin();
            TQValueList<TQT_DBusData>::const_iterator endIt = (*memberList).end();
            for (; it != endIt; ++it)
            {
                signature += (*it).buildDBusSignature();
            }
            signature += DBUS_STRUCT_END_CHAR;
            break;
        }

        case TQT_DBusData::Map:
            signature += DBUS_TYPE_ARRAY_AS_STRING;
            signature += DBUS_DICT_ENTRY_BEGIN_CHAR;

            signature += qDBusTypeForTQT_DBusType(keyType());

            switch (keyType())
            {
                case TQT_DBusData::Byte:
                    signature += qDBusSignatureForMapValue<TQ_UINT8>(
                        *((TQT_DBusDataMap<TQ_UINT8>*) d->value.pointer));
                    break;
                case TQT_DBusData::Int16:
                    signature += qDBusSignatureForMapValue<TQ_INT16>(
                        *((TQT_DBusDataMap<TQ_INT16>*) d->value.pointer));
                    break;
                case TQT_DBusData::UInt16:
                    signature += qDBusSignatureForMapValue<TQ_UINT16>(
                        *((TQT_DBusDataMap<TQ_UINT16>*) d->value.pointer));
                    break;
                case TQT_DBusData::Int32:
                    signature += qDBusSignatureForMapValue<TQ_INT32>(
                        *((TQT_DBusDataMap<TQ_INT32>*) d->value.pointer));
                    break;
                case TQT_DBusData::UInt32:
                    signature += qDBusSignatureForMapValue<TQ_UINT32>(
                        *((TQT_DBusDataMap<TQ_UINT32>*) d->value.pointer));
                    break;
                case TQT_DBusData::Int64:
                    signature += qDBusSignatureForMapValue<TQ_INT64>(
                        *((TQT_DBusDataMap<TQ_INT64>*) d->value.pointer));
                    break;
                case TQT_DBusData::UInt64:
                    signature += qDBusSignatureForMapValue<TQ_UINT64>(
                        *((TQT_DBusDataMap<TQ_UINT64>*) d->value.pointer));
                    break;
                case TQT_DBusData::String:
                    signature += qDBusSignatureForMapValue<TQString>(
                        *((TQT_DBusDataMap<TQString>*) d->value.pointer));
                    break;
                case TQT_DBusData::ObjectPath:
                    signature += qDBusSignatureForMapValue<TQT_DBusObjectPath>(
                        *((TQT_DBusDataMap<TQT_DBusObjectPath>*) d->value.pointer));
                    break;
                case TQT_DBusData::UnixFd:
                    signature += qDBusSignatureForMapValue<TQT_DBusUnixFd>(
                        *((TQT_DBusDataMap<TQT_DBusUnixFd>*) d->value.pointer));
                    break;
                default:
                    break;
            }

            signature += DBUS_DICT_ENTRY_END_CHAR;
            break;

        default:
            signature = qDBusTypeForTQT_DBusType(d->type);
            break;
    }

    return signature;
}
