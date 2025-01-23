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
#include <tqtextstream.h>

// local includes
#include "methodgen.h"

static bool parseDBusSignature(const TQString& signature, Argument& argument)
{
    argument.dbusSignature = signature;

    if (signature.length() == 1)
    {
        if (signature == "b")
        {
            argument.signature = "bool";
            argument.accessor = "Bool";
            argument.isPrimitive = true;
        }
        else if (signature == "y")
        {
            argument.signature = "TQ_UINT8";
            argument.accessor = "Byte";
            argument.isPrimitive = true;
        }
        else if (signature == "n")
        {
            argument.signature = "TQ_INT16";
            argument.accessor = "Int16";
            argument.isPrimitive = true;
        }
        else if (signature == "q")
        {
            argument.signature = "TQ_UINT16";
            argument.accessor = "UInt16";
            argument.isPrimitive = true;
        }
        else if (signature == "i")
        {
            argument.signature = "TQ_INT32";
            argument.accessor = "Int32";
            argument.isPrimitive = true;
        }
        else if (signature == "u")
        {
            argument.signature = "TQ_UINT32";
            argument.accessor = "UInt32";
            argument.isPrimitive = true;
        }
        else if (signature == "x")
        {
            argument.signature = "TQ_INT64";
            argument.accessor = "Int64";
            argument.isPrimitive = true;
        }
        else if (signature == "t")
        {
            argument.signature = "TQ_UINT64";
            argument.accessor = "UInt64";
            argument.isPrimitive = true;
        }
        else if (signature == "d")
        {
            argument.signature = "double";
            argument.accessor = "Double";
            argument.isPrimitive = true;
        }
        else if (signature == "s")
        {
            argument.signature = "TQString";
            argument.accessor = "String";
            argument.isPrimitive = false;

            argument.forwardDeclarations.append("class TQString");
            argument.sourceIncludes["TQt"].append("<tqstring.h>");
        }
        else if (signature == "o")
        {
            argument.signature = "TQT_DBusObjectPath";
            argument.accessor = "ObjectPath";
            argument.isPrimitive = false;

            argument.forwardDeclarations.append("class TQT_DBusObjectPath");
            argument.sourceIncludes["tqdbus"].append("<tqdbusobjectpath.h>");
        }
        else if (signature == "h")
        {
            argument.signature = "TQT_DBusUnixFd";
            argument.accessor = "UnixFd";
            argument.isPrimitive = false;

            argument.forwardDeclarations.append("class TQT_DBusUnixFd");
            argument.sourceIncludes["tqdbus"].append("<tqdbusunixfd.h>");
        }
        else if (signature == "v")
        {
            argument.signature = "TQT_DBusVariant";
            argument.accessor = "Variant";
            argument.isPrimitive = false;

            argument.forwardDeclarations.append("class TQT_DBusVariant");
            argument.sourceIncludes["tqdbus"].append("<tqdbusvariant.h>");
        }
        else
            return false;
    }
    else if (signature.startsWith("a"))
    {
        if (signature == "as")
        {
            argument.signature = "TQStringList";
            argument.accessor = "List";
            argument.subAccessor = "TQStringList";
            argument.isPrimitive = false;

            argument.forwardDeclarations.append("class TQStringList");

            argument.sourceIncludes["tqdbus"].append("<tqdbusdatalist.h>");
            argument.sourceIncludes["TQt"].append("<tqstringlist.h>");
        }
        else if (signature.startsWith("a{"))
        {
            int from = signature.find("{");
            int to = signature.findRev("}");
            if (from == -1 || to == -1 || (to - from - 1) < 2) return false;

            TQString dictSignature = signature.mid(from + 1, (to - from - 1));

            Argument key;
            if (!parseDBusSignature(dictSignature.left(1), key)) return false;

            Argument value;
            if (parseDBusSignature(dictSignature.mid(1), value) && !dictSignature.startsWith("oa"))
            {
                if (!value.subAccessor.isEmpty())
                {
                    argument.isPrimitive = false;
                    argument.containerClass = "TQT_DBusDataMap< " + key.signature + " >";
                    argument.signature = "TQT_DBusDataMap< " + key.signature + " >";
                    argument.accessor = key.accessor + "KeyMap";

                    argument.forwardDeclarations.append("template <typename K> class TQT_DBusDataMap");
                    argument.forwardDeclarations += key.forwardDeclarations;

                    argument.sourceIncludes = key.sourceIncludes;
                    argument.sourceIncludes["tqdbus"].append("<tqdbusdata.h>");
                    argument.sourceIncludes["tqdbus"].append("<tqdbusdatamap.h>");
                }
                else
                {
                    argument.isPrimitive = false;
                    argument.containerClass = "TQT_DBusDataMap< " + key.signature + " >";
                    argument.signature = "TQMap< " + key.signature +
                                        ", " + value.signature + " >";
                    argument.accessor = key.accessor + "KeyMap";
                    argument.subAccessor = value.accessor + "Map";

                    argument.forwardDeclarations.append("template <typename K, typename V> class TQMap");
                    argument.forwardDeclarations += key.forwardDeclarations;
                    argument.forwardDeclarations += value.forwardDeclarations;

                    argument.sourceIncludes = key.sourceIncludes;
                    argument.sourceIncludes["TQt"].append("<tqmap.h>");
                    argument.sourceIncludes["tqdbus"].append("<tqdbusdata.h>");
                    argument.sourceIncludes["tqdbus"].append("<tqdbusdatamap.h>");

                    TQMap<TQString, TQStringList>::const_iterator it =
                        value.sourceIncludes.begin();
                    TQMap<TQString, TQStringList>::const_iterator endIt =
                        value.sourceIncludes.end();
                    for (; it != endIt; ++it)
                    {
                        argument.sourceIncludes[it.key()] += it.data();
                    }
                }
            }
            else
            {
                argument.isPrimitive = false;
                argument.containerClass = "TQT_DBusDataMap< " + key.signature + " >";
                argument.signature = "TQT_DBusDataMap< " + key.signature + " >";
                argument.accessor = key.accessor + "KeyMap";

                argument.forwardDeclarations.append("template <typename K> class TQT_DBusDataMap");
                argument.forwardDeclarations += key.forwardDeclarations;

                argument.sourceIncludes = key.sourceIncludes;
                argument.sourceIncludes["tqdbus"].append("<tqdbusdata.h>");
                argument.sourceIncludes["tqdbus"].append("<tqdbusdatamap.h>");
            }
        }
        else
        {
            TQString itemSignature = signature.mid(1);

            Argument item;
            if (parseDBusSignature(itemSignature, item) && !itemSignature.startsWith("a"))
            {
                argument.isPrimitive = false;
                argument.signature = "TQValueList< " + item.signature + " >";
                argument.accessor = "List";
                argument.subAccessor = item.accessor + "List";
                argument.containerClass = "TQT_DBusDataList";

                argument.forwardDeclarations.append("class TQT_DBusDataList");
                argument.forwardDeclarations.append("template <typename T> class TQValueList");
                argument.forwardDeclarations += item.forwardDeclarations;

                argument.sourceIncludes["TQt"].append("<tqvaluelist.h>");
                argument.sourceIncludes["tqdbus"].append("<tqdbusdatalist.h>");

                TQMap<TQString, TQStringList>::const_iterator it =
                    item.sourceIncludes.begin();
                TQMap<TQString, TQStringList>::const_iterator endIt =
                    item.sourceIncludes.end();
                for (; it != endIt; ++it)
                {
                    argument.sourceIncludes[it.key()] += it.data();
                }
            }
            else
            {
                argument.signature = "TQT_DBusDataList";
                argument.accessor = "List";
                argument.isPrimitive = false;

                argument.forwardDeclarations.append("class TQT_DBusDataList");

                argument.sourceIncludes["tqdbus"].append("<tqdbusdatalist.h>");
            }
        }
    }
    else
        return false;

    return true;
}

