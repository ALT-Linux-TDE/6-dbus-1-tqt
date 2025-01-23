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

// standard includes
#include <iostream>
#include <cstdlib>

// TQt includes
#include <tqdom.h>
#include <tqfile.h>
#include <tqmap.h>
#include <tqtextstream.h>

// local includes
#include "classgen.h"
#include "methodgen.h"

typedef TQMap<TQString, TQString> OptionMap;

void usage();

OptionMap parseOptions(int argc, char** argv);

bool checkForOption(const OptionMap& options, const TQString& option)
{
    return options.find(option) != options.end();
}

int main(int argc, char** argv)
{
    const OptionMap options = parseOptions(argc, argv);

    if (!checkForOption(options, "filename"))
    {
        std::cerr << "dbusxml2qt3: introspection data file missing" << std::endl;
        usage();
        exit(1);
    }

    TQString fileName = options["filename"];
    TQFile file(fileName);
    if (!file.exists())
    {
        std::cerr << "dbusxml2qt3: introspection data file '"
                  << fileName.local8Bit().data()
                  << "' does not exist" << std::endl;
        exit(2);
    }

    if (!file.open(IO_ReadOnly))
    {
        std::cerr << "dbusxml2qt3: introspection data file '"
                  << fileName.local8Bit().data()
                  << "' cannot be read" << std::endl;
        exit(2);
    }

    TQDomDocument document;

    if (!document.setContent(&file))
    {
        file.close();

        std::cerr << "dbusxml2qt3: introspection data file '"
                  << fileName.local8Bit().data()
                  << "' cannot be parsed" << std::endl;
        exit(2);
    }

    file.close();

    TQDomElement rootElement = document.documentElement();
    if (rootElement.isNull() || rootElement.tagName() != "node")
    {
        std::cerr << "dbusxml2qt3: introspection data file '"
                  << fileName.local8Bit().data()
                  << "' does not have a 'node' element as its root node"
                  << std::endl;
        exit(2);
    }

    TQValueList<Class> interfaces;
    bool hasIntrospectable = false;

    TQDomNode child = rootElement.firstChild();
    for (; !child.isNull(); child = child.nextSibling())
    {
        if (!child.isElement()) continue;

        TQDomElement element = child.toElement();

        if (element.tagName() == "interface")
        {
            if (!element.attribute("name").isEmpty())
            {
                Class classData;
                if (ClassGenerator::extractClass(element, classData))
                {
                    if (classData.dbusName == "org.freedesktop.DBus.Introspectable")
                    {
                        hasIntrospectable = true;
                    }
                    interfaces << classData;
                }
            }
        }
    }

    if (interfaces.isEmpty())
    {
        std::cerr << "dbusxml2qt3: introspection data file '"
                  << fileName.local8Bit().data()
                  << "' does not contain any valid interface descriptions"
                  << std::endl;
        exit(3);
    }

    bool generateProxies    = checkForOption(options, "proxy");
    bool generateInterfaces = checkForOption(options, "interface");
    bool generateNode       = checkForOption(options, "node");

    // if no specific option is selected, we generate everything
    bool generateAll = !(generateProxies || generateInterfaces || generateNode);
    TQString customInterfaceFilename = TQString::null;

    if (checkForOption(options, "classname"))
    {
        // class name only useful for single interfaces or node
        if (interfaces.count() > 1 && !(generateNode || generateAll))
        {
            std::cerr << "dbusxml2qt3: class name option specified but "
                      << "introspection data file '"
                      << fileName.local8Bit().data()
                      << "' contains more than one interface description"
                      << std::endl;
            exit(3);
        }

        // class name for node is handled differently later on
        if (!(generateNode || generateAll))
        {
            TQStringList nameParts = TQStringList::split("::", options["classname"]);

            interfaces[0].name = nameParts.back();

            nameParts.pop_back();
            interfaces[0].namespaces = nameParts;
        }
    }

    if (checkForOption(options, "namespace"))
    {
        TQStringList nameParts = TQStringList::split("::", options["namespace"]);

        TQValueList<Class>::iterator it    = interfaces.begin();
        TQValueList<Class>::iterator endIt = interfaces.end();
        for (; it != endIt; ++it)
        {
            (*it).namespaces = nameParts;
        }
    }

    if (generateInterfaces || generateAll)
    {
        TQTextStream headerStream;
        TQTextStream sourceStream;

        TQString baseName = options["interface"];
        customInterfaceFilename = baseName;
        if (!baseName.isEmpty())
        {
            if (!ClassGenerator::initStreams(baseName, headerStream, sourceStream))
            {
                std::cerr << "dbusxml2qt3: proxy files, using base name '"
                          << baseName.local8Bit().data()
                          << "', could not be opened for writing"
                          << std::endl;
                exit(4);
            }
        }

        TQValueList<Class>::const_iterator it    = interfaces.begin();
        TQValueList<Class>::const_iterator endIt = interfaces.end();
        for (; it != endIt; ++it)
        {
            TQString streamName = (*it).name.lower() + "Interface";
            if (baseName.isEmpty())
            {
                if (!ClassGenerator::initStreams(streamName, headerStream, sourceStream))
                {
                    std::cerr << "dbusxml2qt3: interface files, using base name '"
                            << streamName.local8Bit().data()
                            << "', could not be opened for writing"
                            << std::endl;
                    exit(4);
                }
            }

            Class interfaceData = *it;
            if(!checkForOption(options, "classname") || generateNode || generateAll)
            {
                interfaceData.name += "Interface";
            }
            ClassGenerator::generateInterface(interfaceData, headerStream, sourceStream);

            if (baseName.isEmpty())
            {
                ClassGenerator::finishStreams(streamName, headerStream, sourceStream);
            }
        }

        if (!baseName.isEmpty())
            ClassGenerator::finishStreams(baseName, headerStream, sourceStream);
    }

    if (generateProxies || generateAll)
    {
        TQTextStream headerStream;
        TQTextStream sourceStream;

        TQString baseName = options["proxy"];
        if (!baseName.isEmpty())
        {
            if (!ClassGenerator::initStreams(baseName, headerStream, sourceStream))
            {
                std::cerr << "dbusxml2qt3: proxy files, using base name '"
                          << baseName.local8Bit().data()
                          << "', could not be opened for writing"
                          << std::endl;
                exit(4);
            }
        }

        TQValueList<Class>::const_iterator it    = interfaces.begin();
        TQValueList<Class>::const_iterator endIt = interfaces.end();
        for (; it != endIt; ++it)
        {
            if ((*it).dbusName == "org.freedesktop.DBus.Introspectable")
            {
                continue;
            }

            TQString streamName = (*it).name.lower() + "Proxy";
            if (baseName.isEmpty())
            {
                if (!ClassGenerator::initStreams(streamName, headerStream, sourceStream))
                {
                    std::cerr << "dbusxml2qt3: proxy files, using base name '"
                            << streamName.local8Bit().data()
                            << "', could not be opened for writing"
                            << std::endl;
                    exit(4);
                }
            }

            Class proxyData = *it;
            if(!checkForOption(options, "classname") || generateNode || generateAll)
            {
                proxyData.name += "Proxy";
            }
            ClassGenerator::generateProxy(proxyData, headerStream, sourceStream);

            if (baseName.isEmpty())
            {
                ClassGenerator::generateIncludeMoc(streamName, sourceStream);
                ClassGenerator::finishStreams(streamName, headerStream, sourceStream);
            }
        }

        if (!baseName.isEmpty())
        {
            ClassGenerator::generateIncludeMoc(baseName, sourceStream);
            ClassGenerator::finishStreams(baseName, headerStream, sourceStream);
        }
    }

    if (generateNode || generateAll)
    {
        if (!hasIntrospectable)
        {
            tqDebug("Generating org.freedesktop.DBus.Introspectable on demand");

            Class classData;
            TQString streamName = "introspectableInterface";
            classData.name = "IntrospectableInterface";
            classData.dbusName = "org.freedesktop.DBus.Introspectable";

            classData.namespaces << "org" << "freedesktop" << "DBus";

            Method method;
            method.name = "Introspect";
            method.type = Method::_Method;
            method.noReply = false;
            method.async = false;

            Argument argument;
            argument.name = "data";
            argument.direction = Argument::Out;
            argument.signature = "TQString";
            argument.accessor = "String";
            argument.isPrimitive = false;
            argument.dbusSignature = "s";

            argument.forwardDeclarations << "class TQString";
            argument.sourceIncludes["TQt"].append("<tqstring.h>");

            method.arguments << argument;
            classData.methods << method;

            TQTextStream headerStream;
            TQTextStream sourceStream;

            if (!ClassGenerator::initStreams(streamName, headerStream, sourceStream))
            {
                std::cerr << "dbusxml2qt3: interface files, using base name '"
                        << streamName.local8Bit().data()
                        << "', could not be opened for writing"
                        << std::endl;
                exit(4);
            }

            ClassGenerator::generateInterface(classData,
                                              headerStream, sourceStream);

            ClassGenerator::finishStreams(streamName, headerStream, sourceStream);
        }

        TQString nodeClassName = options["classname"];
        if (nodeClassName.isEmpty())
        {
            nodeClassName = rootElement.attribute("name");
            if (nodeClassName.startsWith("/"))
            {
                nodeClassName = nodeClassName.mid(1);
            }
        }

        if (!nodeClassName.isEmpty())
        {
            nodeClassName.replace('/', "::");

            TQStringList nameParts = TQStringList::split("::", nodeClassName);

            Class classData;
            classData.name = nameParts.back();

            nameParts.pop_back();
            classData.namespaces = nameParts;

            if (checkForOption(options, "namespace"))
            {
                nameParts = TQStringList::split("::", options["namespace"]);

                classData.namespaces = nameParts;
            }

            TQTextStream headerStream;
            TQTextStream sourceStream;

            TQString baseName = options["node"];
            if (baseName.isEmpty())
            {
                baseName = classData.name.lower() + "Node";
            }
            if (!checkForOption(options, "classname"))
            {
                classData.name += "Node";
            }

            if (!ClassGenerator::initStreams(baseName, headerStream, sourceStream))
            {
                std::cerr << "dbusxml2qt3: interface files, using base name '"
                        << baseName.local8Bit().data()
                        << "', could not be opened for writing" << std::endl;
                exit(4);
            }

            ClassGenerator::generateNode(classData, interfaces, customInterfaceFilename,
                                     headerStream, sourceStream);

            ClassGenerator::finishStreams(baseName, headerStream, sourceStream);

            // create dummy node to handle the path hierarchy
            if (nameParts.size() > 1)
            {
                TQTextStream headerStreamDBusBaseNode;
                TQTextStream sourceStreamDBusBaseNode;

                TQString baseName = "DBusBase";
                Class classDataDBusBaseNode;
                classDataDBusBaseNode.name = baseName + "Node";
                TQValueList<Class> interfacesDBusBase = TQValueList<Class>();

                TQString baseNameDBusBaseNode = baseName.lower() + "Node";

                if (!ClassGenerator::initStreams(baseNameDBusBaseNode, headerStreamDBusBaseNode, sourceStreamDBusBaseNode))
                {
                    std::cerr << "dbusxml2qt3: interface files, using base name '"
                            << baseNameDBusBaseNode.local8Bit().data()
                            << "', could not be opened for writing"
                            << std::endl;
                    exit(4);
                }

                ClassGenerator::generateNode(classDataDBusBaseNode,
                        interfacesDBusBase, baseNameDBusBaseNode,
                        headerStreamDBusBaseNode, sourceStreamDBusBaseNode);

                ClassGenerator::finishStreams(baseNameDBusBaseNode, headerStreamDBusBaseNode, sourceStreamDBusBaseNode);
            }
        }
    }

    return 0;
}

