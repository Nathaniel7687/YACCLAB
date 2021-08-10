#!/bin/bash

# Copyright(c) 2020 Federico Bolelli, Stefano Allegretti
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met :
# 
# *Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
# 
# * Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and / or other materials provided with the distribution.
# 
# * Neither the name of YACCLAB nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# exit this script if any commmand fails
set -e

function build_linux(){

   echo -e "\n\n------------------------------------------> YACCLAB configuration" 
   mkdir bin
   # The download of the complete YACCLAB dataset is disable in order to reduce the cmake configure time in travis-ci virtual machine
   cmake -D CMAKE_C_FLAGS=-m64 -D CMAKE_CXX_FLAGS=-m64 -D CMAKE_BUILD_TYPE=Release -D OpenCV_DIR=/home/opencv/opencv-4.4.0/gcc_static_bin_cuda/ -D YACCLAB_ENABLE_CUDA=ON -D YACCLAB_ENABLE_3D=ON -D YACCLAB_DOWNLOAD_DATASET=OFF -D YACCLAB_IS_JENKINS=ON -G Unix\ Makefiles -Bbin -Hbin/.. 

   cd bin
   
   if [ ! -f config.yaml ]; then
      echo "Configuration file (config.yaml) was not properly generated by CMake, pull request failed"
	  exit 1
   fi
   cat config.yaml
   echo -e "------------------------------------------> DONE!"
   
   # Download of a reduced version of the YACCLAB dataset
   echo -e "\n\n------------------------------------------> Download of YACCLAB reduced datasets" 
   wget http://imagelab.ing.unimore.it/files/YACCLAB_dataset3D_reduced.zip -O dataset.zip
   unzip -qq dataset.zip
   rm dataset.zip  
   wget http://imagelab.ing.unimore.it/files/YACCLAB_dataset_reduced.zip -O dataset.zip
   unzip -qq dataset.zip
   rm dataset.zip  
   echo -e "------------------------------------------> DONE!"
   
   
   
   echo -e "\n\n------------------------------------------> Build YACCLAB" 
   make -j7
   #./YACCLAB
   echo -e "------------------------------------------> DONE!"
}

function build_windows(){
    echo "pass"
}

function pass(){
	echo "pass"
}

function run_pull_request(){

    # linux
    echo "Build target: $BUILD_TARGET"
    if [ "$BUILD_TARGET" == "linux" ]; then
        build_linux
    fi

    if [ "$BUILD_TARGET" == "windows" ]; then
        build_windows
    fi
}

# build pull request
#if [ "$TRAVIS_PULL_REQUEST" != "false" ]; then
	run_pull_request
#fi
