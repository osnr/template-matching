# template-matching

Template matching using Apple Accelerate (vDSP) API in pure C.

(TODO: see [my !!con 2022
talk](https://twitter.com/bangbangcon/status/1591525312168591361)
about this)

Implements 2D normalized cross-correlation (`normxcorr2`) to quickly
find instances of template (patch) image on another image.

## run

```
$ make slow && ./slow
image 1032x515, templ 62x19 -> result 1093x533 in 1.276023 sec
hit at (142, 108)
hits: 1
$ make fast && ./fast
image 1032x515, templ 62x19 -> result 1093x533 in 0.036434 sec
hit at (142, 108)
hits: 1
```

(also check out `result.png` and `hits.png` after each run)

## sources and notes

Based on a melange of stuff:

- [Sabrewarrior/normxcorr2-python](https://github.com/Sabrewarrior/normxcorr2-python):
  implementation of `normxcorr2` using scipy and numpy

- [Matlab normxcorr2 doc](https://www.mathworks.com/help/images/ref/normxcorr2.html)

- OpenCV impl. I had to read this a few times

- corr thing in C

- etc

https://web.archive.org/web/20190301041758/https://www.cs.ubc.ca/research/deaton/remarks_ncc.html

Key things are:

- use multiplication of the FFTs instead of naive convolution (because
the kernel is really big, it's not your 3x3 or 5x5 or whatever)

- use summed-area tables to help with normalization (some of the
  implementations above get lazy and use convolution with a table of
  ones to get sums, and that's slower)

- use the Apple vDSP APIs, don't use for loops if you can avoid it

- precision is important -- floats are not good enough for the running
  sum if you're summing over arrays of floats, you need to go 1 level
  higher-precision (doubles)

## potential performance wins

We're still maybe 50% slower than OpenCV.

- use real FFT instead of complex FFT. I honestly could not figure out
  how to do this -- the packing is so weird -- and it doesn't seem to
  speed it up that much (you can try just stubbing it in and seeing
  how long it takes to run even if you get garbage values)

- get rid of remaining memcpys?

- incrementality? if you have optical flow or otherwise know how an
  image changed

- cache FFT of template in memory. this saves about 1/7 of runtime I
  think, but it complicates the code a lot right now

## random thoughts

- it would be nice to free all the buffers at the end, or have an
  arena allocator or something

- it would be nice to automatically be able to view every stage of the
  image processing

- there are various baseline things that it's good to know the speed
  of -- how long does an FFT pass take? how long does a for loop
  through a whole image take?
