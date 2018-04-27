---
title: Start the first Program
permalink: /docs/firstProgram/
---
---
## Brief Summary


This topic covers **day001** to **day002**.
We would first introduce the basical use of windows programming and create a window.  
Then we will get to show something on the Screen.

***

## Two basic functions
### WinMain

[WinMain](https://msdn.microsoft.com/en-us/library/windows/desktop/ms633559%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396) is the conventional name used for the application entry point. You can use WinMain to initialize the application, display its main window etc.

{% highlight C %}

int CALLBACK WinMain(
  _In_ HINSTANCE hInstance,
  _In_ HINSTANCE hPrevInstance,
  _In_ LPSTR     lpCmdLine,
  _In_ int       nCmdShow
);

{% endhighlight %}

About the <span style="color:orange"> \_In_</span>, It's a [SAL annotation](https://docs.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2012/hh916383(v=vs.110)). You can use it to describe how a funtion uses its parameters. For <span style="color:orange"> \_In_</span>,  this indicates that this parameter is used for input. Correspondingly , there is a <span style="color:orange"> \_Out_</span> indicates that it is used for output.

 <span style="color:orange"> HINSTANCE</span> means a handle to the  instance of the application. It can alternatively be obtained from the **GetModuleHandle** API function. So there are two parameters, one is *hInstance*, meaning a handle to the current instance, we have to pass this to **winMain** so that we can get to do something to it. 
 
*hPrevInstance* ,meaning the previous instance, is always NULL.

*lpCmdLine* is the command line for the application, excluding the program name. To retrieve the entire command line, use the **GetCommandLine** function.

*nCmdShow* Controls how the window is to be shown.

### Window procedures
In addition to WinMain function, every Windows desktop application must also have a window-procedure function. This function is typically named **WndProc**.In our program, we named it 
**MainWindowCallback**.

{% highlight C %}

LRESULT CALLBACK
MainWindowCallback(
        HWND Window,
	UINT Message,
	WPARAM Wparam,
	LPARAM LParam);

{% endhighlight %}

In this function you write code to handle messages that the application receives from Windows when events occur.For example, if a user clicks an OK button in your application, Windows will send a message to you and you can write code inside your WndProc function that does whatever work is appropriate. This is called *handling* an event. You only handle the events that are relevant for your application.You may find full explanation [here](https://msdn.microsoft.com/library/windows/desktop/ms632593).

<br>
Thus ,now we have two functions to write:

{% highlight C %}

LRESULT CALLBACK
MainWindowCallback(
        HWND Window,
	UINT Message,
	WPARAM Wparam,
	LPARAM LParam)
{
    //deals with events
}

int CALLBACK WinMain(
   HINSTANCE hInstance,
   HINSTANCE hPrevInstance,
   LPSTR     lpCmdLine,
   int       nCmdShow)
{
    //deals with some information about the window
}

{% endhighlight %}
<br>

---
## Add funtionality to the WinMain function

### WNDCLASS

 In the **winMain**, we populate a structure of type [WNDCLASS](https://msdn.microsoft.com/en-us/library/windows/desktop/ms633576)(You may also use [WNDCLASSEX](https://msdn.microsoft.com/en-us/library/windows/desktop/ms633577)).


{% highlight C %}

typedef struct tagWNDCLASS {
  UINT      style;
  WNDPROC   lpfnWndProc;
  int       cbClsExtra;
  int       cbWndExtra;
  HINSTANCE hInstance;
  HICON     hIcon;
  HCURSOR   hCursor;
  HBRUSH    hbrBackground;
  LPCTSTR   lpszMenuName;
  LPCTSTR   lpszClassName;
} WNDCLASS, *PWNDCLASS;

{% endhighlight %}

A typical WNDCLASS structure will looks like this:

{% highlight C %}
  WNDCLASS WindowClass = {};

  WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  WindowClass.lpfnWndProc = MainWindowCallback;
  WindowClass.hInstance = hInstance;
  WindowClass.lpszClassName = "HandmadeHeroWindowClass";

{% endhighlight %}

The first line above takes the default initialization of WINDCLASS.

 You must register the WINDCLASS with Windows so that it knows about your window and how to send message to it. So now here comes the [RegisterClass](https://msdn.microsoft.com/en-us/library/windows/desktop/ms633586(v=vs.85).aspx) ( [RegisterClassEx](https://msdn.microsoft.com/library/windows/desktop/ms633587) )function.

So here comes a typical way of dealing with this:




{% highlight C %}

if (!RegisterClassEx(&WindowClass))  
{  
    MessageBox(NULL,  
        _T("Call to RegisterClassEx failed!"),  
        _T("Win32 Guided Tour"),  
        NULL);  

    return 1;  
}  
{% endhighlight %}


Now we can create a Window. Use the [CreateWindowEx](https://msdn.microsoft.com/en-us/library/windows/desktop/ms632680(v=vs.85).aspx) function.

{% highlight C %}
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
{% endhighlight %}

This function returns an HWND, which is a handle to a window. A handle is somewhat like a pointer that Windows uses to keep track of a window.

At this point, the window has been created, we try to Handle the messages.

{% highlight C %}
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
{% endhighlight %}

Casey defined a infinite loop. But in my opinion, doing this like the following may be more concise:
{% highlight C %}

MSG msg;  
while (GetMessage(&msg, NULL, 0, 0))  
{  
    TranslateMessage(&msg);  
    DispatchMessage(&msg);  
}  

{% endhighlight %}

For more information about this loop, you can see [MSG](https://msdn.microsoft.com/library/windows/desktop/ms644958), [GetMessage](https://msdn.microsoft.com/library/windows/desktop/ms644936),
[TranslateMessage](https://msdn.microsoft.com/library/windows/desktop/ms644955),
and [DispatchMessage](https://msdn.microsoft.com/library/windows/desktop/ms644934).

---
## Add funtionality to the MainWindowCallback function
Recall that the **MainWindowCallback** function ([WindowProc callback function](https://msdn.microsoft.com/en-us/library/windows/desktop/ms633573))is used to handle the messages the application receives.We implement it as a switch statement.  
We define the function like this:

{% highlight C %}
LRESULT CALLBACK
MainWindowCallback(HWND Window,
	UINT Message,
	WPARAM Wparam,
	LPARAM LParam)
{
  //switch statement
}
{% endhighlight %}

As its literal meaning, we have to focus on the *Message* parameter.This is what we have just got from the **GetMessage()**.   

One important message to handle is the [WM_PAINT](https://msdn.microsoft.com/library/windows/desktop/dd145213) message.This application receives this message when part of its displayed window must be updated. to handle a WM_PAINT message, first call [BeginPaint](https://msdn.microsoft.com/library/windows/desktop/dd183362),then handle all the logic to lay out the text, buttons and other controls in the window.
After painting, call [EndPaint](https://msdn.microsoft.com/library/windows/desktop/dd162598).

{% highlight C %}
	case WM_PAINT:
	{
		PAINTSTRUCT Paint;
		HDC DeviceContext = BeginPaint(Window, &Paint);
   
    //Some operations
  
		EndPaint(Window, &Paint);
	}break;

{% endhighlight %}

HDC is a handle to a device context, which is a data structure that Windows uses to enable your application to communicate with the graphics subsystem. 

If we want to show something on screen, we have to use the function [PatBlt](https://msdn.microsoft.com/en-us/library/windows/desktop/dd162778(v=vs.85).aspx).This function paints the specified rectangle using the brush.Then the code is like this:

{% highlight C %}

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

{% endhighlight %}



An application typically handles many other messages, for example ,[WM_CREATE](https://msdn.microsoft.com/library/windows/desktop/ms632619)
when the window is created.[WM_DESTROY](https://msdn.microsoft.com/library/windows/desktop/ms632620)when a window is being destroyed.

One another important thing is we have to define a default operation for our switch statement:

{% highlight C %}

	default:
		Result = DefWindowProc(Window,Message,Wparam,LParam);
		break;
{% endhighlight %}

Notice that you have to pass all other things to **DefWindowProc**.If you omit DefWindowProc then you are saying "For all messages I did not handle above, do nothing." Which means that a lot of messages like "Please draw the buttons" get handled as "do nothing." Result: No buttons.([details here](https://stackoverflow.com/questions/11375812/behaviour-of-defwindowproc-winapi)).

---

By now we have reached the day 002. You may find the **day002.cpp** in history folder, and thank you for your reading.