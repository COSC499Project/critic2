#include <math.h>
#include <sstream>
#include <string>
#include <fstream>
#include <iostream>

#define ToRadian(x) ((x) * M_PI / 180.0f)
#define ToDegree(x) ((x) * 180.0f / M_PI)

struct Matrix4f {
  float m[4][4];
};

struct ProjInfo {
  float FOV;
  float Width;
  float Height;
  float zNear;
  float zFar;
};

class Pipeline
{
public:
  Pipeline(){
    m_scale[0] = 1.f; m_scale[1] = 1.f, m_scale[2] = 1.f;
    m_pos[0] = 0.f; m_pos[1] = 0.f, m_pos[2] = 0.f;
    m_rotate[0] = 0.f; m_rotate[1] = 0.f, m_rotate[2] = 0.f;
  }

  void Scale(float x, float y, float z){
    m_scale[0] = x*0.5f; m_scale[1] = y; m_scale[2] = z;
  }

  void Translate(float x, float y, float z){
    m_pos[0] = x; m_pos[1] = y; m_pos[2] = z;
  }

  void Rotate(float x, float y, float z){
    m_rotate[0] = x; m_rotate[1] = y; m_rotate[2] = z;
  }

  const Matrix4f * GetTrans();

private:
  float m_scale[3];
  float m_pos[3];
  float m_rotate[3];
  Matrix4f m_transform;
  ProjInfo m_projInfo;
};

void ReadMesh(GLfloat *v, unsigned int* i, const char * v_file, const char * i_file){
  FILE * fpv = NULL;
  FILE * fpi = NULL;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  fpv = fopen(v_file, "r");
  if (fpv == NULL)
    exit(EXIT_FAILURE);
  

  fpi = fopen(i_file, "r");
  if (fpv == NULL) 
    exit(EXIT_FAILURE);
  
  int v_i = 0;
  int i_i = 0;
  GLfloat x, y, z;
  unsigned int a, b, c;
  while ((read = getline(&line, &len, fpv)) != -1) {
    sscanf(line, "%f %f %f\n", &x, &y, &z);
    v[v_i] = x;
    v_i += 1;
    v[v_i] = y;
    v_i += 1;
    v[v_i] = z;
    v_i += 1;
  }

  while ((read = getline(&line, &len, fpi)) != -1) {
    sscanf(line, "%d %d %d\n", &a, &b, &c);
    i[i_i] = a;
    i_i += 1;
    i[i_i] = b;
    i_i += 1;
    i[i_i] = c;
    i_i += 1;
  }

  fclose(fpv);
  fclose(fpi);
  if(line) free(line);
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


const Matrix4f * Pipeline::GetTrans(){
  Matrix4f st, rx, ry, rz, tt;
  st.m[0][0] = m_scale[0]; st.m[0][1] = 0.f; st.m[0][2] = 0.f; st.m[0][3] = 0.f;
  st.m[1][0] = 0.f; st.m[1][1] = m_scale[1]; st.m[1][2] = 0.f; st.m[1][3] = 0.f;
  st.m[2][0] = 0.f; st.m[2][1] = 0.f; st.m[2][2] = m_scale[2]; st.m[2][3] = 0.f;
  st.m[3][0] = 0.f; st.m[3][1] = 0.f; st.m[3][2] = 0.f; st.m[3][3] = 1.f;

  const float x = ToRadian(m_rotate[0]);
  const float y = ToRadian(m_rotate[1]);
  const float z = ToRadian(m_rotate[2]);
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
 
  tt.m[0][0] = 1.f; tt.m[0][1] = 0.f; tt.m[0][2] = 0.f; tt.m[0][3] = m_pos[0];
  tt.m[1][0] = 0.f; tt.m[1][1] = 1.f; tt.m[1][2] = 0.f; tt.m[1][3] = m_pos[1];
  tt.m[2][0] = 0.f; tt.m[2][1] = 0.f; tt.m[2][2] = 1.f; tt.m[2][3] = m_pos[2];
  tt.m[3][0] = 0.f; tt.m[3][1] = 0.f; tt.m[3][2] = 0.f; tt.m[3][3] = 1.f;
  
  Matrix4f rotate = MultMatrices(MultMatrices(rx, ry), rz);
  m_transform = MultMatrices(MultMatrices(tt, st), rotate);
  return &m_transform;
}
/*

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

*/








