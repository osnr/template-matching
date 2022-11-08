# template-matching

Template matching using Apple Accelerate (vDSP) API in pure C.

Implements 2D normalized cross-correlation (`normxcorr2`) to quickly
find instances of template (patch) image on another image.

Based on a melange of stuff:

- Python

- Matlab

- etc

## potential performance wins

- cache FFT of template in memory. this saves about 1/7 of runtime I
  think, but it complicates the code a lot

- use real FFT instead of complex FFT. I honestly could not figure out
  how to do this and it doesn't seem to speed it up that much (you can
  try just stubbing it in and seeing how long it takes to run even if
  you get garbage values)

## random thoughts

- it would be nice to free all the buffers at the end, or have an
  arena allocator or something

- it would be nice to automatically be able to view every stage of the
  image processing

