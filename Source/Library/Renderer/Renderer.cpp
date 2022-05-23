#include "Renderer/Renderer.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Renderer

      Summary:  Constructor

      Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                 m_immediateContext, m_immediateContext1, m_swapChain,
                 m_swapChain1, m_renderTargetView, m_depthStencil,
                 m_depthStencilView, m_cbChangeOnResize, m_camera,
                 m_projection, m_renderables, m_vertexShaders,
                 m_pixelShaders].
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
        , m_pszMainSceneName(PCWSTR())
        , m_padding()

        , m_camera(XMVectorSet(0.0f, 4.0f, -10.0f, 0.0f))
        , m_projection(XMMatrixIdentity())

        , m_renderables(std::unordered_map<std::wstring, std::shared_ptr<Renderable>>())
        , m_models(std::unordered_map<std::wstring, std::shared_ptr<Model>>())
        , m_aPointLights{ std::shared_ptr<PointLight>() }
        , m_vertexShaders(std::unordered_map<std::wstring, std::shared_ptr<VertexShader>>())
        , m_pixelShaders(std::unordered_map<std::wstring, std::shared_ptr<PixelShader>>())
        , m_scenes(std::unordered_map<std::wstring, std::shared_ptr<Scene>>())
    {
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Initialize

      Summary:  Creates Direct3D device and swap chain

      Args:     HWND hWnd
                  Handle to the window

      Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
                 m_d3dDevice1, m_immediateContext1, m_swapChain1,
                 m_swapChain, m_renderTargetView, m_cbChangeOnResize,
                 m_projection, m_cbLights, m_camera, m_vertexShaders,
                 m_pixelShaders, m_renderables].

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
        UINT width = static_cast<UINT>(rc.right - rc.left);
        UINT height = static_cast<UINT>(rc.bottom - rc.top);

        UINT createDeviceFlags = 0u;

#ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
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

        for (UINT driverTypeIndex = 0u; driverTypeIndex < numDriverTypes; ++driverTypeIndex)
        {
            m_driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDevice(
                nullptr,
                m_driverType,
                nullptr,
                createDeviceFlags,
                featureLevels,
                numFeatureLevels,
                D3D11_SDK_VERSION, 
                m_d3dDevice.GetAddressOf(), 
                &m_featureLevel, 
                m_immediateContext.GetAddressOf()
            );
            if (hr == E_INVALIDARG)
            {
                hr = D3D11CreateDevice(
                    nullptr, 
                    m_driverType, 
                    nullptr,
                    createDeviceFlags,
                    &featureLevels[1],
                    numFeatureLevels - 1u,
                    D3D11_SDK_VERSION,
                    m_d3dDevice.GetAddressOf(),
                    &m_featureLevel,
                    m_immediateContext.GetAddressOf()
                );
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


        ComPtr<IDXGIFactory1> dxgiFactory(nullptr);
        {
            ComPtr<IDXGIDevice> dxgiDevice(nullptr);

            hr = m_d3dDevice.As(&dxgiDevice);
            if (SUCCEEDED(hr))
            {
                ComPtr<IDXGIAdapter> adapter(nullptr);

                hr = dxgiDevice->GetAdapter(adapter.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(__uuidof(IDXGIFactory1), (&dxgiFactory));
                }
            }
        }
        if (FAILED(hr))
        {
            return hr;
        }

        // Create swap chain
        ComPtr<IDXGIFactory2> dxgiFactory2(nullptr);
        hr = dxgiFactory.As(&dxgiFactory2);
        if (dxgiFactory2)
        {
            // DirectX 11.1 or later
            hr = m_d3dDevice.As(&m_d3dDevice1);
            if (SUCCEEDED(hr))
            {
                hr = m_immediateContext.As(&m_immediateContext1);
            }

            DXGI_SWAP_CHAIN_DESC1 sd =
            {
                .Width = width,
                .Height = height,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc = 
                {
                    .Count = 1u,
                    .Quality = 0u
                },
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
                .BufferDesc =
                {
                    .Width = width,
                    .Height = height,
                    .RefreshRate = {.Numerator = 60u, .Denominator = 1u },
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                },
                .SampleDesc =
                {
                    .Count = 1u,
                    .Quality = 0u
                },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1u,
                .OutputWindow = hWnd,
                .Windowed = FALSE,
                .SwapEffect = DXGI_SWAP_EFFECT_DISCARD
            };

            hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
        }

        dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

        if (FAILED(hr))
        {
            return hr;
        }

        // Create a render target view
        ComPtr<ID3D11Texture2D> pBackBuffer(nullptr);

        hr = m_swapChain->GetBuffer(0u, __uuidof(ID3D11Texture2D), (&pBackBuffer));
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
            .Width = width,
            .Height = height,
            .MipLevels = 1u,
            .ArraySize = 1u,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc =
            {
                .Count = 1u,
                .Quality = 0u,
            },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0u,
            .MiscFlags = 0u,
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
            .Texture2D =
            {
                .MipSlice = 0u,
            }
        };
        hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_immediateContext->OMSetRenderTargets(1u, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        // Setup the viewport
        D3D11_VIEWPORT vp = 
        {
            .TopLeftX = 0.0f,
            .TopLeftY = 0.0f,
            .Width = static_cast<FLOAT>(width),
            .Height = static_cast<FLOAT>(height),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f
        };

        m_immediateContext->RSSetViewports(1u, &vp);

        // Initialize the projection matrix
        m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, static_cast<float>(width) / static_cast<float>(height), 0.01f, 100.0f);

        D3D11_BUFFER_DESC bd =
        {
            .ByteWidth = sizeof(CBChangeOnResize),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0u,
        };

        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        CBChangeOnResize cbChangesOnResize =
        {
            .Projection = XMMatrixTranspose(m_projection)
        };
        m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0u, nullptr, &cbChangesOnResize, 0u, 0u);

        // Initialize the View Matrix
        m_camera.Initialize(m_d3dDevice.Get());

        // Create the Light Constant Buffer
        bd.ByteWidth = sizeof(CBLights);
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0u;

        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbLights.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Initialize the shaders 
        for (auto pixelShader : m_pixelShaders)
        {
            pixelShader.second->Initialize(m_d3dDevice.Get());
        }
        for (auto vertexShader : m_vertexShaders)
        {
            vertexShader.second->Initialize(m_d3dDevice.Get());
        }

        // Initialize the renderables
        for (auto renderable : m_renderables)
        {
            renderable.second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        }

        // Initialize the models
        for (auto model : m_models)
        {
            model.second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        }

        // Initialize the Scenes
        for (auto scene : m_scenes)
        {
            scene.second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        }

        return hr;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddRenderable

      Summary:  Add a renderable object and initialize the object

      Args:     PCWSTR pszRenderableName
                  Key of the renderable object
                const std::shared_ptr<Renderable>& renderable
                  Unique pointer to the renderable object

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Renderer::AddRenderable(
        _In_ PCWSTR pszRenderableName,
        _In_ const std::shared_ptr<Renderable>& renderable
    )
    {
        if (m_renderables.contains(pszRenderableName))
        {
            return E_FAIL;
        }

        m_renderables.insert(std::make_pair(pszRenderableName, renderable));

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddModel

      Summary:  Add a model object

      Args:     PCWSTR pszModelName
                  Key of the model object
                const std::shared_ptr<Model>& pModel
                  Shared pointer to the model object

      Modifies: [m_models].

      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Renderer::AddModel(
        _In_ PCWSTR pszModelName,
        _In_ const std::shared_ptr<Model>& pModel
    )
    {
        if (m_models.contains(pszModelName))
        {
            return E_FAIL;
        }

        m_models.insert(std::make_pair(pszModelName, pModel));

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddPointLight

      Summary:  Add a point light

      Args:     size_t index
                  Index of the point light
                const std::shared_ptr<PointLight>& pointLight
                  Shared pointer to the point light object

      Modifies: [m_aPointLights].

      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Renderer::AddPointLight(
        _In_ size_t index,
        _In_ const std::shared_ptr<PointLight>& pPointLight)
    {
        if (index >= NUM_LIGHTS)
        {
            return E_FAIL;
        }
        else
        {
            m_aPointLights[index] = pPointLight;
            return S_OK;
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddVertexShader

      Summary:  Add the vertex shader into the renderer

      Args:     PCWSTR pszVertexShaderName
                  Key of the vertex shader
                const std::shared_ptr<VertexShader>&
                  Vertex shader to add

      Modifies: [m_vertexShaders].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Renderer::AddVertexShader(
        _In_ PCWSTR pszVertexShaderName, 
        _In_ const std::shared_ptr<VertexShader>& vertexShader
    )
    {
        if (m_vertexShaders.contains(pszVertexShaderName))
        {
            return E_FAIL;
        }

        m_vertexShaders.insert(std::make_pair(pszVertexShaderName, vertexShader));

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddPixelShader

      Summary:  Add the pixel shader into the renderer

      Args:     PCWSTR pszPixelShaderName
                  Key of the pixel shader
                const std::shared_ptr<PixelShader>&
                  Pixel shader to add

      Modifies: [m_pixelShaders].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Renderer::AddPixelShader(
        _In_ PCWSTR pszPixelShaderName,
        _In_ const std::shared_ptr<PixelShader>& pixelShader
    )
    {
        if (m_pixelShaders.contains(pszPixelShaderName))
        {
            return E_FAIL;
        }

        m_pixelShaders.insert(std::make_pair(pszPixelShaderName, pixelShader));

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddScene

      Summary:  Add a scene

      Args:     PCWSTR pszSceneName
                  Key of a scene
                const std::filesystem::path& sceneFilePath
                  File path to initialize a scene

      Modifies: [m_scenes].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Renderer::AddScene(
        _In_ PCWSTR pszSceneName, 
        const std::filesystem::path& sceneFilePath
    )
    {
        if (m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }

        m_scenes.insert(std::make_pair(pszSceneName, std::make_shared<Scene>(sceneFilePath)));

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetMainScene

      Summary:  Set the main scene

      Args:     PCWSTR pszSceneName
                  Name of the scene to set as the main scene

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
      Method:   Renderer::HandleInput

      Summary:  Add the pixel shader into the renderer and initialize it

      Args:     const DirectionsInput& directions
                  Data structure containing keyboard input data
                const MouseRelativeMovement& mouseRelativeMovement
                  Data structure containing mouse relative input data

      Modifies: [m_camera].
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

      Summary:  Update the renderables and models each frame

      Args:     FLOAT deltaTime
                  Time difference of a frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    void Renderer::Update(
        _In_ FLOAT deltaTime
    )
    {
        for (auto renderable : m_renderables)
        {
            renderable.second->Update(deltaTime);
        }

        for (UINT i = 0u; i < NUM_LIGHTS; ++i)
        {
            m_aPointLights[i]->Update(deltaTime);
        }

        for (auto model : m_models)
        {
            model.second->Update(deltaTime);
        }

        for (auto voxel : m_scenes[m_pszMainSceneName]->GetVoxels())
        {
            voxel->Update(deltaTime);
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Render

      Summary:  Render the frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    void Renderer::Render()
    {
        // Clear the backbuffer
        m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);

        // Clear the depth buffer to 1.0 (max depth)
        m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0F, 0);

        // Update the Camera Constant Buffer
        XMFLOAT4 camPos;
        XMStoreFloat4(&camPos, m_camera.GetEye());
        CBChangeOnCameraMovement cbCam =
        {
            .View = XMMatrixTranspose(m_camera.GetView()),
            .CameraPosition = camPos
        };
        m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0u, nullptr, &cbCam, 0u, 0u);

        // Update the Light Constant Buffer
        CBLights cbLights = {};
        for (UINT i = 0u; i < NUM_LIGHTS; ++i)
        {
            cbLights.LightPositions[i] = m_aPointLights[i]->GetPosition();
            cbLights.LightColors[i] = m_aPointLights[i]->GetColor();
        }
        m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0u, nullptr, &cbLights, 0u, 0u);

        // Bind Buffer(vertex buffer, index buffer, input layout), Update Constant Buffer, 
        // Shader, Draw => Rendering Pipeline(Auto)(Input Assembler ~ ...Shader)
        // CPU 에서 Shader comile, create
        // Vertex Shader : per-vertex operation(Vertex 좌표 변환, Transform, 등등)
        // Pixel Shader : per-piexel operation (Pixel 색 지정, 빛 처리 등등)
        for (auto renderable : m_renderables)
        {
            // Set the vertex buffer
            UINT stride = sizeof(SimpleVertex);
            UINT offset = 0u;
            m_immediateContext->IASetVertexBuffers(0u, 1u, renderable.second->GetVertexBuffer().GetAddressOf(), &stride, &offset);

            // Set the index buffer 
            m_immediateContext->IASetIndexBuffer(renderable.second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);

            // Set the input layout
            m_immediateContext->IASetInputLayout(renderable.second->GetVertexLayout().Get());

            // Set primitive topology
            m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            CBChangesEveryFrame cbChangesEveryFrame =
            {
                .World = XMMatrixTranspose(renderable.second->GetWorldMatrix()),
                .OutputColor = renderable.second->GetOutputColor()
            };
            m_immediateContext->UpdateSubresource(renderable.second->GetConstantBuffer().Get(), 0u, nullptr, &cbChangesEveryFrame, 0u, 0u);

            // Set shaders and constant buffers, shader resources, and samplers
            m_immediateContext->VSSetShader(renderable.second->GetVertexShader().Get(), nullptr, 0u);
            m_immediateContext->VSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1u, 1u, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2u, 1u, renderable.second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

            m_immediateContext->PSSetShader(renderable.second->GetPixelShader().Get(), nullptr, 0u);
            m_immediateContext->PSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2u, 1u, renderable.second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());

            if (renderable.second->HasTexture())
            {
                for (UINT i = 0u; i < renderable.second->GetNumMeshes(); ++i)
                {
                    const UINT materialIndex = renderable.second->GetMesh(i).uMaterialIndex;

                    // Set texture resource view of the renderable into the pixel shader
                    m_immediateContext->PSSetShaderResources(
                        0u,
                        1u,
                        renderable.second->GetMaterial(materialIndex).pDiffuse->GetTextureResourceView().GetAddressOf()
                    );
                    // Set sampler state of the renderable into the pixel shader
                    m_immediateContext->PSSetSamplers(
                        0u,
                        1u,
                        renderable.second->GetMaterial(materialIndex).pDiffuse->GetSamplerState().GetAddressOf()
                    );

                    // Render the triangles
                    m_immediateContext->DrawIndexed(
                        renderable.second->GetMesh(i).uNumIndices,
                        renderable.second->GetMesh(i).uBaseIndex,
                        renderable.second->GetMesh(i).uBaseVertex
                    );
                }
            }
            else
            {
                // Render the triangles
                m_immediateContext->DrawIndexed(renderable.second->GetNumIndices(), 0u, 0);
            }
        }

        for (auto voxel : m_scenes[m_pszMainSceneName]->GetVoxels())
        {
            // Set the vertex buffer
            UINT stride = sizeof(SimpleVertex);
            UINT offset = 0u;
            m_immediateContext->IASetVertexBuffers(0u, 1u, voxel->GetVertexBuffer().GetAddressOf(), &stride, &offset);

            // Set the instance buffer
            stride = sizeof(InstanceData);
            m_immediateContext->IASetVertexBuffers(1u, 1u, voxel->GetInstanceBuffer().GetAddressOf(), &stride, &offset);

            // Set the index buffer
            m_immediateContext->IASetIndexBuffer(voxel->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);

            // Set the input layout
            m_immediateContext->IASetInputLayout(voxel->GetVertexLayout().Get());

            // Set primitive topology
            m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            CBChangesEveryFrame cbChangesEveryFrame =
            {
                .World = XMMatrixTranspose(voxel->GetWorldMatrix()),
                .OutputColor = voxel->GetOutputColor()
            };

            m_immediateContext->UpdateSubresource(voxel->GetConstantBuffer().Get(), 0u, nullptr, &cbChangesEveryFrame, 0u, 0u);

            // Set shaders and constant buffers
            m_immediateContext->VSSetShader(voxel->GetVertexShader().Get(), nullptr, 0u);
            m_immediateContext->VSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1u, 1u, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2u, 1u, voxel->GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());

            m_immediateContext->PSSetShader(voxel->GetPixelShader().Get(), nullptr, 0u);
            m_immediateContext->PSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2u, 1u, voxel->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());

            // Render the triangles
            m_immediateContext->DrawIndexedInstanced(voxel->GetNumIndices(), voxel->GetNumInstances(), 0u, 0, 0u);
        }

        for (auto model : m_models)
        {
            // Set the vertex buffer
            UINT stride0 = sizeof(SimpleVertex);
            UINT offset0 = 0u;
            m_immediateContext->IASetVertexBuffers(0u, 1u, model.second->GetVertexBuffer().GetAddressOf(), &stride0, &offset0);

            UINT stride1 = sizeof(AnimationData);
            UINT offset1 = 0u;
            m_immediateContext->IASetVertexBuffers(1u, 1u, model.second->GetAnimationBuffer().GetAddressOf(), &stride1, &offset1);

            // Set the index buffer
            m_immediateContext->IASetIndexBuffer(model.second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);

            // Set the input layout
            m_immediateContext->IASetInputLayout(model.second->GetVertexLayout().Get());

            // Set primitive topology
            m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            CBChangesEveryFrame cbChangesEveryFrame =
            {
                .World = XMMatrixTranspose(model.second->GetWorldMatrix()),
                .OutputColor = model.second->GetOutputColor()
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
            m_immediateContext->VSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1u, 1u, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2u, 1u, model.second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());  
            m_immediateContext->VSSetConstantBuffers(4u, 1u, model.second->GetSkinningConstantBuffer().GetAddressOf());

            m_immediateContext->PSSetShader(model.second->GetPixelShader().Get(), nullptr, 0u);
            m_immediateContext->PSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2u, 1u, model.second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(4u, 1u, model.second->GetSkinningConstantBuffer().GetAddressOf());

            if (model.second->HasTexture())
            {
                for (UINT i = 0u; i < model.second->GetNumMeshes(); ++i)
                {
                    const UINT materialIndex = model.second->GetMesh(i).uMaterialIndex;

                    m_immediateContext->PSSetShaderResources(0u, 1u, model.second->GetMaterial(materialIndex).pDiffuse->GetTextureResourceView().GetAddressOf());
                    m_immediateContext->PSSetSamplers(0u, 1u, model.second->GetMaterial(materialIndex).pDiffuse->GetSamplerState().GetAddressOf());

                    // Render the triangles
                    m_immediateContext->DrawIndexed(
                        model.second->GetMesh(i).uNumIndices,
                        model.second->GetMesh(i).uBaseIndex,
                        model.second->GetMesh(i).uBaseVertex
                    );
                }
            }
            else
            {
                // Render the triangles
                m_immediateContext->DrawIndexed(model.second->GetNumIndices(), 0u, 0);
            }
        }

        // Present the information rendered to the back buffer to the front buffer (the screen)
        m_swapChain->Present(0u, 0u);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfRenderable

      Summary:  Sets the vertex shader for a renderable

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszVertexShaderName
                  Key of the vertex shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Renderer::SetVertexShaderOfRenderable(
        _In_ PCWSTR pszRenderableName,
        _In_ PCWSTR pszVertexShaderName
    )
    {
        if (!m_renderables.contains(pszRenderableName))
        {
            return E_FAIL;
        }
        if (!m_vertexShaders.contains(pszVertexShaderName))
        {
            return E_FAIL;
        }

        m_renderables[pszRenderableName]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);
        
        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfRenderable

      Summary:  Sets the pixel shader for a renderable

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszPixelShaderName
                  Key of the pixel shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Renderer::SetPixelShaderOfRenderable(
        _In_ PCWSTR pszRenderableName,
        _In_ PCWSTR pszPixelShaderName
    )
    {
        if (!m_renderables.contains(pszRenderableName))
        {
            return E_FAIL;
        }
        if (!m_pixelShaders.contains(pszPixelShaderName))
        {
            return E_FAIL;
        }

        m_renderables[pszRenderableName]->SetPixelShader(m_pixelShaders[pszPixelShaderName]);

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfModel

      Summary:  Sets the pixel shader for a model

      Args:     PCWSTR pszModelName
                  Key of the model
                PCWSTR pszVertexShaderName
                  Key of the vertex shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Renderer::SetVertexShaderOfModel(
        _In_ PCWSTR pszModelName,
        _In_ PCWSTR pszVertexShaderName
    )
    {
        if (!m_models.contains(pszModelName))
        {
            return E_FAIL;
        }
        if (!m_vertexShaders.contains(pszVertexShaderName))
        {
            return E_FAIL;
        }

        m_models[pszModelName]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfModel

      Summary:  Sets the pixel shader for a model

      Args:     PCWSTR pszModelName
                  Key of the model
                PCWSTR pszPixelShaderName
                  Key of the pixel shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Renderer::SetPixelShaderOfModel(
        _In_ PCWSTR pszModelName,
        _In_ PCWSTR pszPixelShaderName
    )
    {
        if (!m_models.contains(pszModelName))
        {
            return E_FAIL;
        }
        if (!m_pixelShaders.contains(pszPixelShaderName))
        {
            return E_FAIL;
        }

        m_models[pszModelName]->SetPixelShader(m_pixelShaders[pszPixelShaderName]);

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfScene

      Summary:  Sets the vertex shader for the voxels in a scene

      Args:     PCWSTR pszSceneName
                  Key of the scene
                PCWSTR pszVertexShaderName
                  Key of the vertex shader

      Modifies: [m_scenes].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Renderer::SetVertexShaderOfScene(
        _In_ PCWSTR pszSceneName, 
        _In_ PCWSTR pszVertexShaderName
    )
    {
        if (!m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }
        if (!m_vertexShaders.contains(pszVertexShaderName))
        {
            return E_FAIL;
        }

        for (auto voxel : m_scenes[pszSceneName]->GetVoxels())
        {
            voxel->SetVertexShader(m_vertexShaders[pszVertexShaderName]);
        }

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfScene

      Summary:  Sets the pixel shader for the voxels in a scene

      Args:     PCWSTR pszSceneName
                  Key of the renderable
                PCWSTR pszPixelShaderName
                  Key of the pixel shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT Renderer::SetPixelShaderOfScene(
        _In_ PCWSTR pszSceneName,
        _In_ PCWSTR pszPixelShaderName
    )
    {
        if (!m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }
        if (!m_pixelShaders.contains(pszPixelShaderName))
        {
            return E_FAIL;
        }

        for (auto voxel : m_scenes[pszSceneName]->GetVoxels())
        {
            voxel->SetPixelShader(m_pixelShaders[pszPixelShaderName]);
        }

        return S_OK;
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
