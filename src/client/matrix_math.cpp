#include "matrix_math.h"

void divideTriangle(GLfloat * a, GLfloat * b, GLfloat * c, GLfloat * result);
void DivideTriangles(GLfloat *, unsigned int *, int, int, GLfloat *, unsigned int *);

struct mesh{
  GLfloat * vertices;
  unsigned int * indices;
  unsigned int nvertices;
  unsigned int nindices;
};

void normalize(GLfloat v[3]){
  GLfloat d = sqrt(pow(v[0], 2.f) + pow(v[1], 2.f) + pow(v[2], 2.f));
  v[0] /= d;
  v[1] /= d;
  v[2] /= d;
}

void GenerateOctohedral(GLfloat * v, unsigned int * i){
  GLfloat vr[6][3] = {{1.f, 0.f, 0.f}, {-1.f, 0.f, 0.f},
    {0.f, 1.f, 0.f}, {0.f, -1.f, 0.f},
    {0.f, 0.f, 1.f}, {0.f, 0.f, -1.f}};
  memcpy(&v[0], vr, sizeof(GLfloat)*18);

  unsigned int ir[8][3] = {{5,2,1}, {1,2,4}, {4,2,0}, {0,2,5},
    {1,4,3}, {5,1,3}, {0,5,3}, {4,0,3}};
  memcpy(i, ir, sizeof(unsigned int)*24);
  DivideTriangles((GLfloat *)vr, (unsigned int *)ir, 18, 24, v, i);
}

void DivideTriangles(GLfloat *v, unsigned int *i, int nv, int ni,
                     GLfloat *vr, unsigned int *ir){
  for (int j=0; j<ni; j+=3){
  /*
    printf("%d, %d, %d    ", i[j], i[j+1], i[j+2]);
    printf("(%.1f, %.1f, %.1f), ", v[i[j]*3], v[i[j]*3+1], v[i[j]*3+2]);
    printf("(%.1f, %.1f, %.1f), ", v[i[j+1]*3], v[i[j+1]*3+1], v[i[j+1]*3+2]);
    printf("(%.1f, %.1f, %.1f)\n", v[i[j+2]*3], v[i[j+2]*3+1], v[i[j+2]*3+2]);
*/
    int a = i[j], b=i[j+1], c=i[j+2];
    float ax = v[i[j]*3],   ay = v[i[j]*3+1],   az = v[i[j]*3+2];
    float bx = v[i[j+1]*3], by = v[i[j+1]*3+1], bz = v[i[j+1]*3+2];
    float cx = v[i[j+2]*3], cy = v[i[j+2]*3+1], cz = v[i[j+2]*3+2];

    float abx = (ax + bx)/2.f;
    float aby = (ay + by)/2.f;
    float abz = (az + bz)/2.f;
    
    float acx = (ax + cx)/2.f;
    float acy = (ay + cy)/2.f;
    float acz = (az + cz)/2.f;

    float bcx = (bx + cx)/2.f;
    float bcy = (by + cy)/2.f;
    float bcz = (bz + cz)/2.f;
  }


}


void GenerateSphere(GLfloat * v){
  //pass in mesh with enough space allocated for
  GLfloat a[] = {1.f, 0.f, -1.f/sqrt(2.f)};
  GLfloat b[] = {-1.f, 0.f, -1.f/sqrt(2.f)};
  GLfloat c[] = {0.f, 1.f, 1.f/sqrt(2.f)};
  GLfloat d[] = {0.f, -1.f, 1.f/sqrt(2.f)};

  divideTriangle(a, b, c, &v[0]);
  divideTriangle(a, d, b, &v[36]);
  divideTriangle(c, b, d, &v[72]);
  divideTriangle(a, c, d, &v[108]);

  for(int i=0; i< 144; i+=3){
    normalize(&v[i]);
  }
/*
  
  memcpy(&v[0], a, sizeof(GLfloat)*3);
  memcpy(&v[3], b, sizeof(GLfloat)*3);
  memcpy(&v[6], c, sizeof(GLfloat)*3);

  memcpy(&v[9], a, sizeof(GLfloat)*3);
  memcpy(&v[12], d, sizeof(GLfloat)*3);
  memcpy(&v[15], b, sizeof(GLfloat)*3);

  memcpy(&v[18], c, sizeof(GLfloat)*3);
  memcpy(&v[21], b, sizeof(GLfloat)*3);
  memcpy(&v[24], d, sizeof(GLfloat)*3);
  
  memcpy(&v[27], a, sizeof(GLfloat)*3);
  memcpy(&v[30], c, sizeof(GLfloat)*3);
  memcpy(&v[33], d, sizeof(GLfloat)*3);
  */
//  return v;
}

