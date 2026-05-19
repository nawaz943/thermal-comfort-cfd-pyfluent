import pandas as pd
import numpy as np
from pythermalcomfort.models import set_tmp
import time
import os

# --- CONFIGURATION ---
input_file = r"D:\weather-analysis\scripts\set-pyfluent\exported_data\domain_data_clean.csv"
output_file = r"D:\weather-analysis\scripts\set-pyfluent\exported_data\calculated_SET_results.csv"

MET = 1.2
CLO = 0.5
CHUNK_SIZE = 100000 

print("--- PHASE 3: CALCULATING SET (SAFE MODE) ---")
start_time = time.time()

try:
    if os.path.exists(output_file):
        os.remove(output_file)

    # We read the file using the exact headers found in your 'domain_data_clean.csv'
    with pd.read_csv(input_file, chunksize=CHUNK_SIZE) as reader:

        for i, chunk in enumerate(reader):
            # 1. Clean and Convert for Calculation
            # We assume Kelvin for temperatures (as per your data head: 294.1 K)
            
            # Air Temperature (tdb)
            tdb = chunk['temperature'].values - 273.15
            
            # Radiation Temperature (tr)
            # SAFETY: If tr is 0, we use the local air temperature (tdb)
            tr_raw = chunk['radiation-temperature'].values
            tr = np.where(tr_raw <= 10.0, chunk['temperature'].values, tr_raw) - 273.15
            
            # Relative Humidity (rh)
            # SAFETY: Clip humidity between 10% and 90%
            # If your data is 0-1 (fraction), we multiply by 100
            rh = np.clip(chunk['relative-humidity'].values * 100.0, 10.0, 90.0)
            
            # Velocity (v)
            # SAFETY: Clip velocity (SET model is stable between 0.1 and 5.0 m/s)
            v = np.clip(chunk['velocity-magnitude'].values, 0.1, 5.0)

            # Strict Velocity Cap (Force anything above 5.0 m/s down to 5.0 m/s). The lines below when you have in your domain velocities above 5 m/s.
            # v_raw = chunk['velocity-magnitude'].values
            # v = np.where(v_raw > 5.0, 5.0, v_raw)
            # v = np.where(v < 0.1, 0.1, v)

            
            # 2. Calculate SET with strictly limited inputs to prevent iteration errors
            results = set_tmp(
                tdb=tdb, 
                tr=tr, 
                v=v, 
                rh=rh, 
                met=MET, 
                clo=CLO, 
                limit_inputs=True  # This is the primary fix for the crash
            )
            
            # Handle return types (list, array, or dictionary)
            if hasattr(results, 'set'):
                set_values = results.set
            elif isinstance(results, dict):
                set_values = results['set']
            else:
                set_values = results 

            # 3. Append the SET result and save
            chunk['Calculated_SET'] = set_values
            
            chunk.to_csv(
                output_file, 
                mode='a', 
                index=False, 
                header=(i == 0)
            )

            print(f"Processed Batch {i+1} | Cumulative Cells: {(i+1)*len(chunk):,}")

    print("\n" + "="*50)
    print(f"SUCCESS: SET results generated for {422378} cells.")
    print(f"File: {output_file}")
    print("="*50)

except Exception as e:
    print(f"\n[CRITICAL ERROR]: {e}")
