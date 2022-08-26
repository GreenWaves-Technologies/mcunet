#include "pmsis.h"
#include "bsp/fs.h"
#include "bsp/fs/hostfs.h"
#include "gaplib/ImgIO.h"
#include "Gap.h"
#include "setup.h"
#include "MCUNetKernels.h"
#include "measurments_utils.h"


#ifdef MODEL_NE16
PI_L2 unsigned char network_output[NETWORK_CLASSES];
#else
PI_L2 signed char network_output[NETWORK_CLASSES];
#endif

int detector_status;

static void nn_inference()
{
    #ifdef PERF
    printf("Start timer\n");
    gap_cl_starttimer();
    gap_cl_resethwtimer();
    #endif

GPIO_HIGH();
    detector_status = MCUNetCNN(network_output);
GPIO_LOW();

    return;
}

void body(void)
{
    OPEN_GPIO_MEAS();
    GPIO_LOW();

    /* Configure And open cluster. */
    struct pi_device cluster_dev;
    struct pi_cluster_conf cl_conf;
    pi_cluster_conf_init(&cl_conf);
    cl_conf.id = 0;
    cl_conf.cc_stack_size = STACK_SIZE;
    pi_open_from_conf(&cluster_dev, (void *) &cl_conf);
    pi_cluster_open(&cluster_dev);
    pi_freq_set(PI_FREQ_DOMAIN_FC, FREQ_FC*1000*1000);
    pi_freq_set(PI_FREQ_DOMAIN_CL, FREQ_CL*1000*1000);
    pi_freq_set(PI_FREQ_DOMAIN_PERIPH, FREQ_PE*1000*1000);
    printf("Set FC Frequency = %d MHz, CL Frequency = %d MHz, PERIIPH Frequency = %d MHz\n",
            pi_freq_get(PI_FREQ_DOMAIN_FC), pi_freq_get(PI_FREQ_DOMAIN_CL), pi_freq_get(PI_FREQ_DOMAIN_PERIPH));
    #ifdef VOLTAGE
    pi_pmu_voltage_set(PI_PMU_VOLTAGE_DOMAIN_CHIP, VOLTAGE);
    pi_pmu_voltage_set(PI_PMU_VOLTAGE_DOMAIN_CHIP, VOLTAGE);
    printf("Voltage: %dmV\n", VOLTAGE);
    #endif

    int status = MCUNetCNN_Construct();
    PRINTF("Detector initialized with status %d\n", status);
    if (status)
    {
        pmsis_exit(-4);
    }
    PRINTF("Network init done\n");

    int input_size = IMAGE_WIDTH*IMAGE_HEIGHT*sizeof(char);
    //char* dir_name = "quant_data";
    char* filename = "../../../dataset/goldfish.ppm";
    if (ReadImageFromFile(filename,
        IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_CHANNELS,
        Input_1, INPUT_SIZE*sizeof(char), IMGIO_OUTPUT_CHAR, 0))
    {
        PRINTF("Image read failed\n");
        pmsis_exit(-2);
    }
    printf("Finished reading image\n");

    struct pi_cluster_task cl_task;
    PRINTF("Stack size is %d and %d\n",STACK_SIZE,SLAVE_STACK_SIZE );
    pi_cluster_task(&cl_task, nn_inference, NULL);
    pi_cluster_task_stacks(&cl_task, NULL, SLAVE_STACK_SIZE);
    #if defined(__GAP8__)
    cl_task.stack_size = STACK_SIZE;
    cl_task.slave_stack_size = SLAVE_STACK_SIZE;
    #endif

    // Execute the function "nn_inference" on the cluster.
    pi_cluster_send_task_to_cl(&cluster_dev, &cl_task);
    
#ifdef PERF
    {
      unsigned int TotalCycles = 0, TotalOper = 0;
      printf("\n");
      for (unsigned int i=0; i<(sizeof(AT_GraphPerf)/sizeof(unsigned int)); i++) {
        TotalCycles += AT_GraphPerf[i]; TotalOper += AT_GraphOperInfosNames[i];
      }
      for (unsigned int i=0; i<(sizeof(AT_GraphPerf)/sizeof(unsigned int)); i++) {
        printf("%45s: Cycles: %10u, Cyc%%: %5.1f%%, Operations: %10u, Op%%: %5.1f%%, Operations/Cycle: %f\n", AT_GraphNodeNames[i], AT_GraphPerf[i], 100*((float) (AT_GraphPerf[i]) / TotalCycles), AT_GraphOperInfosNames[i], 100*((float) (AT_GraphOperInfosNames[i]) / TotalOper), ((float) AT_GraphOperInfosNames[i])/ AT_GraphPerf[i]);
      }
      printf("\n");
      printf("%45s: Cycles: %10u, Cyc%%: 100.0%%, Operations: %10u, Op%%: 100.0%%, Operations/Cycle: %f\n", "Total", TotalCycles, TotalOper, ((float) TotalOper)/ TotalCycles);
      printf("\n");
    }
#endif

    MCUNetCNN_Destruct();

    short int outclass = 0;
    #ifdef MODEL_NE16
    unsigned char max_score = 0;
    #else
    signed char max_score = -128;
    #endif
    for(int i=0; i < NETWORK_CLASSES; i++){
    //PRINTF("%d\n", network_output[i]);
       if (network_output[i] > max_score){
          outclass = i;
          max_score = network_output[i];
       }
    }
    PRINTF("Predicted class:\t%d\n", outclass + 1);
    PRINTF("With confidence:\t%d\n", max_score);


    #ifdef CI
    if(outclass + 1 != 2 && max_score < 65){
        printf("Eval Error...\n");
        pmsis_exit(-1);
    }
    #endif

    #ifdef WRITE_RESULTS
    struct pi_hostfs_conf host_fs_conf;
    struct pi_device host_fs;

    pi_hostfs_conf_init(&host_fs_conf);
    pi_open_from_conf(&host_fs, &host_fs_conf);
     if (pi_fs_mount(&host_fs))
    {
        PRINTF("pi_fs_mount failed\n");
        pmsis_exit(-4);
    }
    char* gap_result = "../../../gap_result.csv";
    PRINTF("Writing output to %s\n", gap_result);
    pi_fs_file_t* host_file = pi_fs_open(&host_fs, gap_result, PI_FS_FLAGS_WRITE);
    if (host_file == NULL)
    {
        PRINTF("Failed to open file, %s\n", gap_result);
        pmsis_exit(-5);
    }
    char buf[2];
    for(int i=0; i < NETWORK_CLASSES; i++){
       sprintf(buf, "%d;", network_output[i]);
       //printf("%s\n", buf);
       pi_fs_write(host_file, buf, strlen(buf));
    }
    pi_fs_close(host_file);

    pi_fs_unmount(&host_fs);
    #endif

    pmsis_exit(0);
}

int main(int argc, char* argv[])
{
    return pmsis_kickoff(body);
}
