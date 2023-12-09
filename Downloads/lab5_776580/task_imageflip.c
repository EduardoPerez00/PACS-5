////////////////////////////////////////////////////////////////////
//File: task_imageflip.c
//
//Description: base file for environment exercises with openCL
//
// 
////////////////////////////////////////////////////////////////////

#define cimg_use_jpeg
#include "CImg/CImg.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#ifdef __APPLE__
  #include <OpenCL/opencl.h>
#else
  #include <CL/cl.h>
#endif
 

 
// check error, in such a case, it exits
void cl_error(cl_int code, const char *string){
    if (code != CL_SUCCESS){
        printf("%d - %s\n", code, string);
        exit(-1);
    }
}

const bool SHOW_IMGS = true;

////////////////////////////////////////////////////////////////////////////////

using namespace cimg_library;

int main(int argc, char** argv)
{
    auto start = std::chrono::steady_clock::now();

    int err;                             // error code returned from api calls
    size_t t_buf = 50;          // size of str_buffer
    char str_buffer[t_buf];     // auxiliary buffer    
    size_t e_buf;               // effective size of str_buffer in use
       
    size_t global_size;                       // global domain size for our calculation
    size_t local_size;                        // local domain size for our calculation

    const cl_uint num_platforms_ids = 10;               // max of allocatable platforms
    cl_platform_id platforms_ids[num_platforms_ids];        // array of platforms
    cl_uint n_platforms;                        // effective number of platforms in use
    const cl_uint num_devices_ids = 10;               // max of allocatable devices
    cl_device_id devices_ids[num_platforms_ids][num_devices_ids];   // array of devices
    cl_uint n_devices[num_platforms_ids];               // effective number of devices in use for each platform
   
    cl_device_id device_id;                     // compute device id
    cl_context context;                         // compute context
    cl_command_queue command_queue;             // compute command queue

    // 1. Scan the available platforms:
    err = clGetPlatformIDs (num_platforms_ids, platforms_ids, &n_platforms);
    cl_error(err, "Error: Failed to Scan for Platforms IDs");
    printf("Number of available platforms: %d\n\n", n_platforms);

    for (int i = 0; i < n_platforms; i++ ){
        err= clGetPlatformInfo(platforms_ids[i], CL_PLATFORM_NAME, sizeof(str_buffer), &str_buffer, NULL);
        cl_error (err, "Error: Failed to get info of the platform\n");
        printf( "\t[%d]-Platform Name: %s\n", i, str_buffer);
    }
    printf("\n");
    // ***Task***: print on the screen the name, host_timer_resolution, vendor, versionm, ...

    // 2. Scan for devices in each platform
    for (int i = 0; i < n_platforms; i++ ){
        err = clGetDeviceIDs(platforms_ids[i], CL_DEVICE_TYPE_ALL, num_devices_ids, devices_ids[i], &(n_devices[i]));
        cl_error(err, "Error: Failed to Scan for Devices IDs");
        printf("\t[%d]-Platform. Number of available devices: %d\n", i, n_devices[i]);

        for(int j = 0; j < n_devices[i]; j++){
            err = clGetDeviceInfo(devices_ids[i][j], CL_DEVICE_NAME, sizeof(str_buffer), &str_buffer, NULL);
            cl_error(err, "clGetDeviceInfo: Getting device name");
            printf("\t\t [%d]-Platform [%d]-Device CL_DEVICE_NAME: %s\n", i, j,str_buffer);

            cl_uint max_compute_units_available;
            err = clGetDeviceInfo(devices_ids[i][j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(max_compute_units_available), &max_compute_units_available, NULL);
            cl_error(err, "clGetDeviceInfo: Getting device max compute units available");
            printf("\t\t [%d]-Platform [%d]-Device CL_DEVICE_MAX_COMPUTE_UNITS: %d\n\n", i, j, max_compute_units_available);
        }
    }    
    // ***Task***: print on the screen the cache size, global mem size, local memsize, max work group size, profiling timer resolution and ... of each device

    //CLINFO? NO FUNCIONA EN EL PC
                                         
    // 3. Create a context, with a device
    cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platforms_ids[0], 0};
    context = clCreateContext(properties, n_devices[0], devices_ids[0], NULL, NULL, &err);
    cl_error(err, "Failed to create a compute context\n");

    // 4. Create a command queue
    cl_command_queue_properties proprt[] = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
    command_queue = clCreateCommandQueueWithProperties(context, devices_ids[0][0], proprt, &err);
    cl_error(err, "Failed to create a command queue\n");

    // ! First kernel on the device
    // Calculate size of the file
    FILE *fileHandler = fopen("verticalMirror.cl", "r");
    fseek(fileHandler, 0, SEEK_END);
    size_t fileSize = ftell(fileHandler);
    rewind(fileHandler);

    // read kernel source into buffer
    char * sourceCode = (char*) malloc(fileSize + 1);
    sourceCode[fileSize] = '\0';
    fread(sourceCode, sizeof(char), fileSize, fileHandler);
    fclose(fileHandler);

    // create program from buffer
    cl_program program = clCreateProgramWithSource(context, 1, (const char**)&sourceCode, &fileSize, &err);
    cl_error(err, "Failed to create program with source\n");
    free(sourceCode);

    // Build the executable and check errors
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS){
        size_t len;
        char buffer[2048];
       
        printf("Error: Some error at building process.\n");
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 2048, buffer, NULL); // ? len = 2048 ?
        printf("%s\n", buffer);
        exit(-1);
    }

    // Create a compute kernel with the program we want to run
    cl_kernel kernel = clCreateKernel(program, "verticalMirror", &err);
    cl_error(err, "Failed to create kernel from the program\n");

    // TODO: Recover the image, transform it into a C vector, and declare the outputs

    CImg<unsigned char> img("elephant.jpg");  // Load image file "image.jpg" at object img
    int height = img.height();
    int width = img.width();
    if(SHOW_IMGS) img.display("Original image");

    const unsigned int numElems = height * width * 3; // Number of elements in the input image
    const unsigned int outElems = 256 * 3; // Number of values (256, char) per channel (3, RGB)
    //unsigned int in[numElems];
    unsigned int out[outElems];
    unsigned char * in = (unsigned char*) malloc(numElems);

    printf("Width: %d, height: %d, numElems: %d\n", width, height, numElems);

    for (int i = 0; i < width; i++){
        for (int j = 0; j < height; j++){
            for (int channel = 0; channel < 3; channel++){
                in[i*height*3 + j*3 + channel] = img(i, j, 0, channel);
            }
        }
    }

    // Create OpenCL buffer visible to the OpenCl runtime for exercise 6 we have to put more devices and messagings errors
    cl_mem in_device_object  = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(unsigned char) * numElems, NULL, &err);
    cl_error(err, "Failed to create memory buffer for input image\n");
    cl_mem out_device_object = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int)*outElems, NULL, &err);
    cl_error(err, "Failed to create memory buffer for output image\n");

    // Write date into the memory object , we need one more due parallelism
    err = clEnqueueWriteBuffer(command_queue, in_device_object, CL_TRUE, 0, sizeof(unsigned char) * numElems, in, 0, NULL, NULL);
    cl_error(err, "Failed to enqueue a write command\n");

    // Set the arguments to the kernel
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &in_device_object);
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &out_device_object);
    err = clSetKernelArg(kernel, 2, sizeof(width), &width);
    err = clSetKernelArg(kernel, 3, sizeof(height), &height);

    // Launch Kernel
    global_size = numElems; // ! Si, por ejemplo, hay 128 work items, y cada work group tiene 32 elementos, habrá 4 work groups
    //local_size = 32;
    cl_event event; // Event for profiling the kernel execution time
    err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_size, NULL, 0, NULL, &event);
    cl_error(err, "Failed to launch kernel to the device\n");

    // Read data from device memory back to host memory
    err = clEnqueueReadBuffer(command_queue, out_device_object, CL_TRUE, 0, sizeof(int) * outElems, out, 0, NULL, NULL);

    // ####### KERNEL EXECUTION TIME ########
    clWaitForEvents(1, &event); // Wait for the kernel to finish
    clFinish(command_queue); // Wait for all enqueued tasks to finish
    cl_ulong time_start, time_end;

    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);

    double nanoSeconds = time_end - time_start;
    printf("OpenCL Execution time is: %0.3f milliseconds\n", nanoSeconds / 1000000.0);

    // ####### Print the result image #######
    CImg<unsigned char> result(width, height, 1, 3, 0);
    for (int i = 0; i < width; i++){
        for (int j = 0; j < height; j++){
            for (int channel = 0; channel < 3; channel++){
                result(i, j, 0, channel) = (unsigned char)out[i*height*3 + j*3 + channel];
            }
        }
    }

    if(SHOW_IMGS) result.display("Transformed Image");

    clReleaseMemObject(out_device_object);
    clReleaseMemObject(in_device_object);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);

    auto stop = std::chrono::steady_clock::now();
    printf("Execution time: %ld ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count());

    return 0;
}




