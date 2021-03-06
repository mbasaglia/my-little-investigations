/**
 * Basic header/include file for XmlReader.cpp.
 *
 * @author GabuEx, dawnmew
 * @since 1.0
 *
 * Licensed under the MIT License.
 *
 * Copyright (c) 2014 Equestrian Dreamers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef XMLREADER_H
#define XMLREADER_H

#include <stack>
#include "ticpp/ticpp.h"

using namespace std;
using namespace ticpp;

class Image;

class XmlReader
{
public:
    class ListIterator
    {
    public:
        ListIterator(XmlReader *pParent, const char *pListElementName);
        ListIterator(const ListIterator &other);
        ~ListIterator();
        bool MoveToNextItem();

    private:
        ListIterator()
        {
            pParentReader = NULL;
            pIterator = NULL;
        }

        string listElementName;
        XmlReader *pParentReader;
        Iterator<Element> *pIterator;
        bool hasBegun;
    };

    XmlReader();
    XmlReader(const char *pFilePath);
    XmlReader(const XmlReader &other);
    ~XmlReader();

    void ParseXmlFile(const char *pFilePath);
    void ParseXmlContent(string xmlContent);

    void StartElement(const char *pElementName);
    bool ElementExists(const char *pElementName);
    void EndElement();
    void StartList(const char *pListElementName);
    bool MoveToNextListItem();
    int ReadIntElement(const char *pElementName);
    double ReadDoubleElement(const char *pElementName);
    bool ReadBooleanElement(const char *pElementName);
    string ReadTextElement(const char *pElementName);

#ifdef GAME_EXECUTABLE
    Image * ReadPngElement(const char *pElementName);
#endif

private:
    Element * GetFirstDirectChildElementByName(const char *pElementName);
    Node * GetTopNodeInStack();

    string filePath;
    Document *pDocument;
    stack<ListIterator *> listIteratorStack;
    stack<Element *> elementStack;
};

#endif
