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
It's easiest to add the entire `google/` directory from `gens/` to your XCode project, as there are many inter-dependencies between the api modules. However, you only need some of the included `.cc` files to be added to your target. In particular, no `.cc` files in the `gens/google/protobuf` should be added to the target.
 
#### Link the Libraries and Configure Search Paths
Link the following libraries, which should be found in `/usr/local/lib`:

 * `libgrpc++.dylib`  
 * `libprotobuf.dylib`

Make sure `/usr/local/lib` is in the project's Library Search Paths, as well as `/usr/local/include` and `$(PROJECT_DIR)/google` in your Header Search Paths. (These are in the Build Settings tab)

## Add Your Google Cloud Credentials
Using these steps from Google:

1.  **Create a project in the Google Cloud Platform Console**.
    If you haven't already created a project, create one now. Projects enable
    you to manage all Google Cloud Platform resources for your app, including
    deployment, access control, billing, and services.
    1.  Open the [Cloud Platform Console](https://console.cloud.google.com/).
    1.  In the drop-down menu at the top, select Create a project.
    1.  Click Show advanced options. Under App Engine location, select a
        United States location.
    1.  Give your project a name.
    1.  Make a note of the project ID, which might be different from the project
        name. The project ID is used in commands and in configurations.

1.  **Enable billing for your project**.
    If you haven't already enabled billing for your project, [enable billing now](https://console.cloud.google.com/project/_/settings).
    Enabling billing allows the application to consume billable resources such
    as Speech API calls.  See [Cloud Platform Console Help](https://support.google.com/cloud/answer/6288653) for more information about billing settings.

1.  **Enable APIs for your project**.
    [Click here](https://console.cloud.google.com/flows/enableapi?apiid=speech&showconfirmation=true) to visit Cloud Platform Console and enable the Speech API.

1.  **Download service account credentials**.
    These samples use service accounts for authentication.
    1.  Visit the [Cloud Console](http://cloud.google.com/console), and navigate to:
    `API Manager > Credentials > Create credentials > Service account key`
    1.  Under **Service account**, select `New service account`.
    1.  Under **Service account name**, enter a service account name of your choosing.  For example, `transcriber`.
    1.  Under **Role**, select `Project > Service Account Actor`.
    1.  Under **Key type**, leave `JSON` selected.
    1.  Click **Create** to create a new service account, and download the json credentials file.
    1.  Set the `GOOGLE_APPLICATION_CREDENTIALS` environment variable to point to your downloaded service account credentials:
        ```
        export GOOGLE_APPLICATION_CREDENTIALS=/path/to/your/credentials-key.json
        ```
    See the [Cloud Platform Auth Guide](https://cloud.google.com/docs/authentication#developer_workflow) for more information.
    
    
You can set environment variables in Xcode by modifying your target scheme.    