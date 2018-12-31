import os
import sys
import array
import bpy

def execfile(name):
    exec(open(name).read(), globals())

execfile("vars.py")
execfile("bla.py");

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
        print("  * dumping skeleton "+armature.name);
    cconf["skeletons"]=skeletons;

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
    sys.exit(0);

