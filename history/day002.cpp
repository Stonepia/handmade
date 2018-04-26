#include <windows.h>


LRESULT CALLBACK
MainWindowCallback(HWND Window,
	UINT Message,
	WPARAM Wparam,
	LPARAM LParam)
{

	LRESULT Result = 0;
	switch (Message)
	{
	case WM_CREATE: {

	}
	case WM_SIZE:
	{
		OutputDebugString("WM_SIZE \n");
	}break;
	case WM_DESTROY:
	{
		OutputDebugString("WM_DESTROY \n");
	}break;

	case WM_CLOSE:
	{
		OutputDebugString("WM_CLOSE \n");
	}break;

	case WM_ACTIVATEAPP:
		OutputDebugString("WM_ACTIVATEAPP \n");
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT Paint;
		HDC DeviceContext = BeginPaint(Window, &Paint);
		int X = Paint.rcPaint.left;
		int Y = Paint.rcPaint.top;
		int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
		int Width = Paint.rcPaint.right - Paint.rcPaint.left;
		static DWORD Operation = WHITENESS;
		PatBlt(DeviceContext,X,Y,Width,Height, Operation);
		if (Operation == WHITENESS)
		{
			Operation = BLACKNESS;
		}
		else
		{
			Operation = WHITENESS;
		}
		EndPaint(Window, &Paint);
	}break;
	default:
//		OutputDebugString("default \n");
		Result = DefWindowProc(Window,Message,Wparam,LParam);
		break;
	}
	return (Result);
}
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASS WindowClass = {};
	// TODO :Check if HREDRAW/VREDRAW/OWNDC still matters
	WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WindowClass.lpfnWndProc = MainWindowCallback;
	WindowClass.hInstance = hInstance;
	WindowClass.lpszClassName = "HandmadeHeroWindowClass";
	if (RegisterClass(&WindowClass))
	{
		HWND WindowHandle = CreateWindowEx(
			0,
			WindowClass.lpszClassName,
			"Handmade Hero",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			hInstance, 
			0);
		if (WindowHandle) {
			
			for (;;) {
				MSG Message;
				BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
				if (MessageResult > 0)
				{
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}
				else
				{
					break;
				}

			}
		
		}
		else{
			//TODO
		}
	}
	else {
		//TODO
	}
	//WindowClass.hIcon;
	return(0);
}
