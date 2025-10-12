#pragma once

#include "engine/gfx/GraphicsResources.hpp"
#include "Keyboard.hpp"
#include "Mouse.hpp"
#include "engine/core/CustomException.hpp"

namespace engine::platform
{

class WindowClass
{
public:
	static const wchar_t* GetName() noexcept;
	static HINSTANCE GetInstance() noexcept;

private:
	WindowClass() noexcept;
	~WindowClass();

	WindowClass& operator=(const WindowClass&) = delete;
	WindowClass& operator=(const WindowClass&&) = delete;
	WindowClass(const WindowClass&) = delete;
	WindowClass(const WindowClass&&) = delete;

	static WindowClass wndClass;
	static constexpr const wchar_t* wndClassName = L"ClassName";

	HINSTANCE hInstance;
};

class Window
{
public:
	class Exception : public engine::core::CustomException
	{
		using engine::core::CustomException::CustomException;

	public:
		static std::string TranslateErrorCode(HRESULT hr) noexcept;
	};

	class HrException : public Exception
	{
	public:
		HrException(int line, const char* file, HRESULT hr) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorDescription() const noexcept;

	private:
		HRESULT hr;
	};

	class NoGfxException : public Exception
	{
	public:
		using Exception::Exception;
		const char* GetType() const noexcept override;
	};

public:
	Window() = delete;
	Window(int width, int height, const wchar_t* name);
	~Window();

	Window(const Window&) = delete;
	Window(const Window&&) = delete;
	Window& operator=(const Window&) = delete;
	Window& operator=(const Window&&) = delete;

	void SetTitle(const std::wstring& title);
	Mouse& GetMouse() noexcept;
	Keyboard& GetKeyboard() noexcept;
	HWND& GetWindowHandler() noexcept;

	static std::optional<int> ProcessMessage();

private:
	LRESULT CALLBACK HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK StartupWindowsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK RedirectWindowsProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	friend WindowClass;

private:
	Mouse mouse;
	Keyboard keyboard;

	int width;
	int height;
	HWND hWnd;
};

}  // namespace engine::platform