#pragma comment(lib, "dsound.lib")

#include <windows.h>
#include <stdint.h>
#include <XInput.h>
#include <dsound.h>

#define internal static
#define local_persist static
#define global_variable static
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int32_t bool32;

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
global_variable bool GlobalRunning = false;
global_variable win32_offscreen_buffer GlobalBackBuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;


struct win32_window_dimension
{
	int Width;
	int Height;
};

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)//defines a simple function
//This replaces X_INPUT_GET_STATE with the replacement-list, 
//and the parameter's name is replaced by parameters int he replacement-list
typedef X_INPUT_GET_STATE(x_input_get_state);
//typedef DWORD WINAPI x_input_get_state(DWORD dwUserIndex, XINPUT_STATE* pState);
X_INPUT_GET_STATE(XInputGetStateStub)
{
	return (ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;

#define XInputGetState XInputGetState_
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex,XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
//typedef DWORD WINAPI x_input_set_state(DWORD dwUserIndex,XINPUT_VIBRATION* pVibration);
X_INPUT_SET_STATE(XInputSetStateStub)
{
	return (ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_


//#define DIRECT_SOUND_CREATE8(name) HRESULT WINAPI name( LPCGUID pcGuidDevice,  LPDIRECTSOUND8 *ppDS8,  LPUNKNOWN pUnkOuter)
//typedef DIRECT_SOUND_CREATE8(direct_sound_create);

internal void
Win32LoadXInput(void)
{
	HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
	if (!XInputLibrary)
	{
		XInputLibrary = LoadLibraryA("xinput1_3.dll");
	}
	if (XInputLibrary)
	{
		XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
		if (!XInputGetState) { XInputGetState = XInputGetStateStub; }
		XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
		if (!XInputSetState) { XInputSetState = XInputSetStateStub; }
	}
}
internal void
Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
	LPDIRECTSOUND8 DirectSound;

	if (SUCCEEDED(DirectSoundCreate8(nullptr, &DirectSound, nullptr)))
	{
		WAVEFORMATEX WaveFormat = {};
		WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
		WaveFormat.nChannels = 2;
		WaveFormat.nSamplesPerSec = SamplesPerSecond;
		WaveFormat.wBitsPerSample = 16;
		WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
		WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
		WaveFormat.cbSize = 0;

		if (SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
		{
			DSBUFFERDESC BufferDescription = {};
			SecureZeroMemory(&BufferDescription, sizeof(DSBUFFERDESC));
			BufferDescription.dwSize = sizeof(BufferDescription);
			BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;//This indicates that it's a Primary Buffer
			//BufferDescription.dwBufferBytes = 0;
			//BufferDescription.lpwfxFormat = NULL;


			//Create the Primary Buffer
			//TODO: DSCCAPS_GLOBALFOCUS?
			LPDIRECTSOUNDBUFFER PrimaryBuffer;
			if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
			{
				if (SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat)))
				{
					OutputDebugStringA("Primary Buffer format was set\n");
				}
				else
				{
					//TODO:DIagnostic
				}
			}
			else
			{
				//TODO: Diagnostic
			}
		}
		//create a secondary buffer
		//TODO: DSBCAPS_GETCURRENTPOSITON2
		DSBUFFERDESC BufferDescription = {};
		//SecureZeroMemory(&BufferDescription, sizeof(DSBUFFERDESC));
		BufferDescription.dwSize = sizeof(BufferDescription);
		BufferDescription.dwFlags = 0;//TODO: add  DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY| DSBCAPS_GLOBALFOCUS ?
		BufferDescription.dwBufferBytes = BufferSize;
		BufferDescription.lpwfxFormat = &WaveFormat;



		if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0)))
		{
			OutputDebugStringA("Secondary buffer created successfully\n");
		}
	}
	else
	{
		//TODO: Diagnostic	
	}



	//start it playing

}



