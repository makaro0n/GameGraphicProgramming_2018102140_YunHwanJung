#include "Window/MainWindow.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::Initialize

      Summary:  Initializes main window

      Args:     HINSTANCE hInstance
                  Handle to the instance
                INT nCmdShow
                    Is a flag that says whether the main application window
                    will be minimized, maximized, or shown normally
                PCWSTR pszWindowName
                    The window name

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT MainWindow::Initialize(
        _In_ HINSTANCE hInstance, 
        _In_ INT nCmdShow, 
        _In_ PCWSTR pszWindowName
    )
    {
        HRESULT hr = S_OK;

        hr = initialize(
            hInstance,
            nCmdShow, 
            pszWindowName,
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            0, 0, 800, 600,
            nullptr,
            nullptr
        );
        if (FAILED(hr))
        {
            return hr;
        }

        RECT rc;
        GetClientRect(m_hWnd, &rc);

        // Clip cursor to screen
        POINT p1 =
        {
            .x = rc.left,
            .y = rc.top - 30L
        };

        POINT p2 =
        {
            .x = rc.right,
            .y = rc.bottom
        };

        ClientToScreen(m_hWnd, &p1);
        ClientToScreen(m_hWnd, &p2);

        rc.left = p1.x;
        rc.top = p1.y;
        rc.right = p2.x;
        rc.bottom = p2.y;

        ClipCursor(&rc);

        // Raw Input Device
        RAWINPUTDEVICE rid = {
            .usUsagePage = static_cast<USHORT>(0x01),
            .usUsage = static_cast<USHORT>(0x02) ,
            .dwFlags = 0,
            .hwndTarget = m_hWnd
        };

        if (!RegisterRawInputDevices(&rid, 1u, sizeof(rid)))
        {
            return E_FAIL;
        }

        return hr;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::GetWindowClassName

      Summary:  Returns the name of the window class

      Returns:  PCWSTR
                  Name of the window class
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    PCWSTR MainWindow::GetWindowClassName() const
    {
        return m_pszWindowName;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::HandleMessage

      Summary:  Handles the messages

      Args:     UINT uMessage
                  Message code
                WPARAM wParam
                    Additional data the pertains to the message
                LPARAM lParam
                    Additional data the pertains to the message

      Returns:  LRESULT
                  Integer value that your program returns to Windows
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    LRESULT MainWindow::HandleMessage(
        _In_ UINT uMsg, 
        _In_ WPARAM wParam, 
        _In_ LPARAM lParam
    )
    {
        PAINTSTRUCT ps;
        HDC hdc;

        switch (uMsg)
        {
        case WM_PAINT:
            hdc = BeginPaint(m_hWnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            EndPaint(m_hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_CLOSE:
            if (MessageBox(m_hWnd,
                L"Really quit?",
                L"Game Graphics Programming",
                MB_OKCANCEL) == IDOK)
            {
                DestroyWindow(m_hWnd);
            }
            return 0;

        case WM_INPUT:
        {
            UINT dataSize = 0u;
            
            GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &dataSize, sizeof(RAWINPUTHEADER));
            
            if (dataSize > 0)
            {
                std::unique_ptr<BYTE[]> rawdata = std::make_unique<BYTE[]>(dataSize);
                if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam),
                                    RID_INPUT,
                                    rawdata.get(),
                                    &dataSize,
                                    sizeof(RAWINPUTHEADER)) == dataSize)
                {
                    RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawdata.get());
                    if (raw->header.dwType == RIM_TYPEMOUSE)
                    {
                        m_mouseRelativeMovement.X = raw->data.mouse.lLastX;
                        m_mouseRelativeMovement.Y = raw->data.mouse.lLastY;
                    }
                }
            }
            return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
        }

        case WM_KEYDOWN:
            switch (wParam) {
            case 0x41: // A
                m_directions.bLeft = TRUE;
                break;

            case 0x44: // D
                m_directions.bRight = TRUE;
                break;

            case 0x57: // W
                m_directions.bFront = TRUE;
                break;

            case 0x53: // S
                m_directions.bBack = TRUE;
                break;

            case VK_SHIFT:
                m_directions.bDown = TRUE;
                break;

            case VK_SPACE:
                m_directions.bUp = TRUE;
                break;
            }
            return 0;

        case WM_KEYUP:
            switch (wParam) {
            case 0x41: // A
                m_directions.bLeft = FALSE;
                break;

            case 0x44: // D
                m_directions.bRight = FALSE;
                break;

            case 0x57: // W
                m_directions.bFront = FALSE;
                break;

            case 0x53: // S
                m_directions.bBack = FALSE;
                break;

            case VK_SHIFT:
                m_directions.bDown = FALSE;
                break;

            case VK_SPACE:
                m_directions.bUp = FALSE;
                break;
            }
            return 0;

        default:
            return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
        }

        return 0;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::GetDirections

      Summary:  Returns the keyboard direction input

      Returns:  const DirectionsInput&
                  Keyboard direction input
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    const DirectionsInput& MainWindow::GetDirections() const
    {
        return m_directions;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::GetMouseRelativeMovement

      Summary:  Returns the mouse relative movement

      Returns:  const MouseRelativeMovement&
                  Mouse relative movement
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    
    const MouseRelativeMovement& MainWindow::GetMouseRelativeMovement() const
    {
        return m_mouseRelativeMovement;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::ResetMouseMovement

      Summary:  Reset the mouse relative movement to zero
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    void MainWindow::ResetMouseMovement()
    {
        m_mouseRelativeMovement = { 0L , 0L };
    }
}
