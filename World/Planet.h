/*
    OpenSR - opensource multi-genre game based upon "Space Rangers 2: Dominators"
    Copyright (C) 2015 Kosyak <ObKo@mail.ru>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OPENSR_WORLD_PLANET_H
#define OPENSR_WORLD_PLANET_H

#include "World.h"
#include "SpaceObject.h"
#include "Resource.h"

namespace OpenSR
{
namespace World
{
class OPENSR_WORLD_API PlanetStyle: public Resource
{
    Q_GADGET

    Q_PROPERTY(QString texture READ texture WRITE setTexture)
    //Q_PROPERTY(QColor color READ color WRITE setColor)

public:
    struct Data
    {
        QString texture;
        //QColor color;
    };

    QString texture() const;
    //QColor color() const;

    void setTexture(const QString& texture);
    //void setColor(const QColor& color);
};

QDataStream& operator<<(QDataStream & stream, const PlanetStyle& style);
QDataStream& operator>>(QDataStream & stream, PlanetStyle& style);
QDataStream& operator<<(QDataStream & stream, const PlanetStyle::Data& data);
QDataStream& operator>>(QDataStream & stream, PlanetStyle::Data& data);

class OPENSR_WORLD_API Planet: public SpaceObject
{
    Q_OBJECT
    OPENSR_WORLD_OBJECT

    Q_PROPERTY(OpenSR::World::PlanetStyle style READ style WRITE setStyle NOTIFY styleChanged)

public:
    Q_INVOKABLE Planet(WorldObject *parent = 0, quint32 id = 0);
    virtual ~Planet();

    virtual quint32 typeId() const;
    virtual QString namePrefix() const;

    PlanetStyle style() const;

public slots:
    void setStyle(const PlanetStyle& style);

signals:
    void styleChanged(const PlanetStyle& style);

private:
    PlanetStyle m_style;
};
}
}

Q_DECLARE_METATYPE(OpenSR::World::PlanetStyle)
Q_DECLARE_METATYPE(OpenSR::World::PlanetStyle::Data)

#endif // OPENSR_WORLD_PLANET_H
