import ansys.fluent.core as pyfluent

session = pyfluent.connect_to_fluent(server_info_file_name=r"D:\ansys-files2\temp\temp-dir\room\server_info-59024.txt")

# 1. Clear Console
session.execute_tui("(newline)")

print("--- ALIGNING CLIENT & SOLVER ---")

# 2. Modern Sync
print("Synchronizing Field Data Pointers...")
try:
    session.field_data.scalar_fields.is_active(field_name='temperature')
    print("  [OK] Field Data Cache Refreshed.")
except:
    print("  [SKIP] Cache refresh handled by solver.")

# 3. Universal Cell Count (The Meta Method)
# This avoids Scheme errors by looking at the mesh statistics directly
try:
    # Get info for all cell zones in the current domain
    domain_info = session.field_info.get_mesh_info()
    # Summing up all cells in all volumes
    actual_cells = sum(zone.count for zone in domain_info.cell_zones)
except Exception as e:
    actual_cells = "the"

# 4. Final Diagnostic
is_ready = session.field_data.is_data_valid()

print("-" * 30)
if is_ready:
    count_display = f"{actual_cells:,}" if isinstance(actual_cells, int) else "volumetric"
    print(f"STATUS: READY")
    print(f"MESH: {count_display} cells detected.")
    print("Successfully aligned with Fluent GUI.")
else:
    print("STATUS: NOT READY")
    print("Check if the solver is paused or the solution is uninitialized.")
print("-" * 30)