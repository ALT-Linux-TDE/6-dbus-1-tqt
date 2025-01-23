/* qdbusdataconverter.h TQT_DBusDataConverter template
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

#ifndef TQDBUSDATACONVERTER_H
#define TQDBUSDATACONVERTER_H

#include "tqdbusmacros.h"

class TQT_DBusData;

/**
 * @brief Template based converter for getting complex data into or from TQT_DBusData objects
 *
 * Any data to transport over D-Bus, i.e. method/signal paramaters or properties, need
 * to be converted into a TQT_DBusData instance.
 * 
 * For complex types, e.g. structures or nested containers, this can be quite some code,
 * and will likely be needed for more than one call.
 * Therefore it is more convenient to implement the conversions once per complex type.
 *
 * Example: sending and recieving a TQRect over D-Bus.
 * In D-Bus terminology a TQRect is a struct of four 32-bit signed integers. The code to do
 * this manually looks like this:
 * @code
 * TQRect rect(0, 0, 100, 100);
 * 
 * TQValueList<TQT_DBusData> structMembers;
 * structMembers << TQT_DBusData::fromInt32(rect.x());
 * structMembers << TQT_DBusData::fromInt32(rect.y());
 * structMembers << TQT_DBusData::fromInt32(rect.wdth());
 * structMembers << TQT_DBusData::fromInt32(rect.height());
 *
 * TQT_DBusData rectStruct = TQT_DBusData::fromStruct(structMembers);
 * @endcode
 * and reverse (without the error checking)
 * @code
 * TQT_DBusData dbusData; // assume we got this from a D-Bus call
 * 
 * TQValueList<TQT_DBusData> structMembers = dbudData.toStruct();
 * 
 * int x = structMembers[0].toInt32();
 * int y = structMembers[1].toInt32();
 * int w = structMembers[2].toInt32();
 * int h = structMembers[3].toInt32();
 *
 * TQRect rect(x, y, w, h);
 * @endcode
 *
 * Rather than implementing it in the method which performs the D-Bus call, basically the same
 * code can be used as a spezialisation of the TQT_DBusDataConverter methods and then used like this:
 * @code
 * TQRect rect(0, 0, 100, 100);
 * TQT_DBusData rectStruct;
 *
 * TQT_DBusDataConverter::convertToTQT_DBusData<TQRect>(rect, rectStruct);
 * @endcode
 * and
 * @code
 * TQRect rect;
 * TQT_DBusData dbusData; // assume we got this from a D-Bus call
 *
 * TQT_DBusDataConverter::convertFromTQT_DBusData<TQRect>(dbusData, rect);
 * @endcode
 *
 * @note The bindings library contains the spezialisations for TQRect, TQPoint and TQSize.
 */
class TQDBUS_EXPORT TQT_DBusDataConverter
{
public:
    /**
     * @brief Conversion result values
     */
    enum Result
    {
        /**
         * Conversion successfull
         */
        Success,
        
        /**
         * Conversion failed because the passed TQT_DBusData instance does not contain data
         * of the needed signature, e.g. too few to too many members for a struct or wrong types.
         *
         * @see TQT_DBusError::stdInvalidSignature()
         */
        InvalidSignature,
        
        /**
         * Conversion failed because the passed TQT_DBusData contained values which are not allowed,
         * e.g. out of range for a numerical type used a an enum or flags.
         *
         * @see TQT_DBusError::stdInvalidArgs()
         */
        InvalidArgument
    };

    /**
     * @brief Conversion from a filled TQT_DBusData instance to a native type
     *
     * For example the implementation for TQPoint looks like this:
     * @code
     * template <>
     * TQT_DBusDataConverter::Result
     * TQT_DBusDataConverter::convertFromTQT_DBusData<TQPoint>(const TQT_DBusData& dbusData, TQPoint& typeData)
     * {
     *     if (dbusData.type() != TQT_DBusData::Struct) return InvalidSignature;
     *     
     *     TQValueList<TQT_DBusData> members = dbusData.toStruct();
     *     if (members.count() != 2) return InvalidSignature;
     *
     *     bool ok = false;
     *     int x = members[0].toInt32(&ok);
     *     if (!ok) return InvalidSignature;
     *
     *     int y = members[1].toInt32(&ok);
     *     if (!ok) return InvalidSignature;
     *
     *     typeData = TQPoint(x, y);
     *
     *     return Success;
     * }
     * @endcode
     *
     * And then can be used like this:
     * @code
     * TQT_DBusMessage reply; // assume we got this as a D-Bus call reply
     * TQPoint point;
     *
     * if (TQT_DBusDataConverter::convertFromTQT_DBusData(reply[0], point) != TQT_DBusDataConverter::Success)
     * {
     *     // error handling
     * }
     * @endcode
     *
     * @param dbusData the binding's data instance to get the content from
     * @param typeData the native type instance to put the content into
     *
     * @return the conversion result value
     */
    template <class T>
    static Result convertFromTQT_DBusData(const TQT_DBusData& dbusData, T& typeData);

    /**
     * @brief Conversion from a native type to a TQT_DBusData instance
     *
     * For example the implementation for TQPoint looks like this:
     * @code
     * template <>
     * TQT_DBusDataConversion::Result
     * TQT_DBusDataConversion::convertToTQT_DBusData<TQPoint>(const TQPoint& typeData, TQT_DBusData& dbusData)
     * {
     *     TQValueList<TQT_DBusData> members;
     *     members << TQT_DBusData::fromInt32(typeData.x());
     *     members << TQT_DBusData::fromInt32(typeData.y());
     *
     *     dbusData = TQT_DBusData::fromStruct(members);
     *
     *     return Success;
     * }
     * @endcode
     *
     * And then can be used like this:
     * @code
     * TQPoint point(-10, 100);
     * TQT_DBusMessage methodCall; // assume created by TQBusMessage::methodCall()
     *
     * TQT_DBusData dbusData;
     * if (TQT_DBusDataConverter::convertToTQT_DBusData<TQPoint>(point, dbusData) != TQT_DBusDataConverter::Success)
     * {
     *     // error handling
     * }
     * else
     * {
     *     methodCall << dbusData;
     * }
     * @endcode
     *
     * @param typeData the native type instance to get the content from
     * @param dbusData the binding's data instance to put the content into
     *
     * @return the conversion result value
     */
    template <class T>
    static Result convertToTQT_DBusData(const T& typeData, TQT_DBusData& dbusData);
};

#endif
