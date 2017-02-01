#include <math.h>
#include <sstream>
#include <string>
#include <fstream>
#include <iostream>

#define ToRadian(x) ((x) * M_PI / 180.0f)
#define ToDegree(x) ((x) * 180.0f / M_PI)

void Normalize(float * v);
void Cross(const float left[3], const float right[3], float * result);

struct CameraInfo{
  float Pos[3];
  float Target[3];
  float Up[3];
};

struct PersProjInfo {
  float FOV;
  float Width;
  float Height;
  float zNear;
  float zFar;
};

class Matrix4f
{
public:
  float m[4][4];
  Matrix4f()
  {
  }
  inline void InitIdentity()
  {
    m[0][0] = 1.f; m[0][1] = 0.f; m[0][2] = 0.f; m[0][3] = 0.f;
    m[1][0] = 0.f; m[1][1] = 1.f; m[1][2] = 0.f; m[1][3] = 0.f;
    m[2][0] = 0.f; m[2][1] = 0.f; m[2][2] = 1.f; m[2][3] = 0.f;
    m[3][0] = 0.f; m[3][1] = 0.f; m[3][2] = 0.f; m[3][3] = 1.f;
  }
  inline Matrix4f operator*(const Matrix4f& Right) const
  {
    Matrix4f Ret;
    for (unsigned int i=0; i<4; i++){
      for (unsigned int j=0; j<4; j++){
        Ret.m[i][j] = m[i][0] * Right.m[0][j] +
                      m[i][1] * Right.m[1][j] +
                      m[i][2] * Right.m[2][j] +
                      m[i][3] * Right.m[3][j];
      }
    }
    return Ret;
  }
  
  void InitScaleTransform(float sx, float sy, float sz);
  void InitRotateTransform(float rx, float ry, float rz);
  void InitTranslateTransform(float x, float y, float z);
  void InitCameraTransform(const float Target[3], const float Up[3]);
  void InitPersProjTransform(const PersProjInfo& p);
};

void Matrix4f::InitScaleTransform(float ScaleX, float ScaleY, float ScaleZ)
{
    m[0][0] = ScaleX; m[0][1] = 0.0f;   m[0][2] = 0.0f;   m[0][3] = 0.0f;
    m[1][0] = 0.0f;   m[1][1] = ScaleY; m[1][2] = 0.0f;   m[1][3] = 0.0f;
    m[2][0] = 0.0f;   m[2][1] = 0.0f;   m[2][2] = ScaleZ; m[2][3] = 0.0f;
    m[3][0] = 0.0f;   m[3][1] = 0.0f;   m[3][2] = 0.0f;   m[3][3] = 1.0f;
}

void Matrix4f::InitRotateTransform(float RotateX, float RotateY, float RotateZ)
{
    Matrix4f rx, ry, rz;

    const float x = ToRadian(RotateX);
    const float y = ToRadian(RotateY);
    const float z = ToRadian(RotateZ);

    rx.m[0][0] = 1.0f; rx.m[0][1] = 0.0f   ; rx.m[0][2] = 0.0f    ; rx.m[0][3] = 0.0f;
    rx.m[1][0] = 0.0f; rx.m[1][1] = cosf(x); rx.m[1][2] = -sinf(x); rx.m[1][3] = 0.0f;
    rx.m[2][0] = 0.0f; rx.m[2][1] = sinf(x); rx.m[2][2] = cosf(x) ; rx.m[2][3] = 0.0f;
    rx.m[3][0] = 0.0f; rx.m[3][1] = 0.0f   ; rx.m[3][2] = 0.0f    ; rx.m[3][3] = 1.0f;

    ry.m[0][0] = cosf(y); ry.m[0][1] = 0.0f; ry.m[0][2] = -sinf(y); ry.m[0][3] = 0.0f;
    ry.m[1][0] = 0.0f   ; ry.m[1][1] = 1.0f; ry.m[1][2] = 0.0f    ; ry.m[1][3] = 0.0f;
    ry.m[2][0] = sinf(y); ry.m[2][1] = 0.0f; ry.m[2][2] = cosf(y) ; ry.m[2][3] = 0.0f;
    ry.m[3][0] = 0.0f   ; ry.m[3][1] = 0.0f; ry.m[3][2] = 0.0f    ; ry.m[3][3] = 1.0f;

    rz.m[0][0] = cosf(z); rz.m[0][1] = -sinf(z); rz.m[0][2] = 0.0f; rz.m[0][3] = 0.0f;
    rz.m[1][0] = sinf(z); rz.m[1][1] = cosf(z) ; rz.m[1][2] = 0.0f; rz.m[1][3] = 0.0f;
    rz.m[2][0] = 0.0f   ; rz.m[2][1] = 0.0f    ; rz.m[2][2] = 1.0f; rz.m[2][3] = 0.0f;
    rz.m[3][0] = 0.0f   ; rz.m[3][1] = 0.0f    ; rz.m[3][2] = 0.0f; rz.m[3][3] = 1.0f;

    *this = rz * ry * rx;
}