void usage()
{
    std::cout << "usage: dbusxml2qt3 [options] <introspectionfile>" << std::endl;
    std::cout << std::endl;

    std::cout << "Options:" << std::endl;
    std::cout << "-h, --help" << std::endl;
    std::cout << "\tDisplay this help" << std::endl;
    std::cout << std::endl;

    std::cout << "-c <classname>, --class <classname>" << std::endl;
    std::cout << "\tUse 'classname' instead of last string in interface name"
              << std::endl;
    std::cout << std::endl;

    std::cout << "-N [namespace], --namespace [namespace]" << std::endl;
    std::cout << "\tOverride namespaces. If provided, use 'namespace' instead, otherwise ignore namespaces"
              << std::endl;
    std::cout << std::endl;

    std::cout << "-i [basename], --interface [basename]" << std::endl;
    std::cout << "\tGenerate interface files. If provided, use 'basename' for filenames"
              << std::endl;
    std::cout << std::endl;

    std::cout << "-p [basename], --proxy [basename]" << std::endl;
    std::cout << "\tGenerate proxy files. If provided, use 'basename' for filenames"
              << std::endl;
    std::cout << std::endl;

    std::cout << "-n [basename], --node [basename]" << std::endl;
    std::cout << "\tGenerate node files. If provided, use 'basename' for filenames"
              << std::endl;
    std::cout << std::endl;

    std::cout << "Examples:" << std::endl;
    std::cout << "dbusxml2qt3 myinterface.xml" << std::endl;
    std::cout << "\tGenerates as much as possible, i.e. interfaces, proxies and, "
              << "if a node name is specified in 'myinterface.xml', the node files"
              << std::endl;
    std::cout << "\tUses lowercased interface names as plus type specific suffix "
              << "for the file names" << std::endl;
    std::cout << std::endl;

    std::cout << "dbusxml2qt3 myinterface.xml -N" << std::endl;
    std::cout << "\tSame as first example but does not use namespaces"
              << std::endl;
    std::cout << std::endl;

    std::cout << "dbusxml2qt3 myinterface.xml -N org::myorg" << std::endl;
    std::cout << "\tSame as first example but overrides namespaces with 'org::myorg'"
              << std::endl;
    std::cout << std::endl;

    std::cout << "dbusxml2qt3 myinterface.xml -n mynode -c MyNode" << std::endl;
    std::cout << "\tGenerate only node files, use 'mynode' as the file basename "
              << "and classname 'MyClass'"
              << std::endl;
    std::cout << std::endl;

    std::cout << "dbusxml2qt3 myinterface.xml -p" << std::endl;
    std::cout << "\tGenerate only proxy files, use default file basename"
              << std::endl;
    std::cout << std::endl;

    std::cout << "dbusxml2qt3 myinterface.xml -p myproxy" << std::endl;
    std::cout << "\tGenerate only proxy files, use 'myproxy' as the file basename"
              << std::endl;
    std::cout << std::endl;
}

