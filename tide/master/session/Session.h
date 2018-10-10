/*********************************************************************/
/* Copyright (c) 2013-2018, EPFL/Blue Brain Project                  */
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

#ifndef SESSION_H
#define SESSION_H

#include "types.h"

#include "scene/Scene.h" // member, needed for serialization
#include "serialization/includes.h"

/**
 * The different versions of the xml files.
 */
enum SessionFileVersion
{
    INVALID_FILE_VERSION = -1,
    FIRST_BOOST_FILE_VERSION = 0,
    LEGACY_FILE_VERSION = 1,
    FIRST_PIXEL_COORDINATES_FILE_VERSION = 2,
    WINDOW_TITLES_VERSION = 3,
    FOCUS_MODE_VERSION = 4,
    SCENE_VERSION = 5
};

/**
 * Additional information about the session.
 */
struct SessionInfo
{
    QString filepath;
    SessionFileVersion version = INVALID_FILE_VERSION;
};

/**
 * A user session which can be saved and restored.
 */
class Session
{
public:
    /** Default constructor. */
    Session() = default;

    /**
     * Constructor.
     * @param scene to serialize.
     * @param filepath where the session was last saved or loaded.
     */
    Session(ScenePtr scene, QString filepath = QString(),
            SessionFileVersion version = INVALID_FILE_VERSION);

    /** Assign a new Session without changing the Scene QObject instance. */
    void assign(const Session& other);

    /** @return the additional session information. */
    const SessionInfo& getInfo() const;

    /** Clear the session information. */
    void clearInfo();

    /** @return the scene object. */
    ScenePtr getScene();

    /** Update filepath of the session. */
    void setFilepath(const QString& filepath);

    /** Set the version to the current file version. */
    void setCurrentFileVersion() { _info.version = SCENE_VERSION; }
private:
    ScenePtr _scene;
    SessionInfo _info;

    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        _info.version = static_cast<SessionFileVersion>(version);

        // clang-format off
        if (version < SCENE_VERSION)
        {
            auto group = DisplayGroup::create(QSize());
            if (version < FIRST_PIXEL_COORDINATES_FILE_VERSION)
                group->setCoordinates(UNIT_RECTF);

            if (version < WINDOW_TITLES_VERSION)
            {
                WindowPtrs windows;
                ar & boost::serialization::make_nvp("contentWindows",
                                                    windows);
                group->replaceWindows(windows);
            }
            else
            {
                ar & boost::serialization::make_nvp("displayGroup", group);
            }
            _scene = Scene::create(std::move(group));
        }
        else
        {
            ar & boost::serialization::make_nvp("scene", _scene);
        }
        // clang-format on
    }
};

BOOST_CLASS_VERSION(Session, SCENE_VERSION)

#endif
