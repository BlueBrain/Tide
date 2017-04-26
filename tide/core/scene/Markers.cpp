/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
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

#include "Markers.h"

Markers::Markers(QObject* parent_)
    : QAbstractListModel(parent_)
{
}

QVariant Markers::data(const QModelIndex& index_, const int role) const
{
    if (index_.row() < 0 || index_.row() >= rowCount() || !index_.isValid())
        return QVariant();

    switch (role)
    {
    case XPOSITION_ROLE:
        return QVariant(_markers[index_.row()].second.x());
    case YPOSITION_ROLE:
        return QVariant(_markers[index_.row()].second.y());
    }
    return QVariant();
}

int Markers::rowCount(const QModelIndex& parent_) const
{
    Q_UNUSED(parent_);
    return _markers.size();
}

QHash<int, QByteArray> Markers::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[XPOSITION_ROLE] = "xposition";
    roles[YPOSITION_ROLE] = "yposition";
    return roles;
}

void Markers::addMarker(const int id, const QPointF& position)
{
    if (_findMarker(id) != _markers.end())
        return;

    const int markerIndex = _markers.size();
    beginInsertRows(QModelIndex(), markerIndex, markerIndex);
    _markers.push_back(Marker(id, position));
    endInsertRows();
    emit(updated(shared_from_this()));
}

void Markers::updateMarker(const int id, const QPointF& position)
{
    auto it = _findMarker(id);

    if (it == _markers.end())
        return;

    it->second = position;
    emit(updated(shared_from_this()));

    const int markerIndex = it - _markers.begin();
    emit dataChanged(createIndex(markerIndex, 0), createIndex(markerIndex, 0));
}

void Markers::removeMarker(const int id)
{
    auto it = _findMarker(id);

    if (it == _markers.end())
        return;

    const int markerIndex = it - _markers.begin();
    beginRemoveRows(QModelIndex(), markerIndex, markerIndex);
    _markers.erase(it);
    endRemoveRows();
    emit(updated(shared_from_this()));
}

Markers::MarkersVector::iterator Markers::_findMarker(const int id)
{
    auto it = std::find_if(_markers.begin(), _markers.end(),
                           [&id](const Marker& marker) {
                               return marker.first == id;
                           });
    return it;
}
