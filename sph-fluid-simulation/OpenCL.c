#include "OpenCL.h"
#include <stdio.h>
#include <stdlib.h>

cl_command_queue queue;
cl_kernel kernelCalc;
cl_kernel kernelMove;
cl_mem pos_buf;

void CLInit(particle* particles, int arr_len, float G, float smthing) {

	int n = arr_len / 5;
	//int block_size = BLOCK_SIZE;

	char* sourceName = "Kernel.cl";
	char* shader = RdFstr(sourceName);

	//printf("%s\n", shader);

	cl_uint num_platforms;
	cl_int err;
	err = clGetPlatformIDs(0, NULL, &num_platforms);
	CheckErr(err, "Error getting platform ID count");
	cl_platform_id* platforms = (cl_platform_id*)malloc(num_platforms * sizeof(cl_platform_id));
	err = clGetPlatformIDs(num_platforms, platforms, NULL);
	CheckErr(err, "Error getting platform IDs");

	cl_uint num_devices;
	clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
	cl_device_id* devices = (cl_device_id*)malloc(num_devices * sizeof(cl_device_id));
	clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);
	CheckErr(err, "Error getting device IDs");

	cl_context context = clCreateContext(NULL, num_devices, devices, NULL, NULL, &err);
	CheckErr(err, "Error creating context");

	queue = clCreateCommandQueue(context, devices[0], 0, &err);
	CheckErr(err, "Error creating command queue");

	pos_buf = clCreateBuffer(context, CL_MEM_READ_WRITE, arr_len * sizeof(cl_float), NULL, &err);
	CheckErr(err, "Error creating buffer");

	cl_program program = clCreateProgramWithSource(context, 1, &shader, NULL, &err);
	CheckErr(err, "Error creating program with source");
	free(shader);

	char build_args[1024];
	if (snprintf(build_args, sizeof(build_args), KERNEL_BUILD_ARGS, n) < 0) {
		printf("Error build arguments too long: %s\n", build_args);
		exit(1);
	}
	err = clBuildProgram(program, 1, &devices[0], build_args, NULL, NULL);
	if (err != CL_SUCCESS) {
		size_t log_size;
		clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		char* log = malloc(log_size);
		clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
		printf("Build errors:\n%s\n", log);
		free(log);
		printf("Error building program: %d\n", err);
		exit(1);
	}

	kernelCalc = clCreateKernel(program, "Calc", &err);
	CheckErr(err, "Error creating kernel");
	kernelMove = clCreateKernel(program, "Move", &err);
	CheckErr(err, "Error creating kernel");

	PrintWorkGroupSizes(devices[0], kernelCalc);
	PrintWorkGroupSizes(devices[0], kernelMove);


	err = clSetKernelArg(kernelCalc, 0, sizeof(cl_mem), &pos_buf);
	CheckArgErr(kernelCalc, 0, err);

	err = clSetKernelArg(kernelMove, 0, sizeof(cl_mem), &pos_buf);
	CheckArgErr(kernelMove, 0, err);

	clEnqueueWriteBuffer(queue, pos_buf, CL_FALSE, 0, n * 2 * sizeof(cl_float), particles->pos, 0, NULL, NULL);
	clEnqueueWriteBuffer(queue, pos_buf, CL_FALSE, n * 2 * sizeof(cl_float), n * 2 * sizeof(cl_float), particles->vel, 0, NULL, NULL);
	clEnqueueWriteBuffer(queue, pos_buf, CL_FALSE, n * 2 * sizeof(cl_float) * 2, n * sizeof(cl_float), particles->ran, 0, NULL, NULL);

	clReleaseProgram(program);
	clReleaseContext(context);

	printf("CL init done!\n");
}

void CLRun(particle* particles, int arr_len, int round_size) {
	int n = arr_len / 5;

	size_t global_size = n;
	size_t local_size = 64;
	local_size = local_size <= round_size ? local_size : NULL;

	cl_int err;
	err = clEnqueueNDRangeKernel(queue, kernelCalc, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
	//err = clEnqueueNDRangeKernel(queue, kernelCalc, 1, NULL, &global_size, NULL, 0, NULL, NULL);
	CheckErr(err, "Error executing kernel calc");

	size_t global_size2 = n;
	err = clEnqueueNDRangeKernel(queue, kernelMove, 1, NULL, &global_size2, NULL, 0, NULL, NULL);
	CheckErr(err, "Error executing kernel move");

	err = clEnqueueReadBuffer(queue, pos_buf, CL_FALSE, 0, n * 2 * sizeof(cl_float), particles->pos, 0, NULL, NULL);
	CheckErr(err, "Error reading buffer1");
	err = clEnqueueReadBuffer(queue, pos_buf, CL_FALSE, n * 2 * sizeof(cl_float), n * 2 * sizeof(cl_float), particles->vel, 0, NULL, NULL);
	CheckErr(err, "Error reading buffer2");
	err = clEnqueueReadBuffer(queue, pos_buf, CL_FALSE, n * 2 * sizeof(cl_float) * 2, n * sizeof(cl_float), particles->ran, 0, NULL, NULL);
	CheckErr(err, "Error reading buffer3");

	err = clFinish(queue);
	CheckErr(err, "Error finishing queue");
}


char* RdFstr(char* filename) {
	FILE* fp;
	errno_t error = fopen_s(&fp, filename, "r");
	if (error != 0) {
		perror("Error opening file");
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	size_t file_size = ftell(fp);
	rewind(fp);

	char* string = malloc(file_size + 1);
	if (string == NULL) {
		perror("Error allocating memory for string");
		return -1;
	}

	size_t offset = 0;
	while (fgets(string + offset, file_size - offset, fp) != NULL) {
		offset = strlen(string);
	}

	fclose(fp);
	string[file_size] = '\0';

	return string;
}

void CheckErr(cl_int err, char* msg) {
	if (err != CL_SUCCESS) {
		printf("%s: %d\n", msg, err);
		exit(1);
	}
}

void CheckArgErr(cl_kernel kernel, int arg_indx, cl_int err) {
	if (err != CL_SUCCESS) {
		printf("Error setting argument %d: %d\n", arg_indx, err);

		size_t log_size;
		clGetKernelArgInfo(kernel, arg_indx, CL_KERNEL_ARG_TYPE_NAME, 0, NULL, &log_size);
		char* log_t_name = malloc(log_size);
		clGetKernelArgInfo(kernel, arg_indx, CL_KERNEL_ARG_TYPE_NAME, log_size, log_t_name, NULL);

		clGetKernelArgInfo(kernel, arg_indx, CL_KERNEL_ARG_NAME, 0, NULL, &log_size);
		char* log_name = malloc(log_size);
		clGetKernelArgInfo(kernel, arg_indx, CL_KERNEL_ARG_NAME, log_size, log_name, NULL);

		printf("Argument info:\n%s %s\n", log_t_name, log_name);
		free(log_t_name);
		free(log_name);

		exit(1);
	}
}

void PrintWorkGroupSizes(cl_device_id device, cl_kernel kernel) {
	size_t preferred_work_group_size;

	cl_int err;
	err = clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_WORK_GROUP_SIZE,
		sizeof(preferred_work_group_size), &preferred_work_group_size, NULL);
	CheckErr(err, "Error getting kernel CL_KERNEL_WORK_GROUP_SIZE");
	printf("Work group size: %zu\n", preferred_work_group_size);
	err = clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
		sizeof(preferred_work_group_size), &preferred_work_group_size, NULL);
	CheckErr(err, "Error getting kernel CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE");
	printf("Preferred work group size: %zu\n", preferred_work_group_size);
}