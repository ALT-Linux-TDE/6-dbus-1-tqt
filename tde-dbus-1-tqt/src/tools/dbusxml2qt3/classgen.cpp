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

// TQt includes
#include <tqdom.h>
#include <tqfile.h>
#include <tqstringlist.h>
#include <tqtextstream.h>

// local includes
#include "classgen.h"
#include "methodgen.h"

class Set : public TQMap<TQString, bool>
{
public:
    void insertString(const TQString& key)
    {
        insert(key, true);
    }

    void removeString(const TQString& key)
    {
        erase(key);
    }

    void insertStringList(const TQStringList& list)
    {
        TQStringList::const_iterator it    = list.begin();
        TQStringList::const_iterator endIt = list.end();
        for (; it != endIt; ++it)
        {
            insert(*it, true);
        }
    }
};

static void writeFileHeader(TQTextStream& stream)
{
    stream << "// File autogenerated" << endl;
    stream << endl;
}

static void writeFileFooter(TQTextStream& stream)
{
    stream << "// End of File" << endl;
    stream << endl;
}

static void openIncludeGuard(const TQString& className, TQTextStream& stream)
{
    stream << "#if !defined(" << className.upper() << "_H_INCLUDED)" << endl;
    stream << "#define " << className.upper() << "_H_INCLUDED" << endl;
    stream << endl;
}

static void closeIncludeGuard(const TQString& className, TQTextStream& stream)
{
    stream << "#endif //" << className.upper() << "_H_INCLUDED" << endl;
    stream << endl;
}

static void openNamespaces(const TQStringList& namespaces, TQTextStream& stream)
{
    TQStringList::const_iterator it    = namespaces.begin();
    TQStringList::const_iterator endIt = namespaces.end();
    for (; it != endIt; ++it)
    {
        stream << "namespace " << *it << endl;
        stream << "{" << endl;
    }
    stream << endl;
}

static void closeNamespaces(const TQStringList& namespaces, TQTextStream& stream)
{
    TQStringList::const_iterator it    = namespaces.end();
    TQStringList::const_iterator endIt = namespaces.end();
    for (--it; it != endIt; --it)
    {
        stream << "}; // namespace " << *it << endl;
        stream << endl;
    }
}

static void writeIncludes(const TQString& description, const TQStringList& includes,
        TQTextStream& stream)
{
    if (includes.isEmpty()) return;

    stream << "// " << description << " includes" << endl;

    TQStringList::const_iterator it    = includes.begin();
    TQStringList::const_iterator endIt = includes.end();
    for (;it != endIt; ++it)
    {
        stream << "#include " << *it << endl;
    }

    stream << endl;
}

static void extractHeaderIncludes(const Method& method,
        TQMap<TQString, Set>& includes)
{
    TQValueList<Argument>::const_iterator it    = method.arguments.begin();
    TQValueList<Argument>::const_iterator endIt = method.arguments.end();
    for (; it != endIt; ++it)
    {
        if ((*it).headerIncludes.isEmpty()) continue;

        TQMap<TQString, TQStringList>::const_iterator mapIt =
            (*it).headerIncludes.begin();
        TQMap<TQString, TQStringList>::const_iterator mapEndIt =
            (*it).headerIncludes.end();

        for (; mapIt != mapEndIt; ++mapIt)
        {
            includes[mapIt.key()].insertStringList(mapIt.data());
        }
    }
}

static void extractForwardDeclarations(const Method& method, Set& forwards)
{
    TQValueList<Argument>::const_iterator it    = method.arguments.begin();
    TQValueList<Argument>::const_iterator endIt = method.arguments.end();
    for (; it != endIt; ++it)
    {
        if ((*it).forwardDeclarations.isEmpty()) continue;

        forwards.insertStringList((*it).forwardDeclarations);
    }
}

