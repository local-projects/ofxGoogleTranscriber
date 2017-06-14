# ofxGoogleTranscriber

## Install gRPC
Clone the [gRPC repository](https://github.com/grpc/grpc) and follow [these instructions](https://github.com/grpc/grpc/blob/master/INSTALL.md) to install for C++ (build from source).

## Build and Run the Example
You'll want to make sure the provided C++ `helloworld` runs to verify gRPC and protoc were installed correctly. Build it using [these steps](http://www.grpc.io/docs/quickstart/cpp.html) (starting at the Build the Example section). The program may compile then segfault when run. This means gRPC and/or Protocol Buffers were not installed properly. 

You can follow [these steps](https://github.com/grpc/grpc/issues/10946#issuecomment-298669056) to try installing again. 

## Get the gRPC Google APIs
Clone [the repo](https://github.com/googleapis/googleapis.git).
As cloned, the repository contains Protocol Buffers that describe the API, but no C++ code to actually implement them. You can set the language to C++ via `export LANGUAGE=cpp` then in the `googleapis` directory run `make all`. The C++ code will be placed `googleapis/gens/google/`, which can be used in an ofxApp.  

## Adding Them to an Open Frameworks Project
#### Copy the Source
It's easiest to add the entire `google/` directory from `gens/` to your XCode project, as there are many inter-dependencies between the api modules. However, you only need some of the included `.cc` files to be added to your target.
 
#### Link the Libraries and Configure Search Paths
Link the following libraries, which should be found in `/usr/local/lib`:

 * `libgrpc++.dylib`  
 * `libprotobuf.dylib`

Make sure `/usr/local/lib` is in the project's Library Search Paths and `/usr/local/include` in your Header Search Paths. (These are in the Build Settings tab)