#include "pmsis.h"
#include "bsp/fs.h"
#include "bsp/fs/readfs.h"
#include "bsp/fs/hostfs.h"
#include "bsp/flash/hyperflash.h"
#include "bsp/ram/hyperram.h"
#include "gaplib/ImgIO.h"
#include "Gap.h"
#include "gaplib/ImgIO.h"
#include "setup.h"
#include "modelKernels.h"

signed char network_input[INPUT_SIZE];
signed char network_output[NETWORK_CLASSES];

int detector_status;

void cluster_inference(void* args)
{
    (void)args;
    // TODO: Update model output
    detector_status = modelCNN(network_input, network_output);
}

void body(void* parameters)
{
    (void) parameters;

    PRINTF("main call\n");

    PRINTF("Init cluster...\n");
    struct pi_device cluster_dev;
    struct pi_cluster_conf cluster_conf;
    struct pi_cluster_task cluster_task;

    pi_cluster_conf_init(&cluster_conf);
    cluster_conf.id = 0;
    cluster_conf.device_type = 0;
    pi_open_from_conf(&cluster_dev, &cluster_conf);
    PRINTF("before pi_cluster_open\n");
    pi_cluster_open(&cluster_dev);
    PRINTF("Init cluster...done\n");

    PRINTF("Reading input from host...\n");
    struct pi_hostfs_conf host_fs_conf;
    pi_hostfs_conf_init(&host_fs_conf);
    struct pi_device host_fs;

    pi_open_from_conf(&host_fs, &host_fs_conf);

    if (pi_fs_mount(&host_fs))
    {
        PRINTF("pi_fs_mount failed\n");
        pmsis_exit(-1);
    }

    pi_fs_file_t* host_file = NULL;
    int input_size = IMAGE_WIDTH*IMAGE_HEIGHT*sizeof(char);

    if (ReadImageFromFile("../../../dataset/ILSVRC2012_val_00011158_160.ppm",
        IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_CHANNELS,
        network_input, INPUT_SIZE*sizeof(char), IMGIO_OUTPUT_CHAR, 0))
    {
        PRINTF("Image read failed\n");
        pmsis_exit(-2);
    }
    printf("Finished reading image\n");

    int status = modelCNN_Construct();
    PRINTF("Detector initialized with status %d\n", status);
    if (status)
    {
        pmsis_exit(-4);
    }
    PRINTF("Network init done\n");

    PRINTF("DNN inference\n");
#ifdef PERF_COUNT
    int tm = rt_time_get_us();
#endif
    pi_cluster_task(&cluster_task, (void (*)(void *))cluster_inference, NULL);
    cluster_task.slave_stack_size = CL_SLAVE_STACK_SIZE;
    cluster_task.stack_size = CL_STACK_SIZE;
    pi_cluster_send_task_to_cl(&cluster_dev, &cluster_task);
#ifdef PERF_COUNT
    tm = rt_time_get_us() - tm;
    PRINTF("DNN inference finished in %d us\n", tm);
#endif
    PRINTF("DNN inference done with status: %d\n", detector_status);

    modelCNN_Destruct();

    signed char outclass;
    signed char max_score = -127;
	for(int i=0; i < NETWORK_CLASSES; i++){
        PRINTF("%d\n", network_output[i]);

		if (network_output[i] > max_score){
			outclass = i;
			max_score = network_output[i];
		}
	}
	PRINTF("Predicted class:\t%d\n", outclass);
	PRINTF("With confidence:\t%d\n", max_score);

    pi_fs_unmount(&host_fs);
    PRINTF("FS unmount done!\n");

    pi_cluster_close(&cluster_dev);

    pmsis_exit(0);
}

int main()
{
    PRINTF("Entry point\n");
    return pmsis_kickoff(body);
}
