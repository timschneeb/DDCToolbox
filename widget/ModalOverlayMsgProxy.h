/*
 *  This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *  ThePBone <tim.schneeberger(at)outlook.de> (c) 2020
 */
#ifndef OVERLAYMSGPROXY_H
#define OVERLAYMSGPROXY_H

#include <QObject>

class QMessageOverlay;

class ModalOverlayMsgProxy : public QObject
{
    Q_OBJECT
public:
    ModalOverlayMsgProxy(QWidget* obj);
    void openBase( const QString& title, const QString& desc);
    void hide();
signals:
    void buttonPressed();
    void fadedOut();
private:
    QMessageOverlay *lightBox;
    QWidget* obj;
};

#endif // OVERLAYMSGPROXY_H
