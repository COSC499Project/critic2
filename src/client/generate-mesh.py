from math import pi, sin, cos

def gen_octohedral():
  v_data = [(1.0, 0.0, 0.0),
            (-1.0, 0.0, 0.0),
            (0.0, 1.0, 0.0),
            (0.0, -1.0, 0.0),
            (0.0, 0.0, 1.0),
            (0.0, 0.0, -1.0)]

  i_data = [(5, 2, 1),
            (1, 2, 4),
            (4, 2, 0),
            (0, 2, 5),
            (1, 4, 3),
            (5, 1, 3),
            (0, 5, 3),
            (4, 0, 3)]
  return v_data, i_data

def gen_sphere(vertices, indices, div):
  if div == 0:
    sphere_v, sphere_i = gen_octohedral()
  else:
    sphere_v, sphere_i = gen_sphere(vertices, indices, div-1)

  len_sphere_i_before = len(sphere_i)
  sphere_i_additions = []
  sphere_i_removals = []
  for tri in sphere_i:

    a = tri[0]
    b = tri[1]
    c = tri[2]

    # ab, ac, bc are list of ["new"|"stored", vertex, index]
    ab = get_midpoint(a, b, sphere_v)
    if ab[0] == "new":
      sphere_v.append(ab[1])

    ac = get_midpoint(a, c, sphere_v)
    if ac[0] == "new":
      sphere_v.append(ac[1])

    bc = get_midpoint(b, c, sphere_v)
    if bc[0] == "new":
      sphere_v.append(bc[1])

    sphere_i_removals.append(tri)
    sphere_i_additions.append((a, ac[2], ab[2]))
    sphere_i_additions.append((b, ab[2], bc[2]))
    sphere_i_additions.append((c, bc[2], ac[2]))
    sphere_i_additions.append((ab[2], ac[2], bc[2]))

  for i in sphere_i_removals:
    sphere_i.remove(i)

  sphere_i += sphere_i_additions
    
  return sphere_v, sphere_i

def normalize(v):
  d = (v[0]**2 + v[1]**2 + v[2]**2)**0.5
  return (v[0]/d, v[1]/d, v[2]/d)

def get_midpoint(end1, end2, v):
  a = v[end1]
  b = v[end2]
 
  abx = (a[0] + b[0])/2.0
  aby = (a[1] + b[1])/2.0
  abz = (a[2] + b[2])/2.0
  ab = normalize((abx, aby, abz))
  
  try:
    i = v.index(ab)
    return ["stored", ab, i]
  except:
    i = len(v)
    return ["new", ab, i]

def gen_cylinder(n):
  interval = (2*pi)/n
  angle = 0

  vertices = []
  indices = []

  x = cos(angle)
  y = sin(angle)
  vertices.append((x, y, 1))
  vertices.append((x, y, -1))
  angle += interval

  for i in range(1,n):
    x = cos(angle)
    y = sin(angle)
    vertices.append((x, y, 1))
    vertices.append((x, y, -1))

    j = len(vertices)
    indices.append((j-1, j-4, j-2))
    indices.append((j-4, j-1, j-3))
    angle += interval

  j = len(vertices)
  indices.append((j-1, j-2, 1))
  indices.append((1, 0, j-2))

  return vertices, indices
    

def write_mesh_data(v_data, i_data, filename):
  fpv = open(filename+".v", "w")
  fpi = open(filename+".i", "w")

  v_string = ""
  i_string = ""
  for v in v_data:
    v_string += "%f %f %f" % v
    v_string += "\n"

  for i in i_data:
    i_string += "%d %d %d" % i 
    i_string += "\n"

  fpv.write(v_string)
  fpi.write(i_string)

  print "wrote %d vertices, %d triangles" % (len(v_data), len(i_data))
  print "wrote %d floats, %d indices" % (len(v_data)*3, len(i_data)*3)

  fpv.close()
  fpi.close()


v, i = gen_sphere(0, 0, 3)
write_mesh_data(v, i, "sphere")

v, i = gen_cylinder(40)
write_mesh_data(v, i, "cylinder")
