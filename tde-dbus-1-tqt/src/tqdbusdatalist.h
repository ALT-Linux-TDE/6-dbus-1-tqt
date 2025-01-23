/* qdbusdatalist.h list of DBUS data transport type
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

#ifndef TQDBUSDATALIST_H
#define TQDBUSDATALIST_H

#include "tqdbusdata.h"

template <typename T> class TQValueList;
class TQT_DBusObjectPath;
class TQT_DBusVariant;
class TQT_DBusUnixFd;
class TQString;
class TQStringList;

/**
 * @brief Class to transport lists of D-Bus data types
 *
 * \note while the D-Bus data type is actually called @c array this bindings
 *       use the term @c list since the behavior and characteristics of the
 *       implementation is more list like.
 *
 * There are basically two ways to create TQT_DBusDataList objects:
 * - non-empty from content
 * - empty by specifying the desired element type
 *
 * Example for creating a filled list from content
 * @code
 * TQValueList<TQ_INT16> intList;
 * list << 2 << 3 << 5 << 7;
 *
 * TQT_DBusDataList dbusList(intList);
 * TQT_DBusData data = TQT_DBusData::fromList(dbusList);
 *
 * // or even shorter, using implicit conversion
 * TQT_DBusData other = TQT_DBusData::fromList(intList);
 * @endcode
 *
 * Example for creating an empty list
 * @code
 * // empty list for a simple type
 * TQT_DBusDataList list(TQT_DBusData::Double);
 *
 * // empty list for a list of string lists
 * TQT_DBusData elementType = TQT_DBusData::fromList(TQT_DBusDataList(TQT_DBusData::String));
 * TQT_DBusDataList outerList(elementType);
 * @endcode
 *
 * @see TQT_DBusDataMap
 */
class TQDBUS_EXPORT TQT_DBusDataList
{
public:
    /**
     * @brief Creates an empty and invalid list
     *
     * @see TQT_DBusData::Invalid
     */
    TQT_DBusDataList();

    /**
     * @brief Creates an empty list with the given simple type for elements
     *
     * The given type has be one of the non-container types, i.e. any other than
     * TQT_DBusData::Map, TQT_DBusData::List or TQT_DBusData::Struct
     *
     * For creating a list with elements which are containers themselves, use
     * TQT_DBusDataList(const TQT_DBusData&);
     *
     * @param simpleItemType the type of the elements in the new list
     */
    explicit TQT_DBusDataList(TQT_DBusData::Type simpleItemType);

    /**
     * @brief Creates an empty list with the given container type for elements
     *
     * For creating a list with simple elements you can also use
     * TQT_DBusDataList(TQT_DBusData::Type);
     *
     * @param containerItemType the type of the elements in the new list
     *
     * @see hasContainerItemType()
     */
    explicit TQT_DBusDataList(const TQT_DBusData& containerItemType);

    /**
     * @brief Creates a list from the given @p other list
     *
     * This behaves basically like copying a TQValueList through its copy
     * constructor, i.e. no value are actually copied at this time.
     *
     * @param other the other list object to copy from
     */
    TQT_DBusDataList(const TQT_DBusDataList& other);

    /**
     * @brief Creates a list from the given TQValueList of TQT_DBusData objects
     *
     * If the @p other list is empty, this will behave like TQT_DBusDataList(),
     * i.e. create an empty and invalid list object.
     *
     * Type information for the list object, i.e. element type and, if applicable,
     * container item type, will be derived from the @p other list's elements.
     *
     * \warning if the elements of the @p other list do not all have the same
     *          type, the list object will also be empty and invalid
     *
     * @param other the TQValueList of TQT_DBusData objects to copy from
     *
     * @see toTQValueList()
     */
    TQT_DBusDataList(const TQValueList<TQT_DBusData>& other);