static void writeHeaderIncludes(const Class& classData, Class::Role role,
        TQTextStream& stream)
{
    TQMap<TQString, Set> includes;
    Set forwards;

    TQValueList<Method>::const_iterator it    = classData.methods.begin();
    TQValueList<Method>::const_iterator endIt = classData.methods.end();
    for (; it != endIt; ++it)
    {
        if ((*it).arguments.isEmpty()) continue;

        extractHeaderIncludes(*it, includes);
        extractForwardDeclarations(*it, forwards);
    }

    it    = classData.msignals.begin();
    endIt = classData.msignals.end();
    for (; it != endIt; ++it)
    {
        if ((*it).arguments.isEmpty()) continue;

        extractHeaderIncludes(*it, includes);
        extractForwardDeclarations(*it, forwards);
    }


    TQValueList<Property>::const_iterator propertyIt = classData.properties.begin();
    TQValueList<Property>::const_iterator propertyEndIt = classData.properties.end();
    for (; propertyIt != propertyEndIt; ++propertyIt)
    {
        if (!(*propertyIt).headerIncludes.isEmpty())
        {
            TQMap<TQString, TQStringList>::const_iterator mapIt =
                (*propertyIt).headerIncludes.begin();
            TQMap<TQString, TQStringList>::const_iterator mapEndIt =
                (*propertyIt).headerIncludes.end();

            for (; mapIt != mapEndIt; ++mapIt)
            {
                includes[mapIt.key()].insertStringList(mapIt.data());
            }
        }

        if (!(*propertyIt).forwardDeclarations.isEmpty())
        {
            forwards.insertStringList((*propertyIt).forwardDeclarations);
        }
    }

    switch (role)
    {
        case Class::Interface:
            includes["tqdbus"].insertString("<tqdbusobject.h>");
            forwards.insertString("class TQT_DBusError");
            forwards.insertString("class TQDomElement");
            if (!classData.msignals.isEmpty())
                forwards.insertString("class TQString");
            if (!classData.asyncMethods.isEmpty())
            {
                includes["TQt"].insertString("<tqmap.h>");
                forwards.erase("template <typename K, typename V> class TQMap");

                includes["tqdbus"].insertString("<tqdbusmessage.h>");
                forwards.erase("class TQT_DBusMessage");
            }
            break;

        case Class::Proxy:
            includes["TQt"].insertString("<tqobject.h>");
            forwards.insertString("class TQT_DBusConnection");
            forwards.insertString("class TQT_DBusError");
            forwards.insertString("class TQT_DBusMessage");
            forwards.insertString("class TQT_DBusProxy");
            forwards.insertString("class TQString");
            if (!classData.properties.isEmpty())
                forwards.insertString("class TQT_DBusVariant");
            if (!classData.asyncMethods.isEmpty())
            {
                includes["TQt"].insertString("<tqmap.h>");
                forwards.erase("template <typename K, typename V> class TQMap");
            }
            break;

        case Class::Node:
            includes["tqdbus"].insertString("<tqdbusobject.h>");
            forwards.insertString("class TQT_DBusConnection");
            forwards.insertString("class TQString");
            break;
    }

    includes["tqdbus"].insertString("<tqdbuserror.h>");

    if (!includes["TQt"].isEmpty())
        writeIncludes("TQt", includes["TQt"].keys(), stream);

    if (!includes["tqdbus"].isEmpty())
        writeIncludes("TQt D-Bus", includes["tqdbus"].keys(), stream);

    if (!includes["local"].isEmpty())
        writeIncludes("local", includes["local"].keys(), stream);

    stream << "// forward declarations" << endl;
    Set::const_iterator setIt    = forwards.begin();
    Set::const_iterator setEndIt = forwards.end();
    for (; setIt != setEndIt; ++setIt)
    {
        stream << setIt.key() << ";" << endl;
    }
    stream << endl;
}

static void extractSourceIncludes(const Method& method,
        TQMap<TQString, Set>& includes)
{
    TQValueList<Argument>::const_iterator it    = method.arguments.begin();
    TQValueList<Argument>::const_iterator endIt = method.arguments.end();
    for (; it != endIt; ++it)
    {
        if ((*it).sourceIncludes.isEmpty()) continue;

        TQMap<TQString, TQStringList>::const_iterator mapIt =
            (*it).sourceIncludes.begin();
        TQMap<TQString, TQStringList>::const_iterator mapEndIt =
            (*it).sourceIncludes.end();

        for (; mapIt != mapEndIt; ++mapIt)
        {
            includes[mapIt.key()].insertStringList(mapIt.data());
        }
    }
}

