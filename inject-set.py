import ansys.fluent.core as pyfluent
import pandas as pd
import numpy as np
import os
import shutil

# This looks for that .txt file and connects to your open session
session = pyfluent.connect_to_fluent(server_info_file_name=r"D:\ansys-files2\temp\temp-dir\room\server_info-59024.txt")


# --- CONFIGURATION ---
csv_source = r"D:\weather-analysis\scripts\set-pyfluent\exported_data\calculated_SET_results.csv"
working_dir = r"D:\ansys-files2\temp\temp-dir\room"
udf_filename = "set_importer.c"

print("--- PHASE 13: PARALLEL INDEPENDENT INJECTION ---")

try:
    # 1. Ensure CSV is in working directory
    shutil.copy(csv_source, os.path.join(working_dir, "calculated_SET_results.csv"))
    
    # 2. Compile and Load
    lib_name = "lib_smooth_v20"
    session.settings.setup.user_defined.compiled_udf(
        library_name=lib_name, 
        source_files=[udf_filename], 
        header_files=[]
    )
    session.settings.setup.user_defined.load(udf_library_name=lib_name)

    # 3. Execute
    print("Executing. All 32 nodes are reading the file locally...")
    session.tui.define.user_defined.execute_on_demand(f"inject_set_data::{lib_name}")

    print("\n" + "="*40)
    print("SUCCESS: SET values populated via Independent Read.")
    print("="*40)

except Exception as e:
    print(f"\n[ERROR]: {e}")