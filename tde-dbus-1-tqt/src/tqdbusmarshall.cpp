/* qdbusmarshall.cpp
 *
 * Copyright (C) 2005 Harald Fernengel <harry@kdevelop.org>
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

#include "tqdbusmarshall.h"
#include "tqdbusdata.h"
#include "tqdbusdatalist.h"
#include "tqdbusdatamap.h"
#include "tqdbusobjectpath.h"
#include "tqdbusunixfd.h"
#include "tqdbusvariant.h"

#include <tqvariant.h>
#include <tqvaluelist.h>
#include <tqmap.h>
#include <tqstringlist.h>
#include <tqvaluevector.h>

#include <dbus/dbus.h>

template <typename T>
inline T qIterGet(DBusMessageIter *it)
{
    T t;
    dbus_message_iter_get_basic(it, &t);
    return t;
}

static TQT_DBusData::Type qSingleTypeForDBusSignature(char signature)
{
    switch (signature)
    {
        case 'b': return TQT_DBusData::Bool;
        case 'y': return TQT_DBusData::Byte;
        case 'n': return TQT_DBusData::Int16;
        case 'q': return TQT_DBusData::UInt16;
        case 'i': return TQT_DBusData::Int32;
        case 'u': return TQT_DBusData::UInt32;
        case 'x': return TQT_DBusData::Int64;
        case 't': return TQT_DBusData::UInt64;
        case 'd': return TQT_DBusData::Double;
        case 's': return TQT_DBusData::String;
        case 'o': return TQT_DBusData::ObjectPath;
        case 'g': return TQT_DBusData::String;
        case 'v': return TQT_DBusData::Variant;
        case 'h': return TQT_DBusData::UnixFd;

        default:
            break;
    }

    return TQT_DBusData::Invalid;
}

static TQValueList<TQT_DBusData> parseSignature(TQCString& signature)
{
//    tqDebug("parseSignature(%s)", signature.data());
    TQValueList<TQT_DBusData> result;

    while (!signature.isEmpty())
    {
        switch (signature[0])
        {
            case '(': {
                signature = signature.mid(1);
                TQValueList<TQT_DBusData> memberList = parseSignature(signature);
                result << TQT_DBusData::fromStruct(memberList);
                Q_ASSERT(!signature.isEmpty() && signature[0] == ')');
                signature = signature.mid(1);
                break;
            }
            case ')': return result;
            case '{': {
                TQT_DBusData::Type keyType =
                    qSingleTypeForDBusSignature(signature[1]);
                TQT_DBusData::Type valueType =
                    qSingleTypeForDBusSignature(signature[2]);
                if (valueType != TQT_DBusData::Invalid)
                {
                    switch (keyType)
                    {
                        case TQT_DBusData::Byte:
                            result << TQT_DBusData::fromByteKeyMap(
                                TQT_DBusDataMap<TQ_UINT8>(valueType));
                            break;
                        case TQT_DBusData::Int16:
                            result << TQT_DBusData::fromInt16KeyMap(
                                TQT_DBusDataMap<TQ_INT16>(valueType));
                            break;
                        case TQT_DBusData::UInt16:
                            result << TQT_DBusData::fromUInt16KeyMap(
                                TQT_DBusDataMap<TQ_UINT16>(valueType));
                            break;
                        case TQT_DBusData::Int32:
                            result << TQT_DBusData::fromInt32KeyMap(
                                TQT_DBusDataMap<TQ_INT32>(valueType));
                            break;
                        case TQT_DBusData::UInt32:
                            result << TQT_DBusData::fromUInt32KeyMap(
                                TQT_DBusDataMap<TQ_UINT32>(valueType));
                            break;
                        case TQT_DBusData::Int64:
                            result << TQT_DBusData::fromInt64KeyMap(
                                TQT_DBusDataMap<TQ_INT64>(valueType));
                            break;
                        case TQT_DBusData::UInt64:
                            result << TQT_DBusData::fromUInt64KeyMap(
                                TQT_DBusDataMap<TQ_UINT64>(valueType));
                            break;
                        case TQT_DBusData::String:
                            result << TQT_DBusData::fromStringKeyMap(
                                TQT_DBusDataMap<TQString>(valueType));
                            break;
                        case TQT_DBusData::ObjectPath:
                            result << TQT_DBusData::fromObjectPathKeyMap(
                                TQT_DBusDataMap<TQT_DBusObjectPath>(valueType));
                            break;
                        case TQT_DBusData::UnixFd:
                            result << TQT_DBusData::fromUnixFdKeyMap(
                                TQT_DBusDataMap<TQT_DBusUnixFd>(valueType));
                            break;
                        default:
                            tqWarning("TQT_DBusMarshall: unsupported map key type %s "
                                     "at de-marshalling",
                                     TQT_DBusData::typeName(keyType));
                            break;
                    }
                    signature = signature.mid(3);
                }
                else
                {
                    signature = signature.mid(2);
                    TQValueList<TQT_DBusData> valueContainer =
                        parseSignature(signature);
                    Q_ASSERT(valueContainer.count() == 1);

                    switch (keyType)
                    {
                        case TQT_DBusData::Byte:
                            result << TQT_DBusData::fromByteKeyMap(
                                TQT_DBusDataMap<TQ_UINT8>(valueContainer[0]));
                            break;
                        case TQT_DBusData::Int16:
                            result << TQT_DBusData::fromInt16KeyMap(
                                TQT_DBusDataMap<TQ_INT16>(valueContainer[0]));
                            break;
                        case TQT_DBusData::UInt16:
                            result << TQT_DBusData::fromUInt16KeyMap(
                                TQT_DBusDataMap<TQ_UINT16>(valueContainer[0]));
                            break;
                        case TQT_DBusData::Int32:
                            result << TQT_DBusData::fromInt32KeyMap(
                                TQT_DBusDataMap<TQ_INT32>(valueContainer[0]));
                            break;
                        case TQT_DBusData::UInt32:
                            result << TQT_DBusData::fromUInt32KeyMap(
                                TQT_DBusDataMap<TQ_UINT32>(valueContainer[0]));
                            break;
                        case TQT_DBusData::Int64:
                            result << TQT_DBusData::fromInt64KeyMap(
                                TQT_DBusDataMap<TQ_INT64>(valueContainer[0]));
                            break;
                        case TQT_DBusData::UInt64:
                            result << TQT_DBusData::fromUInt64KeyMap(
                                TQT_DBusDataMap<TQ_UINT64>(valueContainer[0]));
                            break;
                        case TQT_DBusData::String:
                            result << TQT_DBusData::fromStringKeyMap(
                                TQT_DBusDataMap<TQString>(valueContainer[0]));
                            break;
                        case TQT_DBusData::ObjectPath:
                            result << TQT_DBusData::fromObjectPathKeyMap(
                                TQT_DBusDataMap<TQT_DBusObjectPath>(valueContainer[0]));
                            break;
                        case TQT_DBusData::UnixFd:
                            result << TQT_DBusData::fromUnixFdKeyMap(
                                TQT_DBusDataMap<TQT_DBusUnixFd>(valueContainer[0]));
                            break;
                        default:
                            tqWarning("TQT_DBusMarshall: unsupported map key type %s "
                                     "at de-marshalling",
                                     TQT_DBusData::typeName(keyType));
                            break;
                    }
                }
                Q_ASSERT(!signature.isEmpty() && signature[0] == '}');
                signature = signature.mid(1);
                break;
            }
            case '}': return result;
            case 'a': {
                TQT_DBusData::Type elementType =
                    qSingleTypeForDBusSignature(signature[1]);
                if (elementType != TQT_DBusData::Invalid)
                {
                    TQT_DBusDataList list(elementType);
                    result << TQT_DBusData::fromList(list);
                    signature = signature.mid(2);
                }
                else
                {
                    signature = signature.mid(1);
                    bool array = signature[0] != '{';

                    TQValueList<TQT_DBusData> elementContainer =
                        parseSignature(signature);
                    Q_ASSERT(elementContainer.count() == 1);

                    if (array)
                    {
                        TQT_DBusDataList list(elementContainer[0]);
                        result << TQT_DBusData::fromList(list);
                    }
                    else
                        result << elementContainer[0];
                }
                break;
            }
            default:
                TQT_DBusData::Type elementType =
                    qSingleTypeForDBusSignature(signature[0]);
                if (elementType != TQT_DBusData::Invalid)
                {
                    switch (elementType)
                    {
                        case TQT_DBusData::Bool:
                            result << TQT_DBusData::fromBool(
                                (0));
                            break;
                        case TQT_DBusData::Byte:
                            result << TQT_DBusData::fromByte(
                                (0));
                            break;
                        case TQT_DBusData::Int16:
                            result << TQT_DBusData::fromInt16(
                                (0));
                            break;
                        case TQT_DBusData::UInt16:
                            result << TQT_DBusData::fromUInt16(
                                (0));
                            break;
                        case TQT_DBusData::Int32:
                            result << TQT_DBusData::fromInt32(
                                (0));
                            break;
                        case TQT_DBusData::UInt32:
                            result << TQT_DBusData::fromUInt32(
                                (0));
                            break;
                        case TQT_DBusData::Int64:
                            result << TQT_DBusData::fromInt64(
                                (0));
                            break;
                        case TQT_DBusData::UInt64:
                            result << TQT_DBusData::fromUInt64(
                                (0));
                            break;
                        case TQT_DBusData::String:
                            result << TQT_DBusData::fromString(
                                (TQString()));
                            break;
                        case TQT_DBusData::ObjectPath:
                            result << TQT_DBusData::fromObjectPath(
                                (TQT_DBusObjectPath()));
                            break;
                        case TQT_DBusData::UnixFd:
                            result << TQT_DBusData::fromUnixFd(
                                (TQT_DBusUnixFd()));
                            break;
                        default:
                            result << TQT_DBusData();
                            tqWarning("TQT_DBusMarshall: unsupported element type %s "
                                     "at de-marshalling",
                                     TQT_DBusData::typeName(elementType));
                            break;
                    }
                    signature = signature.mid(1);
                }
                else {
                    result << TQT_DBusData();
                    signature = signature.mid(1);
                }
                break;
        }
    }

    return result;
}

static TQT_DBusData qFetchParameter(DBusMessageIter *it);

void qFetchByteKeyMapEntry(TQT_DBusDataMap<TQ_UINT8>& map, DBusMessageIter* it)
{
    DBusMessageIter itemIter;
    dbus_message_iter_recurse(it, &itemIter);
    Q_ASSERT(dbus_message_iter_has_next(&itemIter));

    TQ_UINT8 key = qFetchParameter(&itemIter).toByte();

    dbus_message_iter_next(&itemIter);

    map.insert(key, qFetchParameter(&itemIter));
}

void qFetchInt16KeyMapEntry(TQT_DBusDataMap<TQ_INT16>& map, DBusMessageIter* it)
{
    DBusMessageIter itemIter;
    dbus_message_iter_recurse(it, &itemIter);
    Q_ASSERT(dbus_message_iter_has_next(&itemIter));

    TQ_INT16 key = qFetchParameter(&itemIter).toInt16();

    dbus_message_iter_next(&itemIter);

    map.insert(key, qFetchParameter(&itemIter));
}

void qFetchUInt16KeyMapEntry(TQT_DBusDataMap<TQ_UINT16>& map, DBusMessageIter* it)
{
    DBusMessageIter itemIter;
    dbus_message_iter_recurse(it, &itemIter);
    Q_ASSERT(dbus_message_iter_has_next(&itemIter));

    TQ_UINT16 key = qFetchParameter(&itemIter).toUInt16();

    dbus_message_iter_next(&itemIter);

    map.insert(key, qFetchParameter(&itemIter));
}

void qFetchInt32KeyMapEntry(TQT_DBusDataMap<TQ_INT32>& map, DBusMessageIter* it)
{
    DBusMessageIter itemIter;
    dbus_message_iter_recurse(it, &itemIter);
    Q_ASSERT(dbus_message_iter_has_next(&itemIter));

    TQ_INT32 key = qFetchParameter(&itemIter).toInt32();

    dbus_message_iter_next(&itemIter);

    map.insert(key, qFetchParameter(&itemIter));
}

void qFetchUInt32KeyMapEntry(TQT_DBusDataMap<TQ_UINT32>& map, DBusMessageIter* it)
{
    DBusMessageIter itemIter;
    dbus_message_iter_recurse(it, &itemIter);
    Q_ASSERT(dbus_message_iter_has_next(&itemIter));

    TQ_UINT32 key = qFetchParameter(&itemIter).toUInt32();

    dbus_message_iter_next(&itemIter);

    map.insert(key, qFetchParameter(&itemIter));
}

void qFetchInt64KeyMapEntry(TQT_DBusDataMap<TQ_INT64>& map, DBusMessageIter* it)
{
    DBusMessageIter itemIter;
    dbus_message_iter_recurse(it, &itemIter);
    Q_ASSERT(dbus_message_iter_has_next(&itemIter));

    TQ_INT64 key = qFetchParameter(&itemIter).toInt64();

    dbus_message_iter_next(&itemIter);

    map.insert(key, qFetchParameter(&itemIter));
}

void qFetchUInt64KeyMapEntry(TQT_DBusDataMap<TQ_UINT64>& map, DBusMessageIter* it)
{
    DBusMessageIter itemIter;
    dbus_message_iter_recurse(it, &itemIter);
    Q_ASSERT(dbus_message_iter_has_next(&itemIter));

    TQ_UINT64 key = qFetchParameter(&itemIter).toUInt64();

    dbus_message_iter_next(&itemIter);

    map.insert(key, qFetchParameter(&itemIter));
}

void qFetchObjectPathKeyMapEntry(TQT_DBusDataMap<TQT_DBusObjectPath>& map, DBusMessageIter* it)
{
    DBusMessageIter itemIter;
    dbus_message_iter_recurse(it, &itemIter);
    Q_ASSERT(dbus_message_iter_has_next(&itemIter));

    TQT_DBusObjectPath key = qFetchParameter(&itemIter).toObjectPath();

    dbus_message_iter_next(&itemIter);

    map.insert(key, qFetchParameter(&itemIter));
}

void qFetchStringKeyMapEntry(TQT_DBusDataMap<TQString>& map, DBusMessageIter* it)
{
    DBusMessageIter itemIter;
    dbus_message_iter_recurse(it, &itemIter);
    Q_ASSERT(dbus_message_iter_has_next(&itemIter));

    TQString key = qFetchParameter(&itemIter).toString();

    dbus_message_iter_next(&itemIter);

    map.insert(key, qFetchParameter(&itemIter));
}

static TQT_DBusData qFetchMap(DBusMessageIter *it, const TQT_DBusData& prototype)
{
    if (dbus_message_iter_get_arg_type(it) == DBUS_TYPE_INVALID)
        return prototype;

    DBusMessageIter itemIter;
    dbus_message_iter_recurse(it, &itemIter);
    if (dbus_message_iter_get_arg_type(&itemIter) == DBUS_TYPE_INVALID)
        return prototype;

    switch (dbus_message_iter_get_arg_type(&itemIter)) {
        case DBUS_TYPE_BYTE: {
            TQT_DBusDataMap<TQ_UINT8> map = prototype.toByteKeyMap();
            do {
                qFetchByteKeyMapEntry(map, it);
            } while (dbus_message_iter_next(it));

            return TQT_DBusData::fromByteKeyMap(map);
        }

        case DBUS_TYPE_INT16: {
            TQT_DBusDataMap<TQ_INT16> map = prototype.toInt16KeyMap();
            do {
                qFetchInt16KeyMapEntry(map, it);
            } while (dbus_message_iter_next(it));

            return TQT_DBusData::fromInt16KeyMap(map);
        }

        case DBUS_TYPE_UINT16: {
            TQT_DBusDataMap<TQ_UINT16> map = prototype.toUInt16KeyMap();
            do {
                qFetchUInt16KeyMapEntry(map, it);
            } while (dbus_message_iter_next(it));

            return TQT_DBusData::fromUInt16KeyMap(map);
        }

        case DBUS_TYPE_INT32: {
            TQT_DBusDataMap<TQ_INT32> map = prototype.toInt32KeyMap();
            do {
                qFetchInt32KeyMapEntry(map, it);
            } while (dbus_message_iter_next(it));

            return TQT_DBusData::fromInt32KeyMap(map);
        }

        case DBUS_TYPE_UINT32: {
            TQT_DBusDataMap<TQ_UINT32> map = prototype.toUInt32KeyMap();
            do {
                qFetchUInt32KeyMapEntry(map, it);
            } while (dbus_message_iter_next(it));

            return TQT_DBusData::fromUInt32KeyMap(map);
        }

        case DBUS_TYPE_INT64: {
            TQT_DBusDataMap<TQ_INT64> map = prototype.toInt64KeyMap();
            do {
                qFetchInt64KeyMapEntry(map, it);
            } while (dbus_message_iter_next(it));

            return TQT_DBusData::fromInt64KeyMap(map);
        }

        case DBUS_TYPE_UINT64: {
            TQT_DBusDataMap<TQ_UINT64> map = prototype.toUInt64KeyMap();
            do {
                qFetchUInt64KeyMapEntry(map, it);
            } while (dbus_message_iter_next(it));

            return TQT_DBusData::fromUInt64KeyMap(map);
        }

        case DBUS_TYPE_OBJECT_PATH:  {
            TQT_DBusDataMap<TQT_DBusObjectPath> map = prototype.toObjectPathKeyMap();
            do {
            	qFetchObjectPathKeyMapEntry(map, it);
            } while (dbus_message_iter_next(it));

            return TQT_DBusData::fromObjectPathKeyMap(map);
        }
        case DBUS_TYPE_STRING:      // fall through
        case DBUS_TYPE_SIGNATURE: {
            TQT_DBusDataMap<TQString> map = prototype.toStringKeyMap();
            do {
                qFetchStringKeyMapEntry(map, it);
            } while (dbus_message_iter_next(it));

            return TQT_DBusData::fromStringKeyMap(map);
        }

        default:
            break;
    }

    return prototype;
}

static TQT_DBusData qFetchParameter(DBusMessageIter *it)
{
    switch (dbus_message_iter_get_arg_type(it)) {
    case DBUS_TYPE_BOOLEAN:
        return TQT_DBusData::fromBool(qIterGet<dbus_bool_t>(it));
    case DBUS_TYPE_BYTE:
        return TQT_DBusData::fromByte(qIterGet<unsigned char>(it));
    case DBUS_TYPE_INT16:
       return TQT_DBusData::fromInt16(qIterGet<dbus_int16_t>(it));
    case DBUS_TYPE_UINT16:
        return TQT_DBusData::fromUInt16(qIterGet<dbus_uint16_t>(it));
    case DBUS_TYPE_INT32:
        return TQT_DBusData::fromInt32(qIterGet<dbus_int32_t>(it));
    case DBUS_TYPE_UINT32:
        return TQT_DBusData::fromUInt32(qIterGet<dbus_uint32_t>(it));
    case DBUS_TYPE_INT64:
        return TQT_DBusData::fromInt64(qIterGet<dbus_int64_t>(it));
    case DBUS_TYPE_UINT64:
        return TQT_DBusData::fromUInt64(qIterGet<dbus_uint64_t>(it));
    case DBUS_TYPE_DOUBLE:
        return TQT_DBusData::fromDouble(qIterGet<double>(it));
    case DBUS_TYPE_STRING:
    case DBUS_TYPE_SIGNATURE:
        return TQT_DBusData::fromString(TQString::fromUtf8(qIterGet<char *>(it)));
    case DBUS_TYPE_OBJECT_PATH:
        return TQT_DBusData::fromObjectPath(TQT_DBusObjectPath(qIterGet<char *>(it)));
    case DBUS_TYPE_ARRAY: {
        int arrayType = dbus_message_iter_get_element_type(it);

        char* sig = dbus_message_iter_get_signature(it);
        TQCString signature = sig;
        dbus_free(sig);

        TQValueList<TQT_DBusData> prototypeList = parseSignature(signature);

        if (arrayType == DBUS_TYPE_DICT_ENTRY) {
            DBusMessageIter sub;
            dbus_message_iter_recurse(it, &sub);

            return qFetchMap(&sub, prototypeList[0]);
        } else {
            TQT_DBusDataList list = prototypeList[0].toList();

            DBusMessageIter arrayIt;
            dbus_message_iter_recurse(it, &arrayIt);

            while (dbus_message_iter_get_arg_type(&arrayIt) != DBUS_TYPE_INVALID) {
                list << qFetchParameter(&arrayIt);

                dbus_message_iter_next(&arrayIt);
            }

            return TQT_DBusData::fromList(list);
        }
    }
    case DBUS_TYPE_VARIANT: {
        TQT_DBusVariant dvariant;
        DBusMessageIter sub;
        dbus_message_iter_recurse(it, &sub);

        char* signature = dbus_message_iter_get_signature(&sub);
        dvariant.signature = TQString::fromUtf8(signature);
        dbus_free(signature);

        dvariant.value = qFetchParameter(&sub);

        return TQT_DBusData::fromVariant(dvariant);
    }
    case DBUS_TYPE_STRUCT: {
        TQValueList<TQT_DBusData> memberList;

        DBusMessageIter subIt;
        dbus_message_iter_recurse(it, &subIt);

        uint index = 0;
        while (dbus_message_iter_get_arg_type(&subIt) != DBUS_TYPE_INVALID) {
            memberList << qFetchParameter(&subIt);

            dbus_message_iter_next(&subIt);
            ++index;
        }

        return TQT_DBusData::fromStruct(memberList);
    }
    case DBUS_TYPE_UNIX_FD: {
        TQT_DBusUnixFd unixFd;
        unixFd.giveFileDescriptor(qIterGet<dbus_uint32_t>(it));
        return TQT_DBusData::fromUnixFd(unixFd);
    }
#if 0
    case DBUS_TYPE_INVALID:
        // TODO: check if there is better way to detect empty arrays
        return TQT_DBusData();
        break;
#endif
    default:
        tqWarning("TQT_DBusMarshall: Don't know how to de-marshall type %d '%c'",
                 dbus_message_iter_get_arg_type(it),
                 dbus_message_iter_get_arg_type(it));
        return TQT_DBusData();
        break;
    }
}

void TQT_DBusMarshall::messageToList(TQValueList<TQT_DBusData>& list, DBusMessage* message)
{
    Q_ASSERT(message);

    DBusMessageIter it;
    if (!dbus_message_iter_init(message, &it)) return;

    do
    {
        list << qFetchParameter(&it);
    }
    while (dbus_message_iter_next(&it));
}

static void tqAppendToMessage(DBusMessageIter *it, const TQString &str)
{
    TQByteArray ba = str.utf8();
    const char *cdata = ba.data();
    dbus_message_iter_append_basic(it, DBUS_TYPE_STRING, &cdata);
}

static void tqAppendToMessage(DBusMessageIter *it, const TQT_DBusObjectPath &path)
{
    const char *cdata = path.ascii();
    dbus_message_iter_append_basic(it, DBUS_TYPE_OBJECT_PATH, &cdata);
}

static void tqAppendToMessage(DBusMessageIter *it, const TQT_DBusUnixFd &unixFd)
{
    const dbus_int32_t cdata = unixFd.fileDescriptor();
    dbus_message_iter_append_basic(it, DBUS_TYPE_UNIX_FD, &cdata);
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

        case TQT_DBusData::List:
            return DBUS_TYPE_ARRAY_AS_STRING;

        case TQT_DBusData::Struct:
            return DBUS_TYPE_STRUCT_AS_STRING;

        case TQT_DBusData::Variant:
            return DBUS_TYPE_VARIANT_AS_STRING;

        case TQT_DBusData::Map:
            return DBUS_TYPE_DICT_ENTRY_AS_STRING;

        case TQT_DBusData::UnixFd:
            return DBUS_TYPE_UNIX_FD_AS_STRING;
    }
    return 0;
}

static void qDBusDataToIterator(DBusMessageIter* it, const TQT_DBusData& var);

static void qDBusByteKeyMapToIterator(DBusMessageIter* it, const TQT_DBusData& var)
{
    DBusMessageIter sub;
    TQCString sig;

    TQT_DBusDataMap<TQ_UINT8> map = var.toByteKeyMap();

    sig += DBUS_DICT_ENTRY_BEGIN_CHAR;
    sig += qDBusTypeForTQT_DBusType(map.keyType());

    if (map.hasContainerValueType())
        sig += map.containerValueType().buildDBusSignature();
    else
        sig += qDBusTypeForTQT_DBusType(map.valueType());
    sig += DBUS_DICT_ENTRY_END_CHAR;

    dbus_message_iter_open_container(it, DBUS_TYPE_ARRAY, sig.data(), &sub);

    TQT_DBusDataMap<TQ_UINT8>::const_iterator mit = map.begin();
    for (; mit != map.end(); ++mit)
    {
        DBusMessageIter itemIterator;
        dbus_message_iter_open_container(&sub, DBUS_TYPE_DICT_ENTRY,
                                         0, &itemIterator);

        dbus_message_iter_append_basic(it, DBUS_TYPE_BYTE, &(mit.key()));
        qDBusDataToIterator(&itemIterator, mit.data());

        dbus_message_iter_close_container(&sub, &itemIterator);
    }

    dbus_message_iter_close_container(it, &sub);
}

static void qDBusInt16KeyMapToIterator(DBusMessageIter* it, const TQT_DBusData& var)
{
    DBusMessageIter sub;
    TQCString sig;

    TQT_DBusDataMap<TQ_INT16> map = var.toInt16KeyMap();

    sig += DBUS_DICT_ENTRY_BEGIN_CHAR;
    sig += qDBusTypeForTQT_DBusType(map.keyType());

    if (map.hasContainerValueType())
        sig += map.containerValueType().buildDBusSignature();
    else
        sig += qDBusTypeForTQT_DBusType(map.valueType());
    sig += DBUS_DICT_ENTRY_END_CHAR;

    dbus_message_iter_open_container(it, DBUS_TYPE_ARRAY, sig.data(), &sub);

    TQT_DBusDataMap<TQ_INT16>::const_iterator mit = map.begin();
    for (; mit != map.end(); ++mit)
    {
        DBusMessageIter itemIterator;
        dbus_message_iter_open_container(&sub, DBUS_TYPE_DICT_ENTRY,
                                         0, &itemIterator);

        dbus_message_iter_append_basic(it, DBUS_TYPE_INT16, &(mit.key()));
        qDBusDataToIterator(&itemIterator, mit.data());

        dbus_message_iter_close_container(&sub, &itemIterator);
    }

    dbus_message_iter_close_container(it, &sub);
}

static void qDBusUInt16KeyMapToIterator(DBusMessageIter* it, const TQT_DBusData& var)
{
    DBusMessageIter sub;
    TQCString sig;

    TQT_DBusDataMap<TQ_UINT16> map = var.toUInt16KeyMap();

    sig += DBUS_DICT_ENTRY_BEGIN_CHAR;
    sig += qDBusTypeForTQT_DBusType(map.keyType());

    if (map.hasContainerValueType())
        sig += map.containerValueType().buildDBusSignature();
    else
        sig += qDBusTypeForTQT_DBusType(map.valueType());
    sig += DBUS_DICT_ENTRY_END_CHAR;

    dbus_message_iter_open_container(it, DBUS_TYPE_ARRAY, sig.data(), &sub);

    TQT_DBusDataMap<TQ_UINT16>::const_iterator mit = map.begin();
    for (; mit != map.end(); ++mit)
    {
        DBusMessageIter itemIterator;
        dbus_message_iter_open_container(&sub, DBUS_TYPE_DICT_ENTRY,
                                         0, &itemIterator);

        dbus_message_iter_append_basic(it, DBUS_TYPE_UINT16, &(mit.key()));
        qDBusDataToIterator(&itemIterator, mit.data());

        dbus_message_iter_close_container(&sub, &itemIterator);
    }

    dbus_message_iter_close_container(it, &sub);
}

static void qDBusInt32KeyMapToIterator(DBusMessageIter* it, const TQT_DBusData& var)
{
    DBusMessageIter sub;
    TQCString sig;

    TQT_DBusDataMap<TQ_INT32> map = var.toInt32KeyMap();

    sig += DBUS_DICT_ENTRY_BEGIN_CHAR;
    sig += qDBusTypeForTQT_DBusType(map.keyType());

    if (map.hasContainerValueType())
        sig += map.containerValueType().buildDBusSignature();
    else
        sig += qDBusTypeForTQT_DBusType(map.valueType());
    sig += DBUS_DICT_ENTRY_END_CHAR;

    dbus_message_iter_open_container(it, DBUS_TYPE_ARRAY, sig.data(), &sub);

    TQT_DBusDataMap<TQ_INT32>::const_iterator mit = map.begin();
    for (; mit != map.end(); ++mit)
    {
        DBusMessageIter itemIterator;
        dbus_message_iter_open_container(&sub, DBUS_TYPE_DICT_ENTRY,
                                         0, &itemIterator);

        dbus_message_iter_append_basic(it, DBUS_TYPE_INT32, &(mit.key()));
        qDBusDataToIterator(&itemIterator, mit.data());

        dbus_message_iter_close_container(&sub, &itemIterator);
    }

    dbus_message_iter_close_container(it, &sub);
}

static void qDBusUInt32KeyMapToIterator(DBusMessageIter* it, const TQT_DBusData& var)
{
    DBusMessageIter sub;
    TQCString sig;

    TQT_DBusDataMap<TQ_UINT32> map = var.toUInt32KeyMap();

    sig += DBUS_DICT_ENTRY_BEGIN_CHAR;
    sig += qDBusTypeForTQT_DBusType(map.keyType());

    if (map.hasContainerValueType())
        sig += map.containerValueType().buildDBusSignature();
    else
        sig += qDBusTypeForTQT_DBusType(map.valueType());
    sig += DBUS_DICT_ENTRY_END_CHAR;

    dbus_message_iter_open_container(it, DBUS_TYPE_ARRAY, sig.data(), &sub);

    TQT_DBusDataMap<TQ_UINT32>::const_iterator mit = map.begin();
    for (; mit != map.end(); ++mit)
    {
        DBusMessageIter itemIterator;
        dbus_message_iter_open_container(&sub, DBUS_TYPE_DICT_ENTRY,
                                         0, &itemIterator);

        dbus_message_iter_append_basic(it, DBUS_TYPE_UINT32, &(mit.key()));
        qDBusDataToIterator(&itemIterator, mit.data());

        dbus_message_iter_close_container(&sub, &itemIterator);
    }

    dbus_message_iter_close_container(it, &sub);
}

static void qDBusInt64KeyMapToIterator(DBusMessageIter* it, const TQT_DBusData& var)
{
    DBusMessageIter sub;
    TQCString sig;

    TQT_DBusDataMap<TQ_INT64> map = var.toInt64KeyMap();

    sig += DBUS_DICT_ENTRY_BEGIN_CHAR;
    sig += qDBusTypeForTQT_DBusType(map.keyType());

    if (map.hasContainerValueType())
        sig += map.containerValueType().buildDBusSignature();
    else
        sig += qDBusTypeForTQT_DBusType(map.valueType());
    sig += DBUS_DICT_ENTRY_END_CHAR;

    dbus_message_iter_open_container(it, DBUS_TYPE_ARRAY, sig.data(), &sub);

    TQT_DBusDataMap<TQ_INT64>::const_iterator mit = map.begin();
    for (; mit != map.end(); ++mit)
    {
        DBusMessageIter itemIterator;
        dbus_message_iter_open_container(&sub, DBUS_TYPE_DICT_ENTRY,
                                         0, &itemIterator);

        dbus_message_iter_append_basic(it, DBUS_TYPE_INT64, &(mit.key()));
        qDBusDataToIterator(&itemIterator, mit.data());

        dbus_message_iter_close_container(&sub, &itemIterator);
    }

    dbus_message_iter_close_container(it, &sub);
}

static void qDBusUInt64KeyMapToIterator(DBusMessageIter* it, const TQT_DBusData& var)
{
    DBusMessageIter sub;
    TQCString sig;

    TQT_DBusDataMap<TQ_UINT64> map = var.toUInt64KeyMap();

    sig += DBUS_DICT_ENTRY_BEGIN_CHAR;
    sig += qDBusTypeForTQT_DBusType(map.keyType());

    if (map.hasContainerValueType())
        sig += map.containerValueType().buildDBusSignature();
    else
        sig += qDBusTypeForTQT_DBusType(map.valueType());
    sig += DBUS_DICT_ENTRY_END_CHAR;

    dbus_message_iter_open_container(it, DBUS_TYPE_ARRAY, sig.data(), &sub);

    TQT_DBusDataMap<TQ_UINT64>::const_iterator mit = map.begin();
    for (; mit != map.end(); ++mit)
    {
        DBusMessageIter itemIterator;
        dbus_message_iter_open_container(&sub, DBUS_TYPE_DICT_ENTRY,
                                         0, &itemIterator);

        dbus_message_iter_append_basic(it, DBUS_TYPE_UINT64, &(mit.key()));
        qDBusDataToIterator(&itemIterator, mit.data());

        dbus_message_iter_close_container(&sub, &itemIterator);
    }

    dbus_message_iter_close_container(it, &sub);
}

static void qDBusStringKeyMapToIterator(DBusMessageIter* it, const TQT_DBusData& var)
{
    DBusMessageIter sub;
    TQCString sig;

    TQT_DBusDataMap<TQString> map = var.toStringKeyMap();

    sig += DBUS_DICT_ENTRY_BEGIN_CHAR;
    sig += qDBusTypeForTQT_DBusType(map.keyType());

    if (map.hasContainerValueType())
        sig += map.containerValueType().buildDBusSignature();
    else
        sig += qDBusTypeForTQT_DBusType(map.valueType());
    sig += DBUS_DICT_ENTRY_END_CHAR;

    dbus_message_iter_open_container(it, DBUS_TYPE_ARRAY, sig.data(), &sub);

    TQT_DBusDataMap<TQString>::const_iterator mit = map.begin();
    for (; mit != map.end(); ++mit)
    {
        DBusMessageIter itemIterator;
        dbus_message_iter_open_container(&sub, DBUS_TYPE_DICT_ENTRY,
                                         0, &itemIterator);

        tqAppendToMessage(&itemIterator, mit.key());
        qDBusDataToIterator(&itemIterator, mit.data());

        dbus_message_iter_close_container(&sub, &itemIterator);
    }

    dbus_message_iter_close_container(it, &sub);
}

static void qDBusObjectPathKeyMapToIterator(DBusMessageIter* it,
                                            const TQT_DBusData& var)
{
    DBusMessageIter sub;
    TQCString sig;

    TQT_DBusDataMap<TQT_DBusObjectPath> map = var.toObjectPathKeyMap();

    sig += DBUS_DICT_ENTRY_BEGIN_CHAR;
    sig += qDBusTypeForTQT_DBusType(map.keyType());

    if (map.hasContainerValueType())
        sig += map.containerValueType().buildDBusSignature();
    else
        sig += qDBusTypeForTQT_DBusType(map.valueType());
    sig += DBUS_DICT_ENTRY_END_CHAR;

    dbus_message_iter_open_container(it, DBUS_TYPE_ARRAY, sig.data(), &sub);

    TQT_DBusDataMap<TQT_DBusObjectPath>::const_iterator mit = map.begin();
    for (; mit != map.end(); ++mit)
    {
        DBusMessageIter itemIterator;
        dbus_message_iter_open_container(&sub, DBUS_TYPE_DICT_ENTRY,
                                         0, &itemIterator);

        tqAppendToMessage(&itemIterator, mit.key());
        qDBusDataToIterator(&itemIterator, mit.data());

        dbus_message_iter_close_container(&sub, &itemIterator);
    }

    dbus_message_iter_close_container(it, &sub);
}

static void qDBusUnixFdKeyMapToIterator(DBusMessageIter* it, const TQT_DBusData& var)
{
    DBusMessageIter sub;
    TQCString sig;

    TQT_DBusDataMap<TQT_DBusUnixFd> map = var.toUnixFdKeyMap();

    sig += DBUS_DICT_ENTRY_BEGIN_CHAR;
    sig += qDBusTypeForTQT_DBusType(map.keyType());

    if (map.hasContainerValueType())
        sig += map.containerValueType().buildDBusSignature();
    else
        sig += qDBusTypeForTQT_DBusType(map.valueType());
    sig += DBUS_DICT_ENTRY_END_CHAR;

    dbus_message_iter_open_container(it, DBUS_TYPE_ARRAY, sig.data(), &sub);

    TQT_DBusDataMap<TQT_DBusUnixFd>::const_iterator mit = map.begin();
    for (; mit != map.end(); ++mit)
    {
        DBusMessageIter itemIterator;
        dbus_message_iter_open_container(&sub, DBUS_TYPE_DICT_ENTRY,
                                         0, &itemIterator);

        dbus_message_iter_append_basic(it, DBUS_TYPE_UNIX_FD, &(mit.key()));
        qDBusDataToIterator(&itemIterator, mit.data());

        dbus_message_iter_close_container(&sub, &itemIterator);
    }

    dbus_message_iter_close_container(it, &sub);
}

static void qDBusDataToIterator(DBusMessageIter* it, const TQT_DBusData& var)
{
    switch (var.type())
    {
        case TQT_DBusData::Bool:
        {
            dbus_bool_t value = var.toBool();
            dbus_message_iter_append_basic(it, DBUS_TYPE_BOOLEAN, &value);
            break;
        }
        case TQT_DBusData::Byte:
        {
            TQ_UINT8 value = var.toByte();
            dbus_message_iter_append_basic(it, DBUS_TYPE_BYTE, &value);
            break;
        }
        case TQT_DBusData::Int16: {
            TQ_INT16 value = var.toInt16();
            dbus_message_iter_append_basic(it, DBUS_TYPE_INT16, &value);
            break;
        }
        case TQT_DBusData::UInt16: {
            TQ_UINT16 value = var.toUInt16();
            dbus_message_iter_append_basic(it, DBUS_TYPE_UINT16, &value);
            break;
        }
        case TQT_DBusData::Int32: {
            TQ_INT32 value = var.toInt32();
            dbus_message_iter_append_basic(it, DBUS_TYPE_INT32, &value);
            break;
        }
        case TQT_DBusData::UInt32: {
            TQ_UINT32 value = var.toUInt32();
            dbus_message_iter_append_basic(it, DBUS_TYPE_UINT32, &value);
            break;
        }
        case TQT_DBusData::Int64: {
            TQ_INT64 value = var.toInt64();
            dbus_message_iter_append_basic(it, DBUS_TYPE_INT64, &value);
            break;
        }
        case TQT_DBusData::UInt64: {
            TQ_UINT64 value = var.toUInt64();
            dbus_message_iter_append_basic(it, DBUS_TYPE_UINT64, &value);
            break;
        }
        case TQT_DBusData::Double: {
            double value = var.toDouble();
            dbus_message_iter_append_basic(it, DBUS_TYPE_DOUBLE, &value);
            break;
        }
        case TQT_DBusData::String:
            tqAppendToMessage(it, var.toString());
            break;
        case TQT_DBusData::ObjectPath:
            tqAppendToMessage(it, var.toObjectPath());
            break;
        case TQT_DBusData::UnixFd: {
            tqAppendToMessage(it, var.toUnixFd());
            break;
        }
        case TQT_DBusData::List: {
            TQT_DBusDataList list = var.toList();

            TQCString signature = 0;
            if (list.hasContainerItemType())
                signature = list.containerItemType().buildDBusSignature();
            else
                signature = qDBusTypeForTQT_DBusType(list.type());

            DBusMessageIter sub;
            dbus_message_iter_open_container(it, DBUS_TYPE_ARRAY,
                                             signature.data(), &sub);

            const TQValueList<TQT_DBusData> valueList = var.toTQValueList();
            TQValueList<TQT_DBusData>::const_iterator listIt    = valueList.begin();
            TQValueList<TQT_DBusData>::const_iterator listEndIt = valueList.end();
            for (; listIt != listEndIt; ++listIt)
            {
                qDBusDataToIterator(&sub, *listIt);
            }
            dbus_message_iter_close_container(it, &sub);
            break;
        }
        case TQT_DBusData::Map: {
            switch (var.keyType()) {
                case TQT_DBusData::Byte:
                    qDBusByteKeyMapToIterator(it, var);
                    break;
                case TQT_DBusData::Int16:
                    qDBusInt16KeyMapToIterator(it, var);
                    break;
                case TQT_DBusData::UInt16:
                    qDBusUInt16KeyMapToIterator(it, var);
                    break;
                case TQT_DBusData::Int32:
                    qDBusInt32KeyMapToIterator(it, var);
                    break;
                case TQT_DBusData::UInt32:
                    qDBusUInt32KeyMapToIterator(it, var);
                    break;
                case TQT_DBusData::Int64:
                    qDBusInt64KeyMapToIterator(it, var);
                    break;
                case TQT_DBusData::UInt64:
                    qDBusUInt64KeyMapToIterator(it, var);
                    break;
                case TQT_DBusData::String:
                    qDBusStringKeyMapToIterator(it, var);
                    break;
                case TQT_DBusData::ObjectPath:
                    qDBusObjectPathKeyMapToIterator(it, var);
                    break;
                case TQT_DBusData::UnixFd:
                    qDBusUnixFdKeyMapToIterator(it, var);
                    break;
                default:
                    tqWarning("TQT_DBusMarshall: unhandled map key type %s "
                             "at marshalling",
                             TQT_DBusData::typeName(var.keyType()));
                    break;
            }
            break;
        }
        case TQT_DBusData::Variant: {
            TQT_DBusVariant variant = var.toVariant();
            if (variant.signature.isEmpty() || !variant.value.isValid()) break;

            DBusMessageIter sub;
            dbus_message_iter_open_container(it, DBUS_TYPE_VARIANT,
                                             variant.signature.utf8(), &sub);

            qDBusDataToIterator(&sub, variant.value);

            dbus_message_iter_close_container(it, &sub);
            break;
        }
        case TQT_DBusData::Struct: {
            TQValueList<TQT_DBusData> memberList = var.toStruct();
            if (memberList.isEmpty()) break;

            DBusMessageIter sub;
            dbus_message_iter_open_container(it, DBUS_TYPE_STRUCT, NULL, &sub);

            TQValueList<TQT_DBusData>::const_iterator memberIt    = memberList.begin();
            TQValueList<TQT_DBusData>::const_iterator memberEndIt = memberList.end();
            for (; memberIt != memberEndIt; ++memberIt)
            {
                qDBusDataToIterator(&sub, *memberIt);
            }

            dbus_message_iter_close_container(it, &sub);
        }
#if 0
        case TQVariant::ByteArray: {
            const TQByteArray array = var.toByteArray();
            const char* cdata = array.data();
                DBusMessageIter sub;
                dbus_message_iter_open_container(it, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &sub);
            dbus_message_iter_append_fixed_array(&sub, DBUS_TYPE_BYTE, &cdata, array.size());
                dbus_message_iter_close_container(it, &sub);
        break;
        }
#endif
        default:
            //tqWarning("Don't know how to handle type %s", var.typeName());
            break;
    }
}

void qListToIterator(DBusMessageIter* it, const TQValueList<TQT_DBusData>& list)
{
    if (list.isEmpty()) return;

    TQValueList<TQT_DBusData>::const_iterator listIt    = list.begin();
    TQValueList<TQT_DBusData>::const_iterator listEndIt = list.end();
    for (; listIt != listEndIt; ++listIt)
    {
        qDBusDataToIterator(it, *listIt);
    }
}

void TQT_DBusMarshall::listToMessage(const TQValueList<TQT_DBusData> &list, DBusMessage *msg)
{
    Q_ASSERT(msg);
    DBusMessageIter it;
    dbus_message_iter_init_append(msg, &it);
    qListToIterator(&it, list);
}
