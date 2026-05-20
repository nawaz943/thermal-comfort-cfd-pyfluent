#include "udf.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Structure to hold coordinate data in system RAM (Heap) */
typedef struct {
    float x;
    float y;
    float z;
    float set_val;
} CSVCell;

/* Naming assignment for visualization contours */
DEFINE_EXECUTE_ON_LOADING(set_naming, libname)
{
    Set_User_Memory_Name(0, "set-comfort");
}

/* Binary Search engine to isolate X-coordinate window in O(log N) steps */
long binary_search_x(CSVCell* arr, long size, float target_x, float eps) {
    long low = 0;
    long high = size - 1;

    while (low <= high) {
        long mid = low + (high - low) / 2;

        if (arr[mid].x < target_x - eps) {
            low = mid + 1;
        }
        else if (arr[mid].x > target_x + eps) {
            high = mid - 1;
        }
        else {
            return mid; /* Target is within the epsilon window on the X axis */
        }
    }
    return -1; /* Spatial match does not exist */
}

DEFINE_ON_DEMAND(inject_set_data)
{
    long total_cells = 55326295; /* Exact cell count of your massive mesh */
    float eps = 0.005;           /* Spatial matching tolerance (5mm window) */

    CSVCell* data_array = NULL;
    long idx = 0;
    FILE* fp = NULL;

    /* 1. PARALLEL FILE I/O MANAGEMENT WITH CRITICAL PATH ROBUSTNESS */
#if !PARALLEL
/* Serial Execution Path */
    fp = fopen("D:\\weather-analysis\\scripts\\set-pyfluent\\exported_data\\calculated_SET_results.csv", "r");
    if (fp == NULL) fp = fopen("calculated_SET_results.csv", "r");

    if (fp == NULL) {
        Message("\n[ERROR] 'calculated_SET_results.csv' could not be opened on absolute or relative paths!\n");
        return;
    }
    data_array = (CSVCell*)malloc(total_cells * sizeof(CSVCell));
    if (data_array != NULL) {
        char line[512];
        fgets(line, sizeof(line), fp); /* Skip header */
        long c_num;
        float cx, cy, cz, crh, crad, cvel, ctemp, cset;
        while (fgets(line, sizeof(line), fp) && idx < total_cells) {
            if (sscanf(line, "%ld,%f,%f,%f,%f,%f,%f,%f,%f", &c_num, &cx, &cy, &cz, &crh, &crad, &cvel, &ctemp, &cset) == 9) {
                data_array[idx].x = cx;
                data_array[idx].y = cy;
                data_array[idx].z = cz;
                data_array[idx].set_val = cset;
                idx++;
            }
        }
    }
    fclose(fp);
#else
/* Parallel Execution Path - Attempt Dynamic Node-0 Reading */
    if (myid == 0) {
        /* Try reading from absolute path first */
        fp = fopen("D:\\weather-analysis\\scripts\\set-pyfluent\\exported_data\\calculated_SET_results.csv", "r");
        /* Try reading from localized working folder fallback */
        if (fp == NULL) fp = fopen("calculated_SET_results.csv", "r");

        if (fp != NULL) {
            data_array = (CSVCell*)malloc(total_cells * sizeof(CSVCell));
            if (data_array != NULL) {
                char line[512];
                fgets(line, sizeof(line), fp); /* Skip header */
                long c_num;
                float cx, cy, cz, crh, crad, cvel, ctemp, cset;
                while (fgets(line, sizeof(line), fp) && idx < total_cells) {
                    if (sscanf(line, "%ld,%f,%f,%f,%f,%f,%f,%f,%f", &c_num, &cx, &cy, &cz, &crh, &crad, &cvel, &ctemp, &cset) == 9) {
                        data_array[idx].x = cx;
                        data_array[idx].y = cy;
                        data_array[idx].z = cz;
                        data_array[idx].set_val = cset;
                        idx++;
                    }
                }
            }
            fclose(fp);
        }
    }

    /* Broadcast structural array element tracker sizes to check Node 0 validity */
    host_to_node_long_1(idx);
    host_to_node_long_1(total_cells);

    /* HYBRID CRITICAL RUNTIME FALLBACK: If Node 0 read returned 0 elements, activate Independent Parallel Read */
    if (idx == 0) {
#if !LOGGING_HOST
        /* Every compute engine node attempts to look up and open the local file copy */
        fp = fopen("calculated_SET_results.csv", "r");
        if (fp != NULL) {
            data_array = (CSVCell*)malloc(total_cells * sizeof(CSVCell));
            if (data_array != NULL) {
                char line[512];
                fgets(line, sizeof(line), fp); /* Skip header */
                long c_num;
                float cx, cy, cz, crh, crad, cvel, ctemp, cset;
                while (fgets(line, sizeof(line), fp) && idx < total_cells) {
                    if (sscanf(line, "%ld,%f,%f,%f,%f,%f,%f,%f,%f", &c_num, &cx, &cy, &cz, &crh, &crad, &cvel, &ctemp, &cset) == 9) {
                        data_array[idx].x = cx;
                        data_array[idx].y = cy;
                        data_array[idx].z = cz;
                        data_array[idx].set_val = cset;
                        idx++;
                    }
                }
            }
            fclose(fp);
        }
#endif
    }
    else {
        /* Standard Broadcast Mode: Node 0 populated the values cleanly, sync them */
        if (myid != node_host && myid != 0) {
            data_array = (CSVCell*)malloc(total_cells * sizeof(CSVCell));
        }
        if (myid != node_host && data_array == NULL) {
            Message("Node %d: Critical Heap Allocation Failure!\n", myid);
            return;
        }
        int elements_to_broadcast = (int)((idx * sizeof(CSVCell)) / sizeof(int));
        PRF_GIAND((int*)data_array, elements_to_broadcast, NULL);
    }
#endif

    /* Only local computational nodes handle active spatial assignment mapping */
#if !LOGGING_HOST
    if (idx == 0) {
        if (data_array != NULL) free(data_array);
        Message("Node %d: Terminating - Zero data rows populated.\n", myid);
        return;
    }

    Message("Node %d: Processing high-speed index mapping on %ld records...\n", myid, idx);

    Domain* d = Get_Domain(1);
    Thread* t;
    cell_t c;
    real x_centroid[3];

    /* 2. OPTIMIZED SPATIAL INJECTION LOOP */
    thread_loop_c(t, d) {
        if (FLUID_THREAD_P(t)) {
            begin_c_loop(c, t) {
                C_CENTROID(x_centroid, c, t);
                float xc = (float)x_centroid[0];
                float yc = (float)x_centroid[1];
                float zc = (float)x_centroid[2];

                /* Step A: Perform O(log N) binary index lookup on X-axis */
                long found_idx = binary_search_x(data_array, idx, xc, eps);

                if (found_idx != -1) {
                    /* Step B: Scan backward through the epsilon window */
                    long i = found_idx;
                    short matched = 0;

                    while (i >= 0 && (xc - data_array[i].x) <= eps) {
                        if (fabs(data_array[i].x - xc) <= eps &&
                            fabs(data_array[i].y - yc) <= eps &&
                            fabs(data_array[i].z - zc) <= eps) {

                            C_UDMI(c, t, 0) = (real)data_array[i].set_val;
                            matched = 1;
                            break;
                        }
                        i--;
                    }

                    /* Step C: If no match found, scan forward through the epsilon window */
                    if (!matched) {
                        i = found_idx + 1;
                        while (i < idx && (data_array[i].x - xc) <= eps) {
                            if (fabs(data_array[i].x - xc) <= eps &&
                                fabs(data_array[i].y - yc) <= eps &&
                                fabs(data_array[i].z - zc) <= eps) {

                                C_UDMI(c, t, 0) = (real)data_array[i].set_val;
                                break;
                            }
                            i++;
                        }
                    }
                }
            }
            end_c_loop(c, t)
        }
    }
#endif

    /* 3. MEMORY CLEANUP */
    if (data_array != NULL) {
        free(data_array);
    }

    if (myid == 0) {
        Message("\n==================================================");
        Message("\nSUCCESS: SET Data Injection Complete!");
        Message("\n==================================================\n");
    }
}