/*********************************************************************/
/* Copyright (c) 2013-2017, EPFL/Blue Brain Project                  */
/*                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>*/
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/*   1. Redistributions of source code must retain the above         */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer.                                                  */
/*                                                                   */
/*   2. Redistributions in binary form must reproduce the above      */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer in the documentation and/or other materials       */
/*      provided with the distribution.                              */
/*                                                                   */
/*    THIS  SOFTWARE IS PROVIDED  BY THE  UNIVERSITY OF  TEXAS AT    */
/*    AUSTIN  ``AS IS''  AND ANY  EXPRESS OR  IMPLIED WARRANTIES,    */
/*    INCLUDING, BUT  NOT LIMITED  TO, THE IMPLIED  WARRANTIES OF    */
/*    MERCHANTABILITY  AND FITNESS FOR  A PARTICULAR  PURPOSE ARE    */
/*    DISCLAIMED.  IN  NO EVENT SHALL THE UNIVERSITY  OF TEXAS AT    */
/*    AUSTIN OR CONTRIBUTORS BE  LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL,  SPECIAL, EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES    */
/*    (INCLUDING, BUT  NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE    */
/*    GOODS  OR  SERVICES; LOSS  OF  USE,  DATA,  OR PROFITS;  OR    */
/*    BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON ANY THEORY OF    */
/*    LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR TORT    */
/*    (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY OUT    */
/*    OF  THE  USE OF  THIS  SOFTWARE,  EVEN  IF ADVISED  OF  THE    */
/*    POSSIBILITY OF SUCH DAMAGE.                                    */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#ifndef TYPES_H
#define TYPES_H

#include <deflect/types.h>

#include <QRectF>
#include <QString>

#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <set>
#include <vector>

/**
 * The type of texture formats that Tide can render.
 */
enum class TextureFormat
{
    rgba,
    yuv420,
    yuv422,
    yuv444
};

/**
 * The supported color spaces for rendering image textures.
 */
enum class ColorSpace
{
    undefined,
    yCbCrJpeg,
    yCbCrVideo
};

/**
 * The power state of the wall's displays.
 */
enum class ScreenState
{
    ON,
    OFF,
    UNDEF
};

/**
 * The methods for synchronizing the GL buffer swap.
 */
enum class SwapSync
{
    software,
    hardware
};

/**
 * The different texture update policies.
 */
enum class TextureType
{
    Static,
    Dynamic
};

class Background;
class Configuration;
class Content;
class ContentSynchronizer;
class ContentWindow;
class CountdownStatus;
class DataProvider;
class DataSource;
class DisplayGroup;
class DisplayGroupController;
class DisplayGroupRenderer;
class FFMPEGFrame;
class FFMPEGMovie;
class FFMPEGPicture;
class FFMPEGVideoFrameConverter;
class FFMPEGVideoStream;
class Image;
class ImageSource;
class ImagePyramidDataSource;
class InactivityTimer;
class LoggingUtility;
class Markers;
class MasterConfiguration;
class MovieContent;
class MovieUpdater;
class MPIChannel;
class NetworkBarrier;
class Options;
class PDFContent;
class PixelStreamUpdater;
class PixelStreamWindowManager;
class ScreenLock;
class SharedNetworkBarrier;
class SwapSynchronizer;
class TestPattern;
class Tile;
class WallConfiguration;
class WallSceneRenderer;
class WallToWallChannel;
class WallWindow;
class WebbrowserContent;

typedef std::shared_ptr<Background> BackgroundPtr;
typedef std::unique_ptr<Content> ContentPtr;
typedef std::shared_ptr<ContentSynchronizer> ContentSynchronizerSharedPtr;
typedef std::shared_ptr<ContentWindow> ContentWindowPtr;
typedef std::shared_ptr<CountdownStatus> CountdownStatusPtr;
typedef std::shared_ptr<DisplayGroup> DisplayGroupPtr;
typedef std::shared_ptr<const DisplayGroup> DisplayGroupConstPtr;
typedef std::shared_ptr<Image> ImagePtr;
typedef std::shared_ptr<FFMPEGPicture> PicturePtr;
typedef std::shared_ptr<Markers> MarkersPtr;
typedef std::shared_ptr<MPIChannel> MPIChannelPtr;
typedef std::shared_ptr<Options> OptionsPtr;
typedef std::shared_ptr<ScreenLock> ScreenLockPtr;
typedef std::shared_ptr<Tile> TilePtr;
typedef std::weak_ptr<Tile> TileWeakPtr;

typedef std::set<ContentWindowPtr> ContentWindowSet;
typedef std::vector<ContentWindowPtr> ContentWindowPtrs;
typedef std::set<size_t> Indices;
typedef std::vector<QPointF> Positions;

using BoolCallback = std::function<void(bool)>;

static const QRectF UNIT_RECTF(0.0, 0.0, 1.0, 1.0);
static const QSize UNDEFINED_SIZE(-1, -1);

inline bool operator<(const QSizeF& a, const QSizeF& b)
{
    return (a.width() < b.width() || a.height() < b.height());
}

inline bool operator<=(const QSizeF& a, const QSizeF& b)
{
    return (a.width() <= b.width() || a.height() <= b.height());
}

inline bool operator>(const QSizeF& a, const QSizeF& b)
{
    return (a.width() > b.width() || a.height() > b.height());
}

inline bool operator>=(const QSizeF& a, const QSizeF& b)
{
    return (a.width() >= b.width() || a.height() >= b.height());
}

inline std::ostream& operator<<(std::ostream& str, const QSizeF& s)
{
    str << s.width() << 'x' << s.height();
    return str;
}

inline std::ostream& operator<<(std::ostream& str, const QPointF& p)
{
    str << p.x() << ',' << p.y();
    return str;
}

inline std::ostream& operator<<(std::ostream& str, const ScreenState state)
{
    switch (state)
    {
    case ScreenState::ON:
        str << "ON";
        break;
    case ScreenState::OFF:
        str << "OFF";
        break;
    case ScreenState::UNDEF:
        str << "UNDEF";
        break;
    }
    return str;
}

inline std::ostream& operator<<(std::ostream& str, const QRectF& r)
{
    str << r.x() << ',' << r.y() << ' ' << r.width() << 'x' << r.height();
    return str;
}

inline std::ostream& operator<<(std::ostream& str, const QString& s)
{
    str << s.toStdString();
    return str;
}

// missing make_unique() implementation in C++11 standard
// source: http://herbsutter.com/gotw/_102/
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename R>
bool is_ready(std::future<R> const& f)
{
    return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

template <typename T>
T set_difference(const T& v1, const T& v2)
{
    T diff;
    std::set_difference(v1.begin(), v1.end(), v2.begin(), v2.end(),
                        std::inserter(diff, diff.begin()));
    return diff;
}

#endif
