import os
import sys
import array
import bpy
import json

def execfile(name):
    exec(open(name).read(), globals())

execfile("vars.py")
execfile("bla.py");
execfile("anim.py");

try:
    print("Creating blob for data dumping");
    output = open(os.path.join(outputdir,"humanmodels.bin"), "wb");
    #write payload to move offsets from zero
    output.write(b'K155LA3/3DMDL');
    output.write(bytearray(128-output.tell()));

    print("Importing characters_config.json");
    cconf = loadjson("characters_config.json");

    print("Importing humanoid_library.blend");
    blenderlib = os.path.join(inputdir, "humanoid_library.blend")
    with bpy.data.libraries.load(blenderlib) as (data_from, data_to):
        data_to.meshes = [name for name in data_from.meshes if name.startswith("MBLab")]
        data_to.armatures = [name for name in data_from.armatures if name.startswith("MBLab")]

    template_models = {};
    for mesh in data_to.meshes:
        print("  * dumping mesh "+mesh.name);
        mdldesc = {};
        print("    dumping vertices");
        saveVertices(output, mdldesc, mesh);
        print("    dumping texture coordinates");
        saveUV(output, mdldesc, mesh);
        print("    dumping polygon data");
        savePoly(output, mdldesc, mesh);
        print("    dump complete");
        template_models[mesh.name] = mdldesc;

    cconf["template_models"]=template_models;

    skeletons = {}
    for armature in data_to.armatures:
        #skip muscle armatures because we don't know how to use it
        if "muscle" in armature.name:
            print("  * skipping skeleton "+armature.name);
            continue;
        #skip armatures with to root
        keys = armature.bones.keys();
        if "root" not in keys:
            print("  * skeleton "+armature.name+" has no root bone; skipping");
            continue;

        #https://docs.blender.org/manual/en/latest/rigging/armatures/bones/properties/deform.html

        print("  * dumping skeleton "+armature.name);
        for key in keys:
            bone = armature.bones.get(key);
            #bone effect is
#            print("matrix={}".format(bone.matrix));
            print("=================================");
            #print("children of {} = {}".format(key, bone.children.keys()));
            #print("head = {}".format(bone.head));
            #print("tail = {}".format(bone.tail));
            #if bone.envelope_distance:
            #    print("envelope_distance = {}".format(bone.envelope_distance));
            #if bone.envelope_weight:
            #    print("envelope_weight = {}".format(bone.envelope_weight));
            #print("head_radius = {}".format(bone.head_radius));
            #print("tail_radius = {}".format(bone.tail_radius));
            #print("use_connect = {}".format(bone.use_connect));
            #print("use_inherit_rotation = {}".format(bone.use_inherit_rotation));
            #print("use_inherit_scale = {}".format(bone.use_inherit_scale));
            #print("use_relative_parent = {}".format(bone.use_relative_parent));
            #use_envelope_multiply

#        print("deform_method={}".format(armature.deform_method));
#        print("use_mirror_x={}".format(armature.use_mirror_x));
#        print("transform={}".format(armature.transform));
#        print("bones={}".format(armature.bones));
#        print("bones.keys={}".format(armature.bones.keys()));
#        a = {};
#        a.deform_method = armature.deform_method;
#        skeletons[armature.name] = a;

    cconf["skeletons"]=skeletons;

#    for character in cconf["character_list"]:





    output.close();

    print("Saving model index");
    ojson = open(os.path.join(outputdir,"humanmodels.json"), "w");
    ojson.write(json.dumps(cconf, ensure_ascii=True, indent=3));
    ojson.close();
    print("Import complete");
except:
    print("ERROR: failed to import data from library")
    sys.exit(1);
else:
    sys.exit(1);

