#include "windows.h"
#include "GL\gl.h"
#include "GL\GLU.H"
#include "GL\GLAux.h"

#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glaux.lib")

// Global Variables:
HINSTANCE hInst;                                // current instance
HGLRC hRC = NULL;
HDC hDC = NULL;
HWND hWnd = NULL;

bool keys[256];
bool active = TRUE;
bool fullscreen = FALSE;

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)
{
	if (height == 0)
		height = 1;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int InitGL(GLvoid)
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClearDepth(1.f);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	return TRUE;
}

int DrawGLScene(GLvoid)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glTranslatef(-1.5f, 0.f, -6.f);
	glBegin(GL_TRIANGLES);
	glVertex3f(0.f, 1.f, 0.f);
	glVertex3f(-1.f, -1.f, 0.f);
	glVertex3f(1.f, -1.f, 0.f);
	glEnd();

	glTranslatef(3.f, 0.f, 0.f);
	glBegin(GL_QUADS);
	glVertex3f(-1.f, 1.f, 0.f);
	glVertex3f(1.f, 1.f, 0.f);
	glVertex3f(1.f, -1.f, 0.f);
	glVertex3f(-1.f, -1.f, 0.f);
	glEnd();

	return TRUE;
}

GLvoid KillGLWindow(GLvoid)
{
	if (fullscreen)
	{
		ChangeDisplaySettings(NULL, 0);
		ShowCursor(TRUE);
	}
	if (hRC)
	{
		if (!wglMakeCurrent(NULL, NULL))
		{
			MessageBox(NULL, "Release of DC and RC failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		if (!wglDeleteContext(hRC))
		{
			MessageBox(NULL, "Release Rendering Context failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;
	}
	if (hDC && !ReleaseDC(hWnd, hDC))
	{
		MessageBox(NULL, "Release Device Context failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;
	}
	if (hWnd && !DestroyWindow(hWnd))
	{
		MessageBox(NULL, "Could Not Release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;
	}
	if (!UnregisterClass("OpenGL", hInst))
	{
		MessageBox(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInst = NULL;
	}
}

BOOL CreateGLWindow(char * title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint PixelFormat;
	WNDCLASS wc;
	DWORD dwExStyle;
	DWORD dwStyle;
	RECT WindowRect;
	WindowRect.left = (long)0;
	WindowRect.right = (long)width;
	WindowRect.top = (long)0;
	WindowRect.bottom = (long)height;

	fullscreen = fullscreenflag;
	hInst = GetModuleHandle(NULL);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "OpenGL";

	if (!RegisterClass(&wc))
	{
		MessageBox(NULL, "Failed to register the window class.", "ERROR", MB_OK | MB_ICONINFORMATION);
		return false;
	}

	if (fullscreen)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = width;
		dmScreenSettings.dmPelsHeight = height;
		dmScreenSettings.dmBitsPerPel = bits;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			if (MessageBox(NULL, "The requested fullscreen mode is not supported by\nyour video card. use windowed mode instead?", "NeHE GL", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				fullscreen = FALSE;
			}
			else
			{
				MessageBox(NULL, "Program will now close.", "ERROR", MB_OK | MB_ICONSTOP);
				return FALSE;
			}
		}
	}

	if (fullscreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;
		ShowCursor(FALSE);
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW;
	}
	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

	if (!(hWnd = CreateWindowEx(
		dwExStyle,
		"OpenGL",
		title,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwStyle,
		0, 0,
		WindowRect.right - WindowRect.left,
		WindowRect.bottom - WindowRect.top,
		NULL,
		NULL,
		hInst,
		NULL
		)))
	{
		KillGLWindow();
		MessageBox(NULL, "Window Creation Error.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		bits,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		16,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	if (!(hDC = GetDC(hWnd)))
	{
		KillGLWindow();
		MessageBox(NULL, "Can't Create A GL DC.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
	
	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))
	{
		KillGLWindow();
		MessageBox(NULL, "Can't find a suitable pixelformat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))
	{
		KillGLWindow();
		MessageBox(NULL, "Can't set the pixelformat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (!(hRC = wglCreateContext(hDC)))
	{
		KillGLWindow();
		MessageBox(NULL, "Can't create a GL RC.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (!wglMakeCurrent(hDC, hRC))
	{
		KillGLWindow();
		MessageBox(NULL, "Can't activate the GL RC.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	ReSizeGLScene(width, height);

	if (!InitGL())
	{
		KillGLWindow();
		MessageBox(NULL, "Init Failed.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	return TRUE;
}

LRESULT CALLBACK WndProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	switch (uMsg)
	{
	case WM_ACTIVATE:
	{
		if (!HIWORD(wParam))
		{
			active = TRUE;
		}
		else
		{
			active = FALSE;
		}
		return 0;
	}
	case WM_SYSCOMMAND:
	{
		switch (wParam)
		{
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			return 0;
		}
		break;
	}
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}
	case WM_KEYDOWN:
	{
		keys[wParam] = TRUE;
		return 0;
	}
	case WM_KEYUP:
	{
		keys[wParam] = FALSE;
		return 0;
	}
	case WM_SIZE:
	{
		ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));
		return 0;
	}
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
	)
{
	
	MSG msg;
	BOOL done = FALSE;
	
	/*
	if (MessageBox(NULL, "Would you like to run in full screen mode?", "Start Fullscreen?", MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		fullscreen = FALSE;
	}
	*/

	if (!CreateGLWindow("NeHe's OpenGL Framework", 640, 480, 16, fullscreen))
	{
		return 0;
	}
	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				done = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (active)
			{
				if (keys[VK_ESCAPE])
				{
					done = TRUE;
				}
				else
				{
					DrawGLScene();
					SwapBuffers(hDC);
				}
			}
			if (keys[VK_F1])
			{
				keys[VK_F1] = FALSE;
				KillGLWindow();
				fullscreen = !fullscreen;
				if (CreateGLWindow("NeHe's OpenGL Framework", 640, 480, 16, fullscreen))
				{
					return 0;
				}
			}
		}
	}

	KillGLWindow();
	return (msg.wParam);
}
