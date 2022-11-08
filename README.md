# template-matching

Template matching using Apple Accelerate (vDSP) API in pure C.

Implements 2D normalized cross-correlation (`normxcorr2`) to quickly
find instances of template (patch) image on another image.

Based on a melange of stuff:

- [Sabrewarrior/normxcorr2-python](https://github.com/Sabrewarrior/normxcorr2-python):
  implementation of `normxcorr2` using scipy and numpy

- [Matlab normxcorr2 doc](https://www.mathworks.com/help/images/ref/normxcorr2.html)

- OpenCV impl

- 

- etc

https://web.archive.org/web/20190301041758/https://www.cs.ubc.ca/research/deaton/remarks_ncc.html

Key things are:

- use multiplication of the FFTs instead of naive convolution (because the kernel is really
big, it's not your 3x3 or 5x5 or whatever)

- use summed-area tables to help with normalization (some of the
  implementations above get lazy and use convolution with a table of
  1s to get sums, and that's slower)

- use the Apple vDSP APIs, don't memcpy stuff by hand

- precision is impotant. floats are not good enough for the running
  sum

## potential performance wins

We're still maybe 50% slower than OpenCV.

- use real FFT instead of complex FFT. I honestly could not figure out
  how to do this and it doesn't seem to speed it up that much (you can
  try just stubbing it in and seeing how long it takes to run even if
  you get garbage values)

- get rid of remaining memcpys?

- incrementality? if you have optical flow or otherwise know how an
  image changed

- cache FFT of template in memory. this saves about 1/7 of runtime I
  think, but it complicates the code a lot

## random thoughts

- it would be nice to free all the buffers at the end, or have an
  arena allocator or something

- it would be nice to automatically be able to view every stage of the
  image processing

- there are various baseline things that it's good to know the speed
  of -- how long does an FFT pass take? how long does a for loop
  through a whole image take?