static TQMap<TQString, TQString> extractTypeAnnotations(const TQDomElement& element)
{
    const TQString annotationPrefix = "org.freedesktop.DBus.TQt3.Type.";

    TQMap<TQString, TQString> annotations;

    TQDomNode node = element.firstChild();
    for (uint count = 1; !node.isNull(); node = node.nextSibling(), ++count)
    {
        if (!node.isElement()) continue;

        TQDomElement element = node.toElement();
        if (element.tagName() != "annotation") continue;

        TQString name = element.attribute("name");
        if (name.isEmpty()) continue;

        TQString value = element.attribute("value").stripWhiteSpace();
        if (value.isEmpty()) continue;

        if (!name.startsWith(annotationPrefix)) continue;

        TQString arg = name.mid(annotationPrefix.length());

        annotations.insert(arg, value);
    }

    return annotations;
}

static bool hasAnnotation(const TQDomElement& element, const TQString& annotation, TQString* value = 0)
{
    for (TQDomNode node = element.firstChild(); !node.isNull();
         node = node.nextSibling())
    {
        if (!node.isElement()) continue;

        TQDomElement childElement = node.toElement();
        if (childElement.tagName() != "annotation") continue;
        if (childElement.attribute("name") != annotation) continue;

        if (value != 0) *value = childElement.attribute("value");
        return true;
    }

    return false;
}

static TQValueList<Argument> extractArguments(const TQDomElement& methodElement,
        Class& classData)
{
    TQMap<TQString, TQString> argAnnotations = extractTypeAnnotations(methodElement);

    TQValueList<Argument> arguments;

    bool isSignal = methodElement.tagName() == "signal";

    uint inCount  = 0;
    uint outCount = 0;
    for (TQDomNode node = methodElement.firstChild(); !node.isNull();
         node = node.nextSibling())
    {
        if (!node.isElement()) continue;

        TQDomElement element = node.toElement();
        if (element.tagName() != "arg") continue;
        if (element.attribute("type").isEmpty()) continue;

        Argument argument;
        argument.name = element.attribute("name");
        if (argument.name.isEmpty())
            argument.name = TQString("arg%1").arg(inCount + outCount);

        argument.direction = Argument::In;
        if (isSignal || element.attribute("direction", "in") == "out")
            argument.direction = Argument::Out;

        TQString annotation;
        if (argument.direction == Argument::In)
        {
            annotation = argAnnotations[TQString("In%1").arg(inCount)];
            ++inCount;
        }
        else
        {
            annotation = argAnnotations[TQString("Out%1").arg(outCount)];
            ++outCount;
        }

        if (!annotation.isEmpty())
        {
            // just assume nobody uses annotations for primitives
            argument.annotatedType = annotation;
            argument.signature     = annotation;
            argument.isPrimitive   = false;
            argument.dbusSignature = element.attribute("type");

            TQString includeBase =
                TQString("\"%1type%2.h\"").arg(classData.name.lower());

            argument.headerIncludes["local"].append(includeBase.arg("declarations"));
            argument.sourceIncludes["local"].append(includeBase.arg("includes"));
            argument.sourceIncludes["tqdbus"].append("<tqdbusdataconverter.h>");
        }
        else if (!parseDBusSignature(element.attribute("type"), argument))
        {
            argument.signature = "TQT_DBusData";
            argument.isPrimitive = false;

            argument.forwardDeclarations.append("class TQT_DBusData");
            argument.sourceIncludes["tqdbus"].append("<tqdbusdata.h>");
        }

        arguments.append(argument);
    }

    return arguments;
}

static void writeVariable(const Argument& argument, uint index,
        const TQString& prefix, TQTextStream& stream)
{
    stream << prefix << argument.signature << " _" << argument.name;
    if (argument.direction == Argument::In)
    {
        if (!argument.annotatedType.isEmpty())
        {
            stream << ";" << endl;

            // TODO: error handling?
            stream << prefix << "TQT_DBusDataConverter::convertFromTQT_DBusData<"
                   << argument.annotatedType
                   << TQString(">(message[%1], _").arg(index)
                   << argument.name << ")";
        }
        else if (!argument.accessor.isEmpty())
        {
            stream << TQString::fromUtf8(" = message[%1].to").arg(index);
            stream << argument.accessor;

            if (!argument.subAccessor.isEmpty())
            {
                stream << TQString("().to%1").arg(argument.subAccessor);
            }

            stream << "()";
        }
        else
            stream << TQString::fromUtf8(" = message[%1]").arg(index);
    }

    stream << ";" << endl;
}

static void writeVariables(const TQString& prefix, const Method& method,
        TQTextStream& stream)
{
    uint count = 0;
    TQValueList<Argument>::const_iterator it    = method.arguments.begin();
    TQValueList<Argument>::const_iterator endIt = method.arguments.end();
    for (; it != endIt; ++it)
    {
        writeVariable(*it, count, prefix, stream);

        if ((*it).direction == Argument::In) ++count;
    }
}

static void writeSignalEmit(const Method& signal, TQTextStream& stream)
{
    stream << "        emit " << signal.name << "(";

    TQValueList<Argument>::const_iterator it    = signal.arguments.begin();
    TQValueList<Argument>::const_iterator endIt = signal.arguments.end();
    for (; it != endIt;)
    {
        stream << "_" << (*it).name;

        ++it;
        if (it != endIt) stream << ", ";
    }

    stream << ");" << endl;
}

static void writeMethodIntrospection(const Method& method, bool& firstArgument,
    TQTextStream& stream)
{
    stream << "    methodElement.setAttribute(\"name\", \""
           << method.name << "\");" << endl;

    TQValueList<Argument>::const_iterator it    = method.arguments.begin();
    TQValueList<Argument>::const_iterator endIt = method.arguments.end();
    for (; it != endIt; ++it)
    {
        stream << endl;
        if (firstArgument)
        {
            firstArgument = false;
            stream << "    TQDomElement argumentElement = document.createElement("
                   << "\"arg\");" << endl;
        }
        else
        {
            stream << "    argumentElement = document.createElement("
                   << "\"arg\");" << endl;
        }

        stream << "    argumentElement.setAttribute(\"name\",      \""
               << (*it).name << "\");" << endl;

        stream << "    argumentElement.setAttribute(\"type\",      \""
               << (*it).dbusSignature << "\");" << endl;

        stream << "    argumentElement.setAttribute(\"direction\", \""
               << ((*it).direction == Argument::In ? "in" : "out") << "\");"
               << endl;

        stream << "    methodElement.appendChild(argumentElement);" << endl;
    }
    stream << endl;
}

