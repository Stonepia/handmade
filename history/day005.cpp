#include <windows.h>
#include <stdint.h>
#define internal static
#define local_persist static
#define global_variable static
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
	    
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

struct win32_offscreen_buffer
{
	BITMAPINFO Info;
	void *Memory;
	int Width;
	int Height;
	int Pitch;
	int BytesPerPixel;
};
//TODO: THis is a global for now
global_variable bool Running = false;
global_variable win32_offscreen_buffer GlobalBackBuffer;
struct win32_window_dimension
{
	int Width;
	int Height;
};
win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
	win32_window_dimension Result;
	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;

	return (Result);
}
internal void
RenderWeirdGradient(win32_offscreen_buffer Buffer,int XOffset,int YOffset)
{
	int Width = Buffer.Width;
	int Height = Buffer.Height;
	
	
	uint8 *Row = (uint8 *)Buffer.Memory;

	for (int Y = 0; Y < Buffer.Height; ++Y)
	{
	uint32 *Pixel = (uint32 *)Row;
	for (int X = 0; X < Buffer.Width; ++X)
	{
		uint8 Blue= (X + XOffset);
		uint8 Green = (Y + YOffset);
		//Because its little endian. so the first is blue. not red
		
		*Pixel++ = ((Green<<8)|Blue);
	}
	Row += Buffer.Pitch;
	}

}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer,int Width, int Height)
{	//Device Independent Bitmap

	if(Buffer->Memory)
	{
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE);

	}
	Buffer->Width = Width;
	Buffer->Height = Height;
	Buffer->BytesPerPixel = 4;
	//NOTE: When the biHeight filed is negative, this is the clue to 
	//Windows to treat thsi bitmao as top-down, not bottom-up, meaning that 
	//the first three bytes of the image are the color for the top left pixel
	//in the bitmap,not the bottom left
	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	Buffer->Info.bmiHeader.biHeight = -Buffer->Height;//paint top-down
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;
	int BitmapMemorySize =(Buffer->Width*Buffer->Height) * Buffer->BytesPerPixel ;
	Buffer->Memory = VirtualAlloc(0,BitmapMemorySize,MEM_COMMIT,PAGE_READWRITE);


	Buffer->Pitch = Width * (Buffer->BytesPerPixel);

}

internal void
Win32DisplayBufferInWindow(HDC DeviceContext, int WindowWidth,int WindowHeight,
							win32_offscreen_buffer Buffer,
							int X,int  Y, int  Width, int Height)
{
	//TODO: Aspect ratio correciton
	//TODO: Play with stretch modes
	StretchDIBits(DeviceContext,
		0, 0, WindowWidth, WindowHeight,
		0, 0, Buffer.Width, Buffer.Height,
		Buffer.Memory,&Buffer.Info,
		DIB_RGB_COLORS,SRCCOPY);
}
LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
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
	}break;
	case WM_DESTROY:
	{
		//TODO :Handle as an error - recreate window?
		Running =false;
	}break;

	case WM_CLOSE:
	{
		//TODO: Handle this with a message to the user?
		//PostQuitMessage(0);// day3
		Running = false;
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
	
		win32_window_dimension Dimension = Win32GetWindowDimension(Window);

		Win32DisplayBufferInWindow(DeviceContext,Dimension.Width,Dimension.Height,
			GlobalBackBuffer,
			X,Y,Width,Height);
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
	
	
	Win32ResizeDIBSection(&GlobalBackBuffer,1280,720);

	WindowClass.style =  CS_HREDRAW | CS_VREDRAW|CS_OWNDC;
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
	WindowClass.hInstance = hInstance;
	WindowClass.lpszClassName = "HandmadeHeroWindowClass";
	if (RegisterClass(&WindowClass))
	{
		HWND Window = CreateWindowExA(
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
		if (Window) {
			
			//Since we specified CS_OWNDC, we can just get one device context and use it forever
			//because we are not sharing it with anyone
			HDC DeviceContext = GetDC(Window);
			Running = true;
			int XOffset = 0;
			int YOffset = 0;
			
			while(Running) {
				MSG Message;
				while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
				{
					
					if(Message.message == WM_QUIT)
					{
						Running = false;
					}
					TranslateMessage(&Message);
					DispatchMessageA(&Message);
				}
				RenderWeirdGradient(GlobalBackBuffer,XOffset, YOffset);

				
				win32_window_dimension Dimension = Win32GetWindowDimension(Window);
				Win32DisplayBufferInWindow(DeviceContext, Dimension.Width,Dimension.Height,
						GlobalBackBuffer,0, 0, Dimension.Width, Dimension.Height);
				ReleaseDC(Window, DeviceContext);
				
				++XOffset;
			}
		
		}
		else{
			//TODO:logging
		}
	}
	else {
		//TODO :logging
	}
	//WindowClass.hIcon;
	return(0);
}
