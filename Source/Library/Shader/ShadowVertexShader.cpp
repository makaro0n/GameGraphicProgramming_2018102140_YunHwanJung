#include "Shader/ShadowVertexShader.h"

namespace library
{
    ShadowVertexShader::ShadowVertexShader(
        _In_ PCWSTR pszFileName, 
        _In_ PCSTR pszEntryPoint,
        _In_ PCSTR pszShaderModel
    )
        : VertexShader(pszFileName, pszEntryPoint, pszShaderModel)
    {
    }

    HRESULT ShadowVertexShader::Initialize(_In_ ID3D11Device* pDevice)
    {
        ComPtr<ID3DBlob> vsBlob;
        HRESULT hr = compile(vsBlob.GetAddressOf());
        if (FAILED(hr))
        {
            WCHAR szMessage[256];
            swprintf_s(
                szMessage,
                L"The FX file %s cannot be compiled. Please run this executable from the directory that contains the FX file.",
                m_pszFileName
            );
            MessageBox(
                nullptr,
                szMessage,
                L"Error",
                MB_OK
            );
            return hr;
        }

        hr = pDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Define the input layout
        D3D11_INPUT_ELEMENT_DESC aLayouts[] =
        {
            { "POSITION", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u },
            { "INSTANCE_TRANSFORM", 0u, DXGI_FORMAT_R32G32B32A32_FLOAT, 1u, 0u, D3D11_INPUT_PER_INSTANCE_DATA, 1u },
            { "INSTANCE_TRANSFORM", 1u, DXGI_FORMAT_R32G32B32A32_FLOAT, 1u, 16u, D3D11_INPUT_PER_INSTANCE_DATA, 1u },
            { "INSTANCE_TRANSFORM", 2u, DXGI_FORMAT_R32G32B32A32_FLOAT, 1u, 32u, D3D11_INPUT_PER_INSTANCE_DATA, 1u },
            { "INSTANCE_TRANSFORM", 3u, DXGI_FORMAT_R32G32B32A32_FLOAT, 1u, 48u, D3D11_INPUT_PER_INSTANCE_DATA, 1u },
        };
        UINT uNumElements = ARRAYSIZE(aLayouts);

        // Create the input layout
        hr = pDevice->CreateInputLayout(aLayouts, uNumElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), m_vertexLayout.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        return hr;
    }
}