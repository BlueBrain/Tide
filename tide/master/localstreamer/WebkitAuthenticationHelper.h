/*********************************************************************/
/* Copyright (c) 2013, EPFL/Blue Brain Project                       */
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

#ifndef WEBKITAUTHENTICATIONHELPER_H
#define WEBKITAUTHENTICATIONHELPER_H

#include <QObject>
#include <QUrl>
class QNetworkReply;
class QAuthenticator;
class QWebView;

/**
 * Handle HTTP authentication requests for a QWebView.
 *
 * The WebkitAuthenticationHelper class intercepts HTTP authentication requests
 * an displays a simple html login page.
 *
 * It offers a replacement to the system dialog box when QWebView is used
 * without a window.
 *
 * Usage: Create one WebkitAuthenticationHelper instance for each
 * QWebView that needs to support HTTP authentication.
 */
class WebkitAuthenticationHelper : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param webView The QWebView for which to add authentication handling.
     */
    WebkitAuthenticationHelper(QWebView& webView);

protected slots:
    /** @name Internal Callbacks
     * These slots are accessed internally by the Javascript only but cannot be
     * private, otherwise the compiler ignores them at compilation time.
     */
    //@{
    void loginFormInputChanged(const QString& inputName,
                               const QString& inputValue);
    void loginFormSubmitted();
    //@}

private slots:
    void handleAuthenticationRequest(QNetworkReply*,
                                     QAuthenticator* authenticator);
    void errorPageFinishedLoading(bool ok);

private:
    Q_DISABLE_COPY(WebkitAuthenticationHelper)

    void _displayLoginPage();
    void _registerLoginFormCallbacks();
    void _sendCredentials(QAuthenticator* authenticator) const;
    QString _readQrcFile(const QString& filename);

    QWebView& _webView;

    bool _userHasInputNewCredentials;
    QString _username;
    QString _password;
};

#endif
