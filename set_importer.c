#include "udf.h"
#include <stdio.h>
#include <math.h>

DEFINE_EXECUTE_ON_LOADING(set_naming, libname)
{
    /* Renames UDM 0 to 'set-comfort' globally */
    Set_User_Memory_Name(0, "set-comfort");
}

DEFINE_ON_DEMAND(inject_set_data)
{
    Domain* d = Get_Domain(1);
    Thread* t;
    FILE* fp;
    char buf[1024];
    double csv_x, csv_y, csv_z, val_in;
    cell_t c;
    real cent[3];

    /* 5cm search radius for coverage. Adjust if your mesh is extremely fine. This should be larger than you smallest mesh cell size */
    float eps = 0.005;

    /* Every MPI node opens the file locally */
    fp = fopen("calculated_SET_results.csv", "r");
    if (fp == NULL) {
        if (myid == 0) Message("\n[ERROR] File 'calculated_SET_results.csv' not found!\n");
        return;
    }

    if (myid == 0) Message("\nStarting Universal Fluid Injection (Scanning all Fluid Bodies)...\n");

    /* Skip CSV Header */
    fgets(buf, sizeof(buf), fp);

    /* Read CSV points one by one */
    while (fscanf(fp, "%*d,%lf,%lf,%lf,%*lf,%*lf,%*lf,%*lf,%lf\n", &csv_x, &csv_y, &csv_z, &val_in) == 4)
    {
        /* UNIVERSAL LOOP: Iterate through ALL cell threads (zones) in the domain */
        thread_loop_c(t, d)
        {
            /* Only process the thread if it is a FLUID zone */
            if (FLUID_THREAD_P(t))
            {
                begin_c_loop(c, t)
                {
                    C_CENTROID(cent, c, t);

                    /* Bounding box filter for speed */
                    if (fabs(cent[0] - csv_x) < eps &&
                        fabs(cent[1] - csv_y) < eps &&
                        fabs(cent[2] - csv_z) < eps)
                    {
                        float dx = cent[0] - csv_x;
                        float dy = cent[1] - csv_y;
                        float dz = cent[2] - csv_z;
                        float dist_sq = dx * dx + dy * dy + dz * dz;

                        if (dist_sq < (eps * eps))
                        {
                            /* Average values for smoothness if multiple points hit a cell */
                            if (C_UDMI(c, t, 0) != 0) {
                                C_UDMI(c, t, 0) = (C_UDMI(c, t, 0) + (real)val_in) / 2.0;
                            }
                            else {
                                C_UDMI(c, t, 0) = (real)val_in;
                            }
                        }
                    }
                }
                end_c_loop(c, t)
            }
        }
    }

    fclose(fp);
    if (myid == 0) Message("Universal Injection Complete.\n");
}