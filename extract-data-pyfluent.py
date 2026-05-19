import ansys.fluent.core as pyfluent
import pandas as pd
import os
import sys

# 1. Connect
try:
    server_info = r"D:\ansys-files2\temp\temp-dir\room\server_info-59024.txt"
    session = pyfluent.connect_to_fluent(server_info_file_name=server_info)
    print("--- Successfully Connected to Fluent ---")
except Exception as e:
    print(f"CRITICAL: Connection failed: {e}")
    sys.exit()

output_dir = r"D:\weather-analysis\scripts\set-pyfluent\exported_data"
if not os.path.exists(output_dir):
    os.makedirs(output_dir)

print("--- PHASE 1: UNIVERSAL PRECISION EXTRACTION ---")

try:
    # for one fluid zone. Use the line below to target only one fluid zone. Example cylinder or wind tunnel. 
    # fluid_zones = ["cylinder"]
    # Dynamically find all Fluid zones
    fluid_zones = list(session.settings.setup.cell_zone_conditions.fluid.keys())
    print(f"Targeting Fluid Bodies: {fluid_zones}")
    
    zone_files = []

    for zone in fluid_zones:
        temp_file = os.path.join(output_dir, f"temp_{zone}.csv")
        print(f"Exporting Zone '{zone}'...")
        
        # FIX: Added escaped quotes \" around the zone name.
        # This tells Fluent's Scheme interpreter it's a STRING, not a variable or function.
        # Sequence: (Zone List) (Surface List) no-surfaces variables... exit
        tui_cmd = (
            f'/file/export/ascii "{temp_file}" (\"{zone}\") () no '
            f'temperature velocity-magnitude radiation-temperature relative-humidity q'
        )
        session.execute_tui(tui_cmd)
        
        if os.path.exists(temp_file):
            zone_files.append(temp_file)

    # 2. Merge and Clean the Data
    if zone_files:
        print("Merging and cleaning data columns...")
        df_list = []
        
        for f in zone_files:
            # Fluent ASCII files are space-delimited
            # We use sep=r'\s+' to handle multiple spaces correctly
            temp_df = pd.read_csv(f, sep=r'\s+')
            
            # REMOVE the 1st column (Fluent's local cell-number) to avoid duplicates
            # This leaves us with only the physical data and coordinates
            temp_df = temp_df.iloc[:, 1:] 
            
            df_list.append(temp_df)
        
        # Combine all zones into one dataframe
        final_df = pd.concat(df_list, ignore_index=True)
        
        # INSERT the Global Index (0 to 422377) required for your UDF Mapping
        final_df.insert(0, 'cellnumber', final_df.index)
        
        # Save the final clean CSV
        final_csv = os.path.join(output_dir, "domain_data_clean.csv")
        final_df.to_csv(final_csv, index=False)
        
        print("-" * 40)
        print(f"SUCCESS: {len(final_df)} cells exported.")
        print(f"Clean Data saved to: {final_csv}")
        print("-" * 40)
        
        # Cleanup temporary files
        for f in zone_files: os.remove(f)
    else:
        print("ERROR: No data was exported.")

except Exception as e:
    print(f"CRITICAL ERROR: {e}")

finally:
    print("Script finished.")