static void writeNodeInitialization(const Class& classData,
        const TQValueList<Class>& interfaces, TQTextStream& stream)
{
    stream << "bool " << classData.name
           << "::registerObject(const TQT_DBusConnection& connection, "
           << "const TQString& path)" << endl;
    stream << "{" << endl;
    stream << "    if (path.isEmpty()) return false;" << endl;
    stream << endl;

    stream << "    if (!m_private->objectPath.isEmpty()) unregisterObject();"
           << endl;
    stream << endl;

    stream << "    m_private->connection = connection;" << endl;
    stream << "    m_private->objectPath = path;" << endl;
    stream << endl;
    stream << "    if (!m_private->connection.registerObject(path, this))" << endl;
    stream << "    {" << endl;
    stream << "        m_private->connection = TQT_DBusConnection();" << endl;
    stream << "        m_private->objectPath = TQString();" << endl;
    stream << endl;
    stream << "        return false;" << endl;
    stream << "    }" << endl;
    stream << endl;

    stream << "    if (m_private->interfaces.isEmpty())" << endl;
    stream << "    {" << endl;
    stream << "        TQString name = \"org.freedesktop.DBus.Introspectable\";"
           << endl;
    stream << "        TQT_DBusObjectBase* interface = m_private;" << endl;
    stream << "        m_private->interfaces.insert(name, interface);" << endl;

    TQValueList<Class>::const_iterator it    = interfaces.begin();
    TQValueList<Class>::const_iterator endIt = interfaces.end();
    for (; it != endIt; ++it)
    {
        if ((*it).dbusName == "org.freedesktop.DBus.Introspectable")
        {
            continue;
        }

        stream << endl;
        stream << "        name = \"" << (*it).dbusName << "\";" << endl;
        stream << "        interface = createInterface(name);" << endl;
        stream << "        Q_ASSERT(interface != 0);" << endl;
        stream << "        m_private->interfaces.insert(name, interface);" << endl;
    }

    stream << "    }" << endl;
    stream << endl;
    stream << "    return true;" << endl;
    stream << "}" << endl;
    stream << endl;

    stream << "void " << classData.name
           << "::addChildNode(const TQString& child)" << endl;
    stream << "{" << endl;
    stream << "    m_private->childrenNodes.append(child);" << endl;
    stream << "}" << endl;
    stream << endl;

}

static void writeNodeIntrospection(const Class& classData,
        const TQValueList<Class>& interfaces, TQTextStream& stream)
{
    stream << "void " << classData.name << "::Private"
           << "::cacheIntrospectionData()" << endl;
    stream << "{" << endl;

    stream << "    TQDomDocument doc;" << endl;
    stream << "    TQDomElement interfaceElement;" << endl;
    stream << "    TQDomElement nodeElement = doc.createElement(\"node\");" << endl;
    stream << "    if (!objectPath.isEmpty() && objectPath.compare(\"/\") != 0)" << endl;
    stream << "    {" << endl;
    stream << "         nodeElement.setAttribute ( \"name\", objectPath );" << endl;
    stream << "    }" << endl;
    stream << "    // Introspectable is added by default. Show it only if there is interface" << endl;
    stream << "    if (interfaces.count()>1) {" << endl;
    stream << "         interfaceElement = doc.createElement(\"interface\");"
           << endl;
    stream << "         org::freedesktop::DBus::IntrospectableInterface"
           << "::buildIntrospectionData(interfaceElement);" << endl;
    stream << "         nodeElement.appendChild(interfaceElement);" << endl;
    stream << "    }" << endl;

    TQValueList<Class>::const_iterator it    = interfaces.begin();
    TQValueList<Class>::const_iterator endIt = interfaces.end();
    for (; it != endIt; ++it)
    {
        if ((*it).dbusName == "org.freedesktop.DBus.Introspectable") continue;

        stream << endl;
        stream << "     interfaceElement = doc.createElement(\"interface\");"
               << endl;
        stream << "    " << (*it).namespaces.join("::") + "::" + (*it).name
               << "Interface::buildIntrospectionData(interfaceElement);" << endl;
        stream << "    nodeElement.appendChild(interfaceElement);" << endl;
    }

    stream << endl;
    stream << "    if (!childrenNodes.isEmpty()) {" << endl;
    stream << "        for (TQStringList::Iterator it = childrenNodes.begin(); it != childrenNodes.end(); ++it ) {" << endl;
    stream << "            TQDomElement nodeElement1 = doc.createElement(\"node\");" << endl;
    stream << "            nodeElement1.setAttribute ( \"name\", *it );" << endl;
    stream << "            nodeElement.appendChild(nodeElement1);" << endl;
    stream << "        }" << endl;
    stream << "    }" << endl;

    stream << endl;
    stream << "    doc.appendChild(nodeElement);" << endl;
    stream << endl;

    stream << "    introspectionData = \"<!DOCTYPE node PUBLIC \\\""
           << "-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\\\"\\n"
           << "\\\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd"
           << "\\\">\\n\";" << endl;
    stream << "    introspectionData += doc.toString();" << endl;
    stream << "}" << endl;
    stream << endl;
}

bool MethodGenerator::extractMethods(const TQDomElement& interfaceElement,
        Class& classData)
{
    TQMap<TQString, TQString> propertyAnnotations =
        extractTypeAnnotations(interfaceElement);

    uint propertyCount = 0;
    for (TQDomNode node = interfaceElement.firstChild(); !node.isNull();
         node = node.nextSibling())
    {
        if (!node.isElement()) continue;

        TQDomElement element = node.toElement();
        if (element.attribute("name").isEmpty()) continue;

        if (element.tagName() == "method")
        {
            Method method;
            method.name = element.attribute("name");
            method.type = Method::_Method;
            method.arguments = extractArguments(element, classData);
            method.noReply = false;
            method.async = hasAnnotation(element, "org.freedesktop.DBus.GLib.Async");
            classData.methods.append(method);
        }
        else if (element.tagName() == "signal")
        {
            Method method;
            method.name = element.attribute("name");
            method.type = Method::_Signal;
            method.arguments = extractArguments(element, classData);
            method.noReply = false;
            method.async = false;
            classData.msignals.append(method);
        }
        else if (element.tagName() == "property")
        {
            Property property;
            property.name  = element.attribute("name");
            property.read  = element.attribute("access").find("read")  != -1;
            property.write = element.attribute("access").find("write") != -1;

            TQString annotation =
                propertyAnnotations[TQString("Property%1").arg(propertyCount)];

            if (!annotation.isEmpty())
            {
                property.annotatedType = annotation;
                property.signature     = annotation;
                property.dbusSignature = element.attribute("type");
                property.isPrimitive   = false;

                TQString includeBase =
                    TQString("\"%1type%2.h\"").arg(classData.name.lower());

                property.headerIncludes["local"].append(includeBase.arg("declarations"));
                property.sourceIncludes["local"].append(includeBase.arg("includes"));
                property.sourceIncludes["tqdbus"].append("<tqdbusdataconverter.h>");
            }
            else if (!parseDBusSignature(element.attribute("type"), property))
            {
                property.signature = "TQT_DBusData";
                property.isPrimitive = false;

                property.forwardDeclarations.append("class TQT_DBusData");
                property.sourceIncludes["tqdbus"].append("<tqdbusdata.h>");
            }

            classData.properties.append(property);
            ++propertyCount;
        }
    }

    return !classData.methods.isEmpty() || !classData.msignals.isEmpty() ||
           !classData.properties.isEmpty();
}

