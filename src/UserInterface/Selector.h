/**
 * Basic header/include file for Selector.cpp.
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

#ifndef SELECTOR_H
#define SELECTOR_H

#include "Arrow.h"
#include "../Font.h"
#include "../Rectangle.h"
#include "../Image.h"
#include "../Version.h"

#include <string>
#include <vector>

using namespace std;

class SelectorItem
{
public:
    virtual ~SelectorItem() {}

    virtual string GetDisplayString() const = 0;
    virtual bool GetShouldDisplayStar() const = 0;
};

class CaseSelectorItem : public SelectorItem
{
public:
    CaseSelectorItem(Image *pScreenshotSprite, Image *pScreenshotFullSizeSprite, string caseUuid, string caseTitle, string caseDescription, string caseFilePath, bool isCompatible, Version requiredVersion);

    virtual ~CaseSelectorItem()
    {
        delete pScreenshotSprite;
        pScreenshotSprite = NULL;
    }

    static bool CompareByCaseTitle(CaseSelectorItem *pItem1, CaseSelectorItem *pItem2)
    {
        string caseTitle1 = pItem1->GetCaseTitle();
        string caseTitle2 = pItem1->GetCaseTitle();

        for (unsigned int i = 0; i < min(caseTitle1.length(), caseTitle2.length()); i++)
        {
            if (caseTitle1[i] < caseTitle2[i])
            {
                return true;
            }
            else if (caseTitle2[i] < caseTitle1[i])
            {
                return false;
            }
        }

        return false;
    }

    string GetDisplayString() const { return caseTitle; }
    bool GetShouldDisplayStar() const { return shouldDisplayStar; }
    Image * GetScreenshotSprite() { return pScreenshotSprite; }
    Image * GetScreenshotFullSizeSprite() { return pScreenshotFullSizeSprite; }
    string GetCaseUuid() const { return caseUuid; }
    string GetCaseTitle() const { return caseTitle; }
    string GetCaseDescription() const { return caseDescription; }
    string GetCaseFilePath() const { return caseFilePath; }
    bool GetIsCompatible() const { return isCompatible; }
    Version GetRequiredVersion() const { return requiredVersion; }

private:
    bool shouldDisplayStar;
    Image *pScreenshotSprite;
    Image *pScreenshotFullSizeSprite;
    string caseUuid;
    string caseTitle;
    string caseDescription;
    string caseFilePath;
    bool isCompatible;
    Version requiredVersion;
};

class SaveLoadSelectorItem : public SelectorItem
{
public:
    SaveLoadSelectorItem(string saveName, Image *pScreenshotSprite, time_t timestamp, string description, string filePath)
    {
        this->saveName = saveName;
        this->pScreenshotSprite = pScreenshotSprite;
        this->timestamp = timestamp;
        this->description = description;
        this->filePath = filePath;
    }

    virtual ~SaveLoadSelectorItem()
    {
        delete pScreenshotSprite;
        pScreenshotSprite = NULL;
    }

    static bool CompareByTimestampDescending(SaveLoadSelectorItem *pItem1, SaveLoadSelectorItem *pItem2)
    {
        return pItem1->GetTimestamp() > pItem2->GetTimestamp();
    }

    string GetDisplayString() const { return saveName; }
    bool GetShouldDisplayStar() const { return false; }
    string GetSaveName() const { return saveName; }
    Image * GetScreenshotSprite() { return pScreenshotSprite; }
    time_t GetTimestamp() const { return timestamp; }
    string GetDescription() const { return description; }
    string GetFilePath() const { return filePath; }

private:
    string saveName;
    Image *pScreenshotSprite;
    time_t timestamp;
    string description;
    string filePath;
};

class NewSaveSelectorItem : public SelectorItem
{
public:
    NewSaveSelectorItem(Image *pScreenshotSprite, Image *pScreenshotFullSizeSprite)
    {
        this->pScreenshotSprite = pScreenshotSprite;
        this->pScreenshotFullSizeSprite = pScreenshotFullSizeSprite;
    }

    ~NewSaveSelectorItem()
    {
        delete pScreenshotSprite;
        pScreenshotSprite = NULL;
        delete pScreenshotFullSizeSprite;
        pScreenshotFullSizeSprite = NULL;
    }

    string GetDisplayString() const { return "New Save"; }
    bool GetShouldDisplayStar() const { return false; }
    Image * GetScreenshot() const { return this->pScreenshotSprite; }
    Image * GetScreenshotFullSize() const { return this->pScreenshotFullSizeSprite; }

private:
    Image *pScreenshotSprite;
    Image *pScreenshotFullSizeSprite;
};

class SelectorSection
{
public:
    SelectorSection(string sectionTitle)
    {
        this->sectionTitle = sectionTitle;
    }

    virtual ~SelectorSection()
    {
        for (unsigned int i = 0; i < itemList.size(); i++)
        {
            delete itemList[i];
        }

        itemList.clear();
    }

    string GetTitle() const { return this->sectionTitle; }
    SelectorItem * GetItemAt(unsigned int index) { return this->itemList[index]; }
    string GetItemDisplayStringAt(unsigned int index) { return this->itemList[index]->GetDisplayString(); }
    bool GetItemShouldDisplayStarAt(unsigned int index) { return this->itemList[index]->GetShouldDisplayStar(); }
    void DeleteItemAt(unsigned int index);
    unsigned int GetCount() { return this->itemList.size(); }

    void AddItem(SelectorItem *pSelectorItem)
    {
        itemList.push_back(pSelectorItem);
    }

private:
    string sectionTitle;
    vector<SelectorItem *> itemList;
};

class Selector
{
public:
    static void Initialize(Image *pDividerSprite, Image *pHighlightSprite, Image *pStarSprite);

    Selector(int xPosition, int yPosition, int width, int height);
    Selector(const Selector &other);
    virtual ~Selector();

    static string GetClickSoundEffect() { return "ButtonClick2"; }

    SelectorSection * GetSection(int index) { return this->sectionList[index]; }
    unsigned int GetSectionCount() { return this->sectionList.size(); }

    void AddSection(SelectorSection *pSection)
    {
        sectionList.push_back(pSection);
    }

    void Init();
    void Update(int delta);
    void UpdateAnimation(int delta);
    void UpdateState();
    void Draw();
    void Reset();

    void DeleteCurrentItem();
    void PopulateWithCases(bool requireSaveFilesExist);

private:
    static void EnsureFonts();

    static Font *pLargeFont;
    static Font *pMediumFont;
    static Font *pSmallFont;

    static Image *pDividerSprite;
    static Image *pHighlightSprite;
    static Image *pStarSprite;

    void GetCurrentSectionAndIndex(unsigned int *pCurrentSection, unsigned int *pCurrentSectionStartIndex, unsigned int *pCurrentIndex);
    int GetExtent();
    int GetPartialExtentFrom(int sectionIndex, int itemIndex);

    int xPosition;
    int yPosition;
    int width;
    int height;

    vector<SelectorSection *> sectionList;

    unsigned int topIndex;
    unsigned int selectedIndex;
    unsigned int mouseOverIndex;

    Arrow *pUpArrow;
    Arrow *pDownArrow;
};

#endif
