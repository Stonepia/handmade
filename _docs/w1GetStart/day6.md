---
title: Add Keyboard Control
permalink: /docs/day6/
---
---
This covers **day006** and add the keyboard control of Xbox and phisical keyboard.

## Add Xbox Contol
XInput is an API that allows applications to receive input from the Xbox 360 Controller for Windows. Using this API, any connected Xbox 360 Controller can be queried for its state, and vibration effects can be set. ([Getting Started With XInput](https://msdn.microsoft.com/en-us/library/windows/desktop/ee417001(v=vs.85).aspx))

### Using Xinput
Throughout the duration of an application,getting state from a controller will probably be done most often.From frame to frame in a game application, state should be retrived and game information updated to reflect the controller changes.

To retrive state, use the [XInputGetState](https://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.reference.xinputgetstate(v=vs.85).aspx) function.

{% highlight C %}

for (DWORD ControllerIndex = 0; 
        ControllerIndex< XUSER_MAX_COUNT; 
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
            int16 StickX = Pad->sThumbLX;
            int16 StickY = Pad->sThumbLY;

            if(AButton)
            {
                YOffset += 2;
            }
        }
    }
}

{% endhighlight %}


We add this piece of code in our running block.   
The first for loop is a very common way of dealing with the multiple controllers. The program keeps querying about different controllers by sequence.  
The XInputGetState can be used to determine if the controller is connected. 

Then the thing remained is prerry simple. We use the [XINPUT_GAMEPAD](https://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.reference.xinput_gamepad(v=vs.85).aspx) Structure to describe the current state of the Xbox 360 Controller. Notice that almost every state we need is the `wButtons`. You can describe defferent operations with the Bitmask of `wButtons`.  
This topic is pretty simple, you can just follow the [document](https://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.reference.xinput_gamepad(v=vs.85).aspx) easily.

However, if you just use this piece of code and compile. You will get an error of :
>error LNK2019: unresolved external symbol XInputGetState referenced in function WinMain

### Solving Linking Problem
You may see this kind of error very often in the after days. There are many ways to get this error. You can find more information [here](https://docs.microsoft.com/en-us/cpp/error-messages/tool-errors/linker-tools-error-lnk2019).   

Our program, however ,the problem is clear. The linker doesn't know how to deal with XInputGetState.
If you see the document of [XInpugGetState]((https://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.reference.xinputgetstate(v=vs.85).aspx) ), you can find at the bottom that it requires the library *XInput.lib* and the DLL *Xinput1_3.dll* .

Obviously, we haven't done anything of these files, so this is why we get the *LNK2019* error.

There are two ways of dealing with this: 

- On Windows, you do not link with a `.dll` file directly -- you must use the accompany `.lib` file instead. To do that, go to **Project -> Properties -> Configuration Properties -> Linker -> Additional Dependencies** and add path to your lib.  
Then make sure that `.dll` file is eigher in the directory contained by the **%PATH%** or that its copy is in **Output Directory**(by default, this is Debug\Release folder).  
If you don't like this way, you can also add a pragma like this :
```
#pragma comment(lib, "dll.lib")
```

- The another way is more complex, but will be more safe and portable by our game-programmer. With this ,you do not have to worry about losing the dll in another computer.  
We observe that we just need two function in XInput.h. That is , **XInpugGetState** and **XInputSetState**. Then all other things is a waste of memory.  
So why not just take these two functions out and define it by ourselves?

### Define own version of XInputGetState and XInputSetState
In Visual Studio, right click on the *#include "XInput.h"* statement and choose **Open Document**. Then we get XInput.h.
Find the function that we need:

{% highlight C %}

DWORD WINAPI XInputGetState
(
    _In_  DWORD         dwUserIndex,  // Index of the gamer associated with the device
    _Out_ XINPUT_STATE* pState        // Receives the current state
);

DWORD WINAPI XInputSetState
(
    _In_ DWORD             dwUserIndex,  // Index of the gamer associated with the device
    _In_ XINPUT_VIBRATION* pVibration    // The vibration information to send to the controller
);

{% endhighlight %}


Then we rewrite the code to our own:

{% highlight C %}

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
//typedef DWORD WINAPI x_input_get_state(DWORD dwUserIndex, XINPUT_STATE* pState);
X_INPUT_GET_STATE(XInputGetStateStub)
{
	return (0);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_
{% endhighlight %}

This is hard to understand, let's read it line by line.

So we get the first line of code :

{% highlight C %}
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
{% endhighlight %}

To read this code ,just refer to the definition of [#define derectives](http://en.cppreference.com/w/cpp/preprocessor/replace):
We find the Version (2) of the syntax is close to this code:

{% highlight C %}
#define identifier( parameters ) replacement-list(optional)	 
{% endhighlight %}

And it reads:
>Function-like macros replace each occurrence of defined identifier with replacement-list, additionally taking a number of arguments, which then replace **corresponding occurrences** of any of the parameters in the replacement-list.  
The syntax of a function-like macro invocation is similar to the syntax of a function call: each instance of the macro name followed by a ( as the next preprocessing token introduces the sequence of tokens that is replaced by the replacement-list. The sequence is terminated by the matching ) token, skipping intervening matched pairs of left and right parentheses.

OK, then this is clear, the X_INPUT_GET_STATE is our identifier, it will be reaplaced by our replacement-list *DWORD WINAPI ...* and the (name) the parameter,this indicates that every *name* will be replaced.
Thus , if we call X_INPUT_GET_STATE (Foo), we will get the function of  
DWORD WINAPI Foo (DWORD dwUserIndex, XINPUT_STATE * pState)

Then the second line of typedef this is easy to understand.
Then the first two lines just equal to : 

{% highlight C %}
typedef DWORD WINAPI x_input_get_state(DWORD dwUserIndex, XINPUT_STATE* pState);
{% endhighlight %}

The third line, add a Stub. The stub is there to have something to call when the DLL cannot be loaded(or something the state get wrong).This is just make the code more secure.

The fourth line, uses a function pointer, this pointer's type is global_variable xinput_get_state *. And it's name is XInputGetState_ (notice that there is a underscore). Ande this pointer's vaule is XInputGetStateStub.

Then finally ,we override the XInputGetState_ with the name XInputGetState. Which is exactly as the name of the original.

In short, these five lines just defines a function called XInputGetState, nothing else.The XInputSetState is very similar to this.


### Load Library and DLL
After all this have done, it's time to load our library:

{% highlight C %}
internal void
Win32LoadXInput(void)
{
	HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
	if(XInputLibrary)
	{
		XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary,"XInputGetState");
		XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary,"XInputSetState");
	}
}
{% endhighlight %}

This firstly load the dll. The [GetProcAddress](https://msdn.microsoft.com/en-us/library/windows/desktop/ms683212(v=vs.85).aspx) function retrieves the address of an exported function or variable from the specified dynamic-link library (DLL). This is just what we need. It returns the address of the exported function or variable, then if we want to call this function ,just use this address.
So by now we have successfully imported the DLL. Compile it and find no error.

## Add keyborad control
What if we want to add a keyborad control? For example, press w key to let the character move forward?

We will need four kinds of message:

|message|meaning|
|---|---|
|WM_SYSKEYDOWN|When the user presses the F10 key (which activates the menu bar) or holds down the ALT key and then presses another key,It also occurs when no window currently has the keyboard focus|
|WM_SYSKEYUP| When the user releases a key that was pressed while the ALT key was held down. It also occurs when no window currently has the keyboard focus|
|WM_KEYDOWN|Posted to the window with the keyboard focus when a nonsystem key is pressed. A nonsystem key is a key that is pressed when the ALT key is not pressed.|
|WM_KEYUP|Posted to the window with the keyboard focus when a nonsystem key is released|
 
Then how to use these messages?  
Recall that we have handled some Messages in the switch statement . So it's a good idea to handle these messages here :


{% highlight C %}

	switch (Message)
	{
	case WM_CREATE: {

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
        }
    }
{% endhighlight %}

We can find that the `wParam` is the virtual-key code of the key being released. So you can just let a parameter called `VKCode` equals to `Wparam` and compare it with what key we want.  
The LParam is the key we are hitting. It's a 32-bits message, every bit has a meaning. By looking at the [documents](https://msdn.microsoft.com/en-us/library/windows/desktop/ms646287(v=vs.85).aspx). We can find that we just need the transition state and the previous key state, which are used to track we have hit and release the key.
We just use the bitwise operation to do this job.


----

And that's all for day006. You may find the **day006.cpp** in history folder, and thank you for your reading.