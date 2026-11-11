
<p align="center">
  <img src="docs/images/kubechaser.gif" alt="kubechaser" />
</p>

[**KubeChaser**](https://kubechaser.dev) is a 3D renderer for visualizing Kubernetes clusters.

⚠️ **Note**  
> This project is under active development.

## Install
You will need `cmake`  and to install run 
```
git clone --recurse-submodules https://github.com/kabicin/kubechaser.git; cd kubechaser; ./scripts/build_and_run.sh
```

## Dependencies
- Eigen
- GLFW
- GLAD
- STB
- GLM
- Dear ImGUI

## Development

Build the desktop application and tests:

```sh
cmake -S . -B build
cmake --build build --parallel
```

Run the tests:

```sh
./scripts/test.sh
```

This uses an incremental core-only build. To build the desktop application as
well before running the tests, use `./scripts/test.sh --full`.

The platform-independent core and its tests can be built without OpenGL or the
desktop application:

```sh
cmake -S . -B build-core -DKUBECHASER_BUILD_APP=OFF
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
```
