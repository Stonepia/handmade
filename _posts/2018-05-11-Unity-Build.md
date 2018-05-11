---
layout: post
title:  "Unity Build"
author: Stonpia
---

In **day11** , We seperate the one win32_handmade.cpp into multiple files. And Casey just cut the `screen_buffer` and the `sound buffer` definition into handmade.h. 

Then there comes a question: We can see in `handmade.h` of such things :

``` C++
//****handmade.h****
struct game_sound_output_buffer
{
    int SamplesPerSecond;
    int SampleCount;
    int16 *Samples;
};

```

So where is the definition of type `int16` ? We know it is in our win32_handmade.cpp. But the logic is we include handmade.h in win32_handmade.cpp . Not include the win32_handmade.cpp in handmade.h.

Amazingly, using **cl** to compile the win32_handmade.cpp works pretty fine. Not a single error at all! But if you add the .h and .cpp into a project, you will get error.

Well, one thing is definitely true: Everything we have are included in win32_handmade.cpp. So compling it alone should work. Try this by selecting the .h and handmade.cpp and **Exclude from Project**. Bingo!, it works! And that's exactly what we did using cl . What's more, if you use cl to complie handmade.cpp, you will get the same error as we get in the VS.

So what we got here is something we called **"Unity Build"**. You can refer to [this](https://stackoverflow.com/questions/543697/include-all-cpp-files-into-a-single-compilation-unit) for more detail.

The main concept of Unity Build is to reduce the number of files we compile. There are many times that we will include the same .h file multiple times. But can we just complie it once by generating a unity translation unit that simply include all other translation units?

The answer is definitely yes, and that's why we add 

``` C++
#include <handmade.cpp>
```

in our `win32_handmade.cpp`. By doing so , everything, is complied only once. Because we just have one file to compile!
This can significantly helps us to accelarate the time of compliling.

Unity Builds improved build speeds for 3 main reasons:

1. All shared header files only need to be parsed once. Although precompiled files can help to reduce the cost of redundant parsing, there are still a lot of header files which are not precompiled.
2. The compiler is invoked fewer times. There are some startup cost with invoking the compiler.
3.  Finally, The redution in redundant header parsing means a reduction in redundant code-gen for inlined functions, so the total size of object files is smaller, which makes linking faster.

However, unity builds are NOT faster because of reduced disk I/O.Because most time we have enough memory, so the cache will avoid redundant disk I/O. If you don't have enough memory, the unity builds could even make building times worse by causing the compiler's memory footprint to exceed available memory and get paged out!

(This comes from the [answer](https://stackoverflow.com/questions/543697/include-all-cpp-files-into-a-single-compilation-unit) in StackOverFlow, Thanks to Bruce Dawson).


<br>

#### Conclusion

OK, conclusion first : Don't use it.

We do not use Unity Build for our daily developer worker, as minor changes quite frequently cause a rebuild of entire unity. Instead, Unity Builds are used as a compilation method of choice for continuous integration servers.

Also, having too many compilation units collected in a single unit build file can lead to compilation problems, such as out of memory. So you may better to split the unities. This can increase the complexity.

The Unity Build also may lead 'multiple defined symbol' errors. 

So the conclusion is , don't use it . You can find the article [here](https://cheind.wordpress.com/2009/12/10/reducing-compilation-time-unity-builds/).

<br>

#### So why you write this thing!

Although I don't recommend Unity Build. But compilation time is a critical thing for our programming.

**Parrallel Compilation** is what I highly recommended. You can find a detailed article [here](https://randomascii.wordpress.com/2014/03/22/make-vc-compiles-fast-through-parallel-compilation/). 

You can activate it by simply **Property->C/C++->General->Multi-processor Compilation** .Set it to **Yes**. Done!

If you want to get more information, just go to that article. Bruce illustrate it really well.