void MethodGenerator::writeMethodDeclaration(const Method& method, bool pureVirtual,
        bool withError, TQTextStream& stream)
{
    stream << method.name << "(";

    TQValueList<Argument>::const_iterator it    = method.arguments.begin();
    TQValueList<Argument>::const_iterator endIt = method.arguments.end();
    for (; it != endIt;)
    {
        if (!(*it).isPrimitive && ((*it).direction == Argument::In || method.type == Method::_Signal))
        {
            stream << "const ";
        }

        stream << (*it).signature;

        if (!(*it).isPrimitive || ((*it).direction == Argument::Out && method.type != Method::_Signal))
        {
            stream << "&";
        }

        stream << " " << (*it).name;

        ++it;
        if (it != endIt || withError) stream << ", ";
    }

    if (withError)
        stream << "TQT_DBusError& error)";
    else
        stream << ")";

    if (pureVirtual)
        stream << " = 0;" << endl;
    else
        stream << ";" << endl;

    stream << endl;
}

void MethodGenerator::writePropertyDeclaration(const Property& property,
        bool pureVirtual, TQTextStream& stream)
{
    if (property.write)
    {
        stream << "    virtual void set" << property.name << "(";

        if (!property.isPrimitive) stream << "const ";

        stream << property.signature;

        if (!property.isPrimitive) stream << "&";

        stream << " value, TQT_DBusError& error)";

        if (pureVirtual)
            stream << " = 0;" << endl;
        else
            stream << ";" << endl;
    }

    if (property.read)
    {
        stream << "    virtual " << property.signature << " get"
               << property.name << "(TQT_DBusError& error) const";

        if (pureVirtual)
            stream << " = 0;" << endl;
        else
            stream << ";" << endl;
    }

    if (property.read || property.write) stream << endl;
}

void MethodGenerator::writeMethodCallDeclaration(const Method& method,
        TQTextStream& stream)
{
    if (method.async)
        stream << "void call" << method.name << "Async";
    else
        stream << "TQT_DBusMessage call" << method.name;

    stream << "(const TQT_DBusMessage& message);" << endl;
    stream << endl;
}

void MethodGenerator::writeMethodCall(const Class& classData,
        const Method& method, TQTextStream& stream)
{
    if (method.async)
        stream << "void " << classData.name << "::call" << method.name << "Async";
    else
        stream << "TQT_DBusMessage " << classData.name << "::call" << method.name;

    stream << "(const TQT_DBusMessage& message)" << endl;

    stream << "{" << endl;

    if (method.async)
    {
        // FIXME: using writeVariables by removing asyncCallId argument
        Method reducedMethod = method;
        reducedMethod.arguments.pop_front();

        writeVariables("    ", reducedMethod, stream);
    }
    else
    {
        stream << "    TQT_DBusError   error;" << endl;
        stream << "    TQT_DBusMessage reply;" << endl;
        stream << endl;

        writeVariables("    ", method, stream);
    }

    stream << endl;

    if (method.async)
    {
        stream << "    int _asyncCallId = 0;" << endl;
        stream << "    while (m_asyncCalls.find(_asyncCallId) != m_asyncCalls.end())"
               << endl;
        stream << "    {" << endl;
        stream << "        ++_asyncCallId;" << endl;
        stream << "    }" << endl;
        stream << "    m_asyncCalls.insert(_asyncCallId, message);" << endl;
        stream << endl;

        stream << "    " << method.name << "Async(";
    }
    else
        stream << "    if (" << method.name << "(";

    TQValueList<Argument>::const_iterator it    = method.arguments.begin();
    TQValueList<Argument>::const_iterator endIt = method.arguments.end();
    while (it != endIt)
    {
        stream << "_" << (*it).name;

        ++it;
        if (it != endIt) stream << ", ";
    }

    if (method.async)
    {
        stream << ");" << endl;
        stream << endl;

        stream << "    return;" << endl;
        stream << "}" << endl;
        stream << endl;
        return;
    }

	if (method.arguments.count() > 0) stream << ", ";
    stream << "error))" << endl;

    stream << "    {" << endl;
    stream << "        reply = TQT_DBusMessage::methodReply(message);" << endl;

    it = method.arguments.begin();
    for (; it != endIt; ++it)
    {
        if ((*it).direction == Argument::Out)
        {
            if (!(*it).annotatedType.isEmpty())
            {
                stream << "        TQT_DBusData " << (*it).name << "Data;" << endl;
                stream << "        TQT_DBusDataConverter::convertToTQT_DBusData<"
                       << (*it).annotatedType << ">(_"
                       << (*it).name << ", " << (*it).name << "Data);"
                       << endl;
                stream << "        reply << " << (*it).name << "Data";
            }
            else if (!(*it).accessor.isEmpty())
            {
                stream << "        reply << TQT_DBusData::from" << (*it).accessor;
                if (!(*it).subAccessor.isEmpty())
                {
                    stream << "(" << (*it).containerClass;
                }

                stream << "(_" << (*it).name << ")";

                if (!(*it).subAccessor.isEmpty())
                {
                    stream << ")";
                }
            }
            else
                stream << "        reply << _" << (*it).name;

            stream << ";" << endl;
        }
    }
    stream << "    }" << endl;
    stream << "    else" << endl;
    stream << "    {" << endl;

    stream << "        if (!error.isValid())" << endl;
    stream << "        {" << endl;
    stream << "            tqWarning(\"Call to implementation of ";

    TQStringList::const_iterator nsIt    = classData.namespaces.begin();
    TQStringList::const_iterator nsEndIt = classData.namespaces.end();
    for (; nsIt != nsEndIt; ++nsIt)
    {
        stream << *nsIt << "::";
    }

    stream  << classData.name << "::" << method.name;
    stream << " returned 'false' but error object is not valid!\");" << endl;
    stream << endl;
    stream << "            error = TQT_DBusError::stdFailed(\"";

    nsIt = classData.namespaces.begin();
    for (; nsIt != nsEndIt; ++nsIt)
    {
        stream << *nsIt << ".";
    }

    TQString interfaceStr("Interface");
    if (classData.name.endsWith(interfaceStr))
    {
      stream << classData.name.left(classData.name.length() - interfaceStr.length());
    }
    else
    {
      stream << classData.name;
    }
    stream << "." << method.name << " execution failed\");"
           << endl;
    stream << "        }" << endl;
    stream << endl;

    stream << "        reply = TQT_DBusMessage::methodError(message, error);" << endl;

    stream << "    }" << endl;
    stream << endl;
    stream << "    return reply;" << endl;
    stream << "}" << endl;
    stream << endl;
}

