# Repository Change
You can find the latest version of NRobot [here](https://gitlab.com/sotirisp/NRobot).

# NRobot
A C++ library of algorithms for area coverage by a network of mobile robotic
agents. It includes algorithms for both ground and aerial mobile agents.

## Compilation
NRobot uses CMake for the build process. To compile the library as well as some
testing and example simulations run
```
cmake -H. -Bbuild/release -DCMAKE_BUILD_TYPE=Release
cmake --build build/release -- -j3
```

If you also have `make` installed, you can simply do
```
make release
```

## License
Distributed under the [GNU General Public License version 3](LICENSE.txt).
<br>
Copyright © 2016-2017 Sotiris Papatheodorou
<br>
<br>
NRobot uses [Clipper 6.4.2](http://angusj.com/delphi/clipper.php) and
[SDL 2](https://www.libsdl.org/). Symbolic math is done through
[SymPy](http://www.sympy.org/en/index.html).
