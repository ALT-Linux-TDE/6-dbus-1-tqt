/* qdbusdataconverter.cpp TQT_DBusDataConverter template
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

#include "tqdbusdataconverter.h"
#include "tqdbusdata.h"

#include <tqpoint.h>
#include <tqrect.h>
#include <tqsize.h>
#include <tqvaluelist.h>

template <>
TQT_DBusDataConverter::Result TQT_DBusDataConverter::convertFromTQT_DBusData<TQRect>(const TQT_DBusData& dbusData, TQRect& typeData)
{
    if (dbusData.type() != TQT_DBusData::Struct) return InvalidSignature;
    
    TQValueList<TQT_DBusData> members = dbusData.toStruct();
    if (members.count() != 4) return InvalidSignature;
    
    TQ_INT32 values[4];
    
    TQValueList<TQT_DBusData>::const_iterator it    = members.begin();
    TQValueList<TQT_DBusData>::const_iterator endIt = members.end();
    for (uint i = 0; it != endIt; ++it, ++i)
    {
        bool ok = false;
        values[i] = (*it).toInt32(&ok);
        if (!ok) return InvalidSignature;
    }
    
    typeData = TQRect(values[0], values[1], values[2], values[3]);
    
    return Success;
}

template <>
TQT_DBusDataConverter::Result TQT_DBusDataConverter::convertToTQT_DBusData<TQRect>(const TQRect& typeData, TQT_DBusData& dbusData)
{
    TQValueList<TQT_DBusData> members;
    
    members << TQT_DBusData::fromInt32(typeData.x());
    members << TQT_DBusData::fromInt32(typeData.y());
    members << TQT_DBusData::fromInt32(typeData.width());
    members << TQT_DBusData::fromInt32(typeData.height());
    
    dbusData = TQT_DBusData::fromStruct(members);
    
    return Success;
}

template <>
TQT_DBusDataConverter::Result TQT_DBusDataConverter::convertFromTQT_DBusData<TQPoint>(const TQT_DBusData& dbusData, TQPoint& typeData)
{
    if (dbusData.type() != TQT_DBusData::Struct) return InvalidSignature;
    
    TQValueList<TQT_DBusData> members = dbusData.toStruct();
    if (members.count() != 2) return InvalidSignature;
    
    TQ_INT32 values[2];
    
    TQValueList<TQT_DBusData>::const_iterator it    = members.begin();
    TQValueList<TQT_DBusData>::const_iterator endIt = members.end();
    for (uint i = 0; it != endIt; ++it, ++i)
    {
        bool ok = false;
        values[i] = (*it).toInt32(&ok);
        if (!ok) return InvalidSignature;
    }
    
    typeData = TQPoint(values[0], values[1]);
    
    return Success;
}

template <>
TQT_DBusDataConverter::Result TQT_DBusDataConverter::convertToTQT_DBusData<TQPoint>(const TQPoint& typeData, TQT_DBusData& dbusData)
{
    TQValueList<TQT_DBusData> members;
    
    members << TQT_DBusData::fromInt32(typeData.x());
    members << TQT_DBusData::fromInt32(typeData.y());
    
    dbusData = TQT_DBusData::fromStruct(members);
    
    return Success;
}

template <>
TQT_DBusDataConverter::Result TQT_DBusDataConverter::convertFromTQT_DBusData<TQSize>(const TQT_DBusData& dbusData, TQSize& typeData)
{
    if (dbusData.type() != TQT_DBusData::Struct) return InvalidSignature;
    
    TQValueList<TQT_DBusData> members = dbusData.toStruct();
    if (members.count() != 2) return InvalidSignature;
    
    TQ_INT32 values[2];
    
    TQValueList<TQT_DBusData>::const_iterator it    = members.begin();
    TQValueList<TQT_DBusData>::const_iterator endIt = members.end();
    for (uint i = 0; it != endIt; ++it, ++i)
    {
        bool ok = false;
        values[i] = (*it).toInt32(&ok);
        if (!ok) return InvalidSignature;
    }
    
    typeData = TQSize(values[0], values[1]);
    
    return Success;
}

template <>
TQT_DBusDataConverter::Result TQT_DBusDataConverter::convertToTQT_DBusData<TQSize>(const TQSize& typeData, TQT_DBusData& dbusData)
{
    TQValueList<TQT_DBusData> members;
    
    members << TQT_DBusData::fromInt32(typeData.width());
    members << TQT_DBusData::fromInt32(typeData.height());
    
    dbusData = TQT_DBusData::fromStruct(members);
    
    return Success;
}
