#include "Cube/CustomCube.h"

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   CustomCube::Update 

  Summary:  Update the cube each frame

  Args:     FLOAT deltaTime
			  Time difference of a frame
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

void CustomCube::Update(
	_In_ FLOAT deltaTime
)
{
	if (XMMatrixIsIdentity(m_world))
		m_deltaTime = 0.0f;
	m_deltaTime += deltaTime;

	XMVECTOR axis = XMVectorSet(1.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX orbit = XMMatrixRotationAxis(axis, m_deltaTime * 2.0f);

	XMMATRIX rotate = XMMatrixRotationZ(XM_PIDIV4);
	XMMATRIX spin = XMMatrixRotationX(m_deltaTime);
	XMMATRIX translate = XMMatrixTranslation(0.0f, 0.0f, -7.0f);
	XMMATRIX scale = XMMatrixScaling(0.5f, 0.5f, 0.5f);

	m_world = rotate * scale * spin * translate * orbit;
}
