#include "Cube/OrbitCube.h"

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   OrbitCube::Update

  Summary:  Update the cube each frame

  Args:     FLOAT deltaTime
              Time difference of a frame
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

void OrbitCube::Update(
    _In_ FLOAT deltaTime
)
{
	if (XMMatrixIsIdentity(m_world))
		m_deltaTime = 0.0f;
	m_deltaTime += deltaTime;

	XMMATRIX spin = XMMatrixRotationZ(-m_deltaTime);
	XMMATRIX orbit = XMMatrixRotationY(-m_deltaTime * 2.0f);
	XMMATRIX translate = XMMatrixTranslation(-4.0f, 0.0f, 0.0f);
	XMMATRIX scale = XMMatrixScaling(0.3f, 0.3f, 0.3f);

	m_world = scale * spin * translate * orbit;
}

