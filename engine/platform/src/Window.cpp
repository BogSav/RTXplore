#include "Window.hpp"

#include "engine/core/WindowsThrowMacros.hpp"

#include <functional>
#include <sstream>

WindowClass WindowClass::wndClass;

const wchar_t* WindowClass::GetName() noexcept
{
	return wndClassName;
}

HINSTANCE WindowClass::GetInstance() noexcept
{
	return wndClass.hInstance;
}

HWND& Window::GetWindowHandler() noexcept
{
	return hWnd;
}


WindowClass::WindowClass() noexcept : hInstance(GetModuleHandle(nullptr))
{
	using namespace std::placeholders;

	WNDCLASSEX wc = {0};
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = &Window::StartupWindowsProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = wndClassName;

	RegisterClassEx(&wc);
}

WindowClass::~WindowClass()
{
	UnregisterClass(wndClassName, hInstance);
}

Window::Window(int width, int height, const wchar_t* name) : width(width), height(height)
{
	Settings::GetGraphicsSettings().SetWidth(width);
	Settings::GetGraphicsSettings().SetHeight(height);

	RECT uiSpace;
	uiSpace.left = 100;
	uiSpace.right = uiSpace.left + width;
	uiSpace.top = 100;
	uiSpace.bottom = uiSpace.top + height;

	DWORD uiStyle = WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_SIZEBOX;

	if (AdjustWindowRect(&uiSpace, uiStyle, FALSE) == 0)
	{
		throw UPWND_LAST_EXCEPT();
	}

	hWnd = CreateWindowEx(
		0,
		WindowClass::GetName(),
		name,
		uiStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		uiSpace.right - uiSpace.left,
		uiSpace.bottom - uiSpace.top,
		nullptr,
		nullptr,
		WindowClass::GetInstance(),
		this);

	if (hWnd == nullptr)
	{
		throw UPWND_LAST_EXCEPT();
	}

	GraphicsResources::GetInstance().LoadResources(hWnd);

	ShowWindow(hWnd, SW_SHOWDEFAULT);
}

Window::~Window()
{
	DestroyWindow(hWnd);
}

void Window::SetTitle(const std::wstring& title)
{
	if (SetWindowText(hWnd, title.c_str()) == 0)
	{
		throw UPWND_LAST_EXCEPT();
	}
}

Mouse& Window::GetMouse() noexcept
{
	return mouse;
}

Keyboard& Window::GetKeyboard() noexcept
{
	return keyboard;
}

std::optional<int> Window::ProcessMessage()
{
	MSG msg;

	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			return static_cast<int>(msg.wParam);
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return {};
}

LRESULT Window::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KILLFOCUS: keyboard.ClearState(); break;
	/*********** KEYBOARD MESSAGES ***********/
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (!(lParam & 0x40000000) || keyboard.AutorepeatIsEnabled())
		{
			keyboard.OnKeyPressed(static_cast<unsigned char>(wParam));
		}
		break;

	case WM_KEYUP:
	case WM_SYSKEYUP: keyboard.OnKeyReleased(static_cast<unsigned char>(wParam)); break;

	case WM_CHAR: keyboard.OnChar(static_cast<unsigned char>(wParam)); break;
	/*********** END KEYBOARD MESSAGES ***********/

	/************* MOUSE MESSAGES ****************/
	case WM_MOUSEMOVE:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		// in client region -> log move, and log enter + capture mouse (if not previously in window)
		if (pt.x >= 0 && pt.x < width && pt.y >= 0 && pt.y < height)
		{
			mouse.OnMouseMove(pt.x, pt.y);
			if (!mouse.IsInWindow())
			{
				SetCapture(hWnd);
				mouse.OnMouseEnter();
			}
		}
		// not in client -> log move / maintain capture if button down
		else
		{
			if (wParam & (MK_LBUTTON | MK_RBUTTON))
			{
				mouse.OnMouseMove(pt.x, pt.y);
			}
			// button up -> release capture / log event for leaving
			else
			{
				ReleaseCapture();
				mouse.OnMouseLeave();
			}
		}
		break;
	}
	case WM_LBUTTONDOWN:
	{
		SetForegroundWindow(hWnd);
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnLeftPressed(pt.x, pt.y);
		break;
	}
	case WM_RBUTTONDOWN:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnRightPressed(pt.x, pt.y);
		break;
	}
	case WM_LBUTTONUP:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnLeftReleased(pt.x, pt.y);
		break;
	}
	case WM_RBUTTONUP:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnRightReleased(pt.x, pt.y);
		break;
	}
	case WM_MOUSEWHEEL:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
		mouse.OnWheelDelta(pt.x, pt.y, delta);
		break;
	}
	/************** END MOUSE MESSAGES **************/
	case WM_CLOSE: PostQuitMessage(0); return 0;
	case WM_DESTROY: PostQuitMessage(0); return 0;
	case WM_SIZE:
		RECT clientRect = {};
		GetClientRect(hWnd, &clientRect);
		GraphicsResources::GetInstance().OnSizeChanged(
			clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
		width = Settings::GetGraphicsSettings().GetWidth();
		height = Settings::GetGraphicsSettings().GetHeight();

		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK Window::StartupWindowsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_NCCREATE)
	{
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);

		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::RedirectWindowsProcedure));

		return pWnd->HandleMessage(hWnd, uMsg, wParam, lParam);
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT Window::RedirectWindowsProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	return pWnd->HandleMessage(hWnd, uMsg, wParam, lParam);
}

std::string Window::Exception::TranslateErrorCode(HRESULT hr) noexcept
{
	char* pMsgBuf = nullptr;
	// windows will allocate memory for err string and make our pointer point to it
	const DWORD nMsgLen = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		hr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPSTR>(&pMsgBuf),
		0,
		nullptr);
	// 0 string length returned indicates a failure
	if (nMsgLen == 0)
	{
		return "Unidentified error code";
	}
	// copy error string from windows-allocated buffer to std::string
	std::string errorString = pMsgBuf;
	// free windows buffer
	LocalFree(pMsgBuf);
	return errorString;
}


Window::HrException::HrException(int line, const char* file, HRESULT hr) noexcept : Exception(line, file), hr(hr)
{
}

const char* Window::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode() << std::dec << " ("
		<< (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Window::HrException::GetType() const noexcept
{
	return "Chili Window Exception";
}

HRESULT Window::HrException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Window::HrException::GetErrorDescription() const noexcept
{
	return Exception::TranslateErrorCode(hr);
}


const char* Window::NoGfxException::GetType() const noexcept
{
	return "Chili Window Exception [No Graphics]";
}