    /**
     * @brief Creates a list from the given TQValueList of boolean values
     *
     * Type information for the list object will be set to TQT_DBusData::Bool
     * also when the @p other list is empty, i.e. this allows to create an
     * empty but valid list object, comparable to using
     * TQT_DBusDataList(TQT_DBusData::Type) with TQT_DBusData::Bool
     *
     * @param other the TQValueList of boolean values to copy from
     *
     * @see toBoolList()
     */
    TQT_DBusDataList(const TQValueList<bool>& other);

    /**
     * @brief Creates a list from the given TQValueList of byte (unsigned char) values
     *
     * Type information for the list object will be set to TQT_DBusData::Byte
     * also when the @p other list is empty, i.e. this allows to create an
     * empty but valid list object, comparable to using
     * TQT_DBusDataList(TQT_DBusData::Type) with TQT_DBusData::Byte
     *
     * @param other the TQValueList of byte (unsigned char) values to copy from
     *
     * @see toByteList()
     */
    TQT_DBusDataList(const TQValueList<TQ_UINT8>& other);

    /**
     * @brief Creates a list from the given TQValueList of signed 16-bit integer values
     *
     * Type information for the list object will be set to TQT_DBusData::Int16
     * also when the @p other list is empty, i.e. this allows to create an
     * empty but valid list object, comparable to using
     * TQT_DBusDataList(TQT_DBusData::Type) with TQT_DBusData::Int16
     *
     * @param other the TQValueList of signed 16-bit integer values to copy from
     *
     * @see toInt16List()
     */
    TQT_DBusDataList(const TQValueList<TQ_INT16>& other);

    /**
     * @brief Creates a list from the given TQValueList of unsigned 16-bit integer values
     *
     * Type information for the list object will be set to TQT_DBusData::UInt16
     * also when the @p other list is empty, i.e. this allows to create an
     * empty but valid list object, comparable to using
     * TQT_DBusDataList(TQT_DBusData::Type) with TQT_DBusData::UInt16
     *
     * @param other the TQValueList of unsigned 16-bit integer values to copy from
     *
     * @see toUInt16List()
     */
    TQT_DBusDataList(const TQValueList<TQ_UINT16>& other);

    /**
     * @brief Creates a list from the given TQValueList of signed 32-bit integer values
     *
     * Type information for the list object will be set to TQT_DBusData::Int32
     * also when the @p other list is empty, i.e. this allows to create an
     * empty but valid list object, comparable to using
     * TQT_DBusDataList(TQT_DBusData::Type) with TQT_DBusData::Int32
     *
     * @param other the TQValueList of signed 32-bit integer values to copy from
     *
     * @see toInt32List()
     */
    TQT_DBusDataList(const TQValueList<TQ_INT32>& other);

    /**
     * @brief Creates a list from the given TQValueList of unsigned 32-bit integer values
     *
     * Type information for the list object will be set to TQT_DBusData::UInt16
     * also when the @p other list is empty, i.e. this allows to create an
     * empty but valid list object, comparable to using
     * TQT_DBusDataList(TQT_DBusData::Type) with TQT_DBusData::UInt32
     *
     * @param other the TQValueList of unsigned 32-bit integer values to copy from
     *
     * @see toUInt32List()
     */
    TQT_DBusDataList(const TQValueList<TQ_UINT32>& other);

    /**
     * @brief Creates a list from the given TQValueList of signed 64-bit integer values
     *
     * Type information for the list object will be set to TQT_DBusData::Int64
     * also when the @p other list is empty, i.e. this allows to create an
     * empty but valid list object, comparable to using
     * TQT_DBusDataList(TQT_DBusData::Type) with TQT_DBusData::Int64
     *
     * @param other the TQValueList of signed 64-bit integer values to copy from
     *
     * @see toInt64List()
     */
    TQT_DBusDataList(const TQValueList<TQ_INT64>& other);

