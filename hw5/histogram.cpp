#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CL/opencl.h>
#include <fstream>
#include <iostream>
#include <vector>

cl_program load_program(cl_context context, const char* filename)
{
	std::ifstream in(filename, std::ios_base::binary);
	if(!in.good()) {
	return 0;
	}

	// get file length
	in.seekg(0, std::ios_base::end);
	size_t length = in.tellg();
	in.seekg(0, std::ios_base::beg);

	// read program source
	std::vector<char> data(length + 1);
	in.read(&data[0], length);
	data[length] = 0;

	// create and build program 
	const char* source = &data[0];
	cl_program program = clCreateProgramWithSource(context, 1, &source, 0, 0);
	if(program == 0) {
	return 0;
	}

	if(clBuildProgram(program, 0, 0, 0, 0, 0) != CL_SUCCESS) {
	return 0;
	}

	return program;
}

int main(int argc, char const *argv[])
{
	cl_int err;

	cl_uint num_device, num;
	cl_device_id device;
	cl_platform_id platform_id;
	err = clGetPlatformIDs(1, &platform_id, &num);
	err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device,  &num_device);
	cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);

	FILE *inFile = fopen("input", "r");
	FILE *outFile = fopen("0656120.out", "w");

	unsigned int i = 0, a, input_size;

	fscanf(inFile, "%u", &input_size);

	cl_mem img = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(unsigned int) * input_size, NULL, &err);
	cl_mem his_res = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(unsigned int) * 256 * 3, NULL, &err);
	unsigned int *tmp = (unsigned int *) malloc(sizeof(unsigned int) * input_size);
	unsigned int *result = (unsigned int *) malloc(sizeof(unsigned int) * 256 * 3);

	memset(result, 0x00, sizeof(unsigned int) * 256 * 3);
	clEnqueueWriteBuffer(queue, his_res, CL_TRUE, 0, sizeof(unsigned int) * 256 * 3, result, 0, NULL, NULL);
	
    while( fscanf(inFile, "%u", &a) != EOF ) {
		tmp[i++] = a;
	}
	err = clEnqueueWriteBuffer(queue, img, CL_TRUE, 0, sizeof(unsigned int) * input_size, tmp, 0, NULL, NULL);

	//size_t source_size = strlen(histogram);
	unsigned int input_count = input_size/3;
    cl_program program = load_program(context, "histogram.cl");
	//cl_program myprog = clCreateProgramWithSource(context, 1, (const char **) &histogram, &source_size, &err);
	err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
	cl_kernel mykernel = clCreateKernel(program, "histogram", &err);
	clSetKernelArg(mykernel, 0, sizeof(cl_mem), &img);
	clSetKernelArg(mykernel, 1, sizeof(unsigned int), &input_size);
	clSetKernelArg(mykernel, 2, sizeof(cl_mem), &his_res);

	size_t worksize = 768;
	err= clEnqueueNDRangeKernel(queue, mykernel, 1, 0, &worksize, 0, 0, NULL, NULL);

	err = clEnqueueReadBuffer(queue, his_res, CL_TRUE, 0, sizeof(unsigned int) * 256 * 3, result, NULL, NULL, NULL);
	for(i = 0; i < 256 * 3; ++i) {
		if (i % 256 == 0 && i != 0)
			fprintf(outFile, "\n");
		fprintf(outFile, "%u ", result[i]);
    }
	fclose(inFile);
	fclose(outFile);

	return 0;
}