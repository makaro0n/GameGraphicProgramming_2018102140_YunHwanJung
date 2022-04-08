#include "Cube/OrbitCube.h"

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   OrbitCube::Update

  Summary:  Update the cube each frame

  Args:     FLOAT deltaTime
              Time difference of a frame
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

void OrbitCube::Update(_In_ FLOAT deltaTime)
{
    XMMATRIX Spin = XMMatrixRotationZ(-deltaTime);
    XMMATRIX Orbit = XMMatrixRotationY(-deltaTime * 2.0f);
    XMMATRIX Translate = XMMatrixTranslation(-4.0f, 0.0f, 0.0f);
    XMMATRIX Scale = XMMatrixScaling(0.3f, 0.3f, 0.3f);

    m_world = Scale * Spin * Translate * Orbit;
}

