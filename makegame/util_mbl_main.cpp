#include "util_mbl.h"
#include <QCoreApplication>
#include <QSaveFile>
#include <QDir>
#include <stdio.h>
int main(int argc, char ** argv) {
    QCoreApplication app(argc, argv);
    if (argc != 3) {
        fprintf(stderr, "ERROR: invalid arguments\r\n");
        return 1;
    }

    QString input  = QString::fromLocal8Bit(argv[1]);
    QString output = QString::fromLocal8Bit(argv[2]);

    QStringList vbufs;
    vbufs<<input+"/vertices";
    QStringList mbufs;
    mbufs<<input+"/morphs";
    mbufs<<input+"/expressions_morphs";

    if (!convertVertexBuffers(vbufs, output, "vtx")) {
        fprintf(stderr, "failed to convert vertex buffers\r\n");
        return 1;
    }
    if (!convertMorphBuffers(mbufs, output, "morph")) {
        fprintf(stderr, "failed to convert morph buffers\r\n");
        return 1;
    }
    /*
    auto doc = loadJsonFromFile(input + "/characters_config.json");
    if (doc.isNull()) {
        fprintf(stderr, "characters_config.json is not valid\r\n");
        return 1;
    }

    QSaveFile index(output+"/index.bin");
    if (!index.open(QIODevice::WriteOnly)) {
        fprintf(stderr, "failed to open index.bin for writing\r\n");
        return 1;
    }
    QSaveFile data(output+"/data.bin");
    if (!data.open(QIODevice::WriteOnly)) {
        fprintf(stderr, "failed to open data.bin for writing\r\n");
        return 1;
    }
    QDataStream ostream(&output);
    ostream.setVersion(QDataStream::Qt_5_7);
    auto docobj    = doc.object();
    auto tmpl_list = docobj[QLatin1String("templates_list")].toArray();
    auto char_list = docobj[QLatin1String("character_list")].toArray();
    qDebug()<<"TL"<<tmpl_list;
    qDebug()<<"CL"<<char_list;*/
    return 1;
}