static void writeSourceIncludes(const Class& classData, Class::Role role,
        TQTextStream& stream)
{
    TQMap<TQString, Set> includes;

    TQValueList<Method>::const_iterator it    = classData.methods.begin();
    TQValueList<Method>::const_iterator endIt = classData.methods.end();
    for (; it != endIt; ++it)
    {
        if ((*it).arguments.isEmpty()) continue;

        extractSourceIncludes(*it, includes);
    }

    it    = classData.msignals.begin();
    endIt = classData.msignals.end();
    for (; it != endIt; ++it)
    {
        if ((*it).arguments.isEmpty()) continue;

        extractSourceIncludes(*it, includes);
    }

    TQValueList<Property>::const_iterator propertyIt = classData.properties.begin();
    TQValueList<Property>::const_iterator propertyEndIt = classData.properties.end();
    for (; propertyIt != propertyEndIt; ++propertyIt)
    {
        if ((*propertyIt).sourceIncludes.isEmpty()) continue;

        TQMap<TQString, TQStringList>::const_iterator mapIt =
            (*propertyIt).sourceIncludes.begin();
        TQMap<TQString, TQStringList>::const_iterator mapEndIt =
            (*propertyIt).sourceIncludes.end();

        for (; mapIt != mapEndIt; ++mapIt)
        {
            includes[mapIt.key()].insertStringList(mapIt.data());
        }
    }

    switch (role)
    {
        case Class::Interface:
            includes["TQt"].insertString("<tqdom.h>");
            includes["tqdbus"].insertString("<tqdbuserror.h>");
            includes["tqdbus"].insertString("<tqdbusmessage.h>");
            break;

        case Class::Proxy:
            includes["tqdbus"].insertString("<tqdbuserror.h>");
            includes["tqdbus"].insertString("<tqdbusmessage.h>");
            includes["tqdbus"].insertString("<tqdbusproxy.h>");
            if (!classData.properties.isEmpty())
            {
                includes["tqdbus"].insertString("<tqdbusconnection.h>");
                includes["tqdbus"].insertString("<tqdbusvariant.h>");
            }
            break;

        case Class::Node:
            includes["TQt"].insertString("<tqdom.h>");
            includes["TQt"].insertString("<tqmap.h>");
            includes["TQt"].insertString("<tqstringlist.h>");
            includes["tqdbus"].insertString("<tqdbusconnection.h>");
            includes["tqdbus"].insertString("<tqdbusmessage.h>");
            break;
    }

    if (!includes["TQt"].isEmpty())
        writeIncludes("TQt", includes["TQt"].keys(), stream);

    if (!includes["tqdbus"].isEmpty())
        writeIncludes("TQt D-Bus", includes["tqdbus"].keys(), stream);

    if (!includes["local"].isEmpty())
        writeIncludes("local", includes["local"].keys(), stream);

    stream << endl;
}

static void writeInterfaceIncludes(const TQValueList<Class> interfaces,
        const TQString& customInterfaceFilename, TQTextStream& stream)
{
    stream << "// interface classes includes" << endl;

    if (!customInterfaceFilename.isEmpty())
    {
        stream << "#include \"" << customInterfaceFilename << ".h\"" << endl;
    }

    bool hasIntrospectable = false;
    TQValueList<Class>::const_iterator it    = interfaces.begin();
    TQValueList<Class>::const_iterator endIt = interfaces.end();
    for (; it != endIt; ++it)
    {
        if (customInterfaceFilename.isEmpty())
        {
            stream << "#include \"" << (*it).name.lower() << "Interface.h\"" << endl;
        }
        if ((*it).dbusName == "org.freedesktop.DBus.Introspectable")
        {
            hasIntrospectable = true;
        }
    }

    if (!hasIntrospectable)
    {
        stream << "#include \"introspectableInterface.h\"" << endl;
    }

    stream << endl;
}

