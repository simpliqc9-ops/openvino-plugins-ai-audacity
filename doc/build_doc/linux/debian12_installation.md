# **Audacity OpenVINO Module Installation Guide (Debian 12)**

This guide details the process of **building and installing Audacity with OpenVINO support** on **Debian 12**.

---

## **📌 Overview**
This guide walks through:
✔️ Installing **dependencies**  
✔️ Setting up **OpenVINO OpenVINO and LibTorch**  
✔️ **Building Audacity (vanilla version)**  
✔️ **Adding the OpenVINO module** and **rebuilding Audacity**  

**📁 Default Working Directory:** `~/audacity-openvino/`  
_All installation files and builds will be placed here._

---

## **🛠 Step 1: Install Required Dependencies**
First, update your system and install necessary packages:
```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y build-essential cmake git python3-pip python3-venv \
  libgtk2.0-dev libasound2-dev libjack-jackd2-dev uuid-dev \
  libglib2.0-dev libpango1.0-dev libfontconfig-dev \
  libfreetype-dev libharfbuzz-dev libjpeg-dev libpng-dev libtiff-dev \
  libxrender-dev libxext-dev libxi-dev libxrandr-dev unzip
```

---

## **🐍 Step 2: Set Up Python Virtual Environment & Install Conan**
Debian 12 enforces **PEP 668**, which prevents direct `pip` installations in the system Python environment. To work around this, use a **virtual environment**.

```bash
mkdir -p ~/audacity-openvino
cd ~/audacity-openvino
python3 -m venv venv
source venv/bin/activate
pip install --upgrade pip
pip install conan
```
✅ **Verify Conan installation**:
```bash
conan --version
```

---

## **🔄 Step 3: Install OpenVINO GenAI**
```bash
cd ~/audacity-openvino
wget https://storage.openvinotoolkit.org/repositories/openvino_genai/packages/2025.3/linux/openvino_genai_ubuntu22_2025.3.0.0_x86_64.tar.gz
tar xvf openvino_genai_ubuntu22_2025.3.0.0_x86_64.tar.gz
cd openvino_genai_*/install_dependencies/
sudo -E ./install_openvino_dependencies.sh
cd ..
source setupvars.sh
```

🔹 **Make OpenVINO available in every terminal session**:
```bash
echo 'source ~/audacity-openvino/openvino_genai_*/setupvars.sh' >> ~/.bashrc
source ~/.bashrc
```

---

## **🔥 Step 4: Install LibTorch**
LibTorch is required for many AI pipelines.

```bash
cd ~/audacity-openvino
wget https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-2.4.1%2Bcpu.zip
unzip libtorch-cxx11-abi-shared-with-deps-2.4.1+cpu.zip
```

🔹 **Set environment variables**:
```bash
echo 'export LIBTORCH_ROOTDIR=~/audacity-openvino/libtorch' >> ~/.bashrc
echo 'export CMAKE_PREFIX_PATH=${LIBTORCH_ROOTDIR}' >> ~/.bashrc
source ~/.bashrc
```

---

## **🎵 Step 5: Build Audacity (Vanilla Version)**
```bash
cd ~/audacity-openvino
git clone https://github.com/audacity/audacity.git
cd audacity
git checkout release-3.7.5
cd ..

mkdir audacity-build
cd audacity-build
cmake -G "Unix Makefiles" ../audacity -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

✅ **Run Audacity to Test**:
```bash
./Release/bin/audacity
```

---

## **🧩 Step 6: Add OpenVINO Module to Audacity**
```bash
cd ~/audacity-openvino
git clone https://github.com/intel/openvino-plugins-ai-audacity.git -b next
cp -r openvino-plugins-ai-audacity/mod-openvino ~/audacity-openvino/audacity/modules/etc/
```

### **Modify `CMakeLists.txt`**
Edit **`~/audacity-openvino/audacity/modules/etc/CMakeLists.txt`**:

```bash
nano ~/audacity-openvino/audacity/modules/etc/CMakeLists.txt
```

Add this **before `audacity_module_subdirectory("${MODULES}")`**:
```cmake
list( APPEND MODULES mod-openvino )
```

💾 **Save & Exit** (`CTRL+X`, `Y`, `ENTER`).

---

## **🔄 Step 7: Rebuild Audacity with OpenVINO**
```bash
cd ~/audacity-openvino/audacity-build
cmake -G "Unix Makefiles" ../audacity -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

✅ **Check that OpenVINO module is built**:
```bash
ls Release/lib/audacity/modules/
```
If `mod-openvino.so` is listed, you're good to go!

✅ **Run Audacity**:
```bash
./Release/bin/audacity
```

✅ **Check existence of OpenVINO Effects**:
In *Effect* tab, you should see **OpenVINO AI Effects**. If so, you're all set!

🎉 **Installation is Complete!** 🚀  
