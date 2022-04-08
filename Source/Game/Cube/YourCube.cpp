#include "Cube/YourCube.h"

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   YourCube::Update

  Summary:  Update the cube each frame

  Args:     FLOAT deltaTime
              Time difference of a frame
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

void YourCube::Update(_In_ FLOAT deltaTime)
{
    m_world = XMMatrixRotationY(deltaTime);
}