internal win32_window_dimension
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
RenderWeirdGradient(win32_offscreen_buffer *Buffer, int XOffset, int YOffset)
{
	int Width = Buffer->Width;
	int Height = Buffer->Height;


	uint8 *Row = (uint8 *)Buffer->Memory;

	for (int Y = 0; Y < Buffer->Height; ++Y)
	{
		uint32 *Pixel = (uint32 *)Row;
		for (int X = 0; X < Buffer->Width; ++X)
		{
			uint8 Blue = (X + XOffset);
			uint8 Green = (Y + YOffset);
			//Because its little endian. so the first is blue. not red

			*Pixel++ = ((Green << 8) | Blue);
		}
		Row += Buffer->Pitch;
	}

}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{	//Device Independent Bitmap

	if (Buffer->Memory)
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
	int BitmapMemorySize = (Buffer->Width*Buffer->Height) * Buffer->BytesPerPixel;
	Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	Buffer->Pitch = Width * (Buffer->BytesPerPixel);

}

internal void
Win32DisplayBufferInWindow(win32_offscreen_buffer *Buffer,
	HDC DeviceContext, int WindowWidth, int WindowHeight,
	int X, int  Y, int  Width, int Height)
{
	//TODO: Aspect ratio correciton
	//TODO: Play with stretch modes
	StretchDIBits(DeviceContext,
		0, 0, WindowWidth, WindowHeight,
		0, 0, Buffer->Width, Buffer->Height,
		Buffer->Memory, &Buffer->Info,
		DIB_RGB_COLORS, SRCCOPY);
}
internal LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
	UINT Message,
	WPARAM WParam,
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
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		uint32 VKCode = WParam;//VIrtual-Key Codes
		bool WasDown = ((LParam & (1 << 30)) != 0);
		bool IsDown = ((LParam & (1 << 31)) == 0);
		if (WasDown != IsDown)
		{
			if (VKCode == 'W')
			{
			}
			else if (VKCode == 'A')
			{
			}
			else if (VKCode == 'S')
			{
			}
			else if (VKCode == 'D')
			{
			}
			else if (VKCode == 'Q')
			{
			}
			else if (VKCode == 'E')
			{
			}
			else if (VKCode == VK_UP)
			{
			}
			else if (VKCode == VK_DOWN)
			{
			}
			else if (VKCode == VK_RIGHT)
			{
			}
			else if (VKCode == VK_ESCAPE)
			{
				OutputDebugStringA("ESCAPE: ");
				if (IsDown)
					OutputDebugStringA("IsDown ");
				if (WasDown)
					OutputDebugStringA("WasDown");
				OutputDebugStringA("\n");
			}
			else if (VKCode == VK_SPACE)
			{
			}
		}
		bool32 AltKeyWasDown = (LParam &(1 << 29)); //see WM_SYSKEYDOWN,at this point we don't have to set it as bool
			//Because we just have to ``````````````````````````
		if ((VKCode == VK_F4) && AltKeyWasDown)
		{
			GlobalRunning = false;
		}
	}break;
	case WM_DESTROY:
	{
		//TODO :Handle as an error - recreate window?
		GlobalRunning = false;
	}break;

	case WM_CLOSE:
	{
		//TODO: Handle this with a message to the user?
		//PostQuitMessage(0);// day3
		GlobalRunning = false;
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

		Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height,
			X, Y, Width, Height);
		EndPaint(Window, &Paint);
	}break;
	default:
		//		OutputDebugString("default \n");
		Result = DefWindowProc(Window, Message, WParam, LParam);
		break;
	}
	return (Result);
}
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Win32LoadXInput();
	WNDCLASS WindowClass = {};
	Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

	WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
	WindowClass.hInstance = hInstance;
	WindowClass.lpszClassName = "HandmadeHeroWindowClass";
	if (RegisterClassA(&WindowClass))
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
			//Graphics Test
			int XOffset = 0;
			int YOffset = 0;

			//Sound Test
			int SamplesPerSecond = 48000;
			int ToneHz = 256;
			int16 ToneVolume = 6000;
			uint32  RunningSampleIndex = 0;
			//int SquareWaveCounter = 0;
			int SquareWavePeriod = SamplesPerSecond / ToneHz;
			int HalfSquareWavePeriod = SquareWavePeriod / 2;
			int BytesPerSample = sizeof(int16) * 2;
			int SecondaryBufferSize = SamplesPerSecond * BytesPerSample;


			Win32InitDSound(Window, SamplesPerSecond, SecondaryBufferSize);
			bool32 SoundIsPlaying = false;


			GlobalRunning = true;
			while (GlobalRunning) {
				MSG Message;
				while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
				{

					if (Message.message == WM_QUIT)
					{
						GlobalRunning = false;
					}
					TranslateMessage(&Message);
					DispatchMessageA(&Message);
				}

				//Xbox controller
				//TODO should we poll this more frequently?
				for (DWORD ControllerIndex = 0;
					ControllerIndex < XUSER_MAX_COUNT;
					++ControllerIndex)
				{
					XINPUT_STATE ControllerState;
					if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
					{
						//This controller is plugged in
						XINPUT_STATE ControllerState;
						if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
						{
							// TODO:See if ControllerState.dwPacketNumber increments too rapidly
							XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;
							bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
							bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
							bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
							bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
							bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
							bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
							bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
							bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
							bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
							bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
							bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
							bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

							int16 StickX = Pad->sThumbLX;
							int16 StickY = Pad->sThumbLY;
							XOffset += StickX >> 12;
							YOffset += StickY >> 12;
						}
					}
					else
					{
						//Controller not available
					}

				}
				RenderWeirdGradient(&GlobalBackBuffer, XOffset, YOffset);

				DWORD PlayCursor;
				DWORD WriteCursor;
				if (SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
				{

					DWORD BytesToLock = RunningSampleIndex * BytesPerSample % SecondaryBufferSize;
					DWORD BytesToWrite;
					if (BytesToLock == PlayCursor)
					{
						BytesToWrite = SecondaryBufferSize;
					}
					else if (BytesToLock > PlayCursor)
						//	if (BytesToLock > PlayCursor)
					{
						BytesToWrite = SecondaryBufferSize - BytesToLock;
						BytesToWrite += PlayCursor;
					}
					else
					{
						BytesToWrite = PlayCursor - BytesToLock;
					}

					VOID * Region1;
					DWORD Region1Size;
					VOID * Region2;
					DWORD Region2Size;
					if (SUCCEEDED(GlobalSecondaryBuffer->Lock(
						BytesToLock, BytesToWrite,
						&Region1, &Region1Size,
						&Region2, &Region2Size,
						0)))
					{
						//TODO : assert that Region1Size/2Size is valid

						DWORD Region1SampleCount = Region1Size / BytesPerSample;
						int16 *SampleOut = (int16 *)Region1;
						for (DWORD SampleIndex = 0;
							SampleIndex < Region1SampleCount;
							++SampleIndex)
						{
							int16 SampleValue = ((RunningSampleIndex++ / HalfSquareWavePeriod) % 2) ? ToneVolume : -ToneVolume;
							*SampleOut++ = SampleValue;
							*SampleOut++ = SampleValue;
						}


						DWORD Region2SampleCount = Region2Size / BytesPerSample;
						SampleOut = (int16 *)Region2;
						for (DWORD SampleIndex = 0;
							SampleIndex < Region2SampleCount;
							++SampleIndex)
						{
							int16 SampleValue = ((RunningSampleIndex++ / HalfSquareWavePeriod) % 2) ? ToneVolume : -ToneVolume;
							*SampleOut++ = SampleValue;
							*SampleOut++ = SampleValue;
						}
						GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
					}
				}


				if (!SoundIsPlaying)
				{
					GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
					SoundIsPlaying = true;
				}


				win32_window_dimension Dimension = Win32GetWindowDimension(Window);
				Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height,
					0, 0, Dimension.Width, Dimension.Height);
				ReleaseDC(Window, DeviceContext);



			}

		}
		else {
			//TODO:logging
		}
	}
	else {
		//TODO :logging
	}
	//WindowClass.hIcon;
	return(0);
}
