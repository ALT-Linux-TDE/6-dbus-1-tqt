/*
*   Copyright (C) 2007 Kevin Krammer <kevin.krammer@gmx.at>
*
*   Permission is hereby granted, free of charge, to any person obtaining a
*   copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation
*   the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the
*   Software is furnished to do so, subject to the following conditions:
*
*   The above copyright notice and this permission notice shall be included
*   in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
*   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
*   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
*   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
*   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
*   OTHER DEALINGS IN THE SOFTWARE.
*/

#if !defined(METHODGEN_H_INCLUDED)
#define METHODGEN_H_INCLUDED

// TQt includes
#include <tqmap.h>
#include <tqstringlist.h>

// forward declarations
class TQTextStream;

class Argument
{
public:
    enum Direction
    {
        In,
        Out
    };

    TQString name;
    TQString annotatedType;
    TQString signature;
    TQString accessor;
    TQString subAccessor;
    TQString containerClass;
    Direction direction;
    bool isPrimitive;

    TQStringList forwardDeclarations;
    TQMap<TQString, TQStringList> headerIncludes;
    TQMap<TQString, TQStringList> sourceIncludes;

    TQString dbusSignature;
};

class Method
{
public:
    enum Type
    {
        _Method,
        _Signal
    };

    TQString name;
    Type type;
    TQValueList<Argument> arguments;
    bool noReply;
    bool async;
};

class Property : public Argument
{
public:
    bool read;
    bool write;
};

class Class
{
public:
    enum Role
    {
        Interface,
        Proxy,
        Node
    };

    TQString name;
    TQString dbusName;
    TQStringList namespaces;
    TQValueList<Method> methods;
    TQValueList<Method> msignals;
    TQValueList<Property> properties;

    TQValueList<Method> asyncMethods;
    TQValueList<Method> asyncReplySignals;
    TQValueList<Method> asyncReplyMethods;
};

class MethodGenerator
{
public:
    static bool extractMethods(const TQDomElement& interfaceElement,
                               Class& classData);

    static void writeMethodDeclaration(const Method& method, bool pureVirtual,
                                       bool withError, TQTextStream& stream);

    static void writePropertyDeclaration(const Property& property, bool pureVirtual,
                                         TQTextStream& stream);

    static void writeMethodCallDeclaration(const Method& method,
                                           TQTextStream& stream);

    static void writeMethodCall(const Class& classData, const Method& method,
                                TQTextStream& stream);

    static void writeSignalEmitter(const Class& classData, const Method& method,
                                   TQTextStream& stream);

    static void writeInterfaceAsyncReplyHandler(const Class& classData,
                                                const Method& method,
                                                TQTextStream& stream);

    static void writeInterfaceMainMethod(const Class& classData,
                                         TQTextStream& stream);

    static void writeSignalHandler(const Class& classData, TQTextStream& stream);

    static void writeProxyBegin(const Class& classData, TQTextStream& stream);

    static void writeProxyMethod(const TQString& className, const Method& method,
                                 TQTextStream& stream);

    static void writeProxyGenericProperty(const Class& classData,
                                          TQTextStream& stream);

    static void writeProxyProperty(const Class& classData, const Property& property,
                                   TQTextStream& stream);

    static void writeProxyAsyncReplyHandler(const Class& classData,
                                            TQTextStream& stream);

    static void writeIntrospectionDataMethod(const Class& classData,
                                             TQTextStream& stream);

    static void writeNodePrivate(const Class& classData, TQTextStream& stream);

    static void writeNodeBegin(const Class& classData, TQTextStream& stream);

    static void writeNodeMethods(const Class& classData,
            const TQValueList<Class>& interfaces, TQTextStream& stream);
};

#endif

// End of File