    /**
     * @brief Creates a list from the given TQValueList of unsigned 64-bit integer values
     *
     * Type information for the list object will be set to TQT_DBusData::UInt64
     * also when the @p other list is empty, i.e. this allows to create an
     * empty but valid list object, comparable to using
     * TQT_DBusDataList(TQT_DBusData::Type) with TQT_DBusData::UInt64
     *
     * @param other the TQValueList of unsigned 64-bit integer values to copy from
     *
     * @see toUInt64List()
     */
    TQT_DBusDataList(const TQValueList<TQ_UINT64>& other);

    /**
     * @brief Creates a list from the given TQValueList of double values
     *
     * Type information for the list object will be set to TQT_DBusData::Double
     * also when the @p other list is empty, i.e. this allows to create an
     * empty but valid list object, comparable to using
     * TQT_DBusDataList(TQT_DBusData::Type) with TQT_DBusData::Double
     *
     * @param other the TQValueList of double values to copy from
     *
     * @see toDoubleList()
     */
    TQT_DBusDataList(const TQValueList<double>& other);

    /**
     * @brief Creates a list from the given TQValueList of TQT_DBusVariant values
     *
     * Type information for the list object will be set to TQT_DBusData::Variant
     * also when the @p other list is empty, i.e. this allows to create an
     * empty but valid list object, comparable to using
     * TQT_DBusDataList(TQT_DBusData::Type) with TQT_DBusData::Variant
     *
     * @param other the TQValueList of variant values to copy from
     *
     * @see toVariantList()
     */
    TQT_DBusDataList(const TQValueList<TQT_DBusVariant>& other);

    /**
     * @brief Creates a list from the given TQStringList's values
     *
     * Type information for the list object will be set to TQT_DBusData::String
     * also when the @p other list is empty, i.e. this allows to create an
     * empty but valid list object, comparable to using
     * TQT_DBusDataList(TQT_DBusData::Type) with TQT_DBusData::String
     *
     * @param other the TQStringList to copy from
     *
     * @see toTQStringList()
     */
    TQT_DBusDataList(const TQStringList& other);

    /**
     * @brief Creates a list from the given TQValueList of object path values
     *
     * Type information for the list object will be set to TQT_DBusData::ObjectPath
     * also when the @p other list is empty, i.e. this allows to create an
     * empty but valid list object, comparable to using
     * TQT_DBusDataList(TQT_DBusData::Type) with TQT_DBusData::ObjectPath
     *
     * @param other the TQValueList of object path values to copy from
     *
     * @see toObjectPathList()
     */
    TQT_DBusDataList(const TQValueList<TQT_DBusObjectPath>& other);

    /**
     * @brief Creates a list from the given TQValueList of unix file handle values
     *
     * Type information for the list object will be set to TQT_DBusData::UnixFd
     * also when the @p other list is empty, i.e. this allows to create an
     * empty but valid list object, comparable to using
     * TQT_DBusDataList(TQT_DBusData::Type) with TQT_DBusData::UnixFd
     *
     * @param other the TQValueList of unix file handle values to copy from
     *
     * @see toUnixFdList()
     */
    TQT_DBusDataList(const TQValueList<TQT_DBusUnixFd>& other);

    /**
     * @brief Destroys the list object
     */
    ~TQT_DBusDataList();

    /**
     * @brief Copies from the given @p other list
     *
     * This behaves basically like copying a TQValueList through its assignment
     * operator, i.e. no value are actually copied at this time.
     *
     * @param other the other list object to copy from
     *
     * @return a reference to this list object
     */
    TQT_DBusDataList& operator=(const TQT_DBusDataList& other);

    /**
     * @brief Copies from the given @p other list
     *
     * This behaves basically like copying a TQValueList through its assignment
     * operator, i.e. no value are actually copied at this time.
     *
     * \warning the elements of the given @p other list have to be of the same
     *          type. If they aren't this list's content will cleared and the
     *          type will be set to TQT_DBusData::Invalid
     *
     * @param other the other list object to copy from
     *
     * @return a reference to this list object
     */
    TQT_DBusDataList& operator=(const TQValueList<TQT_DBusData>& other);

