#include "util_mbl.h"

bool convertMorphBuffers(const QStringList& input, const QString& output, const QString &prefix) {
    fprintf(stderr, "converting morph buffers...\r\n");

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
        //format={ name: [ [i,x,y,z] ... ], ... }
        ok = false;
        do {
            auto vbuf = loadJsonFromFile(i->absoluteFilePath());
            if (!vbuf.isObject()) {
                break;
            }
            auto vobj = vbuf.object();
            if (vobj.isEmpty()) {
                break;
            }

            ok = true;
            for(auto j = vobj.constBegin(); j != vobj.constEnd(); ++j) {
                auto dstname = i->baseName()+":"+j.key();
                Location location;
                location.start = data.size();

                if (!j.value().isArray()) {
                    ok = false;
                    break;
                }
                auto varr = j.value().toArray();
                for(auto k = varr.constBegin(); k != varr.constEnd(); ++k) {
                    auto vtx = (*k).toArray();
                    if (vtx.size() != 4) {
                        ok = false;
                        break;
                    }
                    Morph m;
                    m.i = vtx[0].toInt();
                    if (m.i < 0) {
                        ok = false;
                        break;
                    }
                    m.x = float(vtx[1].toDouble());
                    m.y = float(vtx[2].toDouble());
                    m.z = float(vtx[3].toDouble());
                    data.write((const char*)&m, sizeof(m));
                }
                if (!ok) break;

                location.size = data.size() - location.start;
                map.insert(dstname, location);
            }
        } while(0);
        if (!ok) {
            fprintf(stderr, "ERROR: %s\r\n", qPrintable(i->baseName()));
            break;
        }
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
