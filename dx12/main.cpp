#include <stdio.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

const char* WINDOW_CLASS = "rtow_window_class";

constexpr int WINDOW_DEFAULT_WIDTH = 1024;
constexpr int WINDOW_DEFAULT_HEIGHT = WINDOW_DEFAULT_WIDTH / (16.0 / 9.0);

void check_handle(HANDLE h, const char* msg)
{
	if (h == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		fprintf(stderr, "%s, Error: %d\n", msg, error);
		exit(error);
	}
}

struct Window
{
	int width, height;
	const char* title;
	HWND handle;
	HDC hdc;
};

LRESULT CALLBACK _window_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_SIZE:
		// renderer_draw(renderer);
		break;
	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

void window_register_class(const char* window_class)
{
	WNDCLASSEX wc{};
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = _window_callback;
	wc.hInstance = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = window_class;
	RegisterClassEx(&wc);
}

Window window_new(int width, int height, const char* title)
{
	Window self{};
	self.width = width;
	self.height = height;
	self.title = title;

	RECT wr{};
	wr.right = LONG(self.width);
	wr.bottom = LONG(self.height);

	self.handle = CreateWindowEx(
		NULL,
		WINDOW_CLASS,
		self.title,
		WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL,
		NULL,
		NULL,
		NULL
	);
	check_handle(self.handle, "CreateWindowEx failed");

	self.hdc = GetDC(self.handle);
	ShowWindow(self.handle, SW_SHOW);
	SetForegroundWindow(self.handle);
	SetFocus(self.handle);

	return self;
}

void window_free(Window& self)
{
	ReleaseDC(self.handle, self.hdc);
	DestroyWindow(self.handle);
}

int main(int argc, char** argv)
{
	window_register_class(WINDOW_CLASS);
	auto window = window_new(WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT, "Ray Tracing in One Weekend");

	MSG msg{};
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// renderer_draw(renderer);
		}
	}

	window_free(window);
	return 0;
}