static void openClassDeclaration(const Class& classData,
    Class::Role role, TQTextStream& stream)
{
    switch (role)
    {
        case Class::Interface:
            stream << "class " << classData.name << " : public TQT_DBusObjectBase"
                   << endl;
            stream << "{" << endl;
            stream << "public:" << endl;
            stream << "    virtual ~" << classData.name << "() {}" << endl;
            stream << endl;
            stream << "    static void buildIntrospectionData(TQDomElement& interfaceElement);" << endl;
            break;

        case Class::Proxy:
            stream << "class " << classData.name << " : public TQObject" << endl;
            stream << "{" << endl;
            stream << "    TQ_OBJECT" << endl;
            stream << "    " << endl;
            stream << "public:" << endl;
            stream << "    " << classData.name
                   << "(const TQString& service, const TQString& path, TQObject* parent = 0, const char* name = 0);" << endl;
            stream << endl;

            stream << "    virtual ~" << classData.name << "();" << endl;
            stream << endl;

            stream << "    void setConnection(const TQT_DBusConnection& connection);"
                   << endl;
            break;

        case Class::Node:
            stream << "class " << classData.name << " : public TQT_DBusObjectBase"
                   << endl;
            stream << "{" << endl;
            stream << "public:" << endl;
            stream << "    " << classData.name << "();" << endl;
            stream << endl;
            stream << "    virtual ~" << classData.name << "();" << endl;
            stream << endl;
            stream << "    bool registerObject(const TQT_DBusConnection& connection, "
                   << "const TQString& path);" << endl;
            stream << "    void addChildNode(const TQString& child);" << endl;
            stream << endl;
            stream << "    void unregisterObject();" << endl;
            stream << endl;
            stream << "protected:" << endl;
            stream << "    virtual TQT_DBusObjectBase* createInterface("
                   << "const TQString& interfaceName) = 0;" << endl;
            stream << endl;
            stream << "protected: // usually no need to reimplement" << endl;
            stream << "    virtual bool handleMethodCall(const TQT_DBusMessage& message);" << endl;
            stream << endl;
            stream << "private:" << endl;
            stream << "    class Private;" << endl;
            stream << "    Private* m_private;" << endl;
            break;
    }

    stream << endl;
}

static void closeClassDeclaration(const Class& classData, Class::Role role,
        TQTextStream& stream)
{
    switch (role)
    {
        case Class::Interface:
            break;

        case Class::Proxy:
            stream << "private: // Hiding copy constructor and assignment operator" << endl;
            stream << "    " << classData.name << "(const "
                   << classData.name << "&);" << endl;
            stream << "    " << classData.name << "& operator=(const "
                   << classData.name << "&);" << endl;
            break;

        case Class::Node:
            stream << "private: // Hiding copy constructor and assignment operator" << endl;
            stream << "    " << classData.name << "(const "
                   << classData.name << "&);" << endl;
            stream << "    " << classData.name << "& operator=(const "
                   << classData.name << "&);" << endl;
            break;
    }
    stream << "}; // class " << classData.name << endl;
    stream << endl;
}

static void writeMethodDeclarations(const Class& classData, Class::Role role,
        TQTextStream& stream)
{
    if (role == Class::Interface && !classData.asyncReplyMethods.isEmpty())
    {
        stream << "public:" << endl;

        TQValueList<Method>::const_iterator it =
            classData.asyncReplyMethods.begin();
        TQValueList<Method>::const_iterator endIt =
            classData.asyncReplyMethods.end();
        for (; it != endIt; ++it)
        {
            Method method = *it;
            method.name += "AsyncReply";

            stream << "    virtual void ";
            MethodGenerator::writeMethodDeclaration(method, false, false, stream);

            stream << "    virtual void " << (*it).name
                   << "AsyncError(int asyncCallId, const TQT_DBusError& error);"
                   << endl;
            stream << endl;
        }
    }

    if (!classData.methods.isEmpty() || !classData.asyncMethods.isEmpty())
    {
        bool pureVirtual = true;
        switch (role)
        {
            case Class::Interface:
                pureVirtual = true;
                stream << "protected:" << endl;
                break;

            case Class::Proxy:
                pureVirtual = false;
                stream << "public:" << endl;
                break;

            case Class::Node: // no variable methods
                break;
        }

        TQValueList<Method>::const_iterator it    = classData.methods.begin();
        TQValueList<Method>::const_iterator endIt = classData.methods.end();
        for (; it != endIt; ++it)
        {
            if ((*it).async) continue;

            stream << "    virtual bool ";
            MethodGenerator::writeMethodDeclaration(*it, pureVirtual, true, stream);
        }

        it    = classData.asyncMethods.begin();
        endIt = classData.asyncMethods.end();
        for (; it != endIt; ++it)
        {
            Method method = *it;
            method.name += "Async";

            switch (role)
            {
                case Class::Interface:
                    stream << "    virtual void ";
                    MethodGenerator::writeMethodDeclaration(method, pureVirtual, false, stream);
                    break;

                case Class::Proxy:
                    stream << "    virtual bool ";
                    MethodGenerator::writeMethodDeclaration(method, pureVirtual, true, stream);
                    break;

                case Class::Node: // no async methods
                    break;
            }
        }
    }

    if (!classData.properties.isEmpty())
    {
        bool pureVirtual = true;
        bool skip = false;
        switch (role)
        {
            case Class::Interface:
                tqWarning("Properties not yet supported for interfaces");
                skip = true;
                pureVirtual = true;
                break;

            case Class::Proxy:
                pureVirtual = false;
                stream << "public:" << endl;
                stream << "    virtual void setDBusProperty(const TQString& name,"
                       << " const TQT_DBusVariant& variant, TQT_DBusError& error);"
                       << endl;
                stream << "    virtual TQT_DBusVariant getDBusProperty(const TQString& name, TQT_DBusError& error) const;" << endl;
                stream << endl;
                break;

            case Class::Node: // no node properties
                skip = true;
                break;
        }

        if (!skip)
        {
            TQValueList<Property>::const_iterator it    = classData.properties.begin();
            TQValueList<Property>::const_iterator endIt = classData.properties.end();
            for (; it != endIt; ++it)
            {
                MethodGenerator::writePropertyDeclaration(*it, pureVirtual, stream);
            }
        }
    }

    switch (role)
    {
        case Class::Interface:
            if (!classData.methods.isEmpty() || !classData.asyncMethods.isEmpty())
            {
                stream << "protected: // implement sending replies" << endl;
                stream << "    virtual void handleMethodReply(const TQT_DBusMessage& reply) = 0;" << endl;
                stream << endl;
                stream << "protected: // usually no need to reimplement" << endl;
                stream << "    virtual bool handleMethodCall(const TQT_DBusMessage& message);" << endl;
            }
            else
            {
                stream << "protected: // no methods to handle" << endl;
                stream << "    virtual bool handleMethodCall(const TQT_DBusMessage&) { return false; }" << endl;
            }
            break;

        case Class::Proxy:
        {
            if (!classData.msignals.isEmpty())
            {
                stream << "protected slots: // usually no need to reimplement" << endl;
                stream << "    virtual void slotHandleDBusSignal(const TQT_DBusMessage& message);" << endl;
                stream << endl;
            }

            if (!classData.asyncReplySignals.isEmpty())
            {
                if (classData.msignals.isEmpty())
                {
                    stream << "protected slots: // usually no need to reimplement" << endl;
                }
                stream << "    virtual void slotHandleAsyncReply(int id, const TQT_DBusMessage& message);" << endl;
                stream << endl;
            }

            stream << "protected:" << endl;
            stream << "    TQT_DBusProxy* m_baseProxy;" << endl;

            if (!classData.asyncMethods.isEmpty())
            {
                stream << endl;
                stream << "    TQMap<int, TQString> m_asyncCalls;" << endl;
            }

            break;
        }

        case Class::Node: // not variable methods
            break;
    }

    stream << endl;
}

static void writeSignalDeclarations(const Class& classData, Class::Role role,
        TQTextStream& stream)
{
    if (classData.msignals.isEmpty() && classData.asyncReplySignals.isEmpty())
        return;

    TQString prefix;
    switch (role)
    {
        case Class::Interface:
            stream << "protected: // implement sending signals" << endl;
            stream << "    virtual bool handleSignalSend(const TQT_DBusMessage& reply) = 0;" << endl;
            stream << "    virtual TQString objectPath() const = 0;" << endl;
            stream << endl;
            stream << "protected: // for sending D-Bus signals" << endl;
            prefix = "    virtual bool emit";
            break;

        case Class::Proxy:
            stream << "signals:" << endl;
            if (!classData.asyncReplySignals.isEmpty())
            {
                stream << "    void AsyncErrorResponseDetected(int asyncCallId, "
                        << "const TQT_DBusError error);" << endl << endl;
            }
            prefix = "    void ";
            break;

        case Class::Node: // no signals
            break;
    }

    TQValueList<Method>::const_iterator it    = classData.msignals.begin();
    TQValueList<Method>::const_iterator endIt = classData.msignals.end();
    for (; it != endIt; ++it)
    {
        stream << prefix;
        MethodGenerator::writeMethodDeclaration(*it, false, false, stream);
    }

    it    = classData.asyncReplySignals.begin();
    endIt = classData.asyncReplySignals.end();
    for (; it != endIt; ++it)
    {
        stream << prefix;

        Method signal = *it;
        signal.name += "AsyncReply";

        MethodGenerator::writeMethodDeclaration(signal, false, false, stream);
    }

    stream << endl;
}

