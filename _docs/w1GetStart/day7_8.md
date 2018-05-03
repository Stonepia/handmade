---
title: Play a Square Wave
permalink: /docs/day7_8/
---
---
This covers **day007** and **day008**, it uses DirectSound API to add a square wave sound to our game.

## Introduction to DirectSound 
>Note: DirectSound is **deprecated** now. You may use the [XAudio2](https://msdn.microsoft.com/en-us/library/windows/desktop/ee415813(v=vs.85).aspx) instead. But due to the fact that we want to keep pace with the course, we still use the DirectSound. AND may switch to XAudio2 somedays after finishing the audio part.

### How Direct Sound Works
DirectSound works off sound buffers. Microsoft DirectSound buffer objects control the delivery of waveform data from source to a destination.For most buffers, the destination is a mixing engine called the **primary buffer**. From the primary buffer, the data goes to the hardware that converts digital samples to sound waves. And the source might be a synthesizer, another buffer, a WAV file, or a resource.  
We use the **secondary buffer** to hold our audio data.You can have as many secondary sound buffer as you like.

![DirectSoundFig](https://github.com/Stonepia/handmade/blob/gh-pages/img/DocsImg/DirectSoundFig.gif)



##### [DirectSound Playback Objects](https://msdn.microsoft.com/en-us/library/windows/desktop/ee416773(v=vs.85).aspx)
---
|Object|Number|Purpose|Main interfaces|
|---|---|---|---|
|Device|One in each application|Manages the device and creates sound buffers.|IDirectSound8|
|Secondary Buffer|One for each sound|Manages a static or streaming sound and plays it into the primary buffer.|IDirectSoundBuffer8,<br> IDirectSound3DBuffer8, <br>IDirectSoundNotify8|
|Primary buffer|One in each application|Mixes and plays sounds from secondary buffers, and controls global 3D parameters.|IDirectSoundBuffer,<br> IDirectSound3DListener8|
|Effect|Zero or more for each secondary buffer|Transforms the sound in a secondary buffer.|Interface for the particular effect, such as IDirectSoundFXChorus8|
---





Then our step is clear. 
1. Create an object that supports the IDirectSound8 interface.This object usually represents the default playback device.
2. Create a secondary buffer
3. Obtain data to play
4. Put data in the buffer
5. Play the buffer

## Initializing
#### Create the Object
We first create a function called `Win32InitDSound` to initialize our sound buffer.

In this function, first of all ,we have to create a Device object, which represents the rendering device.We call it `DirectSound`.And this object's type is [IDirectSound8](https://msdn.microsoft.com/en-us/library/windows/desktop/ee418035(v=vs.85).aspx)

So we call the function [DirectSoundCreate8](https://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.reference.directsoundcreate8(v=vs.85).aspx)

{% highlight C %}

LPDIRECTSOUND8 DirectSound;

if (SUCCEEDED(DirectSoundCreate8(nullptr, &DirectSound, nullptr)))
{}

{% endhighlight %}

Now, we get the DirectSound Object.

#### Set Cooperative Level
After this, we have to call [SetCooperativeLevel](https://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.idirectsound8.idirectsound8.setcooperativelevel(v=vs.85).aspx). This will bind the Object to a window and determine how the sound device will be shared with other application. This must be set before the buffers are able o be played. 

#### Set the Description of the buffer
If we want to Create a sound buffer, we have to pass it a [DSBUFFERDESC](https://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.reference.dsbufferdesc(v=vs.85).aspx) Structure. This structure descirbes the sound buffer to create.So we set the `BufferDescription`:

{% highlight C %}


DSBUFFERDESC BufferDescription = {};
SecureZeroMemory(&BufferDescription, sizeof(DSBUFFERDESC));
BufferDescription.dwSize = sizeof(BufferDescription);
BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;//This indicates that it's a Primary Buffer

{% endhighlight %}

#### Create the buffer and set the format
After all these things, we are ready to create the Buffer.Just use the [CreateSoundBuffer](https://msdn.microsoft.com/en-us/library/microsoft.directx_sdk.idirectsound8.idirectsound8.createsoundbuffer(v=vs.85).aspx) function.


We then need to Set the format of this.
Use the [SetFormat](https://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.idirectsoundbuffer8.idirectsoundbuffer8.setformat(v=vs.85).aspx) Method. This requires a [WAVEFORMATEX](https://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.reference.waveformatex(v=vs.85).aspx) Structure:

{% highlight C %}


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
                //TODO:Diagnostic
            }
        }
        else
        {
            //TODO: Diagnostic
        }
    }

{% endhighlight %}


Creating the Seoncondary buffer is the same.Notice that we use the secondary buffer as a global variable.

---


## Write sound test

We want to write a square wave to our buffer and play it.

So how to get a square Wave?
#### Initialize Square wave
We would like to describe a sequence of variable to describe our square wave. 

{% highlight C %}


int SamplesPerSecond = 48000;
int ToneHz = 256;
int16 ToneVolume = 6000;
uint32  RunningSampleIndex = 0;
int SquareWavePeriod = SamplesPerSecond / ToneHz;
int HalfSquareWavePeriod = SquareWavePeriod / 2;
int BytesPerSample = sizeof(int16) * 2;
int SecondaryBufferSize = SamplesPerSecond * BytesPerSample;

{% endhighlight %}

We first define the `SamplesPerSecond`, for now ,it is 48kHz. And the `ToneHz` is
a certain frequency that you hear, you can get more information [here](https://music.stackexchange.com/questions/3262/what-are-the-differences-between-tone-note-and-pitch). Then why we need HalfSquareWavePeriod? Because we want the *first* half of a period is 1 and the *second* half is -1.  
Then of course the BytesPerSample should have sizeof(int16) times 2.

Later we will use these things like this:

{% highlight C %}

int16 SampleValue = ((RunningSampleIndex++ / HalfSquareWavePeriod) % 2) ? ToneVolume : -ToneVolume;

{% endhighlight %}

To breifly describe this, is that if it is the first part,then the value is +ToneVolume, else it is -ToneVolume, This gives us a squre wave of our samples.

#### Write to buffer
Unlike a static buffer, our buffer is always read something and add something. 
SO there comes a problem, how can we insure that the buffer we want to write is not conflict with other process?

We would use [Lock](https://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.idirectsoundbuffer8.idirectsoundbuffer8.lock(v=vs.85).aspx) method to ready all or part of the buffer for a data write. 

{% highlight C %}


HRESULT Lock(
         DWORD dwOffset,
         DWORD dwBytes,
         LPVOID * ppvAudioPtr1,
         LPDWORD  pdwAudioBytes1,
         LPVOID * ppvAudioPtr2,
         LPDWORD pdwAudioBytes2,
         DWORD dwFlags
)

{% endhighlight %}

By reading its document, we find that it receives an offset to where our lock begins. We call it `BytesToLock`. And then a `BytesToWrite` indicates the size we want the buffer to lock.Then this function gives us two pointers indicating 2 region that our locked part's address.

Why two parts?
It is because we can have two different situations:
- If we have enought space from our Offset to the end of the buffer. In this situation, The Second pointer is `NULL`.

```
              |< BytesToWrite >|
  Buffer  |-----------------------|   
              ^ (Offset)-------^ 
```
- But there are another situation, that is , there is not enough space to write to the end, so we have to "roll back" some part to the beginning:

```
           |<1> part|        |  <2> part |
    Buffer |-----------------------------|
                             ^(Offset)-----------^(END)
           |~~~~~~~~|                    | ~~~~~~|

                <-------- ROLL BACK ---------|           
```
That's why we need two pointer.

OK then this is our logic: Lock the buffer, get the cursor, and write to it.
So we get the Code:


{% highlight C %}

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

{% endhighlight %}

Notice that we have to [Unlock](https://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.idirectsoundbuffer8.idirectsoundbuffer8.unlock(v=vs.85).aspx) to release the buffer!  

#### Play it!

{% highlight C %}

if (!SoundIsPlaying)
{
    GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
    SoundIsPlaying = true;
}

{% endhighlight %}



---
By now we have reached the day 008. You may find the **day008.cpp** in history folder, and thank you for your reading.