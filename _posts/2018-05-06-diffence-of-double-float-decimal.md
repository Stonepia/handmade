---
layout: post
title:  "What's the diffence between double, float"
author: Stonpia
---

As the name implies, a `double` has 2x of `float`. Double has at least as much precision as float.

In general, a double has 15 decimal degits of precision, float has 7.

double has 52 mantissa bits + 1 hidden bit: log(2^53)÷log(10) = 15.95 digits

float has 23 mantissa bits + 1 hidden bit: log(2^24)÷log(10) = 7.22 digits

