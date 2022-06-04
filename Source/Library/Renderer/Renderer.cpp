#include "Renderer/Renderer.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Renderer

      Summary:  Constructor

      Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                  m_immediateContext, m_immediateContext1, m_swapChain,
                  m_swapChain1, m_renderTargetView, m_depthStencil,
                  m_depthStencilView, m_cbChangeOnResize, m_cbShadowMatrix,
                  m_pszMainSceneName, m_camera, m_projection, m_scenes
                  m_invalidTexture, m_shadowMapTexture, m_shadowVertexShader,
                  m_shadowPixelShader].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    Renderer::Renderer()
        : m_driverType(D3D_DRIVER_TYPE_NULL)
        , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
        , m_d3dDevice(nullptr)
        , m_d3dDevice1(nullptr)
        , m_immediateContext(nullptr)
        , m_immediateContext1(nullptr)
        , m_swapChain(nullptr)
        , m_swapChain1(nullptr)
        , m_renderTargetView(nullptr)
        , m_depthStencil(nullptr)
        , m_depthStencilView(nullptr)

        , m_cbChangeOnResize(nullptr)
        , m_cbLights(nullptr)
        , m_cbShadowMatrix(nullptr)

        , m_pszMainSceneName(nullptr)
        , m_padding{ '\0' }
        , m_camera(XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f))
        , m_projection(XMMatrixIdentity())
        , m_scenes(std::unordered_map<std::wstring, std::shared_ptr<Scene>>())
        , m_invalidTexture(std::make_shared<Texture>(L"Content/Common/InvalidTexture.png"))

        , m_shadowMapTexture(nullptr)
        , m_shadowVertexShader(nullptr)
        , m_shadowPixelShader(nullptr)
    {
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Initialize

      Summary:  Creates Direct3D device and swap chain

      Args:     HWND hWnd
                  Handle to the window

      Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
                  m_d3dDevice1, m_immediateContext1, m_swapChain1,
                  m_swapChain, m_renderTargetView, m_vertexShader,
                  m_vertexLayout, m_pixelShader, m_vertexBuffer
                  m_cbShadowMatrix].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Renderer::Initialize(
        _In_ HWND hWnd
    )
    {
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT uWidth = static_cast<UINT>(rc.right - rc.left);
        UINT uHeight = static_cast<UINT>(rc.bottom - rc.top);

        UINT uCreateDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
        uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_DRIVER_TYPE driverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        UINT numDriverTypes = ARRAYSIZE(driverTypes);

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        UINT numFeatureLevels = ARRAYSIZE(featureLevels);

        for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
        {
            m_driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
            {
                break;
            }
        }
        if (FAILED(hr))
        {
            return hr;
        }

        // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
        ComPtr<IDXGIFactory1> dxgiFactory;
        {
            ComPtr<IDXGIDevice> dxgiDevice;
            hr = m_d3dDevice.As(&dxgiDevice);
            if (SUCCEEDED(hr))
            {
                ComPtr<IDXGIAdapter> adapter;
                hr = dxgiDevice->GetAdapter(&adapter);
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                }
            }
        }
        if (FAILED(hr))
        {
            return hr;
        }

        // Create swap chain
        ComPtr<IDXGIFactory2> dxgiFactory2;
        hr = dxgiFactory.As(&dxgiFactory2);
        if (SUCCEEDED(hr))
        {
            // DirectX 11.1 or later
            hr = m_d3dDevice.As(&m_d3dDevice1);
            if (SUCCEEDED(hr))
            {
                m_immediateContext.As(&m_immediateContext1);
            }

            DXGI_SWAP_CHAIN_DESC1 sd =
            {
                .Width = uWidth,
                .Height = uHeight,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc = {.Count = 1u, .Quality = 0u },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1u
            };

            hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
            if (SUCCEEDED(hr))
            {
                hr = m_swapChain1.As(&m_swapChain);
            }
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd =
            {
                .BufferDesc = {.Width = uWidth, .Height = uHeight, .RefreshRate = {.Numerator = 60, .Denominator = 1 }, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
                .SampleDesc = {.Count = 1, .Quality = 0 },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1u,
                .OutputWindow = hWnd,
                .Windowed = TRUE
            };

            hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
        }

        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

        if (FAILED(hr))
        {
            return hr;
        }

        // Create a render target view
        ComPtr<ID3D11Texture2D> pBackBuffer;
        hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Create depth stencil texture
        D3D11_TEXTURE2D_DESC descDepth =
        {
            .Width = uWidth,
            .Height = uHeight,
            .MipLevels = 1u,
            .ArraySize = 1u,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc = {.Count = 1u, .Quality = 0u },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0u,
            .MiscFlags = 0u
        };
        hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
        {
            .Format = descDepth.Format,
            .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
            .Texture2D = {.MipSlice = 0 }
        };
        hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        // Setup the viewport
        D3D11_VIEWPORT vp =
        {
            .TopLeftX = 0.0f,
            .TopLeftY = 0.0f,
            .Width = static_cast<FLOAT>(uWidth),
            .Height = static_cast<FLOAT>(uHeight),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f,
        };
        m_immediateContext->RSSetViewports(1, &vp);

        // Set primitive topology
        m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Create the constant buffers
        D3D11_BUFFER_DESC bd =
        {
            .ByteWidth = sizeof(CBChangeOnResize),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0
        };
        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Initialize the projection matrix
        m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<FLOAT>(uWidth) / static_cast<FLOAT>(uHeight), 0.01f, 1000.0f);

        CBChangeOnResize cbChangesOnResize =
        {
            .Projection = XMMatrixTranspose(m_projection)
        };
        m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cbChangesOnResize, 0, 0);
        m_immediateContext->VSSetConstantBuffers(1u, 1u, m_cbChangeOnResize.GetAddressOf());

        bd.ByteWidth = sizeof(CBLights);
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0u;

        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbLights.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_immediateContext->VSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());
        m_immediateContext->PSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());
        
        // Create shadow matrix constant buffer
        bd.ByteWidth = sizeof(CBShadowMatrix);
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0u;

        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbShadowMatrix.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Initialize shadow map texture
        m_shadowMapTexture = std::make_shared<RenderTexture>(uWidth, uHeight);
        hr = m_shadowMapTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_camera.Initialize(m_d3dDevice.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        if (!m_scenes.contains(m_pszMainSceneName))
        {
            return E_FAIL;
        }

        hr = m_scenes[m_pszMainSceneName]->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        // Initialize all point lights of main scene
        for (UINT i = 0u; i < NUM_LIGHTS; ++i)
        {
            m_scenes[m_pszMainSceneName]->GetPointLight(i)->Initialize(uWidth, uHeight);
        }

        hr = m_invalidTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        return hr;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddScene
      
      Summary:  Add scene to renderer
      
      Args:     PCWSTR pszSceneName
                  The name of the scene
                const std::shared_ptr<Scene>&
                  The shared pointer to Scene
      
      Modifies: [m_scenes].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Renderer::AddScene(
        _In_ PCWSTR pszSceneName, 
        _In_ const std::shared_ptr<Scene>& scene
    )
    {
        if (m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }

        m_scenes[pszSceneName] = scene;

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetSceneOrNull

      Summary:  Return scene with the given name or null

      Args:     PCWSTR pszSceneName
                  The name of the scene

      Returns:  std::shared_ptr<Scene>
                  The shared pointer to Scene
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    std::shared_ptr<Scene> Renderer::GetSceneOrNull(
        _In_ PCWSTR pszSceneName
    )
    {
        if (m_scenes.contains(pszSceneName))
        {
            return m_scenes[pszSceneName];
        }

        return nullptr;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetMainScene

      Summary:  Set the main scene

      Args:     PCWSTR pszSceneName
                  The name of the scene

      Modifies: [m_pszMainSceneName].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Renderer::SetMainScene(
        _In_ PCWSTR pszSceneName
    )
    {
        if (!m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }

        m_pszMainSceneName = pszSceneName;

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetShadowMapShaders

      Summary:  Set shaders for the shadow mapping

      Args:     std::shared_ptr<ShadowVertexShader>
                  vertex shader
                std::shared_ptr<PixelShader>
                  pixel shader

      Modifies: [m_shadowVertexShader, m_shadowPixelShader].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    void Renderer::SetShadowMapShaders(
        _In_ std::shared_ptr<ShadowVertexShader> vertexShader,
        _In_ std::shared_ptr<PixelShader> pixelShader
    )
    {
        m_shadowVertexShader = move(vertexShader);
        m_shadowPixelShader = move(pixelShader);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::HandleInput

      Summary:  Handle user mouse input

      Args:     DirectionsInput& directions
                MouseRelativeMovement& mouseRelativeMovement
                FLOAT deltaTime
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    void Renderer::HandleInput(
        _In_ const DirectionsInput& directions,
        _In_ const MouseRelativeMovement& mouseRelativeMovement,
        _In_ FLOAT deltaTime
    )
    {
        m_camera.HandleInput(directions, mouseRelativeMovement, deltaTime);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Update

      Summary:  Update the renderables each frame

      Args:     FLOAT deltaTime
                  Time difference of a frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    void Renderer::Update(
        _In_ FLOAT deltaTime
    )
    {
        m_scenes[m_pszMainSceneName]->Update(deltaTime);

        m_camera.Update(deltaTime);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Render

      Summary:  Render the frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    void Renderer::Render()
    {
        RenderSceneToTexture();

        // Clear the backbuffer
        m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);

        // Clear the depth buffer to 1.0 (max depth)
        m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0F, 0);

        // Update camera constant buffer
        CBChangeOnCameraMovement cbChangeOnCameraMovement =
        {
            .View = XMMatrixTranspose(m_camera.GetView()),
        };
        XMStoreFloat4(&cbChangeOnCameraMovement.CameraPosition, m_camera.GetEye());
        m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0u, nullptr, &cbChangeOnCameraMovement, 0u, 0u);
        m_immediateContext->VSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
        m_immediateContext->PSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());

        // Update the Light Constant Buffer
        CBLights cbLights = {};
        for (UINT i = 0u; i < NUM_LIGHTS; ++i)
        {
            cbLights.LightPositions[i] = m_scenes[m_pszMainSceneName]->GetPointLight(i)->GetPosition();
            cbLights.LightColors[i] = m_scenes[m_pszMainSceneName]->GetPointLight(i)->GetColor();
            cbLights.LightViews[i] = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(i)->GetViewMatrix());
            cbLights.LightProjections[i] = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(i)->GetProjectionMatrix());
        }
        m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0u, nullptr, &cbLights, 0u, 0u);
        m_immediateContext->PSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());

        // Shadow texture and sampler state
        m_immediateContext->PSSetShaderResources(2u, 1u, m_shadowMapTexture->GetShaderResourceView().GetAddressOf());
        m_immediateContext->PSSetSamplers(2u, 1u, m_shadowMapTexture->GetSamplerState().GetAddressOf());

        // Bind Buffer(vertex buffer, index buffer, input layout), Update Constant Buffer, 
        // Shader, Draw => Rendering Pipeline(Auto)(Input Assembler ~ ...Shader)
        // CPU 에서 Shader comile, create
        // Vertex Shader : per-vertex operation(Vertex 좌표 변환, Transform, 등등)
        // Pixel Shader : per-piexel operation (Pixel 색 지정, 빛 처리 등등)
        for (auto renderable : m_scenes[m_pszMainSceneName]->GetRenderables())
        {
            // Set the vertex buffer
            UINT stride = sizeof(SimpleVertex);
            UINT offset = 0u;
            m_immediateContext->IASetVertexBuffers(0u, 1u, renderable.second->GetVertexBuffer().GetAddressOf(), &stride, &offset);

            // Set the normal buffer
            stride = sizeof(NormalData);
            m_immediateContext->IASetVertexBuffers(1u, 1u, renderable.second->GetNormalBuffer().GetAddressOf(), &stride, &offset);

            // Set the index buffer 
            m_immediateContext->IASetIndexBuffer(renderable.second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);

            // Set the input layout
            m_immediateContext->IASetInputLayout(renderable.second->GetVertexLayout().Get());

            CBChangesEveryFrame cbChangesEveryFrame =
            {
                .World = XMMatrixTranspose(renderable.second->GetWorldMatrix()),
                .OutputColor = renderable.second->GetOutputColor(),
                .HasNormalMap = renderable.second->HasNormalMap()
            };
            m_immediateContext->UpdateSubresource(renderable.second->GetConstantBuffer().Get(), 0u, nullptr, &cbChangesEveryFrame, 0u, 0u);

            // Set shaders and constant buffers, shader resources, and samplers
            m_immediateContext->VSSetShader(renderable.second->GetVertexShader().Get(), nullptr, 0u);
            m_immediateContext->VSSetConstantBuffers(2u, 1u, renderable.second->GetConstantBuffer().GetAddressOf());

            m_immediateContext->PSSetShader(renderable.second->GetPixelShader().Get(), nullptr, 0u);
            m_immediateContext->PSSetConstantBuffers(2u, 1u, renderable.second->GetConstantBuffer().GetAddressOf());

            for (UINT i = 0u; i < renderable.second->GetNumMeshes(); ++i)
            {
                if (renderable.second->HasTexture())
                {
                    const UINT materialIndex = renderable.second->GetMesh(i).uMaterialIndex;

                    // Set texture resource view of the renderable into the pixel shader
                    m_immediateContext->PSSetShaderResources(
                        0u,
                        1u,
                        renderable.second->GetMaterial(materialIndex)->pDiffuse->GetTextureResourceView().GetAddressOf()
                    );
                    // Set sampler state of the renderable into the pixel shader
                    m_immediateContext->PSSetSamplers(
                        0u,
                        1u,
                        renderable.second->GetMaterial(materialIndex)->pDiffuse->GetSamplerState().GetAddressOf()
                    );

                    if (renderable.second->HasNormalMap())
                    {
                        // Set texture resource view of the renderable into the pixel shader
                        m_immediateContext->PSSetShaderResources(
                            1u,
                            1u,
                            renderable.second->GetMaterial(materialIndex)->pNormal->GetTextureResourceView().GetAddressOf()
                        );
                        // Set sampler state of the renderable into the pixel shader
                        m_immediateContext->PSSetSamplers(
                            1u,
                            1u,
                            renderable.second->GetMaterial(materialIndex)->pNormal->GetSamplerState().GetAddressOf()
                        );
                    }
                }
                // Render the triangles
                m_immediateContext->DrawIndexed(
                    renderable.second->GetMesh(i).uNumIndices,
                    renderable.second->GetMesh(i).uBaseIndex,
                    renderable.second->GetMesh(i).uBaseVertex
                );
            }
        }

        for (auto voxel : m_scenes[m_pszMainSceneName]->GetVoxels())
        {
            // Set the vertex buffer
            UINT stride = sizeof(SimpleVertex);
            UINT offset = 0u;
            m_immediateContext->IASetVertexBuffers(0u, 1u, voxel->GetVertexBuffer().GetAddressOf(), &stride, &offset);

            // Set the normal buffer
            stride = sizeof(NormalData);
            m_immediateContext->IASetVertexBuffers(1u, 1u, voxel->GetNormalBuffer().GetAddressOf(), &stride, &offset);

            // Set the instance buffer
            stride = sizeof(InstanceData);
            m_immediateContext->IASetVertexBuffers(2u, 1u, voxel->GetInstanceBuffer().GetAddressOf(), &stride, &offset);

            // Set the index buffer
            m_immediateContext->IASetIndexBuffer(voxel->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);

            // Set the input layout
            m_immediateContext->IASetInputLayout(voxel->GetVertexLayout().Get());

            CBChangesEveryFrame cbChangesEveryFrame =
            {
                .World = XMMatrixTranspose(voxel->GetWorldMatrix()),
                .OutputColor = voxel->GetOutputColor(),
                .HasNormalMap = voxel->HasNormalMap()
            };
            m_immediateContext->UpdateSubresource(voxel->GetConstantBuffer().Get(), 0u, nullptr, &cbChangesEveryFrame, 0u, 0u);

            // Set shaders and constant buffers
            m_immediateContext->VSSetShader(voxel->GetVertexShader().Get(), nullptr, 0u);
            m_immediateContext->VSSetConstantBuffers(2u, 1u, voxel->GetConstantBuffer().GetAddressOf());

            m_immediateContext->PSSetShader(voxel->GetPixelShader().Get(), nullptr, 0u);
            m_immediateContext->PSSetConstantBuffers(2u, 1u, voxel->GetConstantBuffer().GetAddressOf());

            for (UINT i = 0u; i < voxel->GetNumMeshes(); ++i)
            {
                if (voxel->HasTexture())
                {
                    const UINT materialIndex = voxel->GetMesh(i).uMaterialIndex;

                    // Set texture resource view of the renderable into the pixel shader
                    m_immediateContext->PSSetShaderResources(
                        0u,
                        1u,
                        voxel->GetMaterial(materialIndex)->pDiffuse->GetTextureResourceView().GetAddressOf()
                    );
                    // Set sampler state of the renderable into the pixel shader
                    m_immediateContext->PSSetSamplers(
                        0u,
                        1u,
                        voxel->GetMaterial(materialIndex)->pDiffuse->GetSamplerState().GetAddressOf()
                    );

                    if (voxel->HasNormalMap())
                    {
                        // Set texture resource view of the renderable into the pixel shader
                        m_immediateContext->PSSetSamplers(
                            1u,
                            1u,
                            voxel->GetMaterial(materialIndex)->pNormal->GetSamplerState().GetAddressOf()
                        );
                        // Set sampler state of the renderable into the pixel shader
                        m_immediateContext->PSSetShaderResources(
                            1u,
                            1u,
                            voxel->GetMaterial(materialIndex)->pNormal->GetTextureResourceView().GetAddressOf()
                        );
                    }
                }

                // Render the triangles
                m_immediateContext->DrawIndexedInstanced(
                    voxel->GetMesh(i).uNumIndices,
                    voxel->GetNumInstances(),
                    voxel->GetMesh(i).uBaseIndex,
                    voxel->GetMesh(i).uBaseVertex,
                    0u
                );
            }
        }

        for (auto model : m_scenes[m_pszMainSceneName]->GetModels())
        {
            // Set the vertex buffer
            UINT stride = sizeof(SimpleVertex);
            UINT offset = 0u;
            m_immediateContext->IASetVertexBuffers(0u, 1u, model.second->GetVertexBuffer().GetAddressOf(), &stride, &offset);

            // Set the normal buffer
            stride = sizeof(NormalData);
            m_immediateContext->IASetVertexBuffers(1u, 1u, model.second->GetNormalBuffer().GetAddressOf(), &stride, &offset);

            // Set the animation buffer
            stride = sizeof(AnimationData);
            m_immediateContext->IASetVertexBuffers(2u, 1u, model.second->GetAnimationBuffer().GetAddressOf(), &stride, &offset);

            // Set the index buffer
            m_immediateContext->IASetIndexBuffer(model.second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);

            // Set the input layout
            m_immediateContext->IASetInputLayout(model.second->GetVertexLayout().Get());

            CBChangesEveryFrame cbChangesEveryFrame =
            {
                .World = XMMatrixTranspose(model.second->GetWorldMatrix()),
                .OutputColor = model.second->GetOutputColor(),
                .HasNormalMap = model.second->HasNormalMap()
            };
            m_immediateContext->UpdateSubresource(model.second->GetConstantBuffer().Get(), 0u, nullptr, &cbChangesEveryFrame, 0u, 0u);

            // Update skinning constant buffer
            CBSkinning cbSkinning = { };
            for (UINT i = 0u; i < model.second->GetBoneTransforms().size(); ++i)
            {
                cbSkinning.BoneTransforms[i] = XMMatrixTranspose(model.second->GetBoneTransforms()[i]);
            }
            m_immediateContext->UpdateSubresource(model.second->GetSkinningConstantBuffer().Get(), 0u, nullptr, &cbSkinning, 0u, 0u);

            // Set shaders and constant buffers
            m_immediateContext->VSSetShader(model.second->GetVertexShader().Get(), nullptr, 0u);
            m_immediateContext->VSSetConstantBuffers(2u, 1u, model.second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(4u, 1u, model.second->GetSkinningConstantBuffer().GetAddressOf());

            m_immediateContext->PSSetShader(model.second->GetPixelShader().Get(), nullptr, 0u);
            m_immediateContext->PSSetConstantBuffers(2u, 1u, model.second->GetConstantBuffer().GetAddressOf());
            for (UINT i = 0u; i < model.second->GetNumMeshes(); ++i)
            {
                if (model.second->HasTexture())
                {
                    const UINT materialIndex = model.second->GetMesh(i).uMaterialIndex;

                    // Set texture resource view of the renderable into the pixel shader
                    m_immediateContext->PSSetShaderResources(
                        0u,
                        1u,
                        model.second->GetMaterial(materialIndex)->pDiffuse->GetTextureResourceView().GetAddressOf()
                    );
                    // Set sampler state of the renderable into the pixel shader
                    m_immediateContext->PSSetSamplers(
                        0u,
                        1u,
                        model.second->GetMaterial(materialIndex)->pDiffuse->GetSamplerState().GetAddressOf()
                    );

                    if (model.second->HasNormalMap())
                    {
                        // Set texture resource view of the renderable into the pixel shader
                        m_immediateContext->PSSetShaderResources(
                            1u,
                            1u,
                            model.second->GetMaterial(materialIndex)->pNormal->GetTextureResourceView().GetAddressOf()
                        );
                        // Set sampler state of the renderable into the pixel shader
                        m_immediateContext->PSSetSamplers(
                            1u,
                            1u,
                            model.second->GetMaterial(materialIndex)->pNormal->GetSamplerState().GetAddressOf()
                        );
                    }
                }

                // Render the triangles
                m_immediateContext->DrawIndexed(
                    model.second->GetMesh(i).uNumIndices,
                    model.second->GetMesh(i).uBaseIndex,
                    model.second->GetMesh(i).uBaseVertex
                );
            }
        }

        // Present the information rendered to the back buffer to the front buffer (the screen)
        m_swapChain->Present(0u, 0u);

        // Set Render Target View again (Present call for DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL unbinds backbuffer 0)
        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::RenderSceneToTexture

      Summary:  Render scene to the texture
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    void Renderer::RenderSceneToTexture()
    {
        //Unbind current pixel shader resources
        ID3D11ShaderResourceView* const pSRV[2] = { NULL, NULL };
        m_immediateContext->PSSetShaderResources(0u, 2u, pSRV);
        m_immediateContext->PSSetShaderResources(2u, 1u, pSRV);

        // Change render target to the shadow map texture
        m_immediateContext->OMSetRenderTargets(1u, m_shadowMapTexture->GetRenderTargetView().GetAddressOf(), m_depthStencilView.Get());
        // Clear render target view with white color
        m_immediateContext->ClearRenderTargetView(m_shadowMapTexture->GetRenderTargetView().Get(), Colors::White);
        // Clear depth stencil view
        m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0F, 0u);

        // Set shaders
        m_immediateContext->VSSetShader(m_shadowVertexShader->GetVertexShader().Get(), nullptr, 0u);
        m_immediateContext->PSSetShader(m_shadowPixelShader->GetPixelShader().Get(), nullptr, 0u);

        // Render renderables / voxels / models with shadow map shaders
        for (auto renderable : m_scenes[m_pszMainSceneName]->GetRenderables())
        {
            // Bind vertex buffer, index buffer, input layout
            UINT uStride = sizeof(SimpleVertex);
            UINT uOffset = 0u;
            m_immediateContext->IASetVertexBuffers(0u, 1u, renderable.second->GetVertexBuffer().GetAddressOf(), &uStride, &uOffset);
            m_immediateContext->IASetIndexBuffer(renderable.second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);
            m_immediateContext->IASetInputLayout(m_shadowVertexShader->GetVertexLayout().Get());

            // Update and bind CBShadowMatrix constant buffer
            CBShadowMatrix cbShadowMatrix = 
            {
                .World = XMMatrixTranspose(renderable.second->GetWorldMatrix()),
                .View = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(0)->GetViewMatrix()),
                .Projection = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(0)->GetProjectionMatrix()),
                .IsVoxel = false 
            };
            m_immediateContext->UpdateSubresource(m_cbShadowMatrix.Get(), 0u, nullptr, &cbShadowMatrix, 0u, 0u);
            m_immediateContext->VSSetConstantBuffers(0, 1, m_cbShadowMatrix.GetAddressOf());

            // Draw
            for (UINT i = 0u; i < renderable.second->GetNumMeshes(); ++i)
            {
                m_immediateContext->DrawIndexed(
                    renderable.second->GetMesh(i).uNumIndices,
                    renderable.second->GetMesh(i).uBaseIndex, 
                    static_cast<INT>(renderable.second->GetMesh(i).uBaseVertex)
                );
            }
        }
        for (auto model : m_scenes[m_pszMainSceneName]->GetModels())
        {
            // Bind vertex buffer, index buffer, input layout
            UINT uStride = sizeof(SimpleVertex);
            UINT uOffset = 0u;
            m_immediateContext->IASetVertexBuffers(0u, 1u, model.second->GetVertexBuffer().GetAddressOf(), &uStride, &uOffset);
            m_immediateContext->IASetIndexBuffer(model.second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);
            m_immediateContext->IASetInputLayout(m_shadowVertexShader->GetVertexLayout().Get());

            // Update and bind CBShadowMatrix constant buffer
            CBShadowMatrix cbShadowMatrix =
            {
                .World = XMMatrixTranspose(model.second->GetWorldMatrix()),
                .View = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(0)->GetViewMatrix()),
                .Projection = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(0)->GetProjectionMatrix()),
                .IsVoxel = false
            };
            m_immediateContext->UpdateSubresource(m_cbShadowMatrix.Get(), 0u, nullptr, &cbShadowMatrix, 0u, 0u);
            m_immediateContext->VSSetConstantBuffers(0u, 1u, m_cbShadowMatrix.GetAddressOf());

            // Draw
            for (UINT i = 0u; i < model.second->GetNumMeshes(); ++i)
            {
                m_immediateContext->DrawIndexed(
                    model.second->GetMesh(i).uNumIndices,
                    model.second->GetMesh(i).uBaseIndex,
                    static_cast<INT>(model.second->GetMesh(i).uBaseVertex)
                );
            }
        }
        // After rendering the scene, reset the render target back to the original back buffer
        m_immediateContext->OMSetRenderTargets(1u, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetDriverType

      Summary:  Returns the Direct3D driver type

      Returns:  D3D_DRIVER_TYPE
                  The Direct3D driver type used
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    D3D_DRIVER_TYPE Renderer::GetDriverType() const
    {
        return m_driverType;
    }
}