void MethodGenerator::writeSignalEmitter(const Class& classData,
        const Method& method, TQTextStream& stream)
{
    if (method.type != Method::_Signal)
    {
        return;
    }

    stream << "bool " << classData.name << "::emit" << method.name << "(";

    TQValueList<Argument>::const_iterator it    = method.arguments.begin();
    TQValueList<Argument>::const_iterator endIt = method.arguments.end();
    for (; it != endIt;)
    {
        if (!(*it).isPrimitive)
        {
            stream << "const ";
        }

        stream << (*it).signature;

        if (!(*it).isPrimitive)
        {
            stream << "&";
        }

        stream << " " << (*it).name;

        ++it;
        if (it != endIt)
        {
            stream << ", ";
        }
    }

    stream << ")" << endl;

    stream << "{" << endl;

    // TODO: create error or use enum for return
    stream << "    TQString path = objectPath();" << endl;
    stream << "    Q_ASSERT(!path.isEmpty());" << endl;
    stream << endl;

    stream << "    TQT_DBusMessage message = TQT_DBusMessage::signal(path, \"";
    stream << classData.dbusName << "\", \"" << method.name << "\");" << endl;
    stream << endl;

    it = method.arguments.begin();
    for (; it != endIt; ++it)
    {
        if (!(*it).annotatedType.isEmpty())
        {
            // TODO: error handling
            stream << "    TQT_DBusData " << (*it).name << "Data;" << endl;
            stream << "    if (TQT_DBusDataConverter:convertToTQT_DBusData<"
                   << (*it).annotatedType << ">("
                   << (*it).name << ", " << (*it).name << "Data"
                   << ") != TQT_DBusDataConverter::Success) return false;"
                   << endl;
            stream << "    message << " << (*it).name << "Data";
        }
        else if (!(*it).accessor.isEmpty())
        {
            stream << "    message << TQT_DBusData::from" << (*it).accessor;
            if (!(*it).subAccessor.isEmpty())
            {
                stream << "(" << (*it).containerClass;
            }

            stream << "(" << (*it).name << ")";

            if (!(*it).subAccessor.isEmpty())
            {
                stream << ")";
            }
        }
        else
            stream << "    message << " << (*it).name;

        stream << ";" << endl;
    }
    stream << endl;

    stream << "    return handleSignalSend(message);" << endl;

    stream << "}" << endl;
    stream << endl;
}


void MethodGenerator::writeInterfaceAsyncReplyHandler(const Class& classData,
    const Method& method, TQTextStream& stream)
{
    stream << "void " << classData.name << "::" << method.name
           << "AsyncReply(";

    TQValueList<Argument>::const_iterator it    = method.arguments.begin();
    TQValueList<Argument>::const_iterator endIt = method.arguments.end();
    while (it != endIt)
    {
        if (!(*it).isPrimitive && (*it).direction == Argument::In)
            stream << "const ";

        stream << (*it).signature;

        if (!(*it).isPrimitive || (*it).direction == Argument::Out) stream << "&";

        stream << " " << (*it).name;

        ++it;
        if (it != endIt) stream << ", ";
    }
    stream << ")" << endl;
    stream << endl;
    stream << "{" << endl;

    stream << "    TQMap<int, TQT_DBusMessage>::iterator findIt = m_asyncCalls.find(asyncCallId);" << endl;
    stream << "    if (findIt == m_asyncCalls.end()) return;" << endl;
    stream << endl;

    stream << "    TQT_DBusMessage call = findIt.data();" << endl;
    stream << "    m_asyncCalls.erase(findIt);" << endl;
    stream << endl;

    stream << "    TQT_DBusMessage reply = TQT_DBusMessage::methodReply(call);"
           << endl;

    it = method.arguments.begin();
    for (++it; it != endIt; ++it) // skip asyncCallId at beginning
    {
        if (!(*it).annotatedType.isEmpty())
        {
            stream << "    TQT_DBusData " << (*it).name << "Data;" << endl;

            // TODO error handling
            stream << "    if (TQT_DBusDataConverter::convertToTQT_DBusData<"
                   << (*it).annotatedType << ">(" << (*it).name << ", "
                   << (*it).name << "Data"
                   << ") != TQT_DBusDataConverter::Success) return false;"
                   << endl;
            stream << "    reply << " << (*it).name << "Data;" << endl;
        }
        else if (!(*it).accessor.isEmpty())
        {
            stream << "    reply << TQT_DBusData::from" << (*it).accessor << "(";

            if ((*it).subAccessor.isEmpty())
                stream << (*it).name;
            else
                stream << (*it).containerClass << "(" << (*it).name << ")";

            stream << ");" << endl;
        }
        else
            stream << "    reply << " << (*it).name << ";" << endl;
    }
    stream << endl;

    stream << "    handleMethodReply(reply);" << endl;

    stream << "}" << endl;
    stream << endl;

    stream << "void " << classData.name << "::" << method.name
           << "AsyncError(int asyncCallId, const TQT_DBusError& error)";
    stream << endl;

    stream << "{" << endl;

    stream << "    TQMap<int, TQT_DBusMessage>::iterator findIt = m_asyncCalls.find(asyncCallId);" << endl;
    stream << "    if (findIt == m_asyncCalls.end()) return;" << endl;
    stream << endl;

    stream << "    TQT_DBusMessage call = findIt.data();" << endl;
    stream << "    m_asyncCalls.erase(findIt);" << endl;
    stream << endl;

    stream << "    TQT_DBusMessage reply = TQT_DBusMessage::methodError(call, error);"
           << endl;
    stream << "    handleMethodReply(reply);" << endl;

    stream << "}" << endl;
    stream << endl;
}

void MethodGenerator::writeInterfaceMainMethod(const Class& classData,
        TQTextStream& stream)
{
    if (classData.methods.isEmpty()) return;

    stream << "bool " << classData.name
           << "::handleMethodCall(const TQT_DBusMessage& message)" << endl;
    stream << "{" << endl;

    stream << "    if (message.interface() != \"" << classData.dbusName
           << "\") return false;" << endl;
    stream << endl;

    TQValueList<Method>::const_iterator it    = classData.methods.begin();
    TQValueList<Method>::const_iterator endIt = classData.methods.end();
    for (; it != endIt; ++it)
    {
        stream << "    if (message.member() == \"" << (*it).name << "\")" << endl;
        stream << "    {" << endl;

        if ((*it).async)
        {
            stream << "        call" << (*it).name << "Async(message);" << endl;
            stream << endl;
        }
        else
        {
            stream << "        TQT_DBusMessage reply = call" << (*it).name
                   << "(message);" << endl;
            stream << "        handleMethodReply(reply);" << endl;
            stream << endl;
        }
        stream << "        return true;" << endl;
        stream << "    }" << endl;
        stream << endl;
    }

    stream << "    return false; " << endl;
    stream << "}" << endl;
    stream << endl;
}

void MethodGenerator::writeSignalHandler(const Class& classData,
        TQTextStream& stream)
{
    stream << "void " << classData.name
           << "::slotHandleDBusSignal(const TQT_DBusMessage& message)" << endl;
    stream << "{" << endl;

    TQValueList<Method>::const_iterator it    = classData.msignals.begin();
    TQValueList<Method>::const_iterator endIt = classData.msignals.end();
    bool first = true;
    for (; it != endIt; ++it)
    {
        stream << "    ";

        if (!first)
            stream << "else ";
        else
            first = false;

        stream << "if (message.member() == \"" << (*it).name << "\")" << endl;
        stream << "    {" << endl;

        int count = 0;
        TQValueList<Argument>::const_iterator it1    = (*it).arguments.begin();
        TQValueList<Argument>::const_iterator endIt1 = (*it).arguments.end();
        for (; it1 != endIt1; ++it1)
        {
            stream << "        " << (*it1).signature << " _" << (*it1).name;

            if (!(*it1).accessor.isEmpty())
            {
                stream << TQString::fromUtf8(" = message[%1].to").arg(count++);
                stream << (*it1).accessor;
                if (!(*it1).subAccessor.isEmpty())
                {
                    stream << TQString("().to%1").arg((*it1).subAccessor);
                }

                stream << "()";
            }

            stream << ";" << endl;
        }

        stream << endl;

        writeSignalEmit(*it, stream);

        stream << "    }" << endl;
    }

    stream << "}" << endl;
    stream << endl;
}

