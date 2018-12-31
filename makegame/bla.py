import bpy
import json

def aligns(sz, al=127):
    return (sz+al)&(~al);

def saveVertices(outfile, outjson, mesh):
    data_ba = bytearray(aligns(len(mesh.vertices)*8*4));
    outjson["vertices_start"] = outfile.tell();
    outjson["vertices_len"]   = len(data_ba);
    j = 0;
    data_f  = memoryview(data_ba).cast('f');
    for vx in mesh.vertices:
        data_f[j*8+0] = vx.co[0];
        data_f[j*8+1] = vx.co[1];
        data_f[j*8+2] = vx.co[2];
        data_f[j*8+4] = vx.normal[0];
        data_f[j*8+5] = vx.normal[1];
        data_f[j*8+6] = vx.normal[2];
        j+=1
    return len(data_ba) == outfile.write(data_ba);

def saveUV(outfile, outjson, mesh):
    data_ba = bytearray(aligns(len(mesh.uv_layers.active.data)*2*4));
    outjson["uv_start"] = outfile.tell();
    outjson["uv_len"]   = len(data_ba);
    j = 0;
    data_f  = memoryview(data_ba).cast('f');
    for uvloop in mesh.uv_layers.active.data:
        data_f[j*2+0] = uvloop.uv[0];
        data_f[j*2+1] = uvloop.uv[1];
        j+=1
    return len(data_ba) == outfile.write(data_ba);

def savePoly(outfile, outjson, mesh):
    #calculate total number of vertices
    #we will reorganize them as separate triangles
    #rather triangle strips or triangle fans
    num_triangles = 0;
    for poly in mesh.polygons:
        if poly.loop_total < 3:
            continue;
        num_triangles += 1 + (poly.loop_total-3);
    data_ba = bytearray(aligns(num_triangles*4*6));
    outjson["poly_start"] = outfile.tell();
    outjson["poly_len"]   = len(data_ba);
    outjson["triangles"]  = num_triangles;
    j = 0;
    data_i  = memoryview(data_ba).cast('i');
    for poly in mesh.polygons:
        if (poly.loop_total < 3):
            continue;
        #triangle data
        V0A = poly.loop_start;
        V0B = mesh.loops[V0A].vertex_index;
        V1A = V0A+1;
        V1B = mesh.loops[V1A].vertex_index;
        V2A = V1A+1;
        V2B = mesh.loops[V2A].vertex_index;
        #emit triangles
        VE  = V0A + poly.loop_total;
        while True:
            #vertex 0
            data_i[j+0] = V0A;
            data_i[j+1] = V0B;
            #vertex 1
            data_i[j+2] = V1A;
            data_i[j+3] = V1B;
            #vertex 2
            data_i[j+4] = V2A;
            data_i[j+5] = V2B;
            #triangle finished
            j   += 6;
            #move to next vertex
            V1A = V2A;
            V1B = V2B;
            V2A+= 1;
            if V2A == VE:
                break;
            V2B = mesh.loops[V2A].vertex_index;
    return len(data_ba) == outfile.write(data_ba);

def loadjson(*names):
    global inputdir;
    fp = inputdir;
    for n in names:
        fp = os.path.join(fp, n);

    j_file = open(fp, "r");
    j_db   = None;
    try:
        j_db = json.load(j_file);
    except:
        print("ERROR: failed to open {}".format(name));
    j_file.close();
    return j_db;

