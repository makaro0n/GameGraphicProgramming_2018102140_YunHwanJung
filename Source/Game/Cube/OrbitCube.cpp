#include "Cube/OrbitCube.h"

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   OrbitCube::OrbitCube

  Summary:  Constructor

  Args:     const std::filesystem::path& textureFilePath
			  Path to the texture to use
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

OrbitCube::OrbitCube(
	const std::filesystem::path& textureFilePath
)
	: BaseCube::BaseCube(textureFilePath)
{
}

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
	static FLOAT s_totalTime = 0.0f;
	s_totalTime += deltaTime;

	XMMATRIX spin = XMMatrixRotationZ(-s_totalTime);
	XMMATRIX orbit = XMMatrixRotationY(-s_totalTime * 2.0f);
	XMMATRIX translate = XMMatrixTranslation(-4.0f, 0.0f, 0.0f);
	XMMATRIX scale = XMMatrixScaling(0.3f, 0.3f, 0.3f);

	m_world = scale * spin * translate * orbit;
}