    /**
     * @brief Copies from the given @p other list
     *
     * Convenience overload as TQStringList is a very common data type in
     * TQt and D-Bus methods also use "arrays of strings" quite often.
     *
     * The list object's type will be set to TQT_DBusData::String. If the object
     * previously had a container as its element type, this will be reset, i.e.
     * hasContainerItemType() will return @c false
     *
     * @param other the stringlist to copy from
     *
     * @return a reference to this list object
     */
    TQT_DBusDataList& operator=(const TQStringList& other);

    /**
     * @brief Returns the element type of the list object
     *
     * @return one of the values of the TQT_DBusData#Type enum
     *
     * @see hasContainerItemType()
     * @see containerItemType()
     */
    TQT_DBusData::Type type() const;

    /**
     * @brief Checks whether the element type is a data container itself
     *
     * If the elements of the list are containers as well, this will return
     * @c true
     * In this case containerItemType() will return a prototype for such a
     * container.
     *
     * @return @c true if the element type is either TQT_DBusData::Map,
     *         TQT_DBusData::List or TQT_DBusData::Struct, otherwise @c false
     *
     * @see TQT_DBusDataList(const TQT_DBusData&)
     */
    bool hasContainerItemType() const;

    /**
     * @brief Returns a container prototype for the list's element type
     *
     * Lists which have containers as their elements, i.e. hasContainerItemType()
     * returns @c true this will actually specify the details for the use
     * container, i.e. the returned data object can be queried for type and
     * possible further subtypes.
     *
     * @return a data object detailing the element type or an invalid data object
     *         if the list does not have a container as its element type
     *
     * @see TQT_DBusDataList(const TQT_DBusData&);
     * @see type()
     * @see TQT_DBusData::Invalid
     */
    TQT_DBusData containerItemType() const;

    /**
     * @brief Checks whether this list object has a valid element type
     *
     * This is equal to checking type() for not being TQT_DBusData::Invalid
     *
     * @return @c true if the list object is valid, otherwise @c false
     */
    inline bool isValid() const { return type() != TQT_DBusData::Invalid; }

    /**
     * @brief Checks whether this list object has any elements
     *
     * @return @c true if there are no elements in this list, otherwise @c false
     *
     * @see count()
     */
    bool isEmpty() const;

    /**
     * @brief Returns the number of elements of this list object
     *
     * @return the number of elements
     *
     * @see isEmpty()
     */
    uint count() const;

    /**
     * @brief Checks whether the given @p other list is equal to this one
     *
     * Two lists are considered equal when they have the same type (and same
     * container item type if the have one) and the element lists are equal
     * as well.
     *
     * @param other the other list object to compare with
     *
     * @return @c true if the lists are equal, otherwise @c false
     *
     * @see TQT_DBusData::operator==()
     */
    bool operator==(const TQT_DBusDataList& other) const;

    /**
     * @brief Checks whether the given @p other list is different from this one
     *
     * Two lists are considered different when they have the different type (or
     * different container item type if the have one) or the element lists are
     * equal are different.
     *
     * @param other the other list object to compare with
     *
     * @return @c true if the lists are different, otherwise @c false
     *
     * @see TQT_DBusData::operator!=()
     */
    bool operator!=(const TQT_DBusDataList& other) const;

    /**
     * @brief Clears the list
     *
     * Type and, if applicable, container element type will stay untouched.
     */
    void clear();

    /**
     * @brief Appends a given value to the list
     *
     * Basically works like the respective TQValueList operator, but checks if
     * type of the new value matches the type of the list.
     * Lists that are invalid will accept any new type and will then be
     * typed accordingly.
     *
     * If @p data is invalid itself, it will not be appended at any time.
     *
     * \note the more common use case is to work with a TQValueList and then
     *       use the respective constructor to create the TQT_DBusDataList object
     *
     * @param data the data item to append to the list
     *
     * @return a reference to this list object
     */
    TQT_DBusDataList& operator<<(const TQT_DBusData& data);