bool testAndSetOption(OptionMap& options, const TQString& option, const TQString& value)
{
    OptionMap::iterator it = options.find(option);
    if (it == options.end())
    {
        options.insert(option, value);
        return true;
    }

    return false;
}

OptionMap parseOptions(int argc, char** argv)
{
    TQStringList args;
    for (int i = 1; i < argc; ++i)
    {
        args << TQString::fromLocal8Bit(argv[i]);
    }

    OptionMap options;

    while (!args.isEmpty())
    {
        TQString arg = args.front();
        args.pop_front();

        if (arg.startsWith("-"))
        {
            if (arg.endsWith("help"))
            {
                usage();
                exit(0);
            }
            else if (arg == "-p" || arg == "--proxy")
            {
                // test for optional argument
                TQString value;
                if (!args.isEmpty() > 0 && !args[0].startsWith("-"))
                {
                    value = args.front();
                    args.pop_front();
                }

                if (!testAndSetOption(options, "proxy", value))
                {
                    std::cerr << "Error while parsing command line argument '"
                              << arg.local8Bit().data() << "'";

                    if (!value.isEmpty())
                        std::cerr << ", value '" << value.local8Bit().data() << "':";
                    else
                        std::cerr << ":";

                    std::cerr << " already set to '"
                              << options["proxy"].local8Bit().data() << std::endl;
                }
            }
            else if (arg == "-i" || arg == "--interface")
            {
                // test for optional argument
                TQString value;
                if (!args.isEmpty() > 0 && !args[0].startsWith("-"))
                {
                    value = args.front();
                    args.pop_front();
                }

                if (!testAndSetOption(options, "interface", value))
                {
                    std::cerr << "Error while parsing command line argument '"
                              << arg.local8Bit().data() << "'";

                    if (!value.isEmpty())
                        std::cerr << ", value '" << value.local8Bit().data() << "':";
                    else
                        std::cerr << ":";

                    std::cerr << " already set to '"
                              << options["interface"].local8Bit().data() << std::endl;
                }
            }
            else if (arg == "-n" || arg == "--node")
            {
                // test for optional argument
                TQString value;
                if (!args.isEmpty() > 0 && !args[0].startsWith("-"))
                {
                    value = args.front();
                    args.pop_front();
                }

                if (!testAndSetOption(options, "node", value))
                {
                    std::cerr << "Error while parsing command line argument '"
                              << arg.local8Bit().data() << "'";

                    if (!value.isEmpty())
                        std::cerr << ", value '" << value.local8Bit().data() << "':";
                    else
                        std::cerr << ":";

                    std::cerr << " already set to '"
                              << options["node"].local8Bit().data() << std::endl;
                }
            }
            else if (arg == "-N" || arg == "--namespace")
            {
                // test for optional argument
                TQString value;
                if (!args.isEmpty() > 0 && !args[0].startsWith("-"))
                {
                    value = args.front();
                    args.pop_front();
                }

                if (!testAndSetOption(options, "namespace", value))
                {
                    std::cerr << "Error while parsing command line argument '"
                              << arg.local8Bit().data() << "'";

                    if (!value.isEmpty())
                        std::cerr << ", value '" << value.local8Bit().data() << "':";
                    else
                        std::cerr << ":";

                    std::cerr << " already set to '"
                              << options["namespace"].local8Bit().data() << std::endl;
                }
            }
            else if (arg == "-c" || arg == "--class")
            {
                // test for mandatory argument
                if (args.isEmpty() || args[0].startsWith("-"))
                {
                    std::cerr << "Error while parsing command line argument '"
                              << arg.local8Bit().data()
                              << "': mandatory parameter missing" << std::endl;
                    usage();
                    exit(1);
                }

                TQString value = args.front();
                args.pop_front();

                if (!testAndSetOption(options, "classname", value))
                {
                    std::cerr << "Error while parsing command line argument '"
                              << arg.local8Bit().data() << "'";

                    if (!value.isEmpty())
                        std::cerr << ", value '" << value.local8Bit().data() << "':";
                    else
                        std::cerr << ":";

                    std::cerr << " already set to '"
                              << options["classname"].local8Bit().data() << std::endl;
                }
            }
            else
            {
                std::cerr << "Error while parsing command line argument '"
                          << arg.local8Bit().data()
                          << "': unknown option" << std::endl;
                usage();
                exit(1);
            }
        }
        else
        {
            if (!testAndSetOption(options, "filename", arg))
            {
                std::cerr << "Error while parsing command line argument '"
                          << arg.local8Bit().data()
                          << "': introspection file already given as '"
                          << options["filename"].local8Bit().data() << std::endl;
                usage();
                exit(1);
            }
        }
    }

    return options;
}

// End of File
