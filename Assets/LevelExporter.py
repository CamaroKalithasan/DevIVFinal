# DEV4 Simple Level Exporter v1.0
# a simple export script based on this answer from Blender stack exchange:
# https://blender.stackexchange.com/a/146344

import bpy
import os
from bpy_extras.io_utils import axis_conversion
import mathutils
import math

print("----------Begin Level Export----------")

path = os.path.join(os.path.dirname(bpy.data.filepath), "GameLevel.txt")
file = open(path,"w")
file.write("# Game Level Exporter v1.0\n")

scene = bpy.context.scene

def print_heir(ob, levels=10):
    def recurse(ob, parent, depth):
        if depth > levels: 
            return
        # spacing to show hierarchy
        spaces = "  " * depth;
        # print to system console for debugging
        print(spaces, ob.type)
        print(spaces, ob.name)
        print(spaces, ob.matrix_world)
        # send to file
        file.write(spaces + ob.type + "\n")
        file.write(spaces + ob.name + "\n")
        
        # swap from blender space to vulkan/d3d 
        # { rx, ry, rz, 0 } to { rx, rz, ry, 0 }  
        # { ux, uy, uz, 0 }    { ux, uz, uy, 0 }
        # { lx, ly, lz, 0 }    { lx, lz, ly, 0 } 
        # { px, py, pz, 1 }    { px, pz, py, 1 }  
        row_world = ob.matrix_world.transposed()
        converted = mathutils.Matrix.Identity(4)
        converted[0][0:3] = row_world[0][0], row_world[0][2], row_world[0][1]
        converted[1][0:3] = row_world[1][0], row_world[1][2], row_world[1][1] 
        converted[2][0:3] = row_world[2][0], row_world[2][2], row_world[2][1] 
        converted[3][0:3] = row_world[3][0], row_world[3][2], row_world[3][1]  
        
        # flip the local Z axis for winding and transpose for export
        scaleZ = mathutils.Matrix.Scale(-1.0, 4, (0.0, 0.0, 1.0))
        converted = scaleZ.transposed() @ converted  
        file.write(spaces + str(converted) + "\n")
         
        # TODO: For a game ready exporter we would
        # probably want the delta(pivot) matrix, lights,
        # detailed mesh hierarchy information
        # and bounding box/collission data at minimum

        for child in ob.children:
            recurse(child, ob,  depth + 1)
    recurse(ob, ob.parent, 0)

root_obs = (o for o in scene.objects if not o.parent)

for o in root_obs:
    print_heir(o)
    
file.close()

print("----------End Level Export----------")

# check the blender python API docs under "Object(ID)"
# that is where I found the "type" and "matrix_world"
# there is so much more useful stuff for a game level!