/**
 * Basic header/include file for ResourceLoader.cpp.
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

#ifndef RESOURCELOADER_H
#define RESOURCELOADER_H

#include <SDL2/SDL.h>
#ifdef __OSX
#include <SDL2_ttf/SDL_ttf.h>
#include <SDL2_image/SDL_image.h>
#else
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#endif

#include "Image.h"
#include "miniz.h"

#include <map>
#include <vector>
#include <deque>

#include <cryptopp/sha.h>

extern "C"
{
    #include <libavformat/avformat.h>
}

class ArchiveSource;

const int IOContextBufferSize = 32768;

class RWOpsIOContext
{
public:
    RWOpsIOContext(SDL_RWops *pRW) :
        bufferSize(IOContextBufferSize),
        pBuffer(static_cast<unsigned char*>(av_malloc(IOContextBufferSize))),
        pRW(pRW)
    {
        pIOContext =
            avio_alloc_context(
                pBuffer,
                bufferSize,
                0,
                this,
                &RWOpsIOContext::Read,
                &RWOpsIOContext::Write,
                &RWOpsIOContext::Seek);
    }

    ~RWOpsIOContext()
    {
        av_freep(&pIOContext);
        av_freep(&pBuffer);
    }

    static int Read(void *opaque, unsigned char *buf, int buf_size)
    {
        RWOpsIOContext* pRWOpsIOContext = static_cast<RWOpsIOContext*>(opaque);
        SDL_RWops *pRW = pRWOpsIOContext->pRW;
        return SDL_RWread(pRW, buf, 1, buf_size);
    }

    static int Write(void *opaque, unsigned char *buf, int buf_size)
    {
        RWOpsIOContext* pRWOpsIOContext = static_cast<RWOpsIOContext*>(opaque);
        SDL_RWops *pRW = pRWOpsIOContext->pRW;
        return SDL_RWwrite(pRW, buf, 1, buf_size);
    }

    static int64_t Seek(void *opaque, int64_t offset, int whence)
    {
        RWOpsIOContext* pRWOpsIOContext = static_cast<RWOpsIOContext*>(opaque);
        SDL_RWops *pRW = pRWOpsIOContext->pRW;
        return (int64_t)SDL_RWseek(pRW, (Sint64)offset, whence);
    }

    AVIOContext *GetAVIOContext() { return pIOContext; }
    unsigned char * GetBuffer() { return pBuffer; }

private:
    int bufferSize;
    unsigned char *pBuffer;
    SDL_RWops *pRW;
    AVIOContext *pIOContext;
};

class ResourceLoader
{
private:
    class ArchiveSource
    {
    public:
        ArchiveSource()
            : zip_archive(mz_zip_archive())
        {
        }

        ~ArchiveSource();

        static bool CreateAndInit(string archiveFilePath, ArchiveSource **ppSource);
        SDL_RWops * LoadFile(string relativeFilePath, void **ppMemToFree);
        void * LoadFileToMemory(string relativeFilePath, unsigned int *pSize);

    private:
        bool Init(string archiveFilePath);

        mz_zip_archive zip_archive;
    };

    class LoadResourceStep
    {
    public:
        virtual ~LoadResourceStep() { }
        virtual void Execute() = 0;
    };

    class LoadImageStep : public LoadResourceStep
    {
    public:
        LoadImageStep(string spriteId)
        {
            this->spriteId = spriteId;
        }

        void Execute();
        string GetSpriteId() { return this->spriteId; }

    private:
        string spriteId;
    };

    class DeleteImageStep : public LoadResourceStep
    {
    public:
        DeleteImageStep(string spriteId)
        {
            this->spriteId = spriteId;
        }

        void Execute();
        string GetSpriteId() { return this->spriteId; }

    private:
        string spriteId;
    };

    class LoadVideoStep : public LoadResourceStep
    {
    public:
        LoadVideoStep(Video *pVideo)
        {
            this->pVideo = pVideo;
        }

        void Execute();
        Video * GetVideo() { return this->pVideo; }

    private:
        Video *pVideo;
    };

    class DeleteVideoStep : public LoadResourceStep
    {
    public:
        DeleteVideoStep(Video *pVideo)
        {
            this->pVideo = pVideo;
        }

        void Execute();
        Video * GetVideo() { return this->pVideo; }

    private:
        Video *pVideo;
    };

public:
    static void Close();

    static ResourceLoader * GetInstance()
    {
        if (pInstance == NULL)
        {
            pInstance = new ResourceLoader();
        }

        return pInstance;
    }

    bool Init(string commonResourcesFilePath);
    bool LoadCase(string caseFilePath);
    bool LoadTemporaryCase(string caseFilePath);
    void UnloadTemporaryCase();
    void UnloadCase();
    SDL_Surface * LoadRawSurface(string relativeFilePath);
    Image * LoadImage(string relativeFilePath);
    void ReloadImage(Image *pSprite, string originFilePath);
    Document * LoadDocument(string relativeFilePath);
    TTF_Font * LoadFont(string relativeFilePath, int ptSize);

    void LoadVideo(
        string relativeFilePath,
        RWOpsIOContext **ppRWOpsIOContext,
        AVFormatContext **ppFormatContext,
        int *pVideoStream,
        AVCodecContext **ppCodecContext,
        AVCodec **ppCodec,
        void **ppMemToFree);

    void PreloadMusic(string id, string relativeFilePath);
    void UnloadMusic(string id);
    void PreloadSound(string id, string relativeFilePath);
    void UnloadSound(string id);
    void PreloadDialog(string id, string relativeFilePath);
    void UnloadDialog(string id);

    void * LoadFileToMemory(string relativeFilePath, unsigned int *pFileSize);
    void HashFile(string relativeFilePath, byte hash[CryptoPP::SHA256::DIGESTSIZE]);

    void AddImage(Image *pImage);
    void RemoveImage(Image *pImage);
    void TryLoadOneImageTexture();
    bool HasImageTexturesToLoad();
    void FlushImages();

    void AddImageIdToLoadList(string id);
    void AddImageIdToDeleteList(string id);
    void AddVideoToLoadList(Video *pVideo);
    void AddVideoToDeleteList(Video *pVideo);

    void SnapLoadStepQueue();
    bool HasLoadStep();
    void TryRunOneLoadStep();

private:
    ResourceLoader();
    ~ResourceLoader();

    static ResourceLoader *pInstance;

    ArchiveSource *pCommonResourcesSource;
    ArchiveSource *pCaseResourcesSource;
    ArchiveSource *pCachedCaseResourcesSource;

    SDL_sem *pLoadingSemaphore;

    map<string, void *> musicIdToMemToFreeMap;

    deque<Image *> smartSpriteQueue;
    deque<SDL_Texture *> deleteTextureQueue;
    SDL_sem *pQueueSemaphore;

    deque<LoadResourceStep *> loadResourceStepList;
    deque<LoadResourceStep *> cachedLoadResourceStepList;
    SDL_sem *pLoadQueueSemaphore;
};

#endif