    /**
     * @brief Converts the list object into a TQValueList with TQT_DBusData elements
     *
     * @return the values of the list object as a TQValueList
     */
    TQValueList<TQT_DBusData> toTQValueList() const;

    /**
     * @brief Tries to get the list object's elements as a TQStringList
     *
     * This is a convenience overload for the case when the list is of
     * type TQT_DBusData::String.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of type
     *        TQT_DBusData::String)
     *
     * @return a TQStringList containing the list object's string elements or
     *         an empty list when converting fails
     *
     * @see toStringList()
     * @see TQT_DBusData::toString()
     */
    TQStringList toTQStringList(bool* ok = 0) const;

    /**
     * @brief Tries to get the list object's elements as a TQValueList of bool
     *
     * This is a convenience overload for the case when the list is of
     * type TQT_DBusData::Bool.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of type
     *        TQT_DBusData::Bool)
     *
     * @return a TQValueList of bool containing the list object's boolean
     *         elements or an empty list when converting fails
     *
     * @see TQT_DBusData::toBool()
     */
    TQValueList<bool> toBoolList(bool* ok = 0) const;

    /**
     * @brief Tries to get the list object's elements as a TQValueList of TQ_UINT8
     *
     * This is a convenience overload for the case when the list is of
     * type TQT_DBusData::Byte.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of type
     *        TQT_DBusData::Byte)
     *
     * @return a TQValueList of TQ_UINT8 containing the list object's byte
     *         elements or an empty list when converting fails
     *
     * @see TQT_DBusData::toByte()
     */
    TQValueList<TQ_UINT8> toByteList(bool* ok = 0) const;

    /**
     * @brief Tries to get the list object's elements as a TQValueList of TQ_INT16
     *
     * This is a convenience overload for the case when the list is of
     * type TQT_DBusData::Int16.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of type
     *        TQT_DBusData::Int16)
     *
     * @return a TQValueList of TQ_INT16 containing the list object's
     *         signed 16-bit integer elements or an empty list when converting
     *         fails
     *
     * @see TQT_DBusData::toInt16()
     */
    TQValueList<TQ_INT16> toInt16List(bool* ok = 0) const;

    /**
     * @brief Tries to get the list object's elements as a TQValueList of TQ_UINT16
     *
     * This is a convenience overload for the case when the list is of
     * type TQT_DBusData::UInt16.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of type
     *        TQT_DBusData::UInt16)
     *
     * @return a TQValueList of TQ_UINT16 containing the list object's
     *         unsigned 16-bit integer elements or an empty list when converting
     *         fails
     *
     * @see TQT_DBusData::toUInt16()
     */
    TQValueList<TQ_UINT16> toUInt16List(bool* ok = 0) const;

    /**
     * @brief Tries to get the list object's elements as a TQValueList of TQ_INT32
     *
     * This is a convenience overload for the case when the list is of
     * type TQT_DBusData::Int32.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of type
     *        TQT_DBusData::Int32)
     *
     * @return a TQValueList of TQ_INT32 containing the list object's
     *         signed 32-bit integer elements or an empty list when converting
     *         fails
     *
     * @see TQT_DBusData::toInt32()
     */
    TQValueList<TQ_INT32> toInt32List(bool* ok = 0) const;

    /**
     * @brief Tries to get the list object's elements as a TQValueList of TQ_UINT32
     *
     * This is a convenience overload for the case when the list is of
     * type TQT_DBusData::UInt32.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of type
     *        TQT_DBusData::UInt32)
     *
     * @return a TQValueList of TQ_UINT32 containing the list object's
     *         unsigned 32-bit integer elements or an empty list when converting
     *         fails
     *
     * @see TQT_DBusData::toUInt32()
     */
    TQValueList<TQ_UINT32> toUInt32List(bool* ok = 0) const;