void Matrix4f::InitTranslateTransform(float x, float y, float z)
{
    m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = x;
    m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = y;
    m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = z;
    m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
}

void Matrix4f::InitCameraTransform(const float Target[3], const float Up[3])
{
    Normalize((float *)Target);
    float * N = (float *)Target;
    Normalize((float *)Up);
    float * U = (float *)Up;
    float V[3];
  
    Cross(N, U, V);

    m[0][0] = U[0];   m[0][1] = U[1];   m[0][2] = U[2];   m[0][3] = 0.0f;
    m[1][0] = V[0];   m[1][1] = V[1];   m[1][2] = V[2];   m[1][3] = 0.0f;
    m[2][0] = N[0];   m[2][1] = N[1];   m[2][2] = N[2];   m[2][3] = 0.0f;
    m[3][0] = 0.0f;  m[3][1] = 0.0f;  m[3][2] = 0.0f;  m[3][3] = 1.0f;
}

void Matrix4f::InitPersProjTransform(const PersProjInfo& p)
{
    const float ar         = p.Width / p.Height;
    const float zRange     = p.zNear - p.zFar;
    const float tanHalfFOV = tanf(ToRadian(p.FOV / 2.0f));

    m[0][0] = 1.0f/(tanHalfFOV * ar); m[0][1] = 0.0f;            m[0][2] = 0.0f;            m[0][3] = 0.0;
    m[1][0] = 0.0f;                   m[1][1] = 1.0f/tanHalfFOV; m[1][2] = 0.0f;            m[1][3] = 0.0;
    m[2][0] = 0.0f;                   m[2][1] = 0.0f;            m[2][2] = (-p.zNear - p.zFar)/zRange ; m[2][3] = 2.0f*p.zFar*p.zNear/zRange;
    m[3][0] = 0.0f;                   m[3][1] = 0.0f;            m[3][2] = 1.0f;            m[3][3] = 0.0;    
}



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

  void SetCamera(float Pos[3], float Target[3], float Up[3]){
    memcpy(&m_camera.Pos, Pos, sizeof(float)*3);
    memcpy(&m_camera.Target, Target, sizeof(float)*3);
    memcpy(&m_camera.Up, Up, sizeof(float)*3);
  }

  void SetPersProjInfo(float FOV, float Width, float Height, float zNear, float zFar){
    m_projInfo.FOV = FOV;
    m_projInfo.Width = Width;
    m_projInfo.Height = Height;
    m_projInfo.zNear = zNear;
    m_projInfo.zFar = zFar;
  }


  const Matrix4f * GetTrans();
  const Matrix4f * GetCTrans();

private:
  float m_scale[3];
  float m_pos[3];
  float m_rotate[3];
  Matrix4f m_transform;
  PersProjInfo m_projInfo;
  CameraInfo m_camera;
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

void Cross(const float left[3], const float right[3], float * result)
{
 result[0] = left[1]*right[2] - left[2]*right[1];
 result[1] = left[2]*right[0] - left[0]*right[2];
 result[2] = left[0]*right[1] - left[1]*right[0];
}

void Normalize(float * v)
{
  float d = sqrt(pow(v[0], 2) + pow(v[1], 2) + pow(v[2], 2));
  v[0] = v[0]/d;
  v[1] = v[1]/d;
  v[2] = v[2]/d;
}

const Matrix4f * Pipeline::GetTrans(){
  Matrix4f ScaleTrans, RotateTrans, TranslateTrans, CamTranslateTrans, CamRotateTrans,
           PersProjTrans;
  ScaleTrans.InitScaleTransform(m_scale[0], m_scale[1], m_scale[2]);
  RotateTrans.InitRotateTransform(m_rotate[0], m_rotate[1], m_rotate[2]);
  TranslateTrans.InitTranslateTransform(m_pos[0], m_pos[1], m_pos[2]);
  CamTranslateTrans.InitTranslateTransform(m_camera.Pos[0], -m_camera.Pos[1], -m_camera.Pos[2]);
  CamRotateTrans.InitCameraTransform(m_camera.Target, m_camera.Up);
  PersProjTrans.InitPersProjTransform(m_projInfo);

  m_transform =  PersProjTrans * CamRotateTrans * CamTranslateTrans * TranslateTrans *
                RotateTrans * ScaleTrans;

  return &m_transform;
}


class Camera
{
public:
  Camera();
  Camera(const float Pos[3], const float Target[3], const float Up[3]);
  bool OnKeyboard(int key);
  const float * GetPos();
  const float * GetTarget();
  const float * GetUp();

private:
  float m_pos[3];
  float m_target[3];
  float m_up[3];

};
