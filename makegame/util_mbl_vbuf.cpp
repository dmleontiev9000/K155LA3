#include "util_mbl.h"

bool convertVertexBuffers(const QStringList& input, const QString& output, const QString &prefix) {
    fprintf(stderr, "converting vertex buffers...\r\n");

    QFileInfo indexInfo(output+"/"+prefix+"_index.bin");
    QFileInfo dataInfo(output+"/"+prefix+"_data.bin");
    QFileInfoList ents;

    for(QString s : input) {
        QDir dir(s);
        if (!dir.exists()) {
            fprintf(stderr, "directory missing: %s\r\n", qPrintable(s));
            return false;
        }
        ents.append(dir.entryInfoList(QStringList("*.json"), QDir::Files|QDir::Readable));
    }

    if (indexInfo.exists() && dataInfo.exists())
    {
        auto m1 = indexInfo.lastModified();
        auto m2 = dataInfo.lastModified();
        auto mod = m1 < m2 ? m1:m2;

        bool changed = false;
        for(auto i = ents.constBegin(); i != ents.constEnd(); ++i) {
            if (i->lastModified() > mod) {
                changed = true;
                break;
            }
        }
        if (!changed)
            return true;
    }

    QSaveFile index(indexInfo.absoluteFilePath());
    if (!index.open(QIODevice::WriteOnly)) {
        fprintf(stderr, "ERROR: %s (%d)\r\n",
                qPrintable(index.errorString()),
                int(index.error()));
        return false;
    }
    QSaveFile data(dataInfo.absoluteFilePath());
    if (!data.open(QIODevice::WriteOnly)) {
        fprintf(stderr, "ERROR: %s (%d)\r\n",
                qPrintable(data.errorString()),
                int(data.error()));
        return false;
    }

    BufferMap map;
    bool ok = true;
    for(auto i = ents.constBegin(); i != ents.constEnd(); ++i) {
        //format=[ [x,y,z] ... ]
        Location location;
        location.start = data.size();

        ok = false;
        do {
            auto vbuf = loadJsonFromFile(i->absoluteFilePath());
            if (!vbuf.isArray()) break;
            auto varr = vbuf.array();
            if (varr.isEmpty()) break;

            ok = true;
            for(auto i = varr.constBegin(); i != varr.constEnd(); ++i) {
                auto vtx = (*i).toArray();
                if (vtx.size() != 3) {
                    ok = false;
                    break;
                }
                Vertex v;
                v.x = float(vtx[0].toDouble());
                v.y = float(vtx[1].toDouble());
                v.z = float(vtx[2].toDouble());
                v.w = 1.0;
                data.write((const char*)&v, sizeof(v));
            }
            location.size = data.size() - location.start;
        } while(0);
        if (!ok) {
            fprintf(stderr, "ERROR: %s\r\n", qPrintable(i->baseName()));
            break;
        }
        map.insert(i->baseName(), location);
    }
    if (ok) {
        QDataStream ostream(&index);
        ostream.setVersion(QDataStream::Qt_5_7);
        ostream<<map;
        ok = ostream.status() == 0;
        if (!ok)
            fprintf(stderr, "ERROR: index not saved\r\n");
    }
    if (ok) {
        ok = data.commit() & index.commit();
        if (!ok)
            fprintf(stderr, "ERROR: data not saved\r\n");
    }
    return ok;
}
