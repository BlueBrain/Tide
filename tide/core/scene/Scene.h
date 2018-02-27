/*********************************************************************/
/* Copyright (c) 2018, EPFL/Blue Brain Project                       */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
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

#ifndef SCENE_H
#define SCENE_H

#include "scene/Surface.h"
#include "serialization/includes.h"
#include "types.h"

class invalid_surface_index_error : public std::runtime_error
{
    using runtime_error::runtime_error;
};
class window_not_found_error : public std::runtime_error
{
    using runtime_error::runtime_error;
};

/**
 * Contains all the DisplayGroups for the different surfaces to be rendered.
 */
class Scene : public QObject, public std::enable_shared_from_this<Scene>
{
    Q_OBJECT
    Q_DISABLE_COPY(Scene)

public:
    static ScenePtr create(const QSize& size);
    static ScenePtr create(const std::vector<SurfaceConfig>& surfaces);
    static ScenePtr create(const std::vector<DisplayGroupPtr>& groups);
    static ScenePtr create(DisplayGroupPtr group);

    /** Destructor. */
    ~Scene();

    /** @return the number of surfaces. */
    size_t getSurfaceCount() const;

    /** @return the surfaces that are part of the scene. */
    const std::vector<Surface>& getSurfaces() const;

    /**
     * @return the group for a certain surface index
     * @throw invalid_surface_index
     */
    DisplayGroup& getGroup(const size_t surfaceIndex);
    const DisplayGroup& getGroup(const size_t surfaceIndex) const;

    /** Get all windows from all groups. */
    ContentWindowPtrs getContentWindows() const;

    /**
     * Move this object and its member QObjects to the given QThread.
     *
     * This intentionally shadows the default QObject::moveToThread to include
     * member QObjects which are stored using shared_ptr and thus can't be made
     * direct children of this class.
     * @param thread the target thread.
     */
    void moveToThread(QThread* thread);

    /** Find a window by its id. */
    ContentWindowPtr findWindow(const QUuid& id) const;

    /**
     * Find a window and the group it belongs to.
     * @param id of the window to find.
     * @return window and group.
     * @throw window_not_found_error
     */
    std::pair<ContentWindow&, DisplayGroup&> findWindowAndGroup(
        const QUuid& id);
    std::pair<ContentWindowPtr, DisplayGroup&> findWindowPtrAndGroup(
        const QUuid& id);

signals:
    /** Emitted when any value is changed by one of the setters. */
    void modified(ScenePtr);

private:
    friend class boost::serialization::access;

    /** Default constructor for serialization. */
    Scene() = default;
    Scene(const std::vector<SurfaceConfig>& surfaces);
    Scene(const std::vector<DisplayGroupPtr>& groups);

    void _forwardSceneModifiedSignals();
    void _sendScene();

    template <class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
        // clang-format off
        ar & boost::serialization::make_nvp("surfaces", _surfaces);
        // clang-format on
    }

    std::vector<Surface> _surfaces;
};

#endif
