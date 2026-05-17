# thermal-comfort-cfd-pyfluent
Calculate DBT, RH, V, MRT using Ansys Fluent. Extract these four values from the whole CFD domain to calculate Standard Effective Temperature using pythermalcomfort library. Inject SET from pythermalcomfort in your full CFD domain in Fluent. View contours of SET in Fluent. THE ACCURATE PIPLINE FOR CALCULATING SET FROM CFD RESULTS. ESPECIALLY FOR VERY LARGE SCALE OUTDOOR THERMAL COMFORT PROJECTS.

# Please read the PDF of problem statement in this repository

# steps
The following are the steps to calculate Standard Effective Temperature using pythermalcomfort library and inject the results back in full CFD domain in Ansys Fluent 2026R1 as a SET comfort contour.

1. Install the python libraries: pythermalcomfort, pyfluent, pandas, numpy, etc. The libraries used can be seen in the first lines of the script. Install all of them through pip commands in Windows PowerShell. Also install IDE (VS Code).

2. Install precompiled google protocol buffer for your platform. See the protobuf documentation for installation. 

3. The bin folder of protobuf library should also point in PATH of System Environment Variables.

4. Run your CFD simulation with the models: Energy, Turbulence, Species, Radiation (DO) till deem to be converged.

5. Stop your simulation. Change the number of iterations to 1. Run, finish 1 iteration to get 100% completion bar. This will fill the solver memory with results.

6. Type in Fluent console: server start-grpc-server. Hit Enter.

7. Hit enter again with default text file details. A text file will be created in your working directory. Example: server_info-59024.txt

7.1 Change all the paths of input output files and working directory of ansys files in all the scripts as per your requirement. Before running any script confirm the paths.

8. This file will allow PyFluent library to communicate with currently open Fluent session.

9. Check the connection by running the script: pyfluent-connection.py This script should show you all the names of your solid and fluid zones in your case. This will verify your connection and communication with Fluent session.

10. Run sync-fluent-pyfluent.py to sync the CFD results with pyfluent so that we can read the results in pyfluent.

11. Run extract-data-pyfluent.py This will create a new folder (exported_data) in the directory of this script. This will contain your extracted domain_data_clean.csv. This file will be used to calculate the value of Standard Effective Temperature using pythermalcomfort library.

12. Run calc-set.py. You will get a new file calculated_SET_results.csv with last column filled with SET values. Each row represents the cell location with x v z coordinates. Total number of rows is equal to the total mesh cell in your CFD domain.

13. Place the set_importer.c UDF file in your working directory of Fluent session.

14. Change the value of float eps (floating point epsilon) in the UDF file as per the smallest size in your mesh. The value of this should be higher than the value of your smallest mesh size. This is a safety clipping used to match coordinates when there is difference in 10^-5 decimal levels.

15. Go to Fluent. Allocate User Defined Memory to 1 manually by GUI or TUI command. This will created UDM 0.

15.1. The set_importer.c carries out two tasks. First, it changes the name of UDM 0 to set-comfort. Second, it reads the coordinates and SET values from the csv file and injects them in the UDM0 (set-comfort) of Fluent session parallely. 

16. Run inject-set.py This will carry out the injection of SET in Fluent Session.

17. Finish.

18. Go to Fluent. View and postprocess the SET contour under User Defined Memory - set-comfort. 
