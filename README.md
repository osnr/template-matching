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

Based on a mix of stuff (Googling `normxcorr2` is a good place to
start, that seems to be a nice well-defined shorthand name that means
the same function when people use it):

- [Fast Normalized
  Cross-Correlation](http://scribblethink.org/Work/nvisionInterface/nip.html)
  Lewis 1995: this is really all you technically need, but the other
  sources are helpful to reinforce understanding and explain stuff in
  different ways

- [Template Matching using Fast Normalized Cross
  Correlation](https://isas.iar.kit.edu/pdf/SPIE01_BriechleHanebeck_CrossCorr.pdf)
  Briechle and Hanebeck 2001: almost nobody seems to actually use the
  technique in this paper but they cite it for its summary of Lewis
  1995, as far as I can tell

- [Sabrewarrior/normxcorr2-python](https://github.com/Sabrewarrior/normxcorr2-python):
  implementation of `normxcorr2` using scipy and numpy

- [Matlab normxcorr2
  doc](https://www.mathworks.com/help/images/ref/normxcorr2.html)

- [Octave implementation](https://sourceforge.net/p/octave/image/ci/e9c18bff13be86a0d067969c2a3dcfc405edb0b2/tree/inst/normxcorr2.m)

- https://github.com/scikit-image/scikit-image/blob/main/skimage/feature/template.py

- https://web.psi.edu/spc_wiki/imXcorr.py

- [OpenCV implementation of
  matchTemplate.](https://answers.opencv.org/question/83870/why-is-opencvs-template-matching-method-tm_sqdiff-so-fast/)
  I found this pretty confusing and didn't understand it until reading
  a lot of other sources; it's not straight-line code since it has a
  lot of different modes and settings, and you get confused about what
  part actually does the convolution vs. normalization if you don't
  understand the algorithm well.

- [Daniel Eaton remarks](https://web.archive.org/web/20190301041758/https://www.cs.ubc.ca/research/deaton/remarks_ncc.html) corr thing in C

- [kiranpradeep/vDSPxcorr2d](https://github.com/kiranpradeep/vDSPxcorr2D)
  kind of messy vDSP-based non-normalized 2D cross-correlation (but
  they do use the real FFT and pack/unpack it properly)

[I literally implemented it in terms of basic numpy operations and FFT
first so I could understand it and compare the code to known-good
code, then went and translated each line to the 2-5 equivalent lines
of C.](https://github.com/osnr/template-matching-play)

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
  how to do this -- the
  [packing](https://stackoverflow.com/questions/28851910/packing-real-to-complex-fft-2d-using-vdsp)
  is [so](https://stackoverflow.com/questions/13889163/choosing-real-vs-complex-2d-ffts-using-apple-accelerate-framework) weird -- and it doesn't seem to speed it up that much (you can
  try just stubbing it in and seeing how long it takes to run even if
  you get garbage values)

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