void MethodGenerator::writeProxyBegin(const Class& classData, TQTextStream& stream)
{
    stream << classData.name << "::" << classData.name
           << "(const TQString& service, const TQString& path, TQObject* parent, const char* name)" << endl;
    stream << "    : TQObject(parent, name)," << endl;
    stream << "      m_baseProxy(new TQT_DBusProxy())" << endl;
    stream << "{" << endl;
    stream << "    m_baseProxy->setInterface(\""
           << classData.dbusName << "\");" << endl;
    stream << "    m_baseProxy->setPath(path);" << endl;
    stream << "    m_baseProxy->setService(service);" << endl;
    stream << endl;

    if (!classData.msignals.isEmpty())
    {
        stream << "    TQObject::connect(m_baseProxy, "
               << "TQ_SIGNAL(dbusSignal(const TQT_DBusMessage&))," << endl;
        stream << "                     this, "
               << "       TQ_SLOT(slotHandleDBusSignal(const TQT_DBusMessage&)));"
               << endl;
    }

    if (!classData.asyncReplySignals.isEmpty())
    {
        stream << "    TQObject::connect(m_baseProxy, "
               << "TQ_SIGNAL(asyncReply(int, const TQT_DBusMessage&))," << endl;
        stream << "                     this, "
               << "       TQ_SLOT(slotHandleAsyncReply(int, const TQT_DBusMessage&)));"
               << endl;
    }

    stream << "}" << endl;

    stream << endl;

    stream << classData.name << "::~" << classData.name << "()" << endl;
    stream << "{" << endl;
    stream << "    delete m_baseProxy;" << endl;
    stream << "}" << endl;
    stream << endl;

    stream << "void " << classData.name
           << "::setConnection(const TQT_DBusConnection& connection)" << endl;
    stream << "{" << endl;
    stream << "    m_baseProxy->setConnection(connection);" << endl;
    stream << "}" << endl;
    stream << endl;
}

void MethodGenerator::writeProxyMethod(const TQString& className,
        const Method& method, TQTextStream& stream)
{
    stream << "bool " << className << "::" << method.name
           << (method.async ? "Async(" : "(");

    TQValueList<Argument>::const_iterator it    = method.arguments.begin();
    TQValueList<Argument>::const_iterator endIt = method.arguments.end();
    for (; it != endIt; ++it)
    {
        if (!(*it).isPrimitive && (*it).direction == Argument::In)
            stream << "const ";

        stream << (*it).signature;

        if (!(*it).isPrimitive || (*it).direction == Argument::Out) stream << "&";

        stream << " " << (*it).name << ", ";
    }

    stream << "TQT_DBusError& error)" << endl;

    stream << "{" << endl;
    stream << "    TQValueList<TQT_DBusData> parameters;" << endl;
    stream << endl;

    uint outCount = 0;

    it = method.arguments.begin();
    for (; it != endIt; ++it)
    {
        if ((*it).direction == Argument::Out)
        {
            ++outCount;
            continue;
        }

        if (!(*it).annotatedType.isEmpty())
        {
            stream << "    TQT_DBusData " << (*it).name << "Data;" << endl;

            // TODO error handling
            stream << "    if (TQT_DBusDataConverter::convertToTQT_DBusData<"
                   << (*it).annotatedType << ">(" << (*it).name << ", "
                   << (*it).name << "Data"
                   << ") != TQT_DBusDataConverter::Success) return false;"
                   << endl;
            stream << "    parameters << " << (*it).name << "Data;" << endl;
        }
        else if (!(*it).accessor.isEmpty())
        {
            stream << "    parameters << TQT_DBusData::from" << (*it).accessor << "(";

            if ((*it).subAccessor.isEmpty())
                stream << (*it).name;
            else
                stream << (*it).containerClass << "(" << (*it).name << ")";

            stream << ");" << endl;
        }
        else
            stream << "    parameters << " << (*it).name << ";" << endl;
    }

    stream << endl;

    if (outCount == 0 && method.noReply)
    {
        stream << "    if (!m_baseProxy->send(\"" << method.name
               << "\", parameters))" << endl;
        stream << "    {" << endl;
        stream << "        error = m_baseProxy->lastError();" << endl;
        stream << "        return false;" << endl;
        stream << "    }" << endl;
        stream << "    return true;" << endl;
        stream << "}" << endl;
        stream << endl;
        return;
    }

    if (method.async)
    {
        stream << "    asyncCallId = m_baseProxy->sendWithAsyncReply(\"";
        stream << method.name << "\", parameters);" << endl;
        stream << endl;

        stream << "    if (asyncCallId != 0) m_asyncCalls[asyncCallId] = \""
               << method.name << "\";" << endl;
        stream << endl;

        stream << "    error = TQT_DBusError();";
        stream << endl;

        stream << "    return (asyncCallId != 0);" << endl;
        stream << "}" << endl;
        stream << endl;
        return;
    }

    stream << "    TQT_DBusMessage reply = m_baseProxy->sendWithReply(\"";
    stream << method.name << "\", parameters, &error);" << endl;
    stream << endl;

    stream << "    if (reply.type() != TQT_DBusMessage::ReplyMessage) return false;"
           << endl;

    if (outCount == 0)
    {
        stream << "    return true;" << endl;
        stream << "}" << endl;
        stream << endl;
        return;
    }

    stream << endl;

    // TODO: create error or use enum for return
    stream << "    if (reply.count() != " << outCount << ") return false;" << endl;
    stream << endl;

    bool firstAccessor    = true;
    bool firstSubAccessor = true;

    it = method.arguments.begin();
    for (; it != endIt; ++it)
    {
        if ((*it).direction == Argument::In) continue;

        --outCount;

        if (!(*it).annotatedType.isEmpty())
        {
            // TODO error handling
            stream << "    if (TQT_DBusDataConverter::convertFromTQT_DBusData<"
                   << (*it).annotatedType << ">(reply.front(), "
                   << (*it).name
                   << ") != TQT_DBusDataConverter::Success) return false;"
                   << endl;
        }
        else if (!(*it).accessor.isEmpty())
        {
            if (firstAccessor)
            {
                stream << "    bool ok = false;" << endl;
                stream << endl;
                firstAccessor = false;
            }

            if ((*it).subAccessor.isEmpty())
            {
                stream << "    " << (*it).name << " = reply.front().to"
                    << (*it).accessor << "(&ok);" << endl;
            }
            else
            {
                if (firstSubAccessor)
                {
                    stream << "    bool subOK = false;" << endl;
                    stream << endl;
                    firstSubAccessor = false;
                }

                stream << "    " << (*it).name << " = reply.front().to"
                    << (*it).accessor << "(&ok).to" << (*it).subAccessor
                    << "(&subOK);" << endl;

                // TODO: create error or use enum for return
                stream << "    if (!subOK) return false;" << endl;
            }

            // TODO: create error or use enum for return
            stream << "    if (!ok) return false;" << endl;
        }
        else
            stream << "    " << (*it).name << " = reply.front();" << endl;
        stream << endl;

        if (outCount > 0)
        {
            stream << "    reply.pop_front();" << endl;
            stream << endl;
        }
    }

    stream << "    return true;" << endl;

    stream << "}" << endl;
    stream << endl;
}

