# Audacity OpenVINO module support on Linux

There are a few options for running the OpenVINO module on Linux:

1. [Install the Audacity Snap](#install-the-audacity-snap)
2. [Audacity OpenVINO module build for Linux (Ubuntu 24.04)](#audacity-openvino-module-build-for-linux-ubuntu-2204)

## Install the Audacity Snap

If your Linux distribution supports snaps (check [here](https://snapcraft.io/docs/distro-support)), the [Audacity snap](https://snapcraft.io/audacity) comes with support for the OpenVINO module out-of-the-box. To install the snap:
```
sudo snap install audacity
```

To enable NPU and GPU support, ensure you are in the `render` group and also install the `intel-npu-driver` snap:
```
sudo usermod -a -G render $USER # log out and log back in
sudo snap install intel-npu-driver
```

Note that the AI models are **not** built into the snap. For convenience, there is an interactive command-line tool distributed with the snap that will download and install the models:
```
sudo audacity.fetch-models
```

To simply install all models with a single command:
```
sudo audacity.fetch-models --batch
```

More details about the snap and OpenVINO module can be found [here](https://github.com/snapcrafters/audacity?tab=readme-ov-file#openvino-ai-plugins).

# Audacity OpenVINO module build for Linux (Ubuntu 24.04)

Hi! The following is the process that we use when building the Audacity modules for Linux. These instructions are for Ubuntu 24.04, so you may need to adapt them slightly for other Linux distros.

## High-Level Overview
Before we get into the specifics, at a high-level we will be doing the following:
* Installing build dependencies including OpenVINO GenAI & libtorch.
* Cloning & building Audacity without modifications (just to make sure 'vanilla' build works fine).
* Adding our OpenVINO module src's to the Audacity source tree, and re-building it.

You should open a terminal and run through the following steps. Note that if you close / open a new terminal later on, you may need to set some environment variables again. All of the following commands are assuming a `bash` shell.

## Create a workspace
Create a workspace on your system, cd to it, and set an env. variable so that we can easily cd back to it at certain points during the build steps.
```bash
mkdir audacity_openvino_workspace
cd audacity_openvino_workspace
export audacity_ov_ws=${PWD}
```

## Dependencies
In this section we'll install all dependencies needed to build Audacity & these plugins.

* Install some required apt packages:
```bash
sudo apt-get install -y build-essential cmake git python3-pip python3-venv libgtk2.0-dev libasound2-dev libjack-jackd2-dev uuid-dev
```

* **OpenVINO GenAI**  You can download appropriate version from [here](https://storage.openvinotoolkit.org/repositories/openvino_genai/packages/2025.3/linux/).
  For these instructions, we will use `openvino_genai_ubuntu24_2025.3.0.0_x86_64.tar.gz`.
  
  You can run this to download it from your running terminal:
  ```bash
  wget https://storage.openvinotoolkit.org/repositories/openvino_genai/packages/2025.3/linux/openvino_genai_ubuntu24_2025.3.0.0_x86_64.tar.gz
  ```

  Let's extract & setup OpenVINO for our system / environment.
  ```bash
  # extract it
  tar xvf openvino_genai_ubuntu24_2025.3.0.0_x86_64.tar.gz

  # install system dependencies that OpenVINO needs.
  sudo -E openvino_genai_ubuntu24_2025.3.0.0_x86_64/install_dependencies/install_openvino_dependencies.sh

  # setup env
  source openvino_genai_ubuntu24_2025.3.0.0_x86_64/setupvars.sh
  ```

* **Libtorch (C++ distribution of pytorch)** - This is a dependency for many of the pipelines that we ported from pytorch (musicgen, htdemucs, etc). We are currently using this version: [libtorch-cxx11-abi-shared-with-deps-2.4.1+cpu.zip ](https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-2.4.1%2Bcpu.zip). 

  Download & extract it:
  ```bash
  wget https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-2.4.1%2Bcpu.zip
  unzip libtorch-cxx11-abi-shared-with-deps-2.4.1+cpu.zip
  ```

  Setup environment like this:
  ```bash
  export LIBTORCH_ROOTDIR=${PWD}/libtorch
  ```

## Audacity 

Okay, moving on to actually building Audacity. Just a reminder, we're first going to just build Audacity without any modifications. Once that is done, we'll copy our openvino-module into the Audacity src tree, and built that.

cd back to our workspace directory:
```bash
cd ${audacity_ov_ws}
```

### Audacity vanilla build
```bash
#Create a python venv & install conan
python3 -m venv build_env
source build_env/bin/activate
pip install conan

# clone Audacity
git clone https://github.com/audacity/audacity.git -b release-3.7.5

# Create build directory
mkdir audacity-build
cd audacity-build

# Note, you may want to clear any existing conan 2.0 cache you may have before running cmake:
rm -rf ~/.conan2/

# Run cmake. 
# Note that we need to build audacity with networking, as the model-manager feature uses this module to download the selected models.
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -Daudacity_has_networking=ON ../audacity 

# build it 
make -j`nproc`
```

At this point you can optionally launch Audacity to make sure that everything is working properly before moving on to building it with these plugins.
```bash
Release/bin/audacity
```

### Audacity OpenVINO module build

Now we'll run through the steps to actually build the OpenVINO-based Audacity module.

cd back to our workspace directory:
```bash
cd ${audacity_ov_ws}
```

First, clone the following repo. This is where the actual Audacity module code lives today.
```bash
# clone it
# TODO: Remove 'next' once merged into main.
git clone https://github.com/intel/openvino-plugins-ai-audacity.git -b next
```

We need to copy the `mod-openvino` folder into the Audacity source tree.
i.e. Copy `openvino-plugins-ai-audacity/mod-openvino` folder to ```audacity/modules/etc```:
```bash
cp -R openvino-plugins-ai-audacity/mod-openvino/ audacity/modules/etc/
```

We now need to edit `audacity/modules/CMakeLists.txt` to add mod-openvino as a build target. You just need to append `mod-openvino` to the `MODULES` list.

```cmake
...
# insert this before audacity_module_subdirectory!
list( APPEND MODULES mod-openvino )

audacity_module_subdirectory("${MODULES}")
```

Okay, now we're going to (finally) build the module. Here's a recap of the environment variables that you should have set:

Okay, on to the build:  
```bash
# cd back to the same Audacity folder you used to build Audacity before
cd ${audacity_ov_ws}/audacity-build

# and re-run cmake step (it will go faster this time)
cmake -G "Unix Makefiles" ../audacity -DCMAKE_BUILD_TYPE=Release

# and re-run make command
make -j`nproc`
```

If it all builds correctly, you should see `mod-openvino.so` sitting in `Release/lib/audacity/modules/` folder.

You can go ahead and run `audacity-build/Release/bin/audacity`

# Need Help? :raising_hand_man:
For any questions about this build procedure, feel free to submit an issue [here](https://github.com/intel/openvino-plugins-ai-audacity/issues)
