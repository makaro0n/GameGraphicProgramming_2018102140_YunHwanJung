#include "Cube/CustomCube.h"

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   CustomCube::CustomCube

  Summary:  Constructor

  Args:     const std::filesystem::path& textureFilePath
			  Path to the texture to use
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

CustomCube::CustomCube(
	const std::filesystem::path& textureFilePath
)
	: BaseCube::BaseCube(textureFilePath)
{
}


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
	static FLOAT s_totalTime = 0.0f;
	s_totalTime += deltaTime;

	XMVECTOR axis = XMVectorSet(1.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX orbit = XMMatrixRotationAxis(axis, s_totalTime * 2.0f);

	XMMATRIX rotate = XMMatrixRotationZ(XM_PIDIV4);
	XMMATRIX xSpin = XMMatrixRotationX(s_totalTime);
	XMMATRIX ySpin = XMMatrixRotationY(s_totalTime * 5.0f);
	XMMATRIX translate = XMMatrixTranslation(0.0f, 0.0f, -7.0f);
	XMMATRIX scale = XMMatrixScaling(0.5f, 0.5f, 0.5f);

	m_world = rotate * scale * xSpin * ySpin * translate * orbit;
}