void divideTriangle(GLfloat * a, GLfloat * c, GLfloat * b, GLfloat * result){
  // takes 3 arrays of 3 floats = 9 floats
  // outputs 36 floats
  GLfloat ab[3], ac[3], bc[3];
  for (int i=0; i<3; i++){
    ab[i] = (a[i] + b[i])/2.f;
    ac[i] = (a[i] + c[i])/2.f;
    bc[i] = (b[i] + c[i])/2.f;
  }

  memcpy(&result[0], a, sizeof(GLfloat)*3);
  memcpy(&result[3], ab, sizeof(GLfloat)*3);
  memcpy(&result[6], ac, sizeof(GLfloat)*3);

  memcpy(&result[9], ab, sizeof(GLfloat)*3);
  memcpy(&result[12], ac, sizeof(GLfloat)*3);
  memcpy(&result[15], bc, sizeof(GLfloat)*3);

  memcpy(&result[18], ac, sizeof(GLfloat)*3);
  memcpy(&result[21], c, sizeof(GLfloat)*3);
  memcpy(&result[24], bc, sizeof(GLfloat)*3);
  
  memcpy(&result[27], ab, sizeof(GLfloat)*3);
  memcpy(&result[30], b, sizeof(GLfloat)*3);
  memcpy(&result[33], bc, sizeof(GLfloat)*3); 
  
}


Matrix4f MultMatrices(Matrix4f Left, Matrix4f Right)
{
  Matrix4f Ret;
  for (unsigned int i=0; i < 4; i ++){
    for (unsigned int j=0; j<4; j++){
      Ret.m[i][j] = Left.m[i][0] * Right.m[0][j] +
                    Left.m[i][1] * Right.m[1][j] +
                    Left.m[i][2] * Right.m[2][j] +
                    Left.m[i][3] * Right.m[3][j];
    }
  }
  return Ret;
}

void Pipeline::Rotate(float x, float y, float z){
  m_rotateInfo.x = x;
  m_rotateInfo.y = y;
  m_rotateInfo.z = z;
}

