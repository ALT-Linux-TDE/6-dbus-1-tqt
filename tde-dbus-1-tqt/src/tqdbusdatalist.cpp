/* qdbusdatalist.cpp list of DBUS data transport type
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

#include "tqdbusdatalist.h"
#include "tqdbusobjectpath.h"
#include "tqdbusunixfd.h"
#include "tqdbusvariant.h"

#include <tqstringlist.h>

class TQT_DBusDataList::Private
{
public:
    Private() : type(TQT_DBusData::Invalid) {}

public:
    TQT_DBusData::Type type;
    TQT_DBusData containerItem;
    TQValueList<TQT_DBusData> list;
};

TQT_DBusDataList::TQT_DBusDataList() : d(new Private())
{
}

TQT_DBusDataList::TQT_DBusDataList(TQT_DBusData::Type simpleItemType) : d(new Private())
{
    d->type = simpleItemType;
}

TQT_DBusDataList::TQT_DBusDataList(const TQT_DBusData& containerItemType) : d(new Private())
{
    d->type = containerItemType.type();

    switch(d->type)
    {
        case TQT_DBusData::List:   // fall through
        case TQT_DBusData::Struct: // fall through
        case TQT_DBusData::Map:
            d->containerItem = containerItemType;
            break;

        default:   // not a container
            break;
    }
}

TQT_DBusDataList::TQT_DBusDataList(const TQT_DBusDataList& other) : d(new Private())
{
    d->type = other.d->type;
    d->list = other.d->list;
    d->containerItem = other.d->containerItem;
}

TQT_DBusDataList::TQT_DBusDataList(const TQValueList<TQT_DBusData>& other) : d(new Private())
{
    if (other.isEmpty()) return;

    TQValueList<TQT_DBusData>::const_iterator it    = other.begin();
    TQValueList<TQT_DBusData>::const_iterator endIt = other.end();

    d->type = (*it).type();

    TQCString elementSignature;
    if (hasContainerItemType())
    {
        d->containerItem = other[0]; // would be nice to get an empty one
        elementSignature = d->containerItem.buildDBusSignature();
    }

    for (++it; it != endIt; ++it)
    {
        if (d->type != (*it).type())
        {
            d->type = TQT_DBusData::Invalid;
            d->containerItem = TQT_DBusData();

            return;
        }
        else if (hasContainerItemType())
        {
            if ((*it).buildDBusSignature() != elementSignature)
            {
                d->type = TQT_DBusData::Invalid;
                d->containerItem = TQT_DBusData();

                return;
            }
        }
    }

    d->list = other;
}

TQT_DBusDataList::TQT_DBusDataList(const TQValueList<bool>& other) : d(new Private())
{
    d->type = TQT_DBusData::Bool;

    if (other.isEmpty()) return;

    TQValueList<bool>::const_iterator it    = other.begin();
    TQValueList<bool>::const_iterator endIt = other.end();
    for (; it != endIt; ++it)
    {
        d->list << TQT_DBusData::fromBool(*it);
    }
}

TQT_DBusDataList::TQT_DBusDataList(const TQValueList<TQ_UINT8>& other) : d(new Private())
{
    d->type = TQT_DBusData::Byte;

    if (other.isEmpty()) return;

    TQValueList<TQ_UINT8>::const_iterator it    = other.begin();
    TQValueList<TQ_UINT8>::const_iterator endIt = other.end();
    for (; it != endIt; ++it)
    {
        d->list << TQT_DBusData::fromByte(*it);
    }
}

TQT_DBusDataList::TQT_DBusDataList(const TQValueList<TQ_INT16>& other) : d(new Private())
{
    d->type = TQT_DBusData::Int16;

    if (other.isEmpty()) return;

    TQValueList<TQ_INT16>::const_iterator it    = other.begin();
    TQValueList<TQ_INT16>::const_iterator endIt = other.end();
    for (; it != endIt; ++it)
    {
        d->list << TQT_DBusData::fromInt16(*it);
    }
}

TQT_DBusDataList::TQT_DBusDataList(const TQValueList<TQ_UINT16>& other) : d(new Private())
{
    d->type = TQT_DBusData::UInt16;

    if (other.isEmpty()) return;

    TQValueList<TQ_UINT16>::const_iterator it    = other.begin();
    TQValueList<TQ_UINT16>::const_iterator endIt = other.end();
    for (; it != endIt; ++it)
    {
        d->list << TQT_DBusData::fromUInt16(*it);
    }
}

TQT_DBusDataList::TQT_DBusDataList(const TQValueList<TQ_INT32>& other) : d(new Private())
{
    d->type = TQT_DBusData::Int32;

    if (other.isEmpty()) return;

    TQValueList<TQ_INT32>::const_iterator it    = other.begin();
    TQValueList<TQ_INT32>::const_iterator endIt = other.end();
    for (; it != endIt; ++it)
    {
        d->list << TQT_DBusData::fromInt32(*it);
    }
}

TQT_DBusDataList::TQT_DBusDataList(const TQValueList<TQ_UINT32>& other) : d(new Private())
{
    d->type = TQT_DBusData::UInt32;

    if (other.isEmpty()) return;

    TQValueList<TQ_UINT32>::const_iterator it    = other.begin();
    TQValueList<TQ_UINT32>::const_iterator endIt = other.end();
    for (; it != endIt; ++it)
    {
        d->list << TQT_DBusData::fromUInt32(*it);
    }
}

TQT_DBusDataList::TQT_DBusDataList(const TQValueList<TQ_INT64>& other) : d(new Private())
{
    d->type = TQT_DBusData::Int64;

    if (other.isEmpty()) return;

    TQValueList<TQ_INT64>::const_iterator it    = other.begin();
    TQValueList<TQ_INT64>::const_iterator endIt = other.end();
    for (; it != endIt; ++it)
    {
        d->list << TQT_DBusData::fromInt64(*it);
    }
}

TQT_DBusDataList::TQT_DBusDataList(const TQValueList<TQ_UINT64>& other) : d(new Private())
{
    d->type = TQT_DBusData::UInt64;

    if (other.isEmpty()) return;

    TQValueList<TQ_UINT64>::const_iterator it    = other.begin();
    TQValueList<TQ_UINT64>::const_iterator endIt = other.end();
    for (; it != endIt; ++it)
    {
        d->list << TQT_DBusData::fromUInt64(*it);
    }
}

TQT_DBusDataList::TQT_DBusDataList(const TQValueList<double>& other) : d(new Private())
{
    d->type = TQT_DBusData::Double;

    if (other.isEmpty()) return;

    TQValueList<double>::const_iterator it    = other.begin();
    TQValueList<double>::const_iterator endIt = other.end();
    for (; it != endIt; ++it)
    {
        d->list << TQT_DBusData::fromDouble(*it);
    }
}

TQT_DBusDataList::TQT_DBusDataList(const TQValueList<TQT_DBusVariant>& other)
    : d(new Private())
{
    d->type = TQT_DBusData::Variant;

    if (other.isEmpty()) return;

    TQValueList<TQT_DBusVariant>::const_iterator it    = other.begin();
    TQValueList<TQT_DBusVariant>::const_iterator endIt = other.end();
    for (; it != endIt; ++it)
    {
        d->list << TQT_DBusData::fromVariant(*it);
    }
}

TQT_DBusDataList::TQT_DBusDataList(const TQStringList& other) : d(new Private())
{
    d->type = TQT_DBusData::String;

    if (other.isEmpty()) return;

    TQStringList::const_iterator it    = other.begin();
    TQStringList::const_iterator endIt = other.end();
    for (; it != endIt; ++it)
    {
        d->list << TQT_DBusData::fromString(*it);
    }
}

TQT_DBusDataList::TQT_DBusDataList(const TQValueList<TQT_DBusObjectPath>& other)
    : d(new Private())
{
    d->type = TQT_DBusData::ObjectPath;

    if (other.isEmpty()) return;

    TQValueList<TQT_DBusObjectPath>::const_iterator it    = other.begin();
    TQValueList<TQT_DBusObjectPath>::const_iterator endIt = other.end();
    for (; it != endIt; ++it)
    {
        d->list << TQT_DBusData::fromObjectPath(*it);
    }
}

TQT_DBusDataList::TQT_DBusDataList(const TQValueList<TQT_DBusUnixFd>& other)
    : d(new Private())
{
    d->type = TQT_DBusData::UnixFd;

    if (other.isEmpty()) return;

    TQValueList<TQT_DBusUnixFd>::const_iterator it    = other.begin();
    TQValueList<TQT_DBusUnixFd>::const_iterator endIt = other.end();
    for (; it != endIt; ++it)
    {
        d->list << TQT_DBusData::fromUnixFd(*it);
    }
}

TQT_DBusDataList::~TQT_DBusDataList()
{
    delete d;
}

TQT_DBusDataList& TQT_DBusDataList::operator=(const TQT_DBusDataList& other)
{
    if (&other == this) return *this;

    d->type = other.d->type;
    d->list = other.d->list;
    d->containerItem = other.d->containerItem;

    return *this;
}

TQT_DBusDataList& TQT_DBusDataList::operator=(const TQValueList<TQT_DBusData>& other)
{
    d->list.clear();
    d->type = TQT_DBusData::Invalid;
    d->containerItem = TQT_DBusData();

    if (other.isEmpty()) return *this;

    TQValueList<TQT_DBusData>::const_iterator it    = other.begin();
    TQValueList<TQT_DBusData>::const_iterator endIt = other.end();

    d->type = (*it).type();

    TQCString elementSignature;
    if (hasContainerItemType())
    {
        d->containerItem = other[0]; // would be nice to get an empty one

        elementSignature = d->containerItem.buildDBusSignature();
    }

    for (++it; it != endIt; ++it)
    {
        if (d->type != (*it).type())
        {
            d->type = TQT_DBusData::Invalid;
            d->containerItem = TQT_DBusData();

            return *this;
        }
        else if (hasContainerItemType())
        {
            if ((*it).buildDBusSignature() != elementSignature)
            {
                d->type = TQT_DBusData::Invalid;
                d->containerItem = TQT_DBusData();

                return *this;
            }
        }
    }

    d->list = other;

    return *this;
}

TQT_DBusDataList& TQT_DBusDataList::operator=(const TQStringList& other)
{
    d->list.clear();
    d->type = TQT_DBusData::String;
    d->containerItem = TQT_DBusData();

    TQStringList::const_iterator it    = other.begin();
    TQStringList::const_iterator endIt = other.end();
    for (; it != endIt; ++it)
    {
        d->list << TQT_DBusData::fromString(*it);
    }

    return *this;
}

TQT_DBusData::Type TQT_DBusDataList::type() const
{
    return d->type;
}

bool TQT_DBusDataList::hasContainerItemType() const
{
    return d->type == TQT_DBusData::List || d->type == TQT_DBusData::Map
                                      || d->type == TQT_DBusData::Struct;
}

TQT_DBusData TQT_DBusDataList::containerItemType() const
{
    return d->containerItem;
}

bool TQT_DBusDataList::isEmpty() const
{
    return d->list.isEmpty();
}

uint TQT_DBusDataList::count() const
{
    return d->list.count();
}

bool TQT_DBusDataList::operator==(const TQT_DBusDataList& other) const
{
    if (&other == this) return true;
    if (d == other.d) return true;

    bool containerEqual = true;
    if (hasContainerItemType())
    {
        if (other.hasContainerItemType())
        {
            containerEqual = d->containerItem.buildDBusSignature() ==
                             other.d->containerItem.buildDBusSignature();
        }
        else
            containerEqual = false;
    }
    else if (other.hasContainerItemType())
        containerEqual = false;

    return d->type == other.d->type && containerEqual && d->list == other.d->list;
}

bool TQT_DBusDataList::operator!=(const TQT_DBusDataList& other) const
{
    if (&other == this) return false;
    if (d == other.d) return false;

    bool containerEqual = true;
    if (hasContainerItemType())
    {
        if (other.hasContainerItemType())
        {
            containerEqual = d->containerItem.buildDBusSignature() ==
                             other.d->containerItem.buildDBusSignature();
        }
        else
            containerEqual = false;
    }
    else if (other.hasContainerItemType())
        containerEqual = false;

    return d->type != other.d->type || !containerEqual || d->list != other.d->list;
}

void TQT_DBusDataList::clear()
{
    d->list.clear();
}

TQT_DBusDataList& TQT_DBusDataList::operator<<(const TQT_DBusData& data)
{
    if (data.type() == TQT_DBusData::Invalid) return *this;

    if (d->type == TQT_DBusData::Invalid)
    {
        d->type = data.type();

        // check if we are now have container items
        if (hasContainerItemType()) d->containerItem = data;

        d->list << data;
    }
    else if (d->type != data.type())
    {
        tqWarning("TQT_DBusDataList: trying to add data of type %s to list of type %s",
                 data.typeName(), TQT_DBusData::typeName(d->type));
    }
    else if (hasContainerItemType())
    {
        TQCString ourSignature  = d->containerItem.buildDBusSignature();
        TQCString dataSignature = data.buildDBusSignature();

        if (ourSignature != dataSignature)
        {
            tqWarning("TQT_DBusDataList: trying to add data with signature %s "
                     "to list with item signature %s",
                     dataSignature.data(), ourSignature.data());
        }
        else
            d->list << data;
    }
    else
        d->list << data;

    return *this;
}

TQValueList<TQT_DBusData> TQT_DBusDataList::toTQValueList() const
{
    return d->list;
}

TQStringList TQT_DBusDataList::toTQStringList(bool* ok) const
{
    if (d->type != TQT_DBusData::String)
    {
        if (ok != 0) *ok = false;
        return TQStringList();
    }

    TQStringList result;

    TQValueList<TQT_DBusData>::const_iterator it    = d->list.begin();
    TQValueList<TQT_DBusData>::const_iterator endIt = d->list.end();
    for (; it != endIt; ++it)
    {
        result << (*it).toString();
    }

    if (ok != 0) *ok = true;

    return result;
}

TQValueList<bool> TQT_DBusDataList::toBoolList(bool* ok) const
{
    if (d->type != TQT_DBusData::Bool)
    {
        if (ok != 0) *ok = false;
        return TQValueList<bool>();
    }

    TQValueList<bool> result;

    TQValueList<TQT_DBusData>::const_iterator it    = d->list.begin();
    TQValueList<TQT_DBusData>::const_iterator endIt = d->list.end();
    for (; it != endIt; ++it)
    {
        result << (*it).toBool();
    }

    if (ok != 0) *ok = true;

    return result;
}

TQValueList<TQ_UINT8> TQT_DBusDataList::toByteList(bool* ok) const
{
    if (d->type != TQT_DBusData::Byte)
    {
        if (ok != 0) *ok = false;
        return TQValueList<TQ_UINT8>();
    }

    TQValueList<TQ_UINT8> result;

    TQValueList<TQT_DBusData>::const_iterator it    = d->list.begin();
    TQValueList<TQT_DBusData>::const_iterator endIt = d->list.end();
    for (; it != endIt; ++it)
    {
        result << (*it).toByte();
    }

    if (ok != 0) *ok = true;

    return result;
}

TQValueList<TQ_INT16> TQT_DBusDataList::toInt16List(bool* ok) const
{
    if (d->type != TQT_DBusData::Int16)
    {
        if (ok != 0) *ok = false;
        return TQValueList<TQ_INT16>();
    }

    TQValueList<TQ_INT16> result;

    TQValueList<TQT_DBusData>::const_iterator it    = d->list.begin();
    TQValueList<TQT_DBusData>::const_iterator endIt = d->list.end();
    for (; it != endIt; ++it)
    {
        result << (*it).toInt16();
    }

    if (ok != 0) *ok = true;

    return result;
}

TQValueList<TQ_UINT16> TQT_DBusDataList::toUInt16List(bool* ok) const
{
    if (d->type != TQT_DBusData::UInt16)
    {
        if (ok != 0) *ok = false;
        return TQValueList<TQ_UINT16>();
    }

    TQValueList<TQ_UINT16> result;

    TQValueList<TQT_DBusData>::const_iterator it    = d->list.begin();
    TQValueList<TQT_DBusData>::const_iterator endIt = d->list.end();
    for (; it != endIt; ++it)
    {
        result << (*it).toUInt16();
    }

    if (ok != 0) *ok = true;

    return result;
}

TQValueList<TQ_INT32> TQT_DBusDataList::toInt32List(bool* ok) const
{
    if (d->type != TQT_DBusData::Int32)
    {
        if (ok != 0) *ok = false;
        return TQValueList<TQ_INT32>();
    }

    TQValueList<TQ_INT32> result;

    TQValueList<TQT_DBusData>::const_iterator it    = d->list.begin();
    TQValueList<TQT_DBusData>::const_iterator endIt = d->list.end();
    for (; it != endIt; ++it)
    {
        result << (*it).toInt32();
    }

    if (ok != 0) *ok = true;

    return result;
}

TQValueList<TQ_UINT32> TQT_DBusDataList::toUInt32List(bool* ok) const
{
    if (d->type != TQT_DBusData::UInt32)
    {
        if (ok != 0) *ok = false;
        return TQValueList<TQ_UINT32>();
    }

    TQValueList<TQ_UINT32> result;

    TQValueList<TQT_DBusData>::const_iterator it    = d->list.begin();
    TQValueList<TQT_DBusData>::const_iterator endIt = d->list.end();
    for (; it != endIt; ++it)
    {
        result << (*it).toUInt32();
    }

    if (ok != 0) *ok = true;

    return result;
}

TQValueList<TQ_INT64> TQT_DBusDataList::toInt64List(bool* ok) const
{
    if (d->type != TQT_DBusData::Int64)
    {
        if (ok != 0) *ok = false;
        return TQValueList<TQ_INT64>();
    }

    TQValueList<TQ_INT64> result;

    TQValueList<TQT_DBusData>::const_iterator it    = d->list.begin();
    TQValueList<TQT_DBusData>::const_iterator endIt = d->list.end();
    for (; it != endIt; ++it)
    {
        result << (*it).toInt64();
    }

    if (ok != 0) *ok = true;

    return result;
}

TQValueList<TQ_UINT64> TQT_DBusDataList::toUInt64List(bool* ok) const
{
    if (d->type != TQT_DBusData::UInt64)
    {
        if (ok != 0) *ok = false;
        return TQValueList<TQ_UINT64>();
    }

    TQValueList<TQ_UINT64> result;

    TQValueList<TQT_DBusData>::const_iterator it    = d->list.begin();
    TQValueList<TQT_DBusData>::const_iterator endIt = d->list.end();
    for (; it != endIt; ++it)
    {
        result << (*it).toUInt64();
    }

    if (ok != 0) *ok = true;

    return result;
}

TQValueList<double> TQT_DBusDataList::toDoubleList(bool* ok) const
{
    if (d->type != TQT_DBusData::Double)
    {
        if (ok != 0) *ok = false;
        return TQValueList<double>();
    }

    TQValueList<double> result;

    TQValueList<TQT_DBusData>::const_iterator it    = d->list.begin();
    TQValueList<TQT_DBusData>::const_iterator endIt = d->list.end();
    for (; it != endIt; ++it)
    {
        result << (*it).toDouble();
    }

    if (ok != 0) *ok = true;

    return result;
}

TQValueList<TQString> TQT_DBusDataList::toStringList(bool* ok) const
{
    return toTQStringList(ok);
}

TQValueList<TQT_DBusObjectPath> TQT_DBusDataList::toObjectPathList(bool* ok) const
{
    if (d->type != TQT_DBusData::ObjectPath)
    {
        if (ok != 0) *ok = false;
        return TQValueList<TQT_DBusObjectPath>();
    }

    TQValueList<TQT_DBusObjectPath> result;

    TQValueList<TQT_DBusData>::const_iterator it    = d->list.begin();
    TQValueList<TQT_DBusData>::const_iterator endIt = d->list.end();
    for (; it != endIt; ++it)
    {
        result << (*it).toObjectPath();
    }

    if (ok != 0) *ok = true;

    return result;
}

TQValueList<TQT_DBusUnixFd> TQT_DBusDataList::toUnixFdList(bool* ok) const
{
    if (d->type != TQT_DBusData::UnixFd)
    {
        if (ok != 0) *ok = false;
        return TQValueList<TQT_DBusUnixFd>();
    }

    TQValueList<TQT_DBusUnixFd> result;

    TQValueList<TQT_DBusData>::const_iterator it    = d->list.begin();
    TQValueList<TQT_DBusData>::const_iterator endIt = d->list.end();
    for (; it != endIt; ++it)
    {
        result << (*it).toUnixFd();
    }

    if (ok != 0) *ok = true;

    return result;
}

TQValueList<TQT_DBusVariant> TQT_DBusDataList::toVariantList(bool* ok) const
{
    if (d->type != TQT_DBusData::Variant)
    {
        if (ok != 0) *ok = false;
        return TQValueList<TQT_DBusVariant>();
    }

    TQValueList<TQT_DBusVariant> result;

    TQValueList<TQT_DBusData>::const_iterator it    = d->list.begin();
    TQValueList<TQT_DBusData>::const_iterator endIt = d->list.end();
    for (; it != endIt; ++it)
    {
        result << (*it).toVariant();
    }

    if (ok != 0) *ok = true;

    return result;
}
