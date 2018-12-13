#include "humanoid.h"

namespace K {
namespace MakeGame {

class HumanoidPrivate {
public:
    HumanoidPrivate();
    ~HumanoidPrivate();
};

} //namespace MakeGame
} //namespace K

/*
using namespace K::MakeGame;

//"description": "Generate the anime female template",
//"template_model": "MBLab_anime_female",
//"template_polygons": "anime_female_polygs.json",
//"name":"anime_female_base",
//"vertices":13995,
//"faces":13806,
//"label": "Anime female"
HumanoidTemplate::HumanoidTemplate(const QString &datapath,
                                   const QString &id,
                                   const QJsonObject &obj)
    : mId(id)
    , mLabel(
          descr[QLatin1String("label")].toString())
    , mDescription(
          descr[QLatin1String("description")].toString())
{
    if (mLabel.isEmpty()) mLabel = mId;

}
//"description": "Generate a realistic Caucasian female character",
//"template_model": "MBLab_human_female",
//"name":"f_ca01",
//"label": "Caucasian female (F_CA01)",
//"texture_diffuse": "human_female_diffuse.png",
//"texture_displacement": "human_female_displacement.png",
//"morphs_extra_file":"",
//"shared_morphs_file":"human_female_morphs.json",
//"shared_morphs_extra_file":"human_female_morphs_extra.json",
//"bounding_boxes_file":"human_female_bbox.json",
//"proportions_folder":"hu_f_anthropometry",
//"joints_base_file":"human_female_joints.json",
//"joints_offset_file":"human_female_joints_offset.json",
//"measures_file":"human_female_measures.json",
//"presets_folder":"human_female_base",
//"transformations_file":"human_female_base_transf.json",
//"vertexgroup_base_file":"human_female_vgroups_base.json",
//"vertexgroup_muscle_file":"human_female_vgroups_muscles.json"
HumanoidCharacter::HumanoidCharacter(const QString &datapath,
                                     const QString &id,
                                     const QJsonObject &obj)
    : mId(id)
    , mLabel(
          descr[QLatin1String("label")].toString())
    , mDescription(
          descr[QLatin1String("description")].toString())
    , mDiffuseTexture(
          datapath+"/textures/"+
          descr[QLatin1String("texture_diffuse")].toString())
    , mDisplacementTexture(
          datapath+"/textures/"+
          descr[QLatin1String("texture_displacement")].toString())

{
    if (mLabel.isEmpty()) mLabel = mId;


}
static QHash<QString,

HumanoidFactory::HumanoidFactory(const QString &datapath, QObject *parent)
    : QObject(parent)
    , mDataPath(datapath)
{
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    if (dir.exists()) {
        dir.mkdir("makegame");
        if (dir.cd("makegame")) {
            dir.mkdir("vertices");
            mCachePath = dir.absolutePath();
        }
    }

    QJsonDocument conf = loadJsonFromFile(mDataPath+"/characters_config.json");
    QJsonObject   cobj = conf.object();
    QJsonArray    templatesList = cobj[QLatin1String("templates_list")].toArray();
    for(auto iter = templatesList.constBegin();
        iter != templatesList.constEnd(); ++i)
    {
        QString templName = (*iter).toString();
        if (templName.isEmpty())
            continue;

        QJsonObject descr = cobj[templName].toObject();
        if (descr.isEmpty())
            continue;

        HumanoidTemplate templInst;
        templInst.mId          = templName;
        templInst.mLabel       = descr[QLatin1String("label")].toString();
        templInst.mDescription = descr[QLatin1String("description")].toString();
        if (templInst.mLabel.isEmpty())
            templInst.mLabel = templInst.mId;

        mHumanoidTemplates.append(templInst);
    }

    QJsonArray charactersList= cobj[QLatin1String("character_list")].toArray();
    for(auto iter = charactersList.constBegin();
        iter != charactersList.constEnd(); ++i)
    {
        QString charName = (*iter).toString();
        if (charName.isEmpty())
            continue;

        QJsonObject descr = cobj[charName].toObject();
        if (descr.isEmpty())
            continue;

        HumanoidCharacter charInst;
        charInst.mId = charName;
        charInst.mLabel = descr[QLatin1String("label")].toString();
        charInst.mDescription = descr[QLatin1String("description")].toString();


    }
}
*/
