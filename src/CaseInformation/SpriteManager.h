/**
 * Basic header/include file for SpriteManager.cpp.
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

#ifndef SPRITEMANAGER_H
#define SPRITEMANAGER_H

#include "../enums.h"
#include "../Image.h"
#include "../Sprite.h"
#include "../XmlReader.h"
#include <map>

class SpriteManager
{
public:
    SpriteManager(ManagerSource managerSource);
    ~SpriteManager();

    Sprite * GetSpriteFromId(string id);
    Image * GetImageFromId(string id);
    void AddSprite(string id, string spriteSheetId, RectangleWH spriteClipRect);
    void AddImage(string id, Image *pImage);
    void LoadImageFromFilePath(string id);
    void DeleteImage(string id);
    void LoadFromXml(XmlReader *pReader);
    void FinishUpdateLoadedTextures(string newLocationId);
    void UnloadResources();

private:
    map<string, Sprite *> spriteByIdMap;
    map<string, Image *> smartSpriteByIdMap;
    map<string, string> smartSpriteFilePathByIdMap;

    ManagerSource managerSource;
    SDL_sem *pImageByIdSemaphore;
};

#endif
