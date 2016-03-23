/*********************************************************************/
/* Copyright (c) 2015, EPFL/Blue Brain Project                       */
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

#ifndef CONTENTACTION_H
#define CONTENTACTION_H

#include <QtCore/QObject>
#include <QtCore/QUuid>

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

/**
 * A content-specific action for use in QML by ContentActionsModel.
 */
class ContentAction : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QString icon READ getIcon NOTIFY iconChanged )
    Q_PROPERTY( QString iconChecked READ getIconChecked NOTIFY iconCheckedChanged )
    Q_PROPERTY( bool checkable READ isCheckable WRITE setCheckable NOTIFY checkableChanged )
    Q_PROPERTY( bool checked READ isChecked WRITE setChecked NOTIFY checkedChanged )
    Q_PROPERTY( bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged )

public:
    /** Constructor. */
    explicit ContentAction( const QUuid& actionId = QUuid::createUuid( ));

    /** @name QProperty getters */
    //@{
    const QString& getIcon() const;
    const QString& getIconChecked() const;
    bool isCheckable() const;
    bool isChecked() const;
    bool isEnabled() const;
    //@}

public slots:
    /** Trigger the action. */
    void trigger();

    /** @name QProperty setters */
    //@{
    void setIcon( QString icon );
    void setIconChecked( QString icon );
    void setChecked( bool value );
    void setCheckable( bool value );
    void setEnabled( bool value );
    //@}

signals:
    /** The action has been checked. */
    void checked();

    /** The action has been unchecked. */
    void unchecked();

    /** The action has been triggered. */
    void triggered( QUuid actionId, bool checked );

    /** @name QProperty notifiers */
    //@{
    void iconChanged();
    void iconCheckedChanged();
    void checkedChanged();
    void checkableChanged();
    void enabledChanged();
    //@}

private:
    Q_DISABLE_COPY( ContentAction )

    friend class boost::serialization::access;

    template< class Archive >
    void serialize( Archive & ar, const unsigned int )
    {
        ar & boost::serialization::make_nvp( "icon", _icon );
        ar & boost::serialization::make_nvp( "iconChecked", _iconChecked );
        ar & boost::serialization::make_nvp( "checkable", _checkable );
        ar & boost::serialization::make_nvp( "checked", _checked );
        ar & boost::serialization::make_nvp( "enabled", _enabled );
    }

    QUuid _uuid;
    QString _icon;
    QString _iconChecked;
    bool _checkable;
    bool _checked;
    bool _enabled;
};

#endif // CONTENTACTION_H
