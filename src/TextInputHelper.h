/**
 * Basic header/include file for TextInputHelper.cpp.
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

#ifndef TEXTINPUTHELPER_H
#define TEXTINPUTHELPER_H

#include <SDL2/SDL.h>
#include <string>

using namespace std;

class Font;

class TextInputHelper
{
public:
    static void Init();

    static void StartSession();
    static void EndSession();

    static void NotifyKeyDown(SDL_Keycode keycode);
    static void NotifyKeyUp(SDL_Keycode keycode);
    static void NotifyTextInput(string newText);
    static void Update(int delta);
    static void Reset();

    static string GetCurrentText() { return currentText; }
    static void SetCurrentText(string currentText) { TextInputHelper::currentText = currentText; caretPosition = currentText.length(); }
    static unsigned int GetCaretPosition() { return caretPosition; }
    static bool GetIsCaretShowing() { return isCaretShowing; }
    static void SetMaxPixelWidth(int pixelWidth, Font *pFontToCheckAgainst) { maxPixelWidth = pixelWidth; TextInputHelper::pFontToCheckAgainst = pFontToCheckAgainst; }

    static bool GetUserHasConfirmed() { return userHasConfirmed; }
    static bool GetUserHasCanceled() { return userHasCanceled; }

private:
    static bool SetNewText(string newText, unsigned int newCaretPosition);
    static void HandleSpecialKey(SDL_Keycode keycode);
    static void ResetCaret();

    static string currentText;
    static unsigned int caretPosition;
    static bool isCaretShowing;
    static int timeBeforeCaretToggle;

    static SDL_Keycode keyDownForRepeat;
    static int msSinceKeyDown;
    static bool isRepeating;

    static int maxPixelWidth;
    static Font *pFontToCheckAgainst;

    static bool userHasConfirmed;
    static bool userHasCanceled;
};

#endif
