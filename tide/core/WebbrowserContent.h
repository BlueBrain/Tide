/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
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

#ifndef WEBBROWSER_CONTENT_H
#define WEBBROWSER_CONTENT_H

#include "PixelStreamContent.h" // Base class
#include "WebbrowserHistory.h"  // Member

#include <boost/serialization/base_object.hpp>

/**
 * The Webbrowser is a PixelStream extended with history navigation.
 */
class WebbrowserContent : public PixelStreamContent
{
    Q_OBJECT
    Q_PROPERTY( int cursorPosition READ getCursorPosition
                WRITE setCursorPosition NOTIFY cursorPositionChanged )
    Q_PROPERTY( int page READ getPage NOTIFY pageChanged )
    Q_PROPERTY( int pageCount READ getPageCount NOTIFY pageCountChanged )
    Q_PROPERTY( int restPort READ getRestPort NOTIFY restPortChanged )
    Q_PROPERTY( int selectionStart READ getSelectionStart
                WRITE setSelectionStart NOTIFY selectionStartChanged )
    Q_PROPERTY( int selectionEnd READ getSelectionEnd
                WRITE setSelectionEnd NOTIFY selectionEndChanged )
    Q_PROPERTY( QString url READ getUrl WRITE setUrl NOTIFY urlChanged )
    Q_PROPERTY( bool addressBarFocused READ isAddressBarFocused
                WRITE setAddressBarFocused NOTIFY addressBarFocusedChanged )

public:
    /**
     * Constructor.
     * @param uri The unique stream identifier.
     */
    explicit WebbrowserContent( const QString& uri );

    /** Get the content type **/
    CONTENT_TYPE getType() const final;

    /** @return false, webbrowsers can adjust their aspect ratio. */
    bool hasFixedAspectRatio() const final;

    /** @return the qml file which contains the webbrowser controls. */
    QString getQmlControls() const final;

    /** Get the index of the page navigation history. */
    int getPage() const;

    /** Get the number of pages in the navigation history. */
    int getPageCount() const;

    /** Get the port number of the webbrowser's REST interface. */
    int getRestPort() const;

    /** Get the position (index) of the cursor. */
    int getCursorPosition() const;

    /** Set the position (index) of the cursor. */
    void setCursorPosition( int index );

    /** Get the start position of the selection in the address bar. */
    int getSelectionStart() const;

    /** Set the start position of the selection in the address bar. */
    void setSelectionStart( int pos );

    /** Get the end position of the selection in the address bar. */
    int getSelectionEnd() const;

    /** Set the end position of the selection in the address bar. */
    void setSelectionEnd( int pos );

    /** Get the url of the current webpage. */
    QString getUrl() const;

    /** Set the url in the address bar. */
    void setUrl( QString url );

    /** @return true if the address bar has focus (capture keyboard events). */
    bool isAddressBarFocused() const;

    /** Give the keyboard focus to the address bar. */
    void setAddressBarFocused( bool set );

    /** Parse navigation history data received from the deflect::Stream. */
    void parseData( QByteArray data ) final;

signals:
    /** @name QProperty notifiers */
    //@{
    void pageChanged();
    void pageCountChanged();
    void restPortChanged();
    void cursorPositionChanged();
    void selectionStartChanged();
    void selectionEndChanged();
    void urlChanged();
    void addressBarFocusedChanged();
    //@}

private:
    friend class boost::serialization::access;

    // Default constructor required for boost::serialization
    WebbrowserContent() {}

    /** Serialize for sending to Wall applications. */
    template< class Archive >
    void serialize( Archive & ar, const unsigned int /*version*/ )
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP( PixelStreamContent );
        ar & _history;
        ar & _addressBarFocused;
        ar & _addressBarUrl;
        ar & _cursorPosition;
        ar & _selectionStart;
        ar & _selectionEnd;
    }

    /** Serialize for saving to an xml file */
    template< class Archive >
    void serialize_members_xml( Archive & ar, const unsigned int /*version*/ )
    {
        // serialize base class information (with NVP for xml archives)
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP( PixelStreamContent );
        ar & boost::serialization::make_nvp( "history", _history );
    }

    /** Loading from xml. */
    void serialize_for_xml( boost::archive::xml_iarchive& ar,
                            const unsigned int version )
    {
        serialize_members_xml( ar, version );
    }

    /** Saving to xml. */
    void serialize_for_xml( boost::archive::xml_oarchive& ar,
                            const unsigned int version )
    {
        serialize_members_xml( ar, version );
    }

    WebbrowserHistory _history;
    int _restPort = 0;

    bool _addressBarFocused = false;
    QString _addressBarUrl;
    int _cursorPosition = 0;
    int _selectionStart = 0;
    int _selectionEnd = 0;
};

DECLARE_SERIALIZE_FOR_XML( WebbrowserContent )

BOOST_CLASS_EXPORT_KEY( WebbrowserContent )

#endif