static void writeSignalEmitters(const Class& classData, TQTextStream& stream)
{
    if (classData.msignals.isEmpty()) return;

    TQValueList<Method>::const_iterator it    = classData.msignals.begin();
    TQValueList<Method>::const_iterator endIt = classData.msignals.end();
    for (; it != endIt; ++it)
    {
        MethodGenerator::writeSignalEmitter(classData, *it, stream);
    }

    stream << endl;
}

static void writeMethodCallDeclarations(const Class& classData,
        TQTextStream& stream)
{
    TQValueList<Method>::const_iterator it    = classData.methods.begin();
    TQValueList<Method>::const_iterator endIt = classData.methods.end();
    for (; it != endIt; ++it)
    {
        stream << "    ";
        MethodGenerator::writeMethodCallDeclaration(*it, stream);
    }

    if (!classData.asyncReplyMethods.isEmpty())
    {
        stream << "protected:" << endl;
        stream << "    TQMap<int, TQT_DBusMessage> m_asyncCalls;" << endl;
        stream << endl;
    }
}

static void writeInterfaceAsyncReplyHandlers(const Class& classData,
    TQTextStream& stream)
{
    if (classData.asyncReplyMethods.isEmpty()) return;

    TQValueList<Method>::const_iterator it    = classData.asyncReplyMethods.begin();
    TQValueList<Method>::const_iterator endIt = classData.asyncReplyMethods.end();
    for (; it != endIt; ++it)
    {
        MethodGenerator::writeInterfaceAsyncReplyHandler(classData, *it, stream);
    }
}

static void writeMethodCalls(const Class& classData, TQTextStream& stream)
{
    TQValueList<Method>::const_iterator it    = classData.methods.begin();
    TQValueList<Method>::const_iterator endIt = classData.methods.end();
    for (; it != endIt; ++it)
    {
        if ((*it).async) continue;

        MethodGenerator::writeMethodCall(classData, *it, stream);
    }

    it    = classData.asyncMethods.begin();
    endIt = classData.asyncMethods.end();
    for (; it != endIt; ++it)
    {
        MethodGenerator::writeMethodCall(classData, *it, stream);
    }
}

static void writeProxyMethods(const Class& classData, TQTextStream& stream)
{
    TQValueList<Method>::const_iterator it    = classData.methods.begin();
    TQValueList<Method>::const_iterator endIt = classData.methods.end();
    for (; it != endIt; ++it)
    {
        if ((*it).async) continue;

        MethodGenerator::writeProxyMethod(classData.name, *it, stream);
    }

    it    = classData.asyncMethods.begin();
    endIt = classData.asyncMethods.end();
    for (; it != endIt; ++it)
    {
        MethodGenerator::writeProxyMethod(classData.name, *it, stream);
    }
}

static void writeProxyProperties(const Class& classData, TQTextStream& stream)
{
    if (classData.properties.isEmpty()) return;

    MethodGenerator::writeProxyGenericProperty(classData, stream);

    TQValueList<Property>::const_iterator it    = classData.properties.begin();
    TQValueList<Property>::const_iterator endIt = classData.properties.end();
    for (; it != endIt; ++it)
    {
        MethodGenerator::writeProxyProperty(classData, *it, stream);
    }
}

