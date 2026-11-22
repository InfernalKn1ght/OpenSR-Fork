/*
    OpenSR - opensource multi-genre game based upon "Space Rangers 2: Dominators"
    Copyright (C) 2014 Kosyak <ObKo@mail.ru>

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

#include <OpenSR/libRangerQt.h>
#include <QBuffer>
#include <cstring>

namespace OpenSR
{
bool checkGAIHeader(QIODevice *dev)
{
    quint32 sig{};
    qint64 size = dev->peek((char *)&sig, sizeof(quint32));

    if (size != sizeof(quint32) || sig != GAI_SIGNATURE)
    {
        return false;
    }

    return true;
}

QVector<int> loadGAITimes(QIODevice *dev, const GAIHeader &header)
{
    QVector<int> times(header.frameCount);

    if (!header.waitSize)
    {
        return times;
    }

    // Reject a corrupt wait block before allocating/reading it.
    if (dev->size() > 0 && static_cast<qint64>(header.waitSize) > dev->size())
    {
        return times;
    }

    QByteArray waitData(static_cast<qsizetype>(header.waitSize), Qt::Uninitialized);
    dev->seek(header.waitSeek);
    const qint64 bytesRead = dev->read(waitData.data(), waitData.size());
    if (bytesRead <= 0)
    {
        return times;
    }
    waitData.resize(bytesRead);

    const char *base = waitData.constData();
    const qsizetype size = waitData.size();

    if (size < 6)
    {
        return times;
    }

    const auto readU32 = [](const char *p) {
        uint32_t value = 0;
        std::memcpy(&value, p, sizeof(value));
        return value;
    };

    const uint32_t timeBlockCount = readU32(base);

    // The block-offset table must lie entirely inside the wait block.
    if (static_cast<qsizetype>(timeBlockCount) * 8 + 6 > size)
    {
        return times;
    }

    const char *p = base + 4 + static_cast<qsizetype>(timeBlockCount) * 8 + 2;

    for (uint32_t i = 0; i < timeBlockCount; i++)
    {
        if (p + 4 > base + size)
        {
            break;
        }

        const uint32_t blockFrameCount = readU32(p);

        // Every frame record is 8 bytes and must stay inside the block.
        if (p + 4 + static_cast<qsizetype>(blockFrameCount) * 8 > base + size)
        {
            break;
        }

        for (uint32_t j = 0; j < blockFrameCount; j++)
        {
            const uint32_t frame = readU32(p + 4 + static_cast<qsizetype>(j) * 8);
            const uint32_t time = readU32(p + 4 + static_cast<qsizetype>(j) * 8 + 4);
            if (frame < header.frameCount)
            {
                times[frame] = static_cast<int>(time);
            }
        }

        p += static_cast<qsizetype>(blockFrameCount) * 8 + 4 + 2;
    }

    return times;
}

QImage loadGAIFrame(QIODevice *dev, const GAIHeader &header, int i, const QImage &background)
{
    if (i >= header.frameCount)
    {
        return QImage();
    }

    QImage result;
    uint32_t giSeek{}, giSize{};
    dev->seek(static_cast<qint64>(sizeof(GAIHeader)) +
              static_cast<qint64>(i) * 2 * static_cast<qint64>(sizeof(uint32_t)));
    dev->read((char *)&giSeek, sizeof(uint32_t));
    dev->read((char *)&giSize, sizeof(uint32_t));

    if (giSeek && giSize)
    {
        qint64 giOffset = giSeek;
        uint32_t signature{};
        dev->seek(giOffset);
        dev->peek((char *)&signature, sizeof(uint32_t));

        if (signature == ZL01_SIGNATURE || signature == ZL02_SIGNATURE)
        {
            QByteArray zlibData = dev->read(giSize);
            QByteArray data = unpackZL(zlibData);

            QBuffer bufdev(&data);
            bufdev.open(QIODevice::ReadOnly);
            result =
                loadGIFrame(&bufdev, true, background, static_cast<int>(header.startX), static_cast<int>(header.startY),
                            static_cast<int>(header.finishX), static_cast<int>(header.finishY));
        }
        else
        {
            result =
                loadGIFrame(dev, true, background, static_cast<int>(header.startX), static_cast<int>(header.startY),
                            static_cast<int>(header.finishX), static_cast<int>(header.finishY));
        }
    }
    else
    {
        if (!background.isNull())
        {
            result = background;
        }
        else
        {
            result = QImage(static_cast<int>(header.finishX - header.startX),
                            static_cast<int>(header.finishY - header.startY), QImage::Format_ARGB32);
        }
    }
    return result;
}

Animation loadGAIAnimation(QIODevice *dev, const QImage &background)
{
    if (!checkGAIHeader(dev))
    {
        return Animation();
    }

    GAIHeader header = readGAIHeader(dev);
    QImage bg = background;

    Animation result;
    result.images = QVector<QImage>(header.frameCount);
    result.times = loadGAITimes(dev, header);

    for (int i = 0; i < header.frameCount; i++)
    {
        result.images[i] = loadGAIFrame(dev, header, i, bg);
        bg = result.images[i];
    }

    return result;
}

GAIHeader peekGAIHeader(QIODevice *dev)
{
    GAIHeader result{};
    dev->peek((char *)&result, sizeof(GAIHeader));
    return result;
}

GAIHeader readGAIHeader(QIODevice *dev)
{
    GAIHeader result{};
    dev->read((char *)&result, sizeof(GAIHeader));
    return result;
}

} // namespace OpenSR