void MethodGenerator::writeProxyGenericProperty(const Class& classData,
        TQTextStream& stream)
{
    stream << "void " << classData.name
           << "::setDBusProperty(const TQString& name, "
           << "const TQT_DBusVariant& value, TQT_DBusError& error)"
           << endl;
    stream << "{" << endl;

    stream << "    TQT_DBusConnection connection = m_baseProxy->connection();" << endl;
    stream << endl;
    stream << "    TQT_DBusMessage message = TQT_DBusMessage::methodCall("
           << "m_baseProxy->service(), m_baseProxy->path(), "
           << "\"org.freedesktop.DBus.Properties\", \"Set\");" << endl;
    stream << endl;

    stream << "    message << TQT_DBusData::fromString(m_baseProxy->interface());"
           << endl;
    stream << "    message << TQT_DBusData::fromString(name);" << endl;
    stream << "    message << TQT_DBusData::fromVariant(value);" << endl;
    stream << endl;

    stream << "    connection.sendWithReply(message, &error);" << endl;

    stream << "}" << endl;

    stream << endl;

    stream << "TQT_DBusVariant " << classData.name
           << "::getDBusProperty(const TQString& name, TQT_DBusError& error) const"
           << endl;
    stream << "{" << endl;

    stream << "    TQT_DBusConnection connection = m_baseProxy->connection();" << endl;
    stream << endl;

    stream << "    TQT_DBusMessage message = TQT_DBusMessage::methodCall("
           << "m_baseProxy->service(), m_baseProxy->path(), "
           << "\"org.freedesktop.DBus.Properties\", \"Get\");" << endl;
    stream << endl;

    stream << "    message << TQT_DBusData::fromString(m_baseProxy->interface());"
           << endl;
    stream << "    message << TQT_DBusData::fromString(name);" << endl;
    stream << endl;

    stream << "    TQT_DBusMessage reply = connection.sendWithReply(message, &error);"
           << endl;
    stream << endl;

    stream << "    if (reply.type() != TQT_DBusMessage::ReplyMessage)"
           << " return TQT_DBusVariant();" << endl;
    stream << "    if (reply.count() != 1) return TQT_DBusVariant();" << endl;

    stream << endl;

    stream << "    bool ok = false;" << endl;
    stream << "    TQT_DBusVariant value = reply.front().toVariant(&ok);" << endl;

    // TODO generate error
    stream << "    if (!ok) return TQT_DBusVariant();" << endl;
    stream << endl;

    stream << "    return value;" << endl;

    stream << "}" << endl;

    stream << endl;
}

void MethodGenerator::writeProxyProperty(const Class& classData,
        const Property& property, TQTextStream& stream)
{
    if (property.write)
    {
        stream << "void " << classData.name << "::set" << property.name << "(";

        if (!property.isPrimitive) stream << "const ";

        stream << property.signature;

        if (!property.isPrimitive) stream << "&";

        stream << " value, TQT_DBusError& error)" << endl;
        stream << "{" << endl;
        stream << "    TQT_DBusVariant variant;" << endl;

        if (!property.annotatedType.isEmpty())
        {
            // TODO: error handling
            stream << "    TQT_DBusDataConverter::convertToTQT_DBusData<"
                   << property.annotatedType << ">(value, variant.value);"
                   << endl;
        }
        else if (!property.accessor.isEmpty())
        {
            stream << "    variant.value = TQT_DBusData::from"
                   << property.accessor << "(";

            if (property.subAccessor.isEmpty())
                stream << "value";
            else
                stream << property.containerClass << "(value)";

            stream << ");" << endl;
        }
        else
            stream << "    variant.value = TQT_DBusData(value);" << endl;

        stream << "    variant.signature = \"" << property.dbusSignature << "\";"
               << endl;

        stream << endl;
        stream << "    setDBusProperty(\"" << property.name
               << "\", variant, error);" << endl;

        stream << "}" << endl;
        stream << endl;
    }

    if (property.read)
    {
        stream << property.signature << " " << classData.name
               << "::get" << property.name << "(TQT_DBusError& error) const" << endl;
        stream << "{" << endl;

        stream << "    TQT_DBusVariant variant = getDBusProperty(\""
               << property.name << "\", error);" << endl;
        stream << endl;
        stream << "    if (error.isValid()) return "
               << property.signature << "();" << endl;
        stream << endl;

        if (!property.annotatedType.isEmpty())
        {
            stream << "    " << property.signature << " result;" << endl;

            // TODO error handling
            stream << "    TQT_DBusDataConverter::convertFromTQT_DBusData<"
                   << property.annotatedType << ">(variant.value, result);"
                   << endl;
        }
        else if (!property.accessor.isEmpty())
        {
            stream << "    bool ok = false;" << endl;
            stream << endl;

            if (property.subAccessor.isEmpty())
            {
                stream << "    " << property.signature << " result = ";
                stream << " variant.value.to" << property.accessor
                       << "(&ok);" << endl;
            }
            else
            {
                stream << "    bool subOK = false;" << endl;
                stream << endl;

                stream << "    " << property.signature << " result = ";
                stream << " variant.value.to"
                       << property.accessor << "(&ok).to" << property.subAccessor
                       << "(&subOK);" << endl;

                // TODO: create error
                stream << "    if (!subOK) {}" << endl;
            }

            // TODO: create error
            stream << "    if (!ok) {}" << endl;
        }
        else
            stream << "    " << property.signature << " result = variant.value;";
        stream << endl;

        stream << "    return result;" << endl;
        stream << "}" << endl;
        stream << endl;
    }
}

void MethodGenerator::writeProxyAsyncReplyHandler(const Class& classData,
        TQTextStream& stream)
{
    stream << "void " << classData.name
           << "::slotHandleAsyncReply(int asyncCallId, const TQT_DBusMessage& message)" << endl;
    stream << "{" << endl;

    stream << "    TQMap<int, TQString>::iterator findIt = "
           << "m_asyncCalls.find(asyncCallId);" << endl;
    stream << "    if (findIt == m_asyncCalls.end()) return;" << endl;
    stream << endl;
    stream << "    const TQString signalName = findIt.data();" << endl;
    stream << "    m_asyncCalls.erase(findIt);" << endl;
    stream << endl;

    TQValueList<Method>::const_iterator it    = classData.asyncReplySignals.begin();
    TQValueList<Method>::const_iterator endIt = classData.asyncReplySignals.end();
    bool first = true;
    for (; it != endIt; ++it)
    {
        stream << "    ";

        if (!first)
            stream << "else ";
        else
            first = false;

        stream << "if (signalName == \"" << (*it).name << "\")" << endl;
        stream << "    {" << endl;

        // FIXME tricking writeVariables and writeSignalEmit into writing
        // the reply emit code by manipulating arguments and name
        stream << "        int _asyncCallId = asyncCallId;" << endl << endl;

        stream << "        if (message.type() == TQT_DBusMessage::ErrorMessage) {" << endl;
        stream << "            emit AsyncErrorResponseDetected(_asyncCallId, message.error());" << endl;
        stream << "        }" << endl << endl;

        Method signal = *it;
        signal.arguments.pop_front();

        writeVariables("        ", signal, stream);
        stream << endl;

        signal = *it;
        signal.name += "AsyncReply";

        writeSignalEmit(signal, stream);

        stream << "    }" << endl;
    }

    stream << "}" << endl;
    stream << endl;
}

