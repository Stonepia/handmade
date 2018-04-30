---
title: Show Something on Screen
permalink: /docs/day3_5/
---
---

## Brief Summary


This topic covers **day003** to **day005**.
We will learn how to allocate a buffer to Windows and make a very simple animation.


### Get Window Size

If we want to paint something on screen, the first thing we should do is get the size of our window. This job is done by 
[GetClientRect](https://msdn.microsoft.com/en-us/library/windows/desktop/ms633519(v=vs.85).aspx).  

The dimensions are given in screen coordinates that are relative to the **upper-left** corner of the screen.

Because we may use this dimension many times, we would like to define a structure called `win32_window_dimension`. It contains the `Width` and `Height` of a window.

We may also want to define a function, which retrives the window's dimension.

So we can define a function like this:

{% highlight C %}

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
{% endhighlight %}

 *This code refactoring is done in day005. But I think illustrate it first may be more easy to understand.* 

 ### Creating DIB 

A [**DIB**](https://msdn.microsoft.com/en-us/library/windows/desktop/dd183562(v=vs.85).aspx)(Device Independent Bitmap) contains a *color table*. This describes how pixel values correspond to RGB values. A DIB can achieve the proper color scheme on any device.  


we use a [BITMAPINFO](https://msdn.microsoft.com/en-us/library/windows/desktop/dd183375(v=vs.85).aspx) structure and define a DIB himself.

A BITMAPINFO structure is defined as follows:

{% highlight C %}
typedef struct tagBITMAPINFO {
  BITMAPINFOHEADER bmiHeader;
  RGBQUAD          bmiColors[1];
} BITMAPINFO, *PBITMAPINFO;
{% endhighlight %}

A DIB consists of two distinct parts : a `BITMAPINFO` structure describing the dimensions and colors of the bitmap, and an array of bytes defining the pixels of the bitmap(`bmiColors`).

The first parameter `bmiHeader` contains information about the dimensions and color format of this DIB. We would have to assign what we want program to paint to this. If the height of the bitmap is positive, the bitmap is a bottom-up DIB and its origin is the lower-left corner. If the height is negative, the bitmap is a top-down DIB and its origin is the upper left corner.


Let's first define a structure, this sturcture is called **win32_offscreen_buffer**. It stores our BITMAPINFO, and  the Width , Height that we will use.
{% highlight C %}
    struct win32_offscreen_buffer
    {
        BITMAPINFO Info;
        void *Memory;//discuss later
        int Width;
        int Height;
        int Pitch;//how many pixels a line have
        int BytesPerPixel;//how many bytes a pixel have
    };
{% endhighlight %}

### Allocating a BackBuffer
We define a function called **Win32ResizeDIBSection**. This function manages our DIB and allocate a buffer for it.

{% highlight C %}
internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer,int Width, int Height)
{	
	if(Buffer->Memory)
	{
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE); //free first

	}
	Buffer->Width = Width;
	Buffer->Height = Height;
	Buffer->BytesPerPixel = 4;

	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	Buffer->Info.bmiHeader.biHeight = -Buffer->Height;//negative, so paint top-down
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;
	int BitmapMemorySize =(Buffer->Width*Buffer->Height) * Buffer->BytesPerPixel ;
	Buffer->Memory = VirtualAlloc(0,BitmapMemorySize,MEM_COMMIT,PAGE_READWRITE);
	Buffer->Pitch = Width * (Buffer->BytesPerPixel);
}
{% endhighlight %}

This will set the BITMAPINFO of our `Buffer` and allocate Memory for it. Notice that the `biHeight` is negative, so we paint topdown.

The `Pitch` is how many Bytes a line have. You can assume that the pixels are stored continuously, so if we have Row = 1, then this is the first Row=1 's pixel. And Row + Pitch is the pixel correspond to the second Row.
The [VirtualAlloc](https://msdn.microsoft.com/en-us/library/windows/desktop/aa366887(v=vs.85).aspx) function allocate memory in the address space. We should calculate how many pixels we need by Width*Height. Then calculate how many Bytes we need.
The *VirtualAlloc* function just reserve a region of free pages without consuming physical storage, until it is needed.

By now, we have got our windowsize, we have allocated the memory, now it's time to paint.

### Begin to Paint !
We use the [StretchDIBits](https://msdn.microsoft.com/en-us/library/windows/desktop/dd145121(v=vs.85).aspx) function to copy color data for a rectangle of pixels in a DIB. This function automatically stretches the color data to fit the destination rectangle. So if we resize the window, the program can adapt to it.

We can find the StetchDIBits definition as follows:

{% highlight C %}

int StretchDIBits(
  _In_       HDC        hdc,
  _In_       int        XDest,
  _In_       int        YDest,
  _In_       int        nDestWidth,
  _In_       int        nDestHeight,
  _In_       int        XSrc,
  _In_       int        YSrc,
  _In_       int        nSrcWidth,
  _In_       int        nSrcHeight,
  _In_ const VOID       *lpBits,
  _In_ const BITMAPINFO *lpBitsInfo,
  _In_       UINT       iUsage,
  _In_       DWORD      dwRop
);

{% endhighlight %}

One of the most important parameter is *lpBitsInfo*. This is a pointer to a [BITMAPINFO](https://msdn.microsoft.com/en-us/library/windows/desktop/dd183375(v=vs.85).aspx) structure that contains infromation about the DIB. What we should do is pass our `&Buffer->Info` to it.

We could write a function which manages how to display:

{% highlight C %}

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


{% endhighlight %}

### Control how to render it
As is shown above, All our DIB information is stored in `Buffer`, so our Rendering function should do something to it.  
You may find that the memory we allocated is just as much as the number of pixels we have to store. So we need a pointer to our memory ,we can change the color by modifying this pointer.  

We define a function called **RenderWeirdGradient**:


{% highlight C %}

internal void
RenderWeirdGradient(win32_offscreen_buffer Buffer, int XOffset, int YOffset)
{
	int Width = Buffer.Width;
	int Height = Buffer.Height;

	uint8 *Row = (uint8 *)Buffer.Memory;

	for (int Y = 0; Y < Buffer.Height; ++Y)
	{
		uint32 *Pixel = (uint32 *)Row;
		for (int X = 0; X < Buffer.Width; ++X)
		{
			uint8 Blue = (X + XOffset);
			uint8 Green = (Y + YOffset);
			//Because its little endian. so the first is blue. not red
			*Pixel++ = ((Green << 8) | Blue);
		}
		Row += Buffer.Pitch;
	}
}

{% endhighlight %}

This function has two loop. The loop scans line by line. That is, it scans the first line by Y=0 and ++X. Then ++Y, scan the second line.

In the inner loop, we just assigned blue and green dots.
Then we assign the value with  

{% highlight C %}
*Pixel++ = ((Green<<8)|Blue);  
{% endhighlight %}

Notice that our color is stored in `4` Bytes. What we have done by using **(Green<<8)\|Blue**, we may assume that we  store the Blue pixel at the lowest memory. Then, the color may be stored as xx RR GG BB, with the highest color undefined. However ,our machine stores by **Little Endian**. Which means the lowest Byte stores at the "highest" place. That is , we store the color by the order **BB GG RR xx**.


The name is a little tricky, the `Row` is just the actual row ,not just the same as Y.
We have calculated how many bytes a row have.And there are extra padding bytes, so after we finished the inner loop, we should update the Row by Row+=Buffer.Pitch(about Image Stride/Pitch, you can refer to [this](https://msdn.microsoft.com/en-us/library/windows/desktop/aa473780(v=vs.85).aspx)).

>The stride is the number of bytes from one row of pixels in memory to the next row of pixels in memory.  
![Padding and Image](https://msdn.microsoft.com/dynimg/IC156482.gif)
 Stride is also called pitch.The padding bytes affect how the image is stored in memory, but do not affect how the image is displayed.


---
By now we have reached the day 005. You may find the **day005.cpp** in history folder, and thank you for your reading.