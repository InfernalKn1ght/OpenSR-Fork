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

#ifndef OPENSR_WORLD_MANNEDOBJECT_H
#define OPENSR_WORLD_MANNEDOBJECT_H

#include "SpaceObject.h"
#include "World.h"

namespace OpenSR
{
namespace World
{
class OPENSR_WORLD_API MannedObject : public SpaceObject
{
    Q_OBJECT
    OPENSR_WORLD_OBJECT

public:
    Q_INVOKABLE MannedObject(WorldObject *parent = 0, quint32 id = 0);
    ~MannedObject() override;

    quint32 typeId() const override;
    QString namePrefix() const override;
};
} // namespace World
} // namespace OpenSR

#endif // OPENSR_WORLD_MANNEDOBJECT_H