static void splitAsyncProxyMethods(Class& classData)
{
    // create the async identifier
    Argument idArgMethod;
    idArgMethod.name = "asyncCallId";
    idArgMethod.signature = "int";
    idArgMethod.isPrimitive = true;
    idArgMethod.direction = Argument::Out;

    Argument idArgSignal = idArgMethod;
    idArgSignal.direction = Argument::In;

    TQValueList<Method>::iterator it    = classData.methods.begin();
    TQValueList<Method>::iterator endIt = classData.methods.end();
    for (; it != endIt; ++it)
    {
        if (!(*it).async) continue;

        Method method = *it;

        TQValueList<Argument> methodArgs;
        TQValueList<Argument> signalArgs;

        // add id argument
        methodArgs << idArgMethod;
        signalArgs << idArgSignal;

        // split in/out arguments: "in" belong to the method, "out" to the new signal
        TQValueList<Argument>::const_iterator argIt    = method.arguments.begin();
        TQValueList<Argument>::const_iterator argEndIt = method.arguments.end();
        for (; argIt != argEndIt; ++argIt)
        {
            if ((*argIt).direction == Argument::Out)
            {
                // signal parameters are "out" but have "in" signature,
                // e.g. "const T&"
                Argument arg = *argIt;
                arg.direction = Argument::In;

                signalArgs << arg;
            }
            else
                methodArgs << *argIt;
        }

        // change method
        method.arguments = methodArgs;

        classData.asyncMethods << method;

        // create "callback" signal
        Method signal = method;
        signal.arguments = signalArgs;

        classData.asyncReplySignals << signal;
    }
}

static void splitAsyncInterfaceMethods(Class& classData)
{
    // create the async identifier
    Argument idArgMethod;
    idArgMethod.name = "asyncCallId";
    idArgMethod.signature = "int";
    idArgMethod.isPrimitive = true;
    idArgMethod.direction = Argument::In;

    Argument idArgReply = idArgMethod;

    TQValueList<Method>::iterator it    = classData.methods.begin();
    TQValueList<Method>::iterator endIt = classData.methods.end();
    for (; it != endIt; ++it)
    {
        if (!(*it).async) continue;

        Method method = *it;

        TQValueList<Argument> methodArgs;
        TQValueList<Argument> replyArgs;

        // add id argument
        methodArgs << idArgMethod;
        replyArgs  << idArgReply;

        // split in/out arguments: "in" belong to the call, "out" to the reply
        TQValueList<Argument>::const_iterator argIt    = method.arguments.begin();
        TQValueList<Argument>::const_iterator argEndIt = method.arguments.end();
        for (; argIt != argEndIt; ++argIt)
        {
            if ((*argIt).direction == Argument::Out)
            {
                // reply parameters are "out" for the service but "in" for
                // the reply handler
                Argument arg = *argIt;
                arg.direction = Argument::In;

                replyArgs << arg;
            }
            else
                methodArgs << *argIt;
        }

        // change method
        method.arguments = methodArgs;

        classData.asyncMethods << method;

        // create reply handler
        Method reply = method;
        reply.arguments = replyArgs;

        classData.asyncReplyMethods << reply;
    }
}

bool ClassGenerator::initStreams(const TQString& baseName,
                                 TQTextStream& headerStream,
                                 TQTextStream& sourceStream)
{
    TQFile* headerFile = new TQFile(baseName + ".h");
    TQFile* sourceFile = new TQFile(baseName + ".cpp");

    if (!headerFile->open(IO_WriteOnly) || !sourceFile->open(IO_WriteOnly))
    {
        delete headerFile;
        delete sourceFile;

        return false;
    }

    headerStream.setDevice(headerFile);
    sourceStream.setDevice(sourceFile);

    // create header
    writeFileHeader(headerStream);
    openIncludeGuard(baseName, headerStream);

    // create source
    writeFileHeader(sourceStream);
    sourceStream << "// declaration include" << endl;
    sourceStream << "#include \"" << baseName << ".h\"" << endl;
    sourceStream << endl;

    return true;
}

bool ClassGenerator::finishStreams(const TQString& baseName,
                                   TQTextStream& headerStream,
                                   TQTextStream& sourceStream)
{
    closeIncludeGuard(baseName, headerStream);
    writeFileFooter(headerStream);
    writeFileFooter(sourceStream);

    TQIODevice* device = headerStream.device();
    headerStream.unsetDevice();
    delete device;

    device = sourceStream.device();
    sourceStream.unsetDevice();
    delete device;

    return true;
}

bool ClassGenerator::extractClass(const TQDomElement& interfaceElement,
                                   Class& classData)
{
    tqDebug("ClassGenerator: processing interface '%s'",
           interfaceElement.attribute("name").latin1());

    classData.dbusName = interfaceElement.attribute("name");

    TQStringList nameParts = TQStringList::split('.', classData.dbusName);

    if (nameParts.count() < 2) return false;

    classData.name = nameParts.back();
    nameParts.pop_back();
    classData.namespaces = nameParts;

    return MethodGenerator::extractMethods(interfaceElement, classData);
}