    /**
     * @brief Tries to get the list object's elements as a TQValueList of TQ_INT64
     *
     * This is a convenience overload for the case when the list is of
     * type TQT_DBusData::Int64.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of type
     *        TQT_DBusData::Int64)
     *
     * @return a TQValueList of TQ_INT64 containing the list object's
     *         signed 64-bit integer elements or an empty list when converting
     *         fails
     *
     * @see TQT_DBusData::toInt64()
     */
    TQValueList<TQ_INT64> toInt64List(bool* ok = 0) const;

    /**
     * @brief Tries to get the list object's elements as a TQValueList of TQ_UINT64
     *
     * This is a convenience overload for the case when the list is of
     * type TQT_DBusData::UInt64.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of type
     *        TQT_DBusData::UInt64)
     *
     * @return a TQValueList of TQ_UINT64 containing the list object's
     *         unsigned 64-bit integer elements or an empty list when converting
     *         fails
     *
     * @see TQT_DBusData::toUInt64()
     */
    TQValueList<TQ_UINT64> toUInt64List(bool* ok = 0) const;

    /**
     * @brief Tries to get the list object's elements as a TQValueList of double
     *
     * This is a convenience overload for the case when the list is of
     * type TQT_DBusData::Double.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of type
     *        TQT_DBusData::Double)
     *
     * @return a TQValueList of double containing the list object's double
     *         elements or an empty list when converting fails
     *
     * @see TQT_DBusData::toDouble()
     */
    TQValueList<double> toDoubleList(bool* ok = 0) const;

    /**
     * @brief Tries to get the list object's elements as a TQValueList of TQString
     *
     * This is a convenience overload for the case when the list is of
     * type TQT_DBusData::String, see also toTQStringList().
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of type
     *        TQT_DBusData::String)
     *
     * @return a TQValueList of TQString containing the list object's string
     *         elements or an empty list when converting fails
     *
     * @see TQT_DBusData::toString()
     */
    TQValueList<TQString> toStringList(bool* ok = 0) const;

    /**
     * @brief Tries to get the list object's elements as a TQValueList of object paths
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of type
     *        TQT_DBusData::ObjectPath)
     *
     * @return a TQValueList of object paths containing the list object's object path
     *         elements or an empty list when converting fails
     *
     * @see TQT_DBusData::toObjectPath()
     */
    TQValueList<TQT_DBusObjectPath> toObjectPathList(bool* ok = 0) const;

    /**
     * @brief Tries to get the list object's elements as a TQValueList of TQT_DBusVariant
     *
     * This is a convenience overload for the case when the list is of
     * type TQT_DBusData::Variant.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of type
     *        TQT_DBusData::Variant)
     *
     * @return a TQValueList of TQT_DBusVariant containing the list object's
     *         TQT_DBusVariant elements or an empty list when converting fails
     *
     * @see TQT_DBusData::toVariant()
     */
    TQValueList<TQT_DBusVariant> toVariantList(bool* ok = 0) const;

    /**
     * @brief Tries to get the list object's elements as a TQValueList of TQT_DBusUnixFd
     *
     * This is a convenience overload for the case when the list is of
     * type TQT_DBusData::UnixFd.
     *
     * @param ok optional pointer to a bool variable to store the
     *        success information in, i.e. will be set to @c true on success
     *        and to @c false if the conversion failed (not of type
     *        TQT_DBusData::UnixFd)
     *
     * @return a TQValueList of TQT_DBusUnixFd containing the list object's
     *         TQT_DBusUnixFd elements or an empty list when converting fails
     *
     * @see TQT_DBusData::toUnixFd()
     */
    TQValueList<TQT_DBusUnixFd> toUnixFdList(bool* ok = 0) const;

private:
    class Private;
    Private* d;
};

#endif
