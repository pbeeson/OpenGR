# User documentation {#mainpage}

## About OpenGR

OpenGR is a set C++ libraries for 3D Global Registration, standalone applications and plugins released under the terms of the *APACHE V2 licence*, which makes it free for commercial and research use.

It provides state of the art global registration techniques for 3d pointclouds. Still under active development, the library is currently improved by adding wrappers to existing software, and by stabilizing the API.

In a nutshell, it provides:
* two core packages: [algorithms](@ref gr::algorithms) and [accelerators](@ref gr::accelerators),
* a convenience packages: [utils](@ref gr::utils),
* several [demonstrators](@ref Super4PCS-Demos):
   * a [command line application](@ref Super4PCS-App),
   * wrappers to use the algorithms with the [Point Cloud Library](@ref Super4PCS-PCL) and [CGAL](@ref Super4PCS-CGAL) (coming soon)
   * a [meshlab plugin](@ref Super4PCS-Meshlab)
* a [test suite](@ref GlobalRegistration::Testing)

### Usage and documentation

 - @subpage gettingstarted : How to use the library in 180 seconds
 - @subpage demos
   - @subpage fileformats : Supported file formats
   - @subpage usage : Hints for usage and parameter tuning
 - Developping with the library:
   - @subpage compilation : Compilation instructions and supported platform(s...). Also describes how to link with the library using cmake.
   - @subpage extend : Describes how to add new registration algorithms to the library.
   - @subpage using-your-point-type : Describes how to use your point type as input to the library.
   - @subpage tests : Monitor continuous integration, run tests on your platform
 - @subpage currentrelease : Changelog and status of the current release
 - @subpage roadmap
 - @subpage datasets

## Compilation status
[stsimg-linux]: https://api.travis-ci.org/STORM-IRIT/OpenGR.svg?branch=master
[stsimg-windw]: https://ci.appveyor.com/api/projects/status/wpilmlfk8obuod8b/branch/master?svg=true



[Travis]: https://travis-ci.org/STORM-IRIT/OpenGR "Travis"
[AppVeyor]: https://ci.appveyor.com/project/nmellado/opengr/ "AppVeyor"

| Linux  \& MacOS | Windows         |
| :----:          | :-----:         |
| ![stsimg-linux] | ![stsimg-windw] |
| [Travis]        | [AppVeyor]      |


## Credits
* Nicolas Mellado: main developper, conception, implementation and examples

This library is based on [Super4PCS library](https://github.com/nmellado/Super4PCS), which was also developped by:
* Dror Aiger: 4pcs implementation
* Niloy Mitra: 4pcs implementation

### Contributors and support
(With pull request ids if any)
* [Lorenz Bung](https://github.com/LorenzBung) Improve PCL wrapper (#56, #65)
* [Alessandro Muntoni](https://github.com/alemuntoni) Fix compilation on windows (#52)
* [Simon Giraudot](https://github.com/sgiraudot) Code cleaning (#46)
* [Necip Fazil Yildiran](https://github.com/necipfazil) Range and point type abstraction layer (#31)
* [Sebastien Loriot](https://github.com/sloriot) Fix cmake package (#15)
* [Sandra Alfaro](https://github.com/SandraAlfaro) Implement congruent point algorithms using functors (#13)
* [Simone Gasparini](https://github.com/simogasp) Pull-Request reviews
* [Kastan Day](https://github.com/kastanday) Added getFitnessScore() to PCL wrapper (#10)
* [xinkang](https://github.com/xinkang) Fix include path of timer.h

We also wish to thanks Simon Giraudot and https://geometryfactory.com/ for their support and help through our participation to
 - the 2019 Google Summer of Code: https://summerofcode.withgoogle.com/projects/#6321265885839360,
 - the 2020 Google Summer of Code: https://summerofcode.withgoogle.com/projects/#5673656849530880


### Citation
```
﻿@misc {OpenGR,
    author = {Mellado, Nicolas.},
    title = {OpenGR: a 3D Global Registration Library},
    howpublished = {https://github.com/STORM-IRIT/OpenGR},
    year = {2018},
}
```