bool ClassGenerator::generateInterface(const Class& classData,
                                       TQTextStream& headerStream,
                                       TQTextStream& sourceStream)
{
    Class classDataCopy = classData;
    splitAsyncInterfaceMethods(classDataCopy);

    // create header
    writeHeaderIncludes(classDataCopy, Class::Interface, headerStream);

    openNamespaces(classDataCopy.namespaces, headerStream);
    openClassDeclaration(classDataCopy, Class::Interface, headerStream);

    writeSignalDeclarations(classDataCopy, Class::Interface, headerStream);
    writeMethodDeclarations(classDataCopy, Class::Interface, headerStream);
    writeMethodCallDeclarations(classDataCopy, headerStream);

    closeClassDeclaration(classDataCopy, Class::Interface, headerStream);
    closeNamespaces(classDataCopy.namespaces, headerStream);

    // create source
    writeSourceIncludes(classDataCopy, Class::Interface, sourceStream);

    openNamespaces(classDataCopy.namespaces, sourceStream);

    MethodGenerator::writeIntrospectionDataMethod(classDataCopy, sourceStream);

    writeSignalEmitters(classDataCopy, sourceStream);
    writeInterfaceAsyncReplyHandlers(classDataCopy, sourceStream);
    writeMethodCalls(classDataCopy, sourceStream);

    MethodGenerator::writeInterfaceMainMethod(classDataCopy, sourceStream);

    closeNamespaces(classDataCopy.namespaces, sourceStream);

    return true;
}

bool ClassGenerator::generateProxy(const Class& classData,
                                   TQTextStream& headerStream,
                                   TQTextStream& sourceStream)
{
    Class classDataCopy = classData;
    splitAsyncProxyMethods(classDataCopy);

    // create header
    writeHeaderIncludes(classDataCopy, Class::Proxy, headerStream);

    openNamespaces(classDataCopy.namespaces, headerStream);
    openClassDeclaration(classDataCopy, Class::Proxy, headerStream);

    writeSignalDeclarations(classDataCopy, Class::Proxy, headerStream);
    writeMethodDeclarations(classDataCopy, Class::Proxy, headerStream);

    closeClassDeclaration(classDataCopy, Class::Proxy, headerStream);
    closeNamespaces(classDataCopy.namespaces, headerStream);

    // create source
    writeSourceIncludes(classDataCopy, Class::Proxy, sourceStream);

    openNamespaces(classDataCopy.namespaces, sourceStream);

    MethodGenerator::writeProxyBegin(classDataCopy, sourceStream);

    writeProxyMethods(classDataCopy, sourceStream);

    writeProxyProperties(classDataCopy, sourceStream);

    if (!classDataCopy.msignals.isEmpty())
        MethodGenerator::writeSignalHandler(classDataCopy, sourceStream);

    if (!classDataCopy.asyncReplySignals.isEmpty())
        MethodGenerator::writeProxyAsyncReplyHandler(classDataCopy, sourceStream);

    closeNamespaces(classDataCopy.namespaces, sourceStream);

    return true;
}

bool ClassGenerator::generateNode(const Class& classData,
                                  const TQValueList<Class>& interfaces,
                                  const TQString& customInterfaceFilename,
                                  TQTextStream& headerStream,
                                  TQTextStream& sourceStream)
{
    // create header
    writeHeaderIncludes(classData, Class::Node, headerStream);

    openNamespaces(classData.namespaces, headerStream);
    openClassDeclaration(classData, Class::Node, headerStream);

    closeClassDeclaration(classData, Class::Node, headerStream);
    closeNamespaces(classData.namespaces, headerStream);

    // create source
    writeSourceIncludes(classData, Class::Node, sourceStream);
    writeInterfaceIncludes(interfaces, customInterfaceFilename, sourceStream);

    openNamespaces(classData.namespaces, sourceStream);

    MethodGenerator::writeNodePrivate(classData, sourceStream);

    MethodGenerator::writeNodeBegin(classData, sourceStream);

    MethodGenerator::writeNodeMethods(classData, interfaces, sourceStream);

    closeNamespaces(classData.namespaces, sourceStream);

    return true;
}

bool ClassGenerator::generateIncludeMoc(const TQString& baseName, TQTextStream& stream)
{
    stream << "#include \"" << baseName << ".moc\"" << endl;
    stream << endl;
    return true;
}

// End of File
