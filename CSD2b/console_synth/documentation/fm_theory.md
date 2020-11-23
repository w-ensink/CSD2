# FM Syntheses

normale sine wave functie

$$y(t) = A * sin(2\pi f t)$$

---

|param|meaning|
|---|---|
|A|amplitude|
|f|frequency in hz|
|t|time|

---
This only works well with a fixed frequency and amplitude. To deal with varying frequency we need to integrate the frequency function ```f``` to determine the accumulated phase at time ```t```.


$$y(t) = A(t) sin(\int_0^t 2\pi f(x) \mathrm{d}x)$$

Integrating the frequency finds the area under the frequency function over the time. This is basically f * t for a changing f

frequency modulation uses a rapidly changing function
$$f(t) = C + D sin(2\pi Mt)$$
- C is the carrier frequency in hz (fundamental)
- D is the depth of modulation
- M is the modulator frequency in hz
- t is the time

If we combine this one with the previous one, we get 
$$f(t)=A sin(2\pi Ct + D sin (2\pi Mt))$$