const Matrix4f * Pipeline::GetTrans(){
  Matrix4f st, rx, ry, rz, tt;
  st.m[0][0] = m_scale.x; st.m[0][1] = 0.f; st.m[0][2] = 0.f; st.m[0][3] = 0.f;
  st.m[1][0] = 0.f; st.m[1][1] = m_scale.y; st.m[1][2] = 0.f; st.m[1][3] = 0.f;
  st.m[2][0] = 0.f; st.m[2][1] = 0.f; st.m[2][2] = m_scale.z; st.m[2][3] = 0.f;
  st.m[3][0] = 0.f; st.m[3][1] = 0.f; st.m[3][2] = 0.f; st.m[3][3] = 1.f;

  const float x = ToRadian(m_rotateInfo.x);
  const float y = ToRadian(m_rotateInfo.y);
  const float z = ToRadian(m_rotateInfo.z);
  rx.m[0][0] = 1.f; rx.m[0][1] = 0.f;     rx.m[0][2] = 0.f;      rx.m[0][3] = 0.f;
  rx.m[1][0] = 0.f; rx.m[1][1] = cosf(x); rx.m[1][2] = -sinf(x); rx.m[1][3] = 0.f;
  rx.m[2][0] = 0.f; rx.m[2][1] = sinf(x); rx.m[2][2] = cosf(x);  rx.m[2][3] = 0.f;
  rx.m[3][0] = 0.f; rx.m[3][1] = 0.f;     rx.m[3][2] = 0.f;      rx.m[3][3] = 1.f;
 
  ry.m[0][0] = cosf(y); ry.m[0][1] = 0.f; ry.m[0][2] = -sinf(y); ry.m[0][3] = 0.f;
  ry.m[1][0] = 0.f;     ry.m[1][1] = 1.f; ry.m[1][2] = 0.f;      ry.m[1][3] = 0.f;
  ry.m[2][0] = sinf(y); ry.m[2][1] = 0.f; ry.m[2][2] = cosf(y);  ry.m[2][3] = 0.f;
  ry.m[3][0] = 0.f;     ry.m[3][1] = 0.f; ry.m[3][2] = 0.f;      ry.m[3][3] = 1.f;

  rz.m[0][0] = cosf(z); rz.m[0][1] = -sinf(z); rz.m[0][2] = 0.f; rz.m[0][3] = 0.f;
  rz.m[1][0] = sinf(z); rz.m[1][1] = cosf(z);  rz.m[1][2] = 0.f; rz.m[1][3] = 0.f;
  rz.m[2][0] = 0.f;     rz.m[2][1] = 0.f;      rz.m[2][2] = 1.f; rz.m[2][3] = 0.f;
  rz.m[3][0] = 0.f;     rz.m[3][1] = 0.f;      rz.m[3][2] = 0.f; rz.m[3][3] = 1.f;
 
  tt.m[0][0] = 1.f; tt.m[0][1] = 0.f; tt.m[0][2] = 0.f; tt.m[0][3] = m_worldPos.x;
  tt.m[1][0] = 0.f; tt.m[1][1] = 1.f; tt.m[1][2] = 0.f; tt.m[1][3] = m_worldPos.y;
  tt.m[2][0] = 0.f; tt.m[2][1] = 0.f; tt.m[2][2] = 1.f; tt.m[2][3] = m_worldPos.z;
  tt.m[3][0] = 0.f; tt.m[3][1] = 0.f; tt.m[3][2] = 0.f; tt.m[3][3] = 1.f;
 
  m_transformation = MultMatrices(MultMatrices(MultMatrices(MultMatrices(tt, rx), ry), rz), st);
  return &m_transformation;
}


void Pipeline::InitPerspectiveProj(Matrix4f * m){
  const float ar = m_persProjInfo.Width / m_persProjInfo.Height;
  const float zNear = m_persProjInfo.zNear;
  const float zFar = m_persProjInfo.zFar;
  const float zRange = zNear - zFar;
  const float tanHalfFOV = tanf(ToRadian(m_persProjInfo.FOV / 2.0f));

  m->m[0][0] = 1.0f / (tanHalfFOV * ar);
  m->m[0][1] = 0.f; m->m[0][2] = 0.f; m->m[0][3] = 0.f;

  m->m[1][1] = 1.0f / tanHalfFOV;
  m->m[1][0] = 0.f; m->m[1][2] = 0.f; m->m[1][3] = 0.f;

  m->m[2][0] = 0.f; m->m[2][1] = 0.f;
  m->m[2][2] = (-zNear - zFar) / zRange;
  m->m[2][3] = 2.0f * zFar * zNear / zRange;

  m->m[3][0] = 0.f; m->m[3][1] = 0.f; m->m[3][2] = 1.f; m->m[3][3] = 0.f;
}


const Matrix4f * Pipeline::GetPTrans(){
  GetTrans();
  Matrix4f PP;
  InitPerspectiveProj(&PP);
  m_transformation = MultMatrices(PP, m_transformation);
  return &m_transformation;
}
