void MethodGenerator::writeIntrospectionDataMethod(const Class& classData,
    TQTextStream& stream)
{
    stream << "void " << classData.name
           << "::buildIntrospectionData(TQDomElement& interfaceElement)" << endl;
    stream << "{" << endl;

    stream << "    interfaceElement.setAttribute(\"name\", \""
           << classData.dbusName << "\");" << endl;
    stream << endl;

    bool firstMethod   = true;
    bool firstArgument = true;
    bool firstAsync    = true;

    TQValueList<Method>::const_iterator it    = classData.methods.begin();
    TQValueList<Method>::const_iterator endIt = classData.methods.end();
    for (; it != endIt; ++it)
    {
        if (firstMethod)
        {
            firstMethod = false;
            stream << "    TQDomDocument document = interfaceElement.ownerDocument();" << endl;
            stream << "    TQDomElement methodElement = document.createElement("
                   << "\"method\");" << endl;
        }
        else
        {
            stream << endl;
            stream << "    methodElement = document.createElement("
                   << "\"method\");" << endl;
        }

        if ((*it).async)
        {
            if (firstAsync) {
                firstAsync=false;
                stream << "    TQDomElement asyncAnnotationElement = document.createElement("
                        << "\"annotation\");" << endl;
            }
            else
            {
                stream << "    asyncAnnotationElement = document.createElement("
                        << "\"annotation\");" << endl;
            }
            stream << "    asyncAnnotationElement.setAttribute(\"name\", "
                    << "\"org.freedesktop.DBus.GLib.Async\");" << endl;
            stream << "    asyncAnnotationElement.setAttribute(\"value\", "
                    << "\"true\");" << endl;
            stream << "    methodElement.appendChild(asyncAnnotationElement);" << endl;
        }

        writeMethodIntrospection(*it, firstArgument, stream);

        stream << "    interfaceElement.appendChild(methodElement);" << endl;
    }

    it    = classData.msignals.begin();
    endIt = classData.msignals.end();
    for (; it != endIt; ++it)
    {
        if (firstMethod)
        {
            firstMethod = false;
            stream << "    TQDomDocument document = interfaceElement.ownerDocument();" << endl;
            stream << endl;
            stream << "    TQDomElement methodElement = document.createElement("
                   << "\"signal\");" << endl;
        }
        else
        {
            stream << endl;
            stream << "    methodElement = document.createElement("
                   << "\"signal\");" << endl;
        }

        writeMethodIntrospection(*it, firstArgument, stream);

        stream << "    interfaceElement.appendChild(methodElement);" << endl;
    }

    stream << "}" << endl;
    stream << endl;
}

void MethodGenerator::writeNodePrivate(const Class& classData, TQTextStream& stream)
{
    stream << "class " << classData.name
           << "::Private : public org::freedesktop::DBus::IntrospectableInterface" << endl;
    stream << "{" << endl;
    stream << "public:" << endl;
    stream << "    virtual ~Private();" << endl;
    stream << endl;

    stream << "public:" << endl;
    stream << "    TQMap<TQString, TQT_DBusObjectBase*> interfaces;" << endl;
    stream << "    TQString introspectionData;" << endl;
    stream << endl;
    stream << "    TQT_DBusConnection connection;" << endl;
    stream << "    TQString objectPath;" << endl;
    stream << "    TQStringList childrenNodes;" << endl;
    stream << endl;

    stream << "protected:" << endl;
    stream << "    virtual bool Introspect(TQString& data, TQT_DBusError& error);"
           << endl;
    stream << endl;
    stream << "    virtual void handleMethodReply(const TQT_DBusMessage& reply);"
           << endl;

    stream << "private:" << endl;
    stream << "    void cacheIntrospectionData();" << endl;

    stream << "};" << endl;
    stream << endl;
}

void MethodGenerator::writeNodeBegin(const Class& classData, TQTextStream& stream)
{
    stream << classData.name << "::" << classData.name
           << "()  : TQT_DBusObjectBase()," << endl;
    stream << "    m_private(new Private())" << endl;
    stream << "{" << endl;
    stream << "}" << endl;
    stream << endl;

    stream << classData.name << "::~" << classData.name << "()" << endl;
    stream << "{" << endl;
    stream << "    unregisterObject();" << endl;
    stream << endl;
    stream << "    delete m_private;" << endl;
    stream << "}" << endl;
    stream << endl;
}

void MethodGenerator::writeNodeMethods(const Class& classData,
            const TQValueList<Class>& interfaces, TQTextStream& stream)
{
    writeNodeInitialization(classData, interfaces, stream);

    stream << "void " << classData.name << "::unregisterObject()" << endl;
    stream << "{" << endl;
    stream << "    if (m_private->objectPath.isEmpty()) return;" << endl;
    stream << endl;
    stream << "    m_private->connection.unregisterObject(m_private->objectPath);" << endl;
    stream << endl;
    stream << "    m_private->connection = TQT_DBusConnection();" << endl;
    stream << "    m_private->objectPath = TQString();" << endl;
    stream << "}" << endl;
    stream << endl;

    stream << "bool " << classData.name
           << "::handleMethodCall(const TQT_DBusMessage& message)" << endl;
    stream << "{" << endl;
    stream << "    TQMap<TQString, TQT_DBusObjectBase*>::iterator findIt = "
           << "m_private->interfaces.find(message.interface());" << endl;
    stream << "    if (findIt == m_private->interfaces.end()) return false;"
           << endl;
    stream << endl;
    stream << "    return delegateMethodCall(message, findIt.data());" << endl;
    stream << "}" << endl;
    stream << endl;

    stream << classData.name << "::Private::~Private()" << endl;
    stream << "{" << endl;
    stream << "    TQMap<TQString, TQT_DBusObjectBase*>::const_iterator it    = "
           << "interfaces.begin();" << endl;
    stream << "    TQMap<TQString, TQT_DBusObjectBase*>::const_iterator endIt = "
           << "interfaces.end();" << endl;
    stream << "    for (; it != endIt; ++it)" << endl;
    stream << "    {" << endl;
    stream << "        TQT_DBusObjectBase* interface = it.data();" << endl;
    stream << "        if (interface != this)" << endl;
    stream << "            delete interface;" << endl;
    stream << "    }" << endl;
    stream << "    interfaces.clear();" << endl;
    stream << "}" << endl;
    stream << endl;

    stream << "bool " << classData.name << "::Private"
           << "::Introspect(TQString& data, TQT_DBusError& error)" << endl;
    stream << "{" << endl;
    stream << "    Q_UNUSED(error);" << endl;
    stream << "    if (introspectionData.isEmpty()) cacheIntrospectionData();"
           << endl;
    stream << endl;
    stream << "    data = introspectionData;" << endl;
    stream << endl;
    stream << "    return true;" << endl;
    stream << "}" << endl;
    stream << endl;

    stream << "void " << classData.name << "::Private"
           << "::handleMethodReply(const TQT_DBusMessage& reply)" << endl;
    stream << "{" << endl;
    stream << "    connection.send(reply);" << endl;
    stream << "}" << endl;
    stream << endl;

    writeNodeIntrospection(classData, interfaces, stream);
}

// End